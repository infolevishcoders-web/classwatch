/*
 * ClassWatchConfigurationPage.cpp - implementation of ClassWatchConfigurationPage
 *
 * Copyright (c) 2026 Levish Coders
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

#include "ClassWatchConfigurationPage.h"
#include "ui_ClassWatchConfigurationPage.h"
#include "VeyonConfiguration.h"
#include "VeyonCore.h"
#include <QMessageBox>

ClassWatchConfigurationPage::ClassWatchConfigurationPage( QWidget* parent ) :
	ConfigurationPage(),
	ui( new Ui::ClassWatchConfigurationPage )
{
	ui->setupUi( this );
	
	// Set window title and icon
	setWindowTitle( tr( "ClassWatch" ) );
	setWindowIcon( QIcon( ":/configurator/classwatch-configurator.png" ) );
}

ClassWatchConfigurationPage::~ClassWatchConfigurationPage()
{
	delete ui;
}

void ClassWatchConfigurationPage::resetWidgets()
{
	// Reset widgets to current configuration values
	VeyonConfiguration& config = VeyonCore::config();
	
	// School Information
	ui->schoolNameEdit->setText( config.schoolName() );
	ui->laboratoryNameEdit->setText( config.laboratoryName() );
	
	// Computer Identification
	ui->idPrefixEdit->setText( config.classWatchIdPrefix() );
	ui->startNumberSpinBox->setValue( config.classWatchIdStartNumber() );
	ui->registryPathEdit->setText( config.classWatchRegistryPath() );
	
	// Features
	ui->autoRegisterCheckBox->setChecked( config.autoRegisterComputers() );
	ui->showIdsInMonitoringCheckBox->setChecked( config.showClassWatchIdsInMonitoring() );
	ui->studentNotificationsCheckBox->setChecked( config.enableStudentNotifications() );
	ui->fileDistributionCheckBox->setChecked( config.enableFileDistribution() );
	ui->audioClassroomCheckBox->setChecked( config.enableAudioClassroom() );
	ui->webFilteringCheckBox->setChecked( config.enableWebFiltering() );
	ui->timetableCheckBox->setChecked( config.enableTimetable() );
	ui->multiSessionCheckBox->setChecked( config.enableMultiSessionSupport() );
	
	// Student Folder
	ui->studentFolderEdit->setText( config.defaultStudentFolder() );
	
	// Audit & Logging
	ui->enableAuditCheckBox->setChecked( config.enableAuditLogging() );
	ui->auditLogPathEdit->setText( config.auditLogPath() );
}

void ClassWatchConfigurationPage::connectWidgetsToProperties()
{
	// Connect widget changes to emit widgetsChanged signal
	connect( ui->schoolNameEdit, &QLineEdit::textChanged, this, &ConfigurationPage::widgetsChanged );
	connect( ui->laboratoryNameEdit, &QLineEdit::textChanged, this, &ConfigurationPage::widgetsChanged );
	connect( ui->idPrefixEdit, &QLineEdit::textChanged, this, &ConfigurationPage::widgetsChanged );
	connect( ui->startNumberSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConfigurationPage::widgetsChanged );
	connect( ui->registryPathEdit, &QLineEdit::textChanged, this, &ConfigurationPage::widgetsChanged );
	connect( ui->autoRegisterCheckBox, &QCheckBox::toggled, this, &ConfigurationPage::widgetsChanged );
	connect( ui->showIdsInMonitoringCheckBox, &QCheckBox::toggled, this, &ConfigurationPage::widgetsChanged );
	connect( ui->studentNotificationsCheckBox, &QCheckBox::toggled, this, &ConfigurationPage::widgetsChanged );
	connect( ui->fileDistributionCheckBox, &QCheckBox::toggled, this, &ConfigurationPage::widgetsChanged );
	connect( ui->audioClassroomCheckBox, &QCheckBox::toggled, this, &ConfigurationPage::widgetsChanged );
	connect( ui->webFilteringCheckBox, &QCheckBox::toggled, this, &ConfigurationPage::widgetsChanged );
	connect( ui->timetableCheckBox, &QCheckBox::toggled, this, &ConfigurationPage::widgetsChanged );
	connect( ui->multiSessionCheckBox, &QCheckBox::toggled, this, &ConfigurationPage::widgetsChanged );
	connect( ui->studentFolderEdit, &QLineEdit::textChanged, this, &ConfigurationPage::widgetsChanged );
	connect( ui->enableAuditCheckBox, &QCheckBox::toggled, this, &ConfigurationPage::widgetsChanged );
	connect( ui->auditLogPathEdit, &QLineEdit::textChanged, this, &ConfigurationPage::widgetsChanged );
	
	// Connect apply/reset buttons
	connect( ui->applyButton, &QPushButton::clicked, this, &ClassWatchConfigurationPage::onApplyClicked );
	connect( ui->resetButton, &QPushButton::clicked, this, &ClassWatchConfigurationPage::onResetClicked );
}

void ClassWatchConfigurationPage::applyConfiguration()
{
	// This method is called when the configuration is applied
	// We don't need to do anything here as the widgets are already connected
	// to the configuration properties through the ConfigurationPage mechanism
}

void ClassWatchConfigurationPage::onApplyClicked()
{
	// Emit widgetsChanged to trigger configuration save
	Q_EMIT widgetsChanged();
}

void ClassWatchConfigurationPage::onResetClicked()
{
	int result = QMessageBox::question( this, tr( "Confirm Reset" ),
									   tr( "Are you sure you want to reset all ClassWatch settings to default values?" ),
									   QMessageBox::Yes | QMessageBox::No );
	
	if ( result == QMessageBox::Yes ) {
		resetWidgets();
		Q_EMIT widgetsChanged();
	}
}
