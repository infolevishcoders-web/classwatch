/*
 * ClassWatchClientApp.cpp - implementation of ClassWatchClientApp
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

#include "ClassWatchClientApp.h"
#include "ClassWatchClientStatus.h"
#include "ClassWatchSecurityManager.h"
#include <QAction>
#include <QHostInfo>
#include <QIcon>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTimer>

ClassWatchClientApp::ClassWatchClientApp( int& argc, char** argv ) :
	QApplication( argc, argv ),
	m_trayIcon( nullptr ),
	m_trayMenu( nullptr ),
	m_updateTimer( nullptr ),
	m_isConnected( false )
{
	// Set application properties
	setApplicationName( QStringLiteral("ClassWatch Client") );
	setApplicationVersion( VeyonCore::versionString() );
	setOrganizationName( QStringLiteral("Levish Coders") );
	setWindowIcon( QIcon( QStringLiteral(":/core/icon64.png") ) );
	
	// Initialize VeyonCore
	VeyonCore::setupApplicationParameters();
	
	// Setup tray icon
	setupTrayIcon();
	setupMenu();
	
	// Setup update timer
	m_updateTimer = new QTimer( this );
	connect( m_updateTimer, &QTimer::timeout, this, &ClassWatchClientApp::updateTrayIcon );
	m_updateTimer->start( 5000 ); // Update every 5 seconds
	
	// Initial update
	updateTrayIcon();
}

ClassWatchClientApp::~ClassWatchClientApp()
{
}

void ClassWatchClientApp::setupTrayIcon()
{
	m_trayIcon = new QSystemTrayIcon( this );
	m_trayIcon->setIcon( style()->standardIcon( QStyle::SP_ComputerIcon ) );
	m_trayIcon->setToolTip( QStringLiteral("ClassWatch Client") );
	m_trayIcon->show();
	
	connect( m_trayIcon, &QSystemTrayIcon::activated, this, [this]( QSystemTrayIcon::ActivationReason reason ) {
		if ( reason == QSystemTrayIcon::DoubleClick ) {
			showStatusDialog();
		}
	} );
}

void ClassWatchClientApp::setupMenu()
{
	m_trayMenu = new QMenu();
	
	// Status action
	auto* statusAction = new QAction( QStringLiteral("Show Status"), this );
	connect( statusAction, &QAction::triggered, this, &ClassWatchClientApp::showStatusDialog );
	m_trayMenu->addAction( statusAction );
	
	m_trayMenu->addSeparator();
	
	// Quit action
	auto* quitAction = new QAction( QStringLiteral("Quit"), this );
	connect( quitAction, &QAction::triggered, this, &ClassWatchClientApp::onQuit );
	m_trayMenu->addAction( quitAction );
	
	m_trayIcon->setContextMenu( m_trayMenu );
}

void ClassWatchClientApp::showStatusDialog()
{
	ClassWatchClientStatus dialog;
	dialog.exec();
}

void ClassWatchClientApp::updateTrayIcon()
{
	// pick up admin warnings raised on this device (lockdown violations,
	// remote commands from the master, etc.) and show them locally
	auto& security = ClassWatchSecurityManager::instance();
	if( security.hasPendingDeviceWarning() )
	{
		const auto warning = security.takePendingDeviceWarning();
		QMessageBox::warning( nullptr,
							  warning.value( QStringLiteral("title") ).toString(),
							  warning.value( QStringLiteral("message") ).toString() +
							  QStringLiteral("\n\nLocal time: %1\nUTC time: %2")
								  .arg( warning.value( QStringLiteral("localTime") ).toString(),
										warning.value( QStringLiteral("utcTime") ).toString() ) );
	}

	QString classWatchId = getClassWatchId();
	QString status = getConnectionStatus();
	
	// Update tooltip
	QString tooltip = QString( QStringLiteral("ClassWatch Client\nID: %1\nStatus: %2") ).arg( classWatchId, status );
	m_trayIcon->setToolTip( tooltip );
	
	// Update icon based on status
	if ( status == QStringLiteral("Connected") ) {
		m_trayIcon->setIcon( style()->standardIcon( QStyle::SP_DialogApplyButton ) );
		m_isConnected = true;
	} else if ( status == QStringLiteral("Connecting...") ) {
		m_trayIcon->setIcon( style()->standardIcon( QStyle::SP_MessageBoxInformation ) );
		m_isConnected = false;
	} else {
		m_trayIcon->setIcon( style()->standardIcon( QStyle::SP_MessageBoxWarning ) );
		m_isConnected = false;
	}
}

void ClassWatchClientApp::onQuit()
{
	auto& security = ClassWatchSecurityManager::instance();

	// classroom lockdown: students cannot disconnect the client without
	// the administrator password
	if( security.isTerminalLockdownActive() && security.hasAdminPassword() )
	{
		bool ok = false;
		const auto password = QInputDialog::getText(
			nullptr, QStringLiteral("ClassWatch Administrator Required"),
			QStringLiteral("ClassWatch is locked down by your administrator.\n"
						   "Enter the admin password to quit the client:" ),
			QLineEdit::Password, QString(), &ok );
		if( !ok || !security.verifyAdminPassword( password ) )
		{
			security.logSecurityEvent(
				QStringLiteral("client_quit_denied"),
				QStringLiteral("Unauthorised attempt to quit ClassWatch Client") );
			security.raiseDeviceWarning(
				QStringLiteral("ClassWatch Security Alert"),
				QStringLiteral("An unauthorised attempt to close ClassWatch was blocked and reported to the administrator.") );
			return;
		}
		security.logSecurityEvent( QStringLiteral("client_quit_authorised"),
								  QStringLiteral("Administrator authorised client quit") );
	}

	// Confirm quit
	int result = QMessageBox::question( nullptr, QStringLiteral("Quit ClassWatch Client"),
									   QStringLiteral("Are you sure you want to quit ClassWatch Client?\nThe client will continue to run as a service."),
									   QMessageBox::Yes | QMessageBox::No );

	if ( result == QMessageBox::Yes ) {
		quit();
	}
}

QString ClassWatchClientApp::getClassWatchId()
{
	// This is a simplified implementation
	// In a real implementation, we'd read from the ClassWatch registry
	// or generate an ID based on the computer's network object UID
	
	VeyonConfiguration& config = VeyonCore::config();
	QString prefix = config.classWatchIdPrefix();
	int startNumber = config.classWatchIdStartNumber();
	
	// For now, return a placeholder ID
	return QStringLiteral("%1-%2").arg( prefix ).arg( startNumber, 3, 10, QChar(QLatin1Char('0')) );
}

QString ClassWatchClientApp::getConnectionStatus()
{
	// This is a simplified implementation
	// In real implementation, we'd check the actual connection status
	// to the ClassWatch Master
	
	// For now, return a placeholder status
	return m_isConnected ? QStringLiteral("Connected") : QStringLiteral("Disconnected");
}
