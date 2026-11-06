/*
 * ClassWatchComputerPanel.cpp - implementation of ClassWatchComputerPanel
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

#include "ClassWatchComputerPanel.h"
#include "ui_ClassWatchComputerPanel.h"
#include "VeyonMaster.h"
#include "ClassWatchComputerManagerExtension.h"
#include "ClassWatchComputer.h"
#include "ComputerManager.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>

ClassWatchComputerPanel::ClassWatchComputerPanel( VeyonMaster* master, QWidget* parent ) :
	QWidget( parent ),
	ui( new Ui::ClassWatchComputerPanel ),
	m_master( master ),
	m_classWatchManager( nullptr )
{
	ui->setupUi( this );
	
	if ( m_master ) {
		m_classWatchManager = &m_master->classWatchComputerManager();
		setupConnections();
	}
	
	// Initialize UI
	ui->computerTreeWidget->setColumnWidth( 0, 100 ); // ClassWatch ID
	ui->computerTreeWidget->setColumnWidth( 1, 150 ); // Computer Name
	ui->computerTreeWidget->setColumnWidth( 2, 100 ); // Status
	ui->computerTreeWidget->setColumnWidth( 3, 120 ); // IP Address
	ui->computerTreeWidget->setColumnWidth( 4, 120 ); // User
	
	// Load existing registry if available
	QString registryPath = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + QStringLiteral("/classwatch_computers.json");
	m_classWatchManager->loadClassWatchRegistry( registryPath );
	
	refreshComputerList();
}

ClassWatchComputerPanel::~ClassWatchComputerPanel()
{
	delete ui;
}

void ClassWatchComputerPanel::setupConnections()
{
	if ( !m_classWatchManager ) {
		return;
	}

	// Connect to ClassWatch manager signals
	connect( m_classWatchManager, &ClassWatchComputerManagerExtension::computerRegistered,
			 this, [this]( const QString&, const ClassWatchComputer& ) {
			 refreshComputerList();
		 } );

	connect( m_classWatchManager, &ClassWatchComputerManagerExtension::computerUnregistered,
			 this, [this]( const QString& ) {
			 refreshComputerList();
		 } );

	connect( m_classWatchManager, &ClassWatchComputerManagerExtension::computerInfoUpdated,
			 this, [this]( const QString& classWatchId ) {
			 updateComputerDetails( classWatchId );
			 populateComputerTree();
		 } );

	connect( m_classWatchManager, &ClassWatchComputerManagerExtension::connectionStatusChanged,
			 this, [this]( const QString& classWatchId, const QString& ) {
			 updateComputerDetails( classWatchId );
			 populateComputerTree();
		 } );

	// Connect UI signals
	connect( ui->refreshButton, &QPushButton::clicked, this, &ClassWatchComputerPanel::refreshComputerList );
	connect( ui->registerButton, &QPushButton::clicked, this, &ClassWatchComputerPanel::onRegisterComputer );
	connect( ui->unregisterButton, &QPushButton::clicked, this, &ClassWatchComputerPanel::onUnregisterComputer );
	connect( ui->saveRegistryButton, &QPushButton::clicked, this, &ClassWatchComputerPanel::onSaveRegistry );
	connect( ui->loadRegistryButton, &QPushButton::clicked, this, &ClassWatchComputerPanel::onLoadRegistry );
	connect( ui->closeButton, &QPushButton::clicked, this, &ClassWatchComputerPanel::onClose );
	connect( ui->computerTreeWidget, &QTreeWidget::itemSelectionChanged, this, &ClassWatchComputerPanel::onComputerSelectionChanged );
}

void ClassWatchComputerPanel::refreshComputerList()
{
	if ( !m_classWatchManager ) {
		return;
	}

	// Sync computers from ComputerManager first
	m_classWatchManager->syncComputersFromComputerManager();
	
	populateComputerTree();
}

void ClassWatchComputerPanel::onComputerSelectionChanged()
{
	QList<QTreeWidgetItem*> selectedItems = ui->computerTreeWidget->selectedItems();
	if ( selectedItems.isEmpty() ) {
		// Clear details
		ui->classWatchIdValue->setText( QStringLiteral("-") );
		ui->computerNameValue->setText( QStringLiteral("-") );
		ui->ipAddressValue->setText( QStringLiteral("-") );
		ui->macAddressValue->setText( QStringLiteral("-") );
		ui->windowsVersionValue->setText( QStringLiteral("-") );
		ui->loggedInUserValue->setText( QStringLiteral("-") );
		ui->connectionStatusValue->setText( QStringLiteral("-") );
		ui->lastConnectionValue->setText( QStringLiteral("-") );
		ui->sessionTypeValue->setText( QStringLiteral("-") );
		return;
	}

	QTreeWidgetItem* item = selectedItems.first();
	QString classWatchId = item->text( 0 );
	updateComputerDetails( classWatchId );
}

void ClassWatchComputerPanel::updateComputerDetails( const QString& classWatchId )
{
	if ( !m_classWatchManager || classWatchId.isEmpty() ) {
		return;
	}

	ClassWatchComputer computer = m_classWatchManager->getClassWatchComputer( classWatchId );
	if ( !computer.isValid() ) {
		return;
	}

	ui->classWatchIdValue->setText( computer.classWatchId() );
	ui->computerNameValue->setText( computer.displayName() );
	ui->ipAddressValue->setText( computer.hostAddress().toString() );
	ui->macAddressValue->setText( computer.macAddress() );
	ui->windowsVersionValue->setText( computer.windowsVersion().isEmpty() ? QStringLiteral("Unknown") : computer.windowsVersion() );
	ui->loggedInUserValue->setText( computer.loggedInUser().isEmpty() ? QStringLiteral("Not logged in") : computer.loggedInUser() );
	ui->connectionStatusValue->setText( computer.connectionStatus() );
	ui->lastConnectionValue->setText( computer.lastConnectionTime().toString( QStringLiteral("yyyy-MM-dd hh:mm:ss") ) );
	ui->sessionTypeValue->setText( computer.sessionType().isEmpty() ? QStringLiteral("Unknown") : computer.sessionType() );
}

void ClassWatchComputerPanel::populateComputerTree()
{
	if ( !m_classWatchManager ) {
		return;
	}

	ui->computerTreeWidget->clear();

	ClassWatchComputerList computers = m_classWatchManager->getAllClassWatchComputers();
	
	for ( const ClassWatchComputer& computer : computers ) {
		QTreeWidgetItem* item = new QTreeWidgetItem( ui->computerTreeWidget );
		item->setText( 0, computer.classWatchId() );
		item->setText( 1, computer.displayName() );
		item->setText( 2, computer.connectionStatus() );
		item->setText( 3, computer.hostAddress().toString() );
		item->setText( 4, computer.loggedInUser() );
		
		// Set status color
		if ( computer.connectionStatus() == QStringLiteral("Connected") ) {
			item->setForeground( 2, QColor( Qt::green ) );
		} else {
			item->setForeground( 2, QColor( Qt::red ) );
		}
		
		// Store classWatchId as user data
		item->setData( 0, Qt::UserRole, computer.classWatchId() );
	}
	
	ui->computerTreeWidget->resizeColumnToContents( 0 );
	ui->computerTreeWidget->resizeColumnToContents( 1 );
	ui->computerTreeWidget->resizeColumnToContents( 2 );
	ui->computerTreeWidget->resizeColumnToContents( 3 );
	ui->computerTreeWidget->resizeColumnToContents( 4 );
}

void ClassWatchComputerPanel::onRegisterComputer()
{
	if ( !m_master || !m_classWatchManager ) {
		return;
	}

	// Get selected computers from ComputerManager
	ComputerManager& computerManager = m_master->computerManager();
	QModelIndex rootIndex;
	ComputerList computers = computerManager.selectedComputers( rootIndex );
	
	if ( computers.isEmpty() ) {
		QMessageBox::warning( this, QStringLiteral("No Computers Selected"), QStringLiteral("Please select computers in the 'Locations & Computers' panel first.") );
		return;
	}

	int registeredCount = 0;
	for ( const Computer& computer : computers ) {
		QString classWatchId = m_classWatchManager->getClassWatchIdForComputer( computer );
		if ( classWatchId.isEmpty() ) {
			m_classWatchManager->registerComputer( computer );
			registeredCount++;
		}
	}

	if ( registeredCount > 0 ) {
		QMessageBox::warning( this, QStringLiteral("Computers Registered"), 
								 QString( QStringLiteral("Successfully registered %1 computer(s).") ).arg( registeredCount ) );
		refreshComputerList();
	} else {
		QMessageBox::warning( this, QStringLiteral("No New Computers"), QStringLiteral("All selected computers are already registered.") );
	}
}

void ClassWatchComputerPanel::onUnregisterComputer()
{
	QList<QTreeWidgetItem*> selectedItems = ui->computerTreeWidget->selectedItems();
	if ( selectedItems.isEmpty() ) {
		QMessageBox::warning( this, QStringLiteral("No Selection"), QStringLiteral("Please select a computer to unregister.") );
		return;
	}

	QTreeWidgetItem* item = selectedItems.first();
	QString classWatchId = item->data( 0, Qt::UserRole ).toString();
	
	if ( classWatchId.isEmpty() ) {
		classWatchId = item->text( 0 );
	}

	int result = QMessageBox::warning( this, QStringLiteral("Confirm Unregister"),
									   QString( QStringLiteral("Are you sure you want to unregister computer %1?") ).arg( classWatchId ),
									   QMessageBox::Yes | QMessageBox::No );
	
	if ( result == QMessageBox::Yes ) {
		m_classWatchManager->unregisterComputer( classWatchId );
		refreshComputerList();
	}
}

void ClassWatchComputerPanel::onSaveRegistry()
{
	QString filePath = QFileDialog::getSaveFileName( this, QStringLiteral("Save ClassWatch Registry"),
												 QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation ) + QStringLiteral("/classwatch_computers.json"),
												 QStringLiteral("JSON Files (*.json)") );
	
	if ( filePath.isEmpty() ) {
		return;
	}

	if ( m_classWatchManager->saveClassWatchRegistry( filePath ) ) {
		QMessageBox::warning( this, QStringLiteral("Success"), QStringLiteral("Registry saved successfully.") );
	} else {
		QMessageBox::warning( this, QStringLiteral("Error"), QStringLiteral("Failed to save registry.") );
	}
}

void ClassWatchComputerPanel::onLoadRegistry()
{
	QString filePath = QFileDialog::getOpenFileName( this, QStringLiteral("Load ClassWatch Registry"),
												 QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation ),
												 QStringLiteral("JSON Files (*.json)") );
	
	if ( filePath.isEmpty() ) {
		return;
	}

	if ( m_classWatchManager->loadClassWatchRegistry( filePath ) ) {
		QMessageBox::warning( this, QStringLiteral("Success"), QStringLiteral("Registry loaded successfully.") );
		refreshComputerList();
	} else {
		QMessageBox::warning( this, QStringLiteral("Error"), QStringLiteral("Failed to load registry.") );
	}
}

void ClassWatchComputerPanel::onClose()
{
	// Save registry before closing
	QString registryPath = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + QStringLiteral("/classwatch_computers.json");
	m_classWatchManager->saveClassWatchRegistry( registryPath );
	
	this->close();
}
