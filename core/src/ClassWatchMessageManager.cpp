/*
 * ClassWatchMessageManager.cpp - implementation of ClassWatchMessageManager
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

#include "ClassWatchMessageManager.h"
#include "VeyonCore.h"
#include "VeyonConfiguration.h"
#include <QDateTime>
#include <QFileInfo>
#include <QDebug>

ClassWatchMessageManager::ClassWatchMessageManager( QObject* parent ) :
	QObject( parent ),
	m_messageTimeoutTimer( new QTimer( this ) ),
	m_cleanupTimer( new QTimer( this ) ),
	m_maxMessageLength( 1000 ),
	m_messageRetentionDays( 30 ),
	m_enableReadReceipts( true ),
	m_messageCounter( 0 ),
	m_transferCounter( 0 ),
	m_chatCounter( 0 )
{
	// Setup timers
	connect( m_messageTimeoutTimer, &QTimer::timeout, this, &ClassWatchMessageManager::onMessageTimeout );
	m_messageTimeoutTimer->setInterval( 5000 ); // Check every 5 seconds
	m_messageTimeoutTimer->start();
	
	connect( m_cleanupTimer, &QTimer::timeout, this, &ClassWatchMessageManager::onCleanupTimer );
	m_cleanupTimer->setInterval( 3600000 ); // Cleanup every hour
	m_cleanupTimer->start();
	
	// Load configuration
	VeyonConfiguration& config = VeyonCore::config();
	m_maxMessageLength = 1000; // Default
	m_messageRetentionDays = 30; // Default
	m_enableReadReceipts = config.enableStudentNotifications();
}

ClassWatchMessageManager::~ClassWatchMessageManager()
{
}

QString ClassWatchMessageManager::sendPrivateMessage( const QString& recipientId, const QString& message, const QString& senderId )
{
	if ( !validateMessage( message ) ) {
		return QString();
	}
	
	QString messageId = generateMessageId();
	QString actualSenderId = senderId.isEmpty() ? getDefaultSenderId() : senderId;
	
	QJsonObject msgObj;
	msgObj[QStringLiteral("id")] = messageId;
	msgObj[QStringLiteral("type")] = QStringLiteral("private");
	msgObj[QStringLiteral("senderId")] = actualSenderId;
	msgObj[QStringLiteral("recipientId")] = recipientId;
	msgObj[QStringLiteral("message")] = message;
	msgObj[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString( Qt::ISODate );
	msgObj[QStringLiteral("status")] = QStringLiteral("pending");
	msgObj[QStringLiteral("readReceipts")] = m_enableReadReceipts;
	
	m_messages[messageId] = msgObj;
	m_messageQueue.append( messageId );
	
	logMessageEvent( messageId, QStringLiteral("message_created"), QString( QStringLiteral("Private message to %1") ).arg( recipientId ) );
	processMessageQueue();
	
	return messageId;
}

QString ClassWatchMessageManager::sendAnnouncement( const QString& message, const QString& senderId )
{
	if ( !validateMessage( message ) ) {
		return QString();
	}
	
	QString messageId = generateMessageId();
	QString actualSenderId = senderId.isEmpty() ? getDefaultSenderId() : senderId;
	
	QJsonObject msgObj;
	msgObj[QStringLiteral("id")] = messageId;
	msgObj[QStringLiteral("type")] = QStringLiteral("announcement");
	msgObj[QStringLiteral("senderId")] = actualSenderId;
	msgObj[QStringLiteral("recipientId")] = QStringLiteral("all"); // Broadcast to all
	msgObj[QStringLiteral("message")] = message;
	msgObj[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString( Qt::ISODate );
	msgObj[QStringLiteral("status")] = QStringLiteral("pending");
	msgObj[QStringLiteral("readReceipts")] = m_enableReadReceipts;
	
	m_messages[messageId] = msgObj;
	m_messageQueue.append( messageId );
	
	logMessageEvent( messageId, QStringLiteral("message_created"), QStringLiteral("Class announcement") );
	processMessageQueue();
	
	return messageId;
}

QString ClassWatchMessageManager::sendBroadcast( const QString& message, const QString& senderId )
{
	if ( !validateMessage( message ) ) {
		return QString();
	}
	
	QString messageId = generateMessageId();
	QString actualSenderId = senderId.isEmpty() ? getDefaultSenderId() : senderId;
	
	QJsonObject msgObj;
	msgObj[QStringLiteral("id")] = messageId;
	msgObj[QStringLiteral("type")] = QStringLiteral("broadcast");
	msgObj[QStringLiteral("senderId")] = actualSenderId;
	msgObj[QStringLiteral("recipientId")] = QStringLiteral("all");
	msgObj[QStringLiteral("message")] = message;
	msgObj[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString( Qt::ISODate );
	msgObj[QStringLiteral("status")] = QStringLiteral("pending");
	msgObj[QStringLiteral("readReceipts")] = m_enableReadReceipts;
	msgObj[QStringLiteral("priority")] = QStringLiteral("high");
	
	m_messages[messageId] = msgObj;
	m_messageQueue.append( messageId );
	
	logMessageEvent( messageId, QStringLiteral("message_created"), QStringLiteral("Broadcast message") );
	processMessageQueue();
	
	return messageId;
}

QString ClassWatchMessageManager::sendSystemMessage( const QString& message, const QString& targetId )
{
	if ( !validateMessage( message ) ) {
		return QString();
	}
	
	QString messageId = generateMessageId();
	
	QJsonObject msgObj;
	msgObj[QStringLiteral("id")] = messageId;
	msgObj[QStringLiteral("type")] = QStringLiteral("system");
	msgObj[QStringLiteral("senderId")] = QStringLiteral("system");
	msgObj[QStringLiteral("recipientId")] = targetId.isEmpty() ? QStringLiteral("all") : targetId;
	msgObj[QStringLiteral("message")] = message;
	msgObj[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString( Qt::ISODate );
	msgObj[QStringLiteral("status")] = QStringLiteral("pending");
	msgObj[QStringLiteral("readReceipts")] = false; // System messages don't need read receipts
	
	m_messages[messageId] = msgObj;
	m_messageQueue.append( messageId );
	
	logMessageEvent( messageId, QStringLiteral("message_created"), QString( QStringLiteral("System message to %1") ).arg( targetId.isEmpty() ? QStringLiteral("all") : targetId ) );
	processMessageQueue();
	
	return messageId;
}

void ClassWatchMessageManager::cancelMessage( const QString& messageId )
{
	if ( !m_messages.contains( messageId ) ) {
		return;
	}
	
	QJsonObject& msgObj = m_messages[messageId];
	QString status = msgObj[QStringLiteral("status")].toString();
	
	if ( status == QStringLiteral("pending") || status == QStringLiteral("sent") ) {
		msgObj[QStringLiteral("status")] = QStringLiteral("cancelled");
		m_messageQueue.removeAll( messageId );
		logMessageEvent( messageId, QStringLiteral("message_cancelled"), QStringLiteral("Message cancelled by user") );
	}
}

void ClassWatchMessageManager::deleteMessage( const QString& messageId )
{
	if ( m_messages.contains( messageId ) ) {
		m_messages.remove( messageId );
		m_messageQueue.removeAll( messageId );
	}
}

QJsonObject ClassWatchMessageManager::getMessage( const QString& messageId ) const
{
	return m_messages.value( messageId );
}

QList<QJsonObject> ClassWatchMessageManager::getMessages( const QString& recipientId, int limit ) const
{
	QList<QJsonObject> result;
	
	for ( auto it = m_messages.begin(); it != m_messages.end(); ++it ) {
		const QJsonObject& msgObj = it.value();
		QString msgRecipient = msgObj[QStringLiteral("recipientId")].toString();
		
		if ( recipientId.isEmpty() || msgRecipient == recipientId || msgRecipient == QStringLiteral("all") ) {
			result.append( msgObj );
			if ( result.size() >= limit ) {
				break;
			}
		}
	}
	
	return result;
}

QList<QJsonObject> ClassWatchMessageManager::getPendingMessages() const
{
	QList<QJsonObject> result;
	
	for ( auto it = m_messages.begin(); it != m_messages.end(); ++it ) {
		const QJsonObject& msgObj = it.value();
		if ( msgObj[QStringLiteral("status")].toString() == QStringLiteral("pending") ) {
			result.append( msgObj );
		}
	}
	
	return result;
}

QList<QJsonObject> ClassWatchMessageManager::getFailedMessages() const
{
	QList<QJsonObject> result;
	
	for ( auto it = m_messages.begin(); it != m_messages.end(); ++it ) {
		const QJsonObject& msgObj = it.value();
		if ( msgObj[QStringLiteral("status")].toString() == QStringLiteral("failed") ) {
			result.append( msgObj );
		}
	}
	
	return result;
}

void ClassWatchMessageManager::markMessageAsDelivered( const QString& messageId )
{
	updateMessageStatus( messageId, MessageStatus::Delivered );
}

void ClassWatchMessageManager::markMessageAsRead( const QString& messageId )
{
	updateMessageStatus( messageId, MessageStatus::Read );
}

void ClassWatchMessageManager::markMessageAsFailed( const QString& messageId, const QString& error )
{
	updateMessageStatus( messageId, MessageStatus::Failed, error );
}

QString ClassWatchMessageManager::createChatRoom( const QStringList& participants )
{
	QString chatId = generateChatId();
	
	QJsonObject chatObj;
	chatObj[QStringLiteral("id")] = chatId;
	chatObj[QStringLiteral("participants")] = QJsonArray::fromStringList( participants );
	chatObj[QStringLiteral("created")] = QDateTime::currentDateTime().toString( Qt::ISODate );
	chatObj[QStringLiteral("isActive")] = true;
	
	m_chatRooms[chatId] = chatObj;
	m_chatHistory[chatId] = QList<QJsonObject>();
	
	return chatId;
}

void ClassWatchMessageManager::addParticipantToChat( const QString& chatId, const QString& participantId )
{
	if ( !m_chatRooms.contains( chatId ) ) {
		return;
	}
	
	QJsonObject& chatObj = m_chatRooms[chatId];
	QJsonArray participants = chatObj[QStringLiteral("participants")].toArray();
	
	if ( !participants.contains( participantId ) ) {
		participants.append( participantId );
		chatObj[QStringLiteral("participants")] = participants;
	}
}

void ClassWatchMessageManager::removeParticipantFromChat( const QString& chatId, const QString& participantId )
{
	if ( !m_chatRooms.contains( chatId ) ) {
		return;
	}
	
	QJsonObject& chatObj = m_chatRooms[chatId];
	QJsonArray participants = chatObj[QStringLiteral("participants")].toArray();
	
	for ( int i = 0; i < participants.size(); ++i ) {
		if ( participants[i].toString() == participantId ) {
			participants.removeAt( i );
			break;
		}
	}
	
	chatObj[QStringLiteral("participants")] = participants;
}

QList<QJsonObject> ClassWatchMessageManager::getChatHistory( const QString& chatId, int limit ) const
{
	if ( !m_chatHistory.contains( chatId ) ) {
		return QList<QJsonObject>();
	}
	
	QList<QJsonObject> history = m_chatHistory[chatId];
	if ( history.size() > limit ) {
		return history.mid( history.size() - limit );
	}
	return history;
}

QStringList ClassWatchMessageManager::getChatParticipants( const QString& chatId ) const
{
	if ( !m_chatRooms.contains( chatId ) ) {
		return QStringList();
	}
	
	QJsonObject chatObj = m_chatRooms[chatId];
	QJsonArray participants = chatObj[QStringLiteral("participants")].toArray();
	
	QStringList result;
	for ( const QJsonValue& value : participants ) {
		result.append( value.toString() );
	}
	
	return result;
}

QString ClassWatchMessageManager::sendFile( const QString& recipientId, const QString& filePath, const QString& senderId )
{
	if ( !validateFilePath( filePath ) ) {
		return QString();
	}
	
	QString transferId = generateTransferId();
	QString actualSenderId = senderId.isEmpty() ? getDefaultSenderId() : senderId;
	QFileInfo fileInfo( filePath );
	
	QJsonObject transferObj;
	transferObj[QStringLiteral("id")] = transferId;
	transferObj[QStringLiteral("type")] = QStringLiteral("file");
	transferObj[QStringLiteral("senderId")] = actualSenderId;
	transferObj[QStringLiteral("recipientId")] = recipientId;
	transferObj[QStringLiteral("filePath")] = filePath;
	transferObj[QStringLiteral("fileName")] = fileInfo.fileName();
	transferObj[QStringLiteral("fileSize")] = fileInfo.size();
	transferObj[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString( Qt::ISODate );
	transferObj[QStringLiteral("status")] = QStringLiteral("pending");
	transferObj[QStringLiteral("progress")] = 0;
	
	m_fileTransfers[transferId] = transferObj;
	
	Q_EMIT fileTransferStarted( transferId, filePath );
	logMessageEvent( transferId, QStringLiteral("file_transfer_started"), QString( QStringLiteral("File %1 to %2") ).arg( fileInfo.fileName() ).arg( recipientId ) );
	
	return transferId;
}

QString ClassWatchMessageManager::sendFileToMultiple( const QStringList& recipientIds, const QString& filePath, const QString& senderId )
{
	QStringList transferIds;
	
	for ( const QString& recipientId : recipientIds ) {
		QString transferId = sendFile( recipientId, filePath, senderId );
		if ( !transferId.isEmpty() ) {
			transferIds.append( transferId );
		}
	}
	
	return transferIds.join( QStringLiteral(",") );
}

QString ClassWatchMessageManager::sendFileToAll( const QString& filePath, const QString& senderId )
{
	// This would send to all connected computers
	// For now, return a placeholder
	return sendFile( QStringLiteral("all"), filePath, senderId );
}

void ClassWatchMessageManager::cancelFileTransfer( const QString& transferId )
{
	if ( !m_fileTransfers.contains( transferId ) ) {
		return;
	}
	
	QJsonObject& transferObj = m_fileTransfers[transferId];
	transferObj[QStringLiteral("status")] = QStringLiteral("cancelled");
	logMessageEvent( transferId, QStringLiteral("file_transfer_cancelled"), QStringLiteral("File transfer cancelled") );
}

QJsonObject ClassWatchMessageManager::getFileTransferStatus( const QString& transferId ) const
{
	return m_fileTransfers.value( transferId );
}

QList<QJsonObject> ClassWatchMessageManager::getFileTransfers( const QString& recipientId ) const
{
	QList<QJsonObject> result;
	
	for ( auto it = m_fileTransfers.begin(); it != m_fileTransfers.end(); ++it ) {
		const QJsonObject& transferObj = it.value();
		QString transferRecipient = transferObj[QStringLiteral("recipientId")].toString();
		
		if ( recipientId.isEmpty() || transferRecipient == recipientId || transferRecipient == QStringLiteral("all") ) {
			result.append( transferObj );
		}
	}
	
	return result;
}

void ClassWatchMessageManager::setMaxMessageLength( int maxLength )
{
	m_maxMessageLength = maxLength;
}

int ClassWatchMessageManager::getMaxMessageLength() const
{
	return m_maxMessageLength;
}

void ClassWatchMessageManager::setMessageRetentionDays( int days )
{
	m_messageRetentionDays = days;
}

int ClassWatchMessageManager::getMessageRetentionDays() const
{
	return m_messageRetentionDays;
}

void ClassWatchMessageManager::setEnableReadReceipts( bool enabled )
{
	m_enableReadReceipts = enabled;
}

bool ClassWatchMessageManager::getEnableReadReceipts() const
{
	return m_enableReadReceipts;
}

void ClassWatchMessageManager::onMessageTimeout()
{
	// Check for messages that have been pending too long
	QDateTime now = QDateTime::currentDateTime();
	
	for ( auto it = m_messages.begin(); it != m_messages.end(); ++it ) {
		QJsonObject& msgObj = it.value();
		QString status = msgObj[QStringLiteral("status")].toString();
		
		if ( status == QStringLiteral("pending") || status == QStringLiteral("sent") ) {
			QDateTime timestamp = QDateTime::fromString( msgObj[QStringLiteral("timestamp")].toString(), Qt::ISODate );
			if ( timestamp.msecsTo( now ) > 30000 ) { // 30 seconds timeout
				msgObj[QStringLiteral("status")] = QStringLiteral("failed");
				msgObj[QStringLiteral("error")] = QStringLiteral("Message delivery timeout");
				logMessageEvent( it.key(), QStringLiteral("message_failed"), QStringLiteral("Delivery timeout") );
			}
		}
	}
}

void ClassWatchMessageManager::onCleanupTimer()
{
	cleanupOldMessages();
	cleanupOldTransfers();
}

QString ClassWatchMessageManager::generateMessageId()
{
	return QString( QStringLiteral("MSG-%1-%2") ).arg( QDateTime::currentDateTime().toString( QStringLiteral("yyyyMMdd-hhmmss") ) ).arg( ++m_messageCounter );
}

QString ClassWatchMessageManager::generateTransferId()
{
	return QString( QStringLiteral("FILE-%1-%2") ).arg( QDateTime::currentDateTime().toString( QStringLiteral("yyyyMMdd-hhmmss") ) ).arg( ++m_transferCounter );
}

QString ClassWatchMessageManager::generateChatId()
{
	return QString( QStringLiteral("CHAT-%1-%2") ).arg( QDateTime::currentDateTime().toString( QStringLiteral("yyyyMMdd-hhmmss") ) ).arg( ++m_chatCounter );
}

void ClassWatchMessageManager::processMessageQueue()
{
	// Process pending messages
	// In a real implementation, this would send messages through the Veyon feature system
	
	for ( const QString& messageId : m_messageQueue ) {
		if ( !m_messages.contains( messageId ) ) {
			continue;
		}
		
		QJsonObject& msgObj = m_messages[messageId];
		if ( msgObj[QStringLiteral("status")].toString() != QStringLiteral("pending") ) {
			continue;
		}
		
		// Simulate sending (in real implementation, this would use Veyon features)
		msgObj[QStringLiteral("status")] = QStringLiteral("sent");
		msgObj[QStringLiteral("sentTime")] = QDateTime::currentDateTime().toString( Qt::ISODate );
		
		Q_EMIT messageSent( messageId, msgObj[QStringLiteral("recipientId")].toString() );
		logMessageEvent( messageId, QStringLiteral("message_sent"), QStringLiteral("Message sent successfully") );
	}
	
	// Clear processed messages from queue
	m_messageQueue.clear();
}

void ClassWatchMessageManager::cleanupOldMessages()
{
	QDateTime cutoff = QDateTime::currentDateTime().addDays( -m_messageRetentionDays );
	QStringList messagesToRemove;
	
	for ( auto it = m_messages.begin(); it != m_messages.end(); ++it ) {
		QJsonObject msgObj = it.value();
		QDateTime timestamp = QDateTime::fromString( msgObj[QStringLiteral("timestamp")].toString(), Qt::ISODate );
		
		if ( timestamp < cutoff ) {
			messagesToRemove.append( it.key() );
		}
	}
	
	for ( const QString& messageId : messagesToRemove ) {
		m_messages.remove( messageId );
	}
}

void ClassWatchMessageManager::cleanupOldTransfers()
{
	QDateTime cutoff = QDateTime::currentDateTime().addDays( -7 ); // Keep transfers for 7 days
	QStringList transfersToRemove;
	
	for ( auto it = m_fileTransfers.begin(); it != m_fileTransfers.end(); ++it ) {
		QJsonObject transferObj = it.value();
		QDateTime timestamp = QDateTime::fromString( transferObj[QStringLiteral("timestamp")].toString(), Qt::ISODate );
		
		if ( timestamp < cutoff && transferObj[QStringLiteral("status")].toString() != QStringLiteral("pending") ) {
			transfersToRemove.append( it.key() );
		}
	}
	
	for ( const QString& transferId : transfersToRemove ) {
		m_fileTransfers.remove( transferId );
	}
}

void ClassWatchMessageManager::updateMessageStatus( const QString& messageId, MessageStatus status, const QString& details )
{
	if ( !m_messages.contains( messageId ) ) {
		return;
	}
	
	QJsonObject& msgObj = m_messages[messageId];
	QString statusStr;
	
	switch ( status ) {
	case MessageStatus::Pending: statusStr = QStringLiteral("pending"); break;
	case MessageStatus::Sent: statusStr = QStringLiteral("sent"); break;
	case MessageStatus::Delivered: statusStr = QStringLiteral("delivered"); break;
	case MessageStatus::Read: statusStr = QStringLiteral("read"); break;
	case MessageStatus::Failed: statusStr = QStringLiteral("failed"); break;
	}
	
	msgObj[QStringLiteral("status")] = statusStr;
	msgObj[QStringLiteral("statusTime")] = QDateTime::currentDateTime().toString( Qt::ISODate );
	
	if ( !details.isEmpty() ) {
		msgObj[QStringLiteral("details")] = details;
	}
	
	logMessageEvent( messageId, QString( QStringLiteral("message_%1") ).arg( statusStr ), details );
	
	switch ( status ) {
	case MessageStatus::Delivered:
		Q_EMIT messageDelivered( messageId );
		break;
	case MessageStatus::Read:
		Q_EMIT messageRead( messageId );
		break;
	case MessageStatus::Failed:
		Q_EMIT messageFailed( messageId, details );
		break;
	default:
		break;
	}
}

void ClassWatchMessageManager::logMessageEvent( const QString& messageId, const QString& eventType, const QString& details )
{
	QJsonObject event;
	event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString( Qt::ISODate );
	event[QStringLiteral("messageId")] = messageId;
	event[QStringLiteral("eventType")] = eventType;
	event[QStringLiteral("details")] = details;
	
	// In a real implementation, this would be stored in a database or file
	qDebug() << "Message Event:" << event;
}

bool ClassWatchMessageManager::validateMessage( const QString& message ) const
{
	return !message.isEmpty() && message.length() <= m_maxMessageLength;
}

bool ClassWatchMessageManager::validateFilePath( const QString& filePath ) const
{
	QFileInfo fileInfo( filePath );
	return fileInfo.exists() && fileInfo.isFile();
}

QString ClassWatchMessageManager::getDefaultSenderId() const
{
	// Return a default sender ID (e.g., teacher name or computer ID)
	return QStringLiteral("teacher");
}
