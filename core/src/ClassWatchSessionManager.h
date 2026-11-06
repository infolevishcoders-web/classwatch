/*
 * ClassWatchSessionManager.h - manages Windows Server multi-session support
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

#include "PlatformSessionFunctions.h"
#include "ComputerControlInterface.h"
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QSet>

class VEYON_CORE_EXPORT ClassWatchSessionManager : public QObject
{
	Q_OBJECT
public:
	explicit ClassWatchSessionManager( QObject* parent = nullptr );
	~ClassWatchSessionManager() override;

	// Session enumeration
	void startSessionMonitoring();
	void stopSessionMonitoring();
	bool isMonitoring() const;

	// Session queries
	QList<PlatformSessionFunctions::SessionInfo> getActiveSessions() const;
	PlatformSessionFunctions::SessionInfo getSession( int sessionId ) const;
	bool hasSession( int sessionId ) const;
	int sessionCount() const;

	// Session classification
	QString getSessionType( int sessionId ) const; // "console", "rdp", "unknown"
	bool isConsoleSession( int sessionId ) const;
	bool isRdpSession( int sessionId ) const;
	bool isActiveSession( int sessionId ) const;

	// Student session management
	void assignStudentToSession( int sessionId, const QString& studentId );
	QString getStudentForSession( int sessionId ) const;
	void removeStudentFromSession( int sessionId );
	QMap<int, QString> getAllStudentAssignments() const;

	// Session history
	void recordSessionEvent( int sessionId, const QString& eventType, const QString& details = {} );
	QList<QJsonObject> getSessionHistory( int sessionId = -1, int limit = 100 ) const;
	void clearSessionHistory();

	// Configuration
	void setMaxSessions( int maxSessions );
	int getMaxSessions() const;
	void setSessionTimeout( int timeoutMs );
	int getSessionTimeout() const;

Q_SIGNALS:
	void sessionDetected( int sessionId, const PlatformSessionFunctions::SessionInfo& info );
	void sessionLost( int sessionId );
	void sessionTypeChanged( int sessionId, const QString& oldType, const QString& newType );
	void studentAssigned( int sessionId, const QString& studentId );
	void studentRemoved( int sessionId );
	void sessionEvent( int sessionId, const QString& eventType, const QString& details );
	void sessionLimitReached( int maxSessions );

private Q_SLOTS:
	void onSessionCheckTimer();
	void onComputerControlStateChanged( ComputerControlInterface::State state );

private:
	void detectSessions();
	void updateSessionInfo( int sessionId, const PlatformSessionFunctions::SessionInfo& info );
	void classifySession( int sessionId );
	void checkSessionTimeouts();
	QString determineSessionType( int sessionId ) const;
	bool validateSessionId( int sessionId ) const;

	QTimer* m_sessionCheckTimer;
	QTimer* m_sessionTimeoutTimer;
	QSet<int> m_activeSessions;
	QMap<int, PlatformSessionFunctions::SessionInfo> m_sessionInfo;
	QMap<int, QString> m_sessionTypes;
	QMap<int, QString> m_studentAssignments;
	QList<QJsonObject> m_sessionHistory;
	int m_maxSessions;
	int m_sessionTimeout;
	bool m_monitoring;
};
