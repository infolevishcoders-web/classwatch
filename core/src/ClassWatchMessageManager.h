/*
 * ClassWatchMessageManager.h - manages ClassWatch messaging and communication
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

#pragma once

#include "ComputerControlInterface.h"
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>

class ClassWatchMessageManager : public QObject
{
	Q_OBJECT
public:
	explicit ClassWatchMessageManager( QObject* parent = nullptr );
	~ClassWatchMessageManager() override;

	// Message types
	enum class MessageType {
		Private,      // One-to-one message
		Announcement, // Class-wide announcement
		Broadcast,    // All connected computers
		System        // System notification
	};
	Q_ENUM(MessageType)

	// Message status
	enum class MessageStatus {
		Pending,
		Sent,
		Delivered,
		Read,
		Failed
	};
	Q_ENUM(MessageStatus)

	// Send messages
	QString sendPrivateMessage( const QString& recipientId, const QString& message, const QString& senderId = {} );
	QString sendAnnouncement( const QString& message, const QString& senderId = {} );
	QString sendBroadcast( const QString& message, const QString& senderId = {} );
	QString sendSystemMessage( const QString& message, const QString& targetId = {} );

	// Message management
	void cancelMessage( const QString& messageId );
	void deleteMessage( const QString& messageId );
	QJsonObject getMessage( const QString& messageId ) const;
	QList<QJsonObject> getMessages( const QString& recipientId = {}, int limit = 100 ) const;
	QList<QJsonObject> getPendingMessages() const;
	QList<QJsonObject> getFailedMessages() const;

	// Message status tracking
	void markMessageAsDelivered( const QString& messageId );
	void markMessageAsRead( const QString& messageId );
	void markMessageAsFailed( const QString& messageId, const QString& error = {} );

	// Chat management
	QString createChatRoom( const QStringList& participants );
	void addParticipantToChat( const QString& chatId, const QString& participantId );
	void removeParticipantFromChat( const QString& chatId, const QString& participantId );
	QList<QJsonObject> getChatHistory( const QString& chatId, int limit = 50 ) const;
	QStringList getChatParticipants( const QString& chatId ) const;

	// File distribution
	QString sendFile( const QString& recipientId, const QString& filePath, const QString& senderId = {} );
	QString sendFileToMultiple( const QStringList& recipientIds, const QString& filePath, const QString& senderId = {} );
	QString sendFileToAll( const QString& filePath, const QString& senderId = {} );
	void cancelFileTransfer( const QString& transferId );
	QJsonObject getFileTransferStatus( const QString& transferId ) const;
	QList<QJsonObject> getFileTransfers( const QString& recipientId = {} ) const;

	// Configuration
	void setMaxMessageLength( int maxLength );
	int getMaxMessageLength() const;
	void setMessageRetentionDays( int days );
	int getMessageRetentionDays() const;
	void setEnableReadReceipts( bool enabled );
	bool getEnableReadReceipts() const;

Q_SIGNALS:
	void messageSent( const QString& messageId, const QString& recipientId );
	void messageDelivered( const QString& messageId );
	void messageRead( const QString& messageId );
	void messageFailed( const QString& messageId, const QString& error );
	void messageReceived( const QString& messageId, const QString& senderId, const QString& message );
	void fileTransferStarted( const QString& transferId, const QString& filePath );
	void fileTransferProgress( const QString& transferId, int progress );
	void fileTransferCompleted( const QString& transferId );
	void fileTransferFailed( const QString& transferId, const QString& error );
	void chatMessageReceived( const QString& chatId, const QString& senderId, const QString& message );

private Q_SLOTS:
	void onMessageTimeout();
	void onCleanupTimer();

private:
	QString generateMessageId();
	QString generateTransferId();
	QString generateChatId();
	void processMessageQueue();
	void cleanupOldMessages();
	void cleanupOldTransfers();
	void updateMessageStatus( const QString& messageId, MessageStatus status, const QString& details = {} );
	void logMessageEvent( const QString& messageId, const QString& eventType, const QString& details = {} );
	bool validateMessage( const QString& message ) const;
	bool validateFilePath( const QString& filePath ) const;
	QString getDefaultSenderId() const;

	QTimer* m_messageTimeoutTimer;
	QTimer* m_cleanupTimer;
	QMap<QString, QJsonObject> m_messages;
	QMap<QString, QJsonObject> m_fileTransfers;
	QMap<QString, QJsonObject> m_chatRooms;
	QMap<QString, QList<QJsonObject>> m_chatHistory;
	QStringList m_messageQueue;
	int m_maxMessageLength;
	int m_messageRetentionDays;
	bool m_enableReadReceipts;
	int m_messageCounter;
	int m_transferCounter;
	int m_chatCounter;
};
