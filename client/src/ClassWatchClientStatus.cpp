/*
 * ClassWatchClientStatus.cpp - implementation of ClassWatchClientStatus
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

#include "ClassWatchClientStatus.h"
#include <QApplication>
#include <QHostInfo>
#include <QDateTime>
#include <QStyle>

ClassWatchClientStatus::ClassWatchClientStatus( QWidget* parent ) :
	QDialog( parent ),
	m_idLabel( nullptr ),
	m_statusLabel( nullptr ),
	m_userLabel( nullptr ),
	m_computerLabel( nullptr ),
	m_closeButton( nullptr ),
	m_updateTimer( nullptr )
{
	setupUI();
	
	// Set window properties
	setWindowTitle( QStringLiteral("ClassWatch Client Status") );
	setWindowFlags( Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint );
	setFixedSize( 400, 300 );
	
	// Start update timer
	m_updateTimer = new QTimer( this );
	connect( m_updateTimer, &QTimer::timeout, this, &ClassWatchClientStatus::updateStatus );
	m_updateTimer->start( 5000 ); // Update every 5 seconds
	
	// Initial update
	updateStatus();
}

ClassWatchClientStatus::~ClassWatchClientStatus()
{
}

void ClassWatchClientStatus::setupUI()
{
	auto* mainLayout = new QVBoxLayout( this );
	mainLayout->setSpacing( 10 );
	mainLayout->setContentsMargins( 20, 20, 20, 20 );
	
	// Title
	auto* titleLabel = new QLabel( QStringLiteral("ClassWatch Client"), this );
	titleLabel->setStyleSheet( QStringLiteral("font-size: 18px; font-weight: bold; color: #2c3e50;") );
	titleLabel->setAlignment( Qt::AlignCenter );
	mainLayout->addWidget( titleLabel );
	
	// Separator
	auto* separator = new QFrame( this );
	separator->setFrameShape( QFrame::HLine );
	separator->setFrameShadow( QFrame::Sunken );
	mainLayout->addWidget( separator );
	
	// Computer info group
	auto* computerGroup = new QGroupBox( QStringLiteral("Computer Information"), this );
	auto* computerLayout = new QFormLayout( computerGroup );
	
	m_computerLabel = new QLabel( this );
	computerLayout->addRow( QStringLiteral("Computer Name:"), m_computerLabel );
	
	m_userLabel = new QLabel( this );
	computerLayout->addRow( QStringLiteral("Logged In User:"), m_userLabel );
	
	auto* ipLabel = new QLabel( QHostInfo::localHostName(), this );
	computerLayout->addRow( QStringLiteral("Hostname:"), ipLabel );
	
	mainLayout->addWidget( computerGroup );
	
	// ClassWatch info group
	auto* classWatchGroup = new QGroupBox( QStringLiteral("ClassWatch Information"), this );
	auto* classWatchLayout = new QFormLayout( classWatchGroup );
	
	m_idLabel = new QLabel( this );
	m_idLabel->setStyleSheet( QStringLiteral("font-weight: bold; color: #3498db;") );
	classWatchLayout->addRow( QStringLiteral("ClassWatch ID:"), m_idLabel );
	
	m_statusLabel = new QLabel( this );
	m_statusLabel->setStyleSheet( QStringLiteral("font-weight: bold;") );
	classWatchLayout->addRow( QStringLiteral("Status:"), m_statusLabel );
	
	mainLayout->addWidget( classWatchGroup );
	
	// Spacer
	mainLayout->addStretch();
	
	// Close button
	m_closeButton = new QPushButton( QStringLiteral("Close"), this );
	m_closeButton->setDefault( true );
	connect( m_closeButton, &QPushButton::clicked, this, &ClassWatchClientStatus::onClose );
	mainLayout->addWidget( m_closeButton );
}

void ClassWatchClientStatus::updateStatus()
{
	// Update computer name
	m_computerLabel->setText( QHostInfo::localHostName() );
	
	// Update user (this is a simplified version - in real implementation,
	// we'd get the actual logged-in user from the system)
	m_userLabel->setText( QStringLiteral("Current User") );
	
	// Get ClassWatch ID from configuration
	QString classWatchId = getClassWatchId();
	m_idLabel->setText( classWatchId.isEmpty() ? QStringLiteral("Not Registered") : classWatchId );
	
	// Get connection status
	QString status = getConnectionStatus();
	m_statusLabel->setText( status );
	
	// Set status color
	if ( status == QStringLiteral("Connected") ) {
		m_statusLabel->setStyleSheet( QStringLiteral("font-weight: bold; color: green;") );
	} else if ( status == QStringLiteral("Connecting...") ) {
		m_statusLabel->setStyleSheet( QStringLiteral("font-weight: bold; color: orange;") );
	} else {
		m_statusLabel->setStyleSheet( QStringLiteral("font-weight: bold; color: red;") );
	}
}

QString ClassWatchClientStatus::getClassWatchId()
{
	// This is a simplified implementation
	// In a real implementation, we'd read from the ClassWatch registry
	// or generate an ID based on the computer's network object UID
	
	VeyonConfiguration& config = VeyonCore::config();
	QString prefix = config.classWatchIdPrefix();
	int startNumber = config.classWatchIdStartNumber();
	
	// For now, return a placeholder ID
	// In real implementation, this would be looked up from the registry
	return QString( QStringLiteral("%1-%2") ).arg( prefix ).arg( startNumber, 3, 10, QChar( u\'0\' ) );
}

QString ClassWatchClientStatus::getConnectionStatus()
{
	// This is a simplified implementation
	// In real implementation, we'd check the actual connection status
	// to the ClassWatch Master
	
	// For now, return a placeholder status
	return QStringLiteral("Connected");
}

void ClassWatchClientStatus::onClose()
{
	accept();
}
