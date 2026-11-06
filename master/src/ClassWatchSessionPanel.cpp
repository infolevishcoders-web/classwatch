/*
 * ClassWatchSessionPanel.cpp - implementation of ClassWatchSessionPanel
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

#include "ClassWatchSessionPanel.h"
#include "ui_ClassWatchSessionPanel.h"
#include "VeyonMaster.h"
#include "ClassWatchSessionManager.h"
#include "VeyonCore.h"
#include "VeyonConfiguration.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>

ClassWatchSessionPanel::ClassWatchSessionPanel( VeyonMaster* master, QWidget* parent ) :
	QWidget( parent ),
	ui( new Ui::ClassWatchSessionPanel ),
	m_master( master ),
	m_sessionManager( nullptr )
{
	ui->setupUi( this );
	
	if ( m_master ) {
		// Create session manager if it doesn't exist
		// In a real implementation, this would be part of VeyonMaster
		m_sessionManager = new ClassWatchSessionManager( this );
		setupConnections();
	}
	
	// Initialize UI
	ui->sessionTreeWidget->setColumnWidth( 0, 80 );   // Session ID
	ui->sessionTreeWidget->setColumnWidth( 1, 80 );   // Type
	ui->sessionTreeWidget->setColumnWidth( 2, 120 );  // User
	ui->sessionTreeWidget->setColumnWidth( 3, 120 );  // Student
	ui->sessionTreeWidget->setColumnWidth( 4, 100 );  // Uptime
	ui->sessionTreeWidget->setColumnWidth( 5, 100 );  // Status
	
	// Set monitor button state based on configuration
	VeyonConfiguration& config = VeyonCore::config();
	ui->monitorToggleButton->setChecked( config.enableMultiSessionSupport() );
	ui->monitorToggleButton->setText( config.enableMultiSessionSupport() ? "Stop Monitoring" : "Start Monitoring" );
	
	refreshSessionList();
}

ClassWatchSessionPanel::~ClassWatchSessionPanel()
{
	delete ui;
}

void ClassWatchSessionPanel::setupConnections()
{
	if ( !m_sessionManager ) {
		return;
	}

	// Connect to session manager signals
	connect( m_sessionManager, &ClassWatchSessionManager::sessionDetected,
			 this, [this]( int sessionId, const PlatformSessionFunctions::SessionInfo& ) {
			 refreshSessionList();
		 } );

	connect( m_sessionManager, &ClassWatchSessionManager::sessionLost,
			 this, [this]( int ) {
			 refreshSessionList();
		 } );

	connect( m_sessionManager, &ClassWatchSessionManager::sessionTypeChanged,
			 this, [this]( int, const QString&, const QString& ) {
			 refreshSessionList();
		 } );

	connect( m_sessionManager, &ClassWatchSessionManager::studentAssigned,
			 this, [this]( int, const QString& ) {
			 refreshSessionList();
		 } );

	connect( m_sessionManager, &ClassWatchSessionManager::studentRemoved,
			 this, [this]( int ) {
			 refreshSessionList();
		 } );

	connect( m_sessionManager, &ClassWatchSessionManager::sessionEvent,
			 this, [this]( int, const QString&, const QString& ) {
			 populateSessionHistory();
		 } );

	// Connect UI signals
	connect( ui->refreshButton, &QPushButton::clicked, this, &ClassWatchSessionPanel::refreshSessionList );
	connect( ui->assignStudentButton, &QPushButton::clicked, this, &ClassWatchSessionPanel::onAssignStudent );
	connect( ui->removeStudentButton, &QPushButton::clicked, this, &ClassWatchSessionPanel::onRemoveStudent );
	connect( ui->clearHistoryButton, &QPushButton::clicked, this, &ClassWatchSessionPanel::onClearHistory );
	connect( ui->exportHistoryButton, &QPushButton::clicked, this, &ClassWatchSessionPanel::onExportHistory );
	connect( ui->monitorToggleButton, &QPushButton::toggled, this, &ClassWatchSessionPanel::onMonitorToggle );
	connect( ui->closeButton, &QPushButton::clicked, this, &ClassWatchSessionPanel::onClose );
	connect( ui->sessionTreeWidget, &QTreeWidget::itemSelectionChanged, this, &ClassWatchSessionPanel::onSessionSelectionChanged );
}

void ClassWatchSessionPanel::refreshSessionList()
{
	if ( !m_sessionManager ) {
		return;
	}

	populateSessionTree();
	populateSessionHistory();
}

void ClassWatchSessionPanel::onSessionSelectionChanged()
{
	QList<QTreeWidgetItem*> selectedItems = ui->sessionTreeWidget->selectedItems();
	if ( selectedItems.isEmpty() ) {
		// Clear details
		ui->sessionIdValue->setText( "-" );
		ui->sessionTypeValue->setText( "-" );
		ui->userNameValue->setText( "-" );
		ui->clientAddressValue->setText( "-" );
		ui->clientNameValue->setText( "-" );
		ui->hostNameValue->setText( "-" );
		ui->uptimeValue->setText( "-" );
		ui->assignedStudentEdit->clear();
		return;
	}

	QTreeWidgetItem* item = selectedItems.first();
	int sessionId = item->data( 0, Qt::UserRole ).toInt();
	updateSessionDetails( sessionId );
}

void ClassWatchSessionPanel::updateSessionDetails( int sessionId )
{
	if ( !m_sessionManager ) {
		return;
	}

	PlatformSessionFunctions::SessionInfo info = m_sessionManager->getSession( sessionId );
	if ( info.id == PlatformSessionFunctions::InvalidSessionId ) {
		return;
	}

	ui->sessionIdValue->setText( QString::number( sessionId ) );
	ui->sessionTypeValue->setText( m_sessionManager->getSessionType( sessionId ) );
	ui->userNameValue->setText( info.clientName );
	ui->clientAddressValue->setText( info.clientAddress );
	ui->clientNameValue->setText( info.clientName );
	ui->hostNameValue->setText( info.hostName );
	ui->uptimeValue->setText( formatUptime( info.uptime ) );
	
	QString studentId = m_sessionManager->getStudentForSession( sessionId );
	ui->assignedStudentEdit->setText( studentId );
}

void ClassWatchSessionPanel::populateSessionTree()
{
	if ( !m_sessionManager ) {
		return;
	}

	ui->sessionTreeWidget->clear();

	QList<PlatformSessionFunctions::SessionInfo> sessions = m_sessionManager->getActiveSessions();
	
	for ( const PlatformSessionFunctions::SessionInfo& info : sessions ) {
		QTreeWidgetItem* item = new QTreeWidgetItem( ui->sessionTreeWidget );
		item->setText( 0, QString::number( info.id ) );
		item->setText( 1, m_sessionManager->getSessionType( info.id ) );
		item->setText( 2, info.clientName );
		item->setText( 3, m_sessionManager->getStudentForSession( info.id ) );
		item->setText( 4, formatUptime( info.uptime ) );
		item->setText( 5, m_sessionManager->isActiveSession( info.id ) ? "Active" : "Inactive" );
		
		// Set status color
		if ( m_sessionManager->isActiveSession( info.id ) ) {
			item->setForeground( 5, QColor( "green" ) );
		} else {
			item->setForeground( 5, QColor( "red" ) );
		}
		
		// Store session ID as user data
		item->setData( 0, Qt::UserRole, info.id );
	}
	
	ui->sessionTreeWidget->resizeColumnToContents( 0 );
	ui->sessionTreeWidget->resizeColumnToContents( 1 );
	ui->sessionTreeWidget->resizeColumnToContents( 2 );
	ui->sessionTreeWidget->resizeColumnToContents( 3 );
	ui->sessionTreeWidget->resizeColumnToContents( 4 );
	ui->sessionTreeWidget->resizeColumnToContents( 5 );
}

void ClassWatchSessionPanel::populateSessionHistory()
{
	if ( !m_sessionManager ) {
		return;
	}

	ui->sessionHistoryList->clear();

	QList<QJsonObject> history = m_sessionManager->getSessionHistory( -1, 50 ); // Last 50 events
	
	for ( const QJsonObject& event : history ) {
		QString timestamp = event["timestamp"].toString();
		QString eventType = event["eventType"].toString();
		QString details = event["details"].toString();
		int sessionId = event["sessionId"].toInt();
		
		QString displayText = QString( "[%1] Session %2: %3" ).arg( timestamp, QString::number( sessionId ), eventType );
		if ( !details.isEmpty() ) {
			displayText += QString( " - %1" ).arg( details );
		}
		
		ui->sessionHistoryList->addItem( displayText );
	}
}

void ClassWatchSessionPanel::onAssignStudent()
{
	QList<QTreeWidgetItem*> selectedItems = ui->sessionTreeWidget->selectedItems();
	if ( selectedItems.isEmpty() ) {
		QMessageBox::warning( this, "No Selection", "Please select a session first." );
		return;
	}

	QString studentId = ui->assignedStudentEdit->text().trimmed();
	if ( studentId.isEmpty() ) {
		QMessageBox::warning( this, "Invalid Input", "Please enter a student ID." );
		return;
	}

	QTreeWidgetItem* item = selectedItems.first();
	int sessionId = item->data( 0, Qt::UserRole ).toInt();
	
	m_sessionManager->assignStudentToSession( sessionId, studentId );
	refreshSessionList();
	
	QMessageBox::information( this, "Success", QString( "Student %1 assigned to session %2." ).arg( studentId ).arg( sessionId ) );
}

void ClassWatchSessionPanel::onRemoveStudent()
{
	QList<QTreeWidgetItem*> selectedItems = ui->sessionTreeWidget->selectedItems();
	if ( selectedItems.isEmpty() ) {
		QMessageBox::warning( this, "No Selection", "Please select a session first." );
		return;
	}

	QTreeWidgetItem* item = selectedItems.first();
	int sessionId = item->data( 0, Qt::UserRole ).toInt();
	
	m_sessionManager->removeStudentFromSession( sessionId );
	refreshSessionList();
	
	QMessageBox::information( this, "Success", QString( "Student removed from session %1." ).arg( sessionId ) );
}

void ClassWatchSessionPanel::onClearHistory()
{
	int result = QMessageBox::question( this, "Confirm Clear",
									   "Are you sure you want to clear all session history?",
									   QMessageBox::Yes | QMessageBox::No );
	
	if ( result == QMessageBox::Yes ) {
		m_sessionManager->clearSessionHistory();
		populateSessionHistory();
	}
}

void ClassWatchSessionPanel::onExportHistory()
{
	QString filePath = QFileDialog::getSaveFileName( this, "Export Session History",
												 QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation ) + "/classwatch_session_history.json",
												 "JSON Files (*.json)" );
	
	if ( filePath.isEmpty() ) {
		return;
	}

	QList<QJsonObject> history = m_sessionManager->getSessionHistory( -1, 1000 );
	QJsonArray historyArray;
	for ( const QJsonObject& event : history ) {
		historyArray.append( event );
	}

	QJsonObject rootObj;
	rootObj["exportTime"] = QDateTime::currentDateTime().toString( Qt::ISODate );
	rootObj["sessionCount"] = m_sessionManager->sessionCount();
	rootObj["history"] = historyArray;

	QJsonDocument doc( rootObj );
	QFile file( filePath );
	
	if ( file.open( QIODevice::WriteOnly ) ) {
		file.write( doc.toJson() );
		file.close();
		QMessageBox::information( this, "Success", "Session history exported successfully." );
	} else {
		QMessageBox::warning( this, "Error", "Failed to export session history." );
	}
}

void ClassWatchSessionPanel::onMonitorToggle()
{
	bool enabled = ui->monitorToggleButton->isChecked();
	
	VeyonConfiguration& config = VeyonCore::config();
	config.setEnableMultiSessionSupport( enabled );
	config.save();
	
	if ( enabled ) {
		m_sessionManager->startSessionMonitoring();
		ui->monitorToggleButton->setText( "Stop Monitoring" );
	} else {
		m_sessionManager->stopSessionMonitoring();
		ui->monitorToggleButton->setText( "Start Monitoring" );
	}
	
	refreshSessionList();
}

void ClassWatchSessionPanel::onClose()
{
	this->close();
}

QString ClassWatchSessionPanel::formatUptime( int seconds ) const
{
	if ( seconds < 0 ) {
		return "-";
	}
	
	int hours = seconds / 3600;
	int minutes = ( seconds % 3600 ) / 60;
	int secs = seconds % 60;
	
	if ( hours > 0 ) {
		return QString( "%1:%2:%3" ).arg( hours ).arg( minutes, 2, 10, QChar( '0' ) ).arg( secs, 2, 10, QChar( '0' ) );
	} else {
		return QString( "%1:%2" ).arg( minutes ).arg( secs, 2, 10, QChar( '0' ) );
	}
}
