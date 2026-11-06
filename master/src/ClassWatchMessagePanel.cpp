/*
 * ClassWatchMessagePanel.cpp - implementation of ClassWatchMessagePanel
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

#include "ClassWatchMessagePanel.h"
#include "ui_ClassWatchMessagePanel.h"
#include "VeyonMaster.h"
#include "ClassWatchMessageManager.h"
#include "ClassWatchComputerManagerExtension.h"
#include "ClassWatchComputer.h"
#include "VeyonCore.h"
#include "VeyonConfiguration.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QFileInfo>

ClassWatchMessagePanel::ClassWatchMessagePanel( VeyonMaster* master, QWidget* parent ) :
	QWidget( parent ),
	ui( new Ui::ClassWatchMessagePanel ),
	m_master( master ),
	m_messageManager( nullptr )
{
	ui->setupUi( this );
	
	if ( m_master ) {
		m_messageManager = new ClassWatchMessageManager( this );
		setupConnections();
		populateRecipientList();
	}
	
	// Initialize filter combo
	ui->filterCombo->addItem( QStringLiteral("All Messages"), QStringLiteral("all") );
	ui->filterCombo->addItem( QStringLiteral("Private"), QStringLiteral("private") );
	ui->filterCombo->addItem( QStringLiteral("Announcements"), QStringLiteral("announcement") );
	ui->filterCombo->addItem( QStringLiteral("Broadcasts"), QStringLiteral("broadcast") );
	ui->filterCombo->addItem( QStringLiteral("System"), QStringLiteral("system") );
	
	refreshAll();
}

ClassWatchMessagePanel::~ClassWatchMessagePanel()
{
	delete ui;
}

void ClassWatchMessagePanel::setupConnections()
{
	if ( !m_messageManager ) {
		return;
	}

	// Connect to message manager signals
	connect( m_messageManager, &ClassWatchMessageManager::messageSent,
			 this, [this]( const QString& messageId, const QString& ) {
			 updateStatus( QString( QStringLiteral("Message sent: %1") ).arg( messageId ) );
			 populateMessageHistory();
		 } );

	connect( m_messageManager, &ClassWatchMessageManager::messageDelivered,
			 this, [this]( const QString& messageId ) {
			 updateStatus( QString( QStringLiteral("Message delivered: %1") ).arg( messageId ) );
			 populateMessageHistory();
		 } );

	connect( m_messageManager, &ClassWatchMessageManager::messageRead,
			 this, [this]( const QString& messageId ) {
			 updateStatus( QString( QStringLiteral("Message read: %1") ).arg( messageId ) );
			 populateMessageHistory();
		 } );

	connect( m_messageManager, &ClassWatchMessageManager::messageFailed,
			 this, [this]( const QString& messageId, const QString& error ) {
			 updateStatus( QString( QStringLiteral("Message failed: %1 - %2") ).arg( messageId, error ) );
			 populateMessageHistory();
		 } );

	connect( m_messageManager, &ClassWatchMessageManager::fileTransferStarted,
			 this, [this]( const QString&, const QString& ) {
			 updateStatus( QStringLiteral("File transfer started") );
			 populateTransferHistory();
		 } );

	connect( m_messageManager, &ClassWatchMessageManager::fileTransferProgress,
			 this, &ClassWatchMessagePanel::onFileTransferProgress );

	connect( m_messageManager, &ClassWatchMessageManager::fileTransferCompleted,
			 this, [this]( const QString& ) {
			 updateStatus( QStringLiteral("File transfer completed") );
			 populateTransferHistory();
		 } );

	connect( m_messageManager, &ClassWatchMessageManager::fileTransferFailed,
			 this, [this]( const QString&, const QString& error ) {
			 updateStatus( QString( QStringLiteral("File transfer failed: %1") ).arg( error ) );
			 populateTransferHistory();
		 } );

	// Connect UI signals
	connect( ui->sendButton, &QPushButton::clicked, this, &ClassWatchMessagePanel::onSendMessage );
	connect( ui->clearButton, &QPushButton::clicked, this, &ClassWatchMessagePanel::onClearMessage );
	connect( ui->refreshHistoryButton, &QPushButton::clicked, this, &ClassWatchMessagePanel::onRefreshHistory );
	connect( ui->clearHistoryButton, &QPushButton::clicked, this, &ClassWatchMessagePanel::onClearHistory );
	connect( ui->exportHistoryButton, &QPushButton::clicked, this, &ClassWatchMessagePanel::onExportHistory );
	connect( ui->browseButton, &QPushButton::clicked, this, &ClassWatchMessagePanel::onBrowseFile );
	connect( ui->sendFileButton, &QPushButton::clicked, this, &ClassWatchMessagePanel::onSendFile );
	connect( ui->sendChatButton, &QPushButton::clicked, this, &ClassWatchMessagePanel::onSendChatMessage );
	connect( ui->closeButton, &QPushButton::clicked, this, &ClassWatchMessagePanel::onClose );
}

void ClassWatchMessagePanel::onSendMessage()
{
	QString message = ui->messageTextEdit->toPlainText().trimmed();
	if ( message.isEmpty() ) {
		QMessageBox::warning( this, QStringLiteral("Empty Message"), QStringLiteral("Please enter a message.") );
		return;
	}

	QString recipient = getSelectedRecipient();
	if ( recipient.isEmpty() && ui->privateRadio->isChecked() ) {
		QMessageBox::warning( this, QStringLiteral("No Recipient"), QStringLiteral("Please select a recipient for private message.") );
		return;
	}

	QString messageType = getMessageType();
	QString messageId;
	
	if ( messageType == QStringLiteral("private") ) {
		messageId = m_messageManager->sendPrivateMessage( recipient, message );
	} else if ( messageType == QStringLiteral("announcement") ) {
		messageId = m_messageManager->sendAnnouncement( message );
	} else if ( messageType == QStringLiteral("broadcast") ) {
		messageId = m_messageManager->sendBroadcast( message );
	}

	if ( messageId.isEmpty() ) {
		QMessageBox::warning( this, QStringLiteral("Error"), QStringLiteral("Failed to send message.") );
		return;
	}

	ui->messageTextEdit->clear();
	updateStatus( QString( QStringLiteral("Message sent: %1") ).arg( messageId ) );
	populateMessageHistory();
}

void ClassWatchMessagePanel::onClearMessage()
{
	ui->messageTextEdit->clear();
	ui->statusLabel->setText( QStringLiteral("Ready") );
}

void ClassWatchMessagePanel::onRefreshHistory()
{
	populateMessageHistory();
	populateTransferHistory();
	populateChatHistory();
}

void ClassWatchMessagePanel::onClearHistory()
{
	// This would clear message history
	// For now, just refresh the display
	populateMessageHistory();
}

void ClassWatchMessagePanel::onExportHistory()
{
	QString filePath = QFileDialog::getSaveFileName( this, QStringLiteral("Export Message History"),
												 QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation ) + QStringLiteral("/classwatch_messages.json"),
												 QStringLiteral("JSON Files (*.json)") );
	
	if ( filePath.isEmpty() ) {
		return;
	}

	QList<QJsonObject> messages = m_messageManager->getMessages();
	QJsonArray messageArray;
	for ( const QJsonObject& msg : messages ) {
		messageArray.append( msg );
	}

	QJsonObject rootObj;
	rootObj[QStringLiteral("exportTime")] = QDateTime::currentDateTime().toString( Qt::ISODate );
	rootObj[QStringLiteral("messageCount")] = messages.size();
	rootObj[QStringLiteral("messages")] = messageArray;

	QJsonDocument doc( rootObj );
	QFile file( filePath );
	
	if ( file.open( QIODevice::WriteOnly ) ) {
		file.write( doc.toJson() );
		file.close();
		QMessageBox::warning( this, QStringLiteral("Success"), QStringLiteral("Message history exported successfully.") );
	} else {
		QMessageBox::warning( this, QStringLiteral("Error"), QStringLiteral("Failed to export message history.") );
	}
}

void ClassWatchMessagePanel::onBrowseFile()
{
	QString filePath = QFileDialog::getOpenFileName( this, QStringLiteral("Select File to Send"),
												 QString(),
												 QStringLiteral("All Files (*.*)") );
	
	if ( !filePath.isEmpty() ) {
		ui->filePathEdit->setText( filePath );
	}
}

void ClassWatchMessagePanel::onSendFile()
{
	QString filePath = ui->filePathEdit->text();
	if ( filePath.isEmpty() ) {
		QMessageBox::warning( this, QStringLiteral("No File"), QStringLiteral("Please select a file to send.") );
		return;
	}

	QString recipient = ui->fileRecipientCombo->currentText();
	if ( recipient.isEmpty() ) {
		QMessageBox::warning( this, QStringLiteral("No Recipient"), QStringLiteral("Please select a recipient.") );
		return;
	}

	QString transferId = m_messageManager->sendFile( recipient, filePath );
	if ( transferId.isEmpty() ) {
		QMessageBox::warning( this, QStringLiteral("Error"), QStringLiteral("Failed to send file.") );
		return;
	}

	ui->filePathEdit->clear();
	updateStatus( QString( QStringLiteral("File transfer started: %1") ).arg( transferId ) );
	populateTransferHistory();
}

void ClassWatchMessagePanel::onSendChatMessage()
{
	QString message = ui->chatInputEdit->text().trimmed();
	if ( message.isEmpty() ) {
		return;
	}

	// For now, this is a simplified chat implementation
	// In a real implementation, this would use the chat room system
	
	QString timestamp = QDateTime::currentDateTime().toString( QStringLiteral("hh:mm:ss") );
	ui->chatHistoryList->addItem( QString( QStringLiteral("[%1] You: %2") ).arg( timestamp, message ) );
	ui->chatInputEdit->clear();
	
	updateStatus( QStringLiteral("Chat message sent") );
}

void ClassWatchMessagePanel::onClose()
{
	this->close();
}

void ClassWatchMessagePanel::onMessageStatusChanged( const QString& messageId )
{
	Q_UNUSED( messageId );
	populateMessageHistory();
}

void ClassWatchMessagePanel::onFileTransferProgress( const QString& transferId, int progress )
{
	updateStatus( QString( QStringLiteral("File transfer %1: %2%") ).arg( transferId ).arg( progress ) );
}

void ClassWatchMessagePanel::populateRecipientList()
{
	if ( !m_master ) {
		return;
	}

	// Get computers from ClassWatch computer manager
	ClassWatchComputerManagerExtension& cwManager = m_master->classWatchComputerManager();
	ClassWatchComputerList computers = cwManager.getAllClassWatchComputers();
	
	ui->recipientCombo->clear();
	ui->fileRecipientCombo->clear();
	
	for ( const ClassWatchComputer& computer : computers ) {
		QString displayText = QString( QStringLiteral("%1 - %2") ).arg( computer.classWatchId(), computer.displayName() );
		ui->recipientCombo->addItem( displayText, computer.classWatchId() );
		ui->fileRecipientCombo->addItem( displayText, computer.classWatchId() );
	}
	
	// Add special options
	ui->recipientCombo->addItem( QStringLiteral("All Computers"), QStringLiteral("all") );
	ui->fileRecipientCombo->addItem( QStringLiteral("All Computers"), QStringLiteral("all") );
}

void ClassWatchMessagePanel::populateMessageHistory()
{
	if ( !m_messageManager ) {
		return;
	}

	ui->messageHistoryTree->clear();

	QString filter = ui->filterCombo->currentData().toString();
	QList<QJsonObject> messages = m_messageManager->getMessages();
	
	for ( const QJsonObject& msg : messages ) {
		QString type = msg[QStringLiteral("type")].toString();
		
		// Apply filter
		if ( filter != QStringLiteral("all") && type != filter ) {
			continue;
		}
		
		QTreeWidgetItem* item = new QTreeWidgetItem( ui->messageHistoryTree );
		item->setText( 0, msg[QStringLiteral("timestamp")].toString() );
		item->setText( 1, type );
		item->setText( 2, msg[QStringLiteral("recipientId")].toString() );
		item->setText( 3, msg[QStringLiteral("message")].toString().left( 50 ) + ( msg[QStringLiteral("message")].toString().length() > 50 ? QStringLiteral("...") : QStringLiteral("") ) );
		item->setText( 4, msg[QStringLiteral("status")].toString() );
		
		// Set status color
		QString status = msg[QStringLiteral("status")].toString();
		if ( status == QStringLiteral("delivered") || status == QStringLiteral("read") ) {
			item->setForeground( 4, QColor( Qt::green ) );
		} else if ( status == QStringLiteral("failed") ) {
			item->setForeground( 4, QColor( Qt::red ) );
		} else if ( status == QStringLiteral("sent") ) {
			item->setForeground( 4, QColor( Qt::blue ) );
		}
	}
}

void ClassWatchMessagePanel::populateTransferHistory()
{
	if ( !m_messageManager ) {
		return;
	}

	ui->transferHistoryTree->clear();

	QList<QJsonObject> transfers = m_messageManager->getFileTransfers();
	
	for ( const QJsonObject& transfer : transfers ) {
		QTreeWidgetItem* item = new QTreeWidgetItem( ui->transferHistoryTree );
		item->setText( 0, transfer[QStringLiteral("timestamp")].toString() );
		item->setText( 1, transfer[QStringLiteral("fileName")].toString() );
		item->setText( 2, transfer[QStringLiteral("recipientId")].toString() );
		
		qint64 size = transfer[QStringLiteral("fileSize")].toVariant().toLongLong();
		QString sizeStr;
		if ( size < 1024 ) {
			sizeStr = QString( QStringLiteral("%1 B") ).arg( size );
		} else if ( size < 1024 * 1024 ) {
			sizeStr = QString( QStringLiteral("%1 KB") ).arg( size / 1024.0, 0, 'f', 1 );
		} else if ( size < 1024 * 1024 * 1024 ) {
			sizeStr = QString( QStringLiteral("%1 MB") ).arg( size / ( 1024.0 * 1024.0 ), 0, 'f', 1 );
		} else {
			sizeStr = QString( QStringLiteral("%1 GB") ).arg( size / ( 1024.0 * 1024.0 * 1024.0 ), 0, 'f', 1 );
		}
		item->setText( 3, sizeStr );
		item->setText( 4, transfer[QStringLiteral("status")].toString() );
	}
}

void ClassWatchMessagePanel::populateChatHistory()
{
	ui->chatHistoryList->clear();
	// This would populate from chat rooms
	// For now, it's empty
}

void ClassWatchMessagePanel::updateStatus( const QString& status )
{
	ui->statusLabel->setText( status );
}

QString ClassWatchMessagePanel::getSelectedRecipient() const
{
	return ui->recipientCombo->currentData().toString();
}

QString ClassWatchMessagePanel::getMessageType() const
{
	if ( ui->privateRadio->isChecked() ) {
		return QStringLiteral("private");
	} else if ( ui->announcementRadio->isChecked() ) {
		return QStringLiteral("announcement");
	} else if ( ui->broadcastRadio->isChecked() ) {
		return QStringLiteral("broadcast");
	}
	return QStringLiteral("private");
}

void ClassWatchMessagePanel::refreshAll()
{
	populateRecipientList();
	populateMessageHistory();
	populateTransferHistory();
	populateChatHistory();
}
