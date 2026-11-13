/*
 * MainWindow.cpp - implementation of MainWindow class
 *
 * Copyright (c) 2010-2026 Tobias Junghans <tobydox@veyon.io>
 *
 * This file is part of ClassWatch - https://levishcoders.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <QAction>
#include <QActionGroup>
#include <QDir>
#include <QLocale>
#include <QMenu>
#include <QProcess>
#include <QCloseEvent>
#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QTimer>
#include <QToolButton>
#include <QTranslator>
#include <QStackedWidget>

#include "Configuration/JsonStore.h"
#include "Configuration/UiMapping.h"

#include "AboutDialog.h"
#include "ClassWatchTheme.h"
#include "ConfigurationPagePluginInterface.h"
#include "ConfigurationManager.h"
#include "VeyonConfiguration.h"
#include "MainWindow.h"
#include "PluginManager.h"
#include "VeyonCore.h"

#include "ui_MainWindow.h"


MainWindow::MainWindow( QWidget* parent ) :
	QMainWindow( parent ),
	ui( new Ui::MainWindow ),
	m_configChanged( false )
{
	ui->setupUi( this );

	// ClassWatch "watery" theme: translucent panels over a softly
	// animated water-gradient backdrop.
	ui->centralwidget->setStyleSheet(
		QStringLiteral( "QWidget { background: transparent; }" ) );
	ClassWatchTheme::startWateryAnimation( this );

	setWindowTitle(tr("ClassWatch Setup %1").arg(VeyonCore::versionString()));

	// compact top tabs for the built-in configuration pages
	static const QHash<QString, QString> tabTitles{
		{ QStringLiteral("GeneralConfigurationPage"), QStringLiteral("General") },
		{ QStringLiteral("ServiceConfigurationPage"), QStringLiteral("Student Service") },
		{ QStringLiteral("MasterConfigurationPage"), QStringLiteral("Teacher App") },
		{ QStringLiteral("AccessControlPage"), QStringLiteral("Access & Security") },
		{ QStringLiteral("ClassWatchConfigurationPage"), QStringLiteral("ClassWatch ID") },
	};
	for( int i = 0; i < ui->configPages->count(); ++i )
	{
		auto page = ui->configPages->widget( i );
		addPageTab( i );
		m_pageButtons.last()->setText(
			tabTitles.value( QString::fromLatin1( page->metaObject()->className() ),
							 page->windowTitle() ) );
	}
	if( !m_pageButtons.isEmpty() )
	{
		m_pageButtons.first()->setChecked( true );
	}

	loadConfigurationPagePlugins();

	// reset all widget's values to current configuration
	reset( true );

	// if local configuration is incomplete, re-enable the apply button
	if( VeyonConfiguration().data().size() < VeyonCore::config().data().size() )
	{
		configurationChanged();
	}

	const auto pages = findChildren<ConfigurationPage *>();
	for( auto page : pages )
	{
		page->connectWidgetsToProperties();
		connect( page, &ConfigurationPage::widgetsChanged, this, &MainWindow::updateView );
	}

	connect( ui->buttonBox, &QDialogButtonBox::clicked, this, &MainWindow::resetOrApply );

	connect( ui->actionLoadSettings, &QAction::triggered, this, &MainWindow::loadSettingsFromFile );
	connect( ui->actionSaveSettings, &QAction::triggered, this, &MainWindow::saveSettingsToFile );

	auto viewModeGroup = new QActionGroup( this );
	viewModeGroup->addAction( ui->viewModeStandard );
	viewModeGroup->addAction( ui->viewModeAdvanced );
	viewModeGroup->setExclusive( true );
	ui->viewModeStandard->setChecked( true );

	connect( viewModeGroup, &QActionGroup::triggered, this, &MainWindow::updateView );

	connect( ui->actionAboutQt, &QAction::triggered, QApplication::instance(), &QApplication::aboutQt );

	connect( &VeyonCore::config(), &VeyonConfiguration::configurationChanged, this, &MainWindow::configurationChanged );

	connect( ui->configPages, &QStackedWidget::currentChanged, this, &MainWindow::updateSizes );

	connect( ui->configPages, &QStackedWidget::currentChanged, this, [this]( int index ) {
		if( index >= 0 && index < m_pageButtons.size() )
		{
			m_pageButtons[index]->setChecked( true );
		}
	} );

	setupLanguageMenu();

	resize( 980, ui->generalConfigurationPage->minimumSizeHint().height() + 160 );

	updateView();
}



MainWindow::~MainWindow()
{
	delete ui;
}



void MainWindow::reset( bool onlyUI )
{
	if( onlyUI == false )
	{
		VeyonCore::config().clear();
		VeyonCore::config().reloadFromStore();
	}

	const auto pages = findChildren<ConfigurationPage *>();
	for( auto page : pages )
	{
		page->resetWidgets();
	}

	ui->buttonBox->setEnabled( false );
	m_configChanged = false;
}




void MainWindow::apply()
{
	const auto showError = [this](const ConfigurationManager& configurationManager) {
		vCritical() << configurationManager.errorString().toUtf8().constData();

		QMessageBox::critical(this, tr("ClassWatch Setup"), configurationManager.errorString());
	};

	ConfigurationManager configurationManager;
	if (configurationManager.saveConfiguration())
	{
		if (configurationManager.applyConfiguration() == false)
		{
			showError(configurationManager);
		}

		const auto pages = findChildren<ConfigurationPage *>();
		for( auto page : pages )
		{
			page->applyConfiguration();
		}

		ui->buttonBox->setEnabled( false );
		m_configChanged = false;
	}
	else
	{
		showError(configurationManager);
	}
}



void MainWindow::resizeEvent( QResizeEvent* event )
{
	QMainWindow::resizeEvent( event );

	updateSizes();
}




void MainWindow::configurationChanged()
{
	ui->buttonBox->setEnabled( true );
	m_configChanged = true;
}




void MainWindow::resetOrApply( QAbstractButton *btn )
{
	if( ui->buttonBox->standardButton( btn ) & QDialogButtonBox::Apply )
	{
		apply();
	}
	else if( ui->buttonBox->standardButton( btn ) & QDialogButtonBox::Reset )
	{
		reset();
	}
}



void MainWindow::loadSettingsFromFile()
{
	QString fileName = QFileDialog::getOpenFileName( this, tr( "Load settings from file" ),
													 QDir::homePath(), tr( "JSON files (*.json)" ) );
	if( !fileName.isEmpty() )
	{
		// write current configuration to output file
		Configuration::JsonStore(Configuration::JsonStore::Scope::System, fileName).load(&VeyonCore::config());
		reset( true );
		configurationChanged();	// give user a chance to apply possible changes
	}
}




void MainWindow::saveSettingsToFile()
{
	QString fileName = QFileDialog::getSaveFileName( this, tr( "Save settings to file" ),
													 QDir::homePath(), tr( "JSON files (*.json)" ) );
	if( !fileName.isEmpty() )
	{
		if( !fileName.endsWith( QStringLiteral(".json"), Qt::CaseInsensitive ) )
		{
			fileName += QStringLiteral(".json");
		}

		bool configChangedPrevious = m_configChanged;

		// write current configuration to output file
		Configuration::JsonStore(Configuration::JsonStore::Scope::System, fileName).flush(&VeyonCore::config());

		m_configChanged = configChangedPrevious;
		ui->buttonBox->setEnabled( m_configChanged );
	}
}



void MainWindow::resetConfiguration()
{
	if( QMessageBox::warning( this, tr( "Reset configuration" ),
							  tr( "Do you really want to reset the local configuration and revert "
								  "all settings to their defaults?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) ==
		QMessageBox::Yes )
	{
		ConfigurationManager().clearConfiguration();
		reset( false );
	}
}



void MainWindow::aboutClassWatch()
{
	AboutDialog( this ).exec();
}



void MainWindow::updateSizes()
{
	ui->configPages->setMinimumSize( ui->scrollArea->width() - ui->scrollArea->verticalScrollBar()->width(),
									 ui->configPages->currentWidget()->minimumSizeHint().height() );
}



void MainWindow::updateView()
{
	if( ui->viewModeAdvanced->isChecked() )
	{
		switchToAdvancedView();
	}
	else
	{
		switchToStandardView();
	}

	QTimer::singleShot( 0, this, &MainWindow::updateSizes );
}



void MainWindow::switchToStandardView()
{
	const auto widgets = findChildren<QWidget *>();

	// hide widgets with advanced property flag
	for( auto widget : widgets )
	{
		const auto flags = Configuration::UiMapping::flags( widget );
		if( flags & Configuration::Property::Flag::Hidden )
		{
			widget->hide();
		}
		else if( flags & Configuration::Property::Flag::Advanced )
		{
			widget->hide();
		}
	}

	// hide top tabs for advanced pages
	for( int i = 0; i < ui->configPages->count() && i < m_pageButtons.size(); ++i )
	{
		auto page = qobject_cast<ConfigurationPage *>( ui->configPages->widget( i ) );
		if( page == nullptr )
		{
			continue;
		}

		const auto flags = Configuration::UiMapping::flags( page );
		const bool advanced = flags & Configuration::Property::Flag::Advanced;

		m_pageButtons[i]->setVisible( !advanced );
		if( advanced && ui->configPages->currentIndex() == i )
		{
			for( int j = 0; j < m_pageButtons.size(); ++j )
			{
				if( m_pageButtons[j]->isVisible() )
				{
					m_pageButtons[j]->setChecked( true );
					break;
				}
			}
		}
	}
}



void MainWindow::switchToAdvancedView()
{
	const auto widgets = findChildren<QWidget *>();
	for( auto widget : widgets )
	{
		const auto flags = Configuration::UiMapping::flags( widget );
		if( flags & Configuration::Property::Flag::Advanced )
		{
			widget->show();
		}
	}

	for( int i = 0; i < ui->configPages->count() && i < m_pageButtons.size(); ++i )
	{
		m_pageButtons[i]->setVisible( true );
	}
}



void MainWindow::loadConfigurationPagePlugins()
{
	for( auto pluginObject : std::as_const( VeyonCore::pluginManager().pluginObjects() ) )
	{
		auto pluginInterface = qobject_cast<PluginInterface *>( pluginObject );
		auto configurationPagePluginInterface = qobject_cast<ConfigurationPagePluginInterface *>( pluginObject );

		if( pluginInterface && configurationPagePluginInterface )
		{
			auto page = configurationPagePluginInterface->createConfigurationPage();
			if( page )
			{
				ui->configPages->addWidget( page );
				addPageTab( ui->configPages->count() - 1 );
			}
		}
	}
}



void MainWindow::addPageTab( int pageIndex )
{
	auto page = ui->configPages->widget( pageIndex );
	if( page == nullptr )
	{
		return;
	}

	auto tab = new QToolButton( this );
	tab->setText( page->windowTitle() );
	if( page->windowIcon().isNull() == false )
	{
		tab->setIcon( page->windowIcon() );
	}
	tab->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
	tab->setCheckable( true );
	tab->setAutoExclusive( true );

	connect( tab, &QToolButton::toggled, this, [this, pageIndex]( bool checked ) {
		if( checked )
		{
			ui->configPages->setCurrentIndex( pageIndex );
		}
	} );

	ui->pageTabsLayout->insertWidget( ui->pageTabsLayout->count() - 1, tab );
	m_pageButtons.append( tab );
}



void MainWindow::setupLanguageMenu()
{
	const auto qmFiles = QDir( VeyonCore::translationsDirectory() )
							.entryList( { QStringLiteral("veyon*.qm") }, QDir::Files, QDir::Name );
	if( qmFiles.isEmpty() )
	{
		ui->languageButton->setVisible( false );
		return;
	}

	auto* menu = new QMenu( tr( "Language" ), ui->languageButton );
	ui->languageButton->setMenu( menu );

	auto* group = new QActionGroup( menu );
	group->setExclusive( true );

	const auto currentLanguage = VeyonCore::config().uiLanguage();

	for( const auto& qmFile : qmFiles )
	{
		// "veyon_de_DE.qm" -> "de_DE"
		const auto localeName = qmFile.mid( 6 ).section( QLatin1Char('.'), 0, 0 );
		QLocale loc( localeName );
		const auto known = loc.language() != QLocale::C;

		const auto text = QStringLiteral( "%1 - %2 (%3)" )
							  .arg( known ? QLocale::languageToString( loc.language() ) : localeName,
									known ? loc.nativeLanguageName() : localeName,
									localeName );
		auto* action = menu->addAction( text );
		action->setCheckable( true );
		group->addAction( action );

		if( currentLanguage.contains( localeName ) )
		{
			action->setChecked( true );
		}

		connect( action, &QAction::triggered, this,
				 [this, text, localeName]() { applyLanguage( text, localeName ); } );
	}

	auto* systemAction = menu->addAction( tr( "System language" ) );
	systemAction->setCheckable( true );
	group->addAction( systemAction );
	if( currentLanguage.isEmpty() )
	{
		systemAction->setChecked( true );
	}
	connect( systemAction, &QAction::triggered, this, [this]() {
		applyLanguage( tr( "System" ), QLocale::system().name() );
	} );
}



void MainWindow::applyLanguage( const QString& languageName, const QString& localeName )
{
	VeyonCore::config().setUiLanguage(
		QStringLiteral( "%1 (%2)" ).arg( languageName, localeName ) );

	const QLocale locale( localeName );
	QLocale::setDefault( locale );

	for( const auto& resource : { QStringLiteral("qtbase"), QStringLiteral("veyon") } )
	{
		const auto dir = resource.startsWith( QLatin1String("qt") )
							 ? VeyonCore::qtTranslationsDirectory()
							 : VeyonCore::translationsDirectory();
		auto* translator = new QTranslator( qApp );
		if( translator->load( QStringLiteral( "%1_%2" ).arg( resource, localeName ), dir ) )
		{
			QCoreApplication::installTranslator( translator );
		}
		else
		{
			delete translator;
		}
	}
}



void MainWindow::closeEvent( QCloseEvent *closeEvent )
{
	if( m_configChanged &&
		QMessageBox::question( this, tr( "Unsaved settings" ),
							   tr( "There are unsaved settings. "
								   "Quit anyway?" ),
							   QMessageBox::Yes | QMessageBox::No ) !=
		QMessageBox::Yes )
	{
		closeEvent->ignore();
		return;
	}

	closeEvent->accept();
	QMainWindow::closeEvent( closeEvent );
}
