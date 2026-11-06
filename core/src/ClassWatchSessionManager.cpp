/*
 * ClassWatchSessionManager.cpp - implementation of ClassWatchSessionManager
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

#include "ClassWatchSessionManager.h"
#include "VeyonCore.h"
#include "VeyonConfiguration.h"
#include <QDateTime>
#include <QDebug>

ClassWatchSessionManager::ClassWatchSessionManager( QObject* parent ) :
	QObject( parent ),
	m_sessionCheckTimer( new QTimer( this ) ),
	m_sessionTimeoutTimer( new QTimer( this ) ),
	m_maxSessions( 100 ),
	m_sessionTimeout( 30000 ), // 30 seconds
	m_monitoring( false )
{
	// Setup session check timer
	connect( m_sessionCheckTimer, &QTimer::timeout, this, &ClassWatchSessionManager::onSessionCheckTimer );
	m_sessionCheckTimer->setInterval( 1000 ); // Check every second
	
	// Setup session timeout timer
	connect( m_sessionTimeoutTimer, &QTimer::timeout, this, &ClassWatchSessionManager::checkSessionTimeouts );
	m_sessionTimeoutTimer->setInterval( 5000 ); // Check every 5 seconds
	
	// Load configuration
	VeyonConfiguration& config = VeyonCore::config();
	m_maxSessions = config.maximumSessionCount();
	
	if ( config.enableMultiSessionSupport() ) {
		startSessionMonitoring();
	}
}

ClassWatchSessionManager::~ClassWatchSessionManager()
{
	stopSessionMonitoring();
}

void ClassWatchSessionManager::startSessionMonitoring()
{
	if ( m_monitoring ) {
		return;
	}
	
	m_monitoring = true;
	m_sessionCheckTimer->start();
	m_sessionTimeoutTimer->start();
	
	// Initial session detection
	detectSessions();
	
	qDebug() << "ClassWatch Session Manager: Started monitoring";
}

void ClassWatchSessionManager::stopSessionMonitoring()
{
	if ( !m_monitoring ) {
		return;
	}
	
	m_monitoring = false;
	m_sessionCheckTimer->stop();
	m_sessionTimeoutTimer->stop();
	
	qDebug() << "ClassWatch Session Manager: Stopped monitoring";
}

bool ClassWatchSessionManager::isMonitoring() const
{
	return m_monitoring;
}

QList<PlatformSessionFunctions::SessionInfo> ClassWatchSessionManager::getActiveSessions() const
{
	QList<PlatformSessionFunctions::SessionInfo> sessions;
	for ( auto it = m_sessionInfo.begin(); it != m_sessionInfo.end(); ++it ) {
		sessions.append( it.value() );
	}
	return sessions;
}

PlatformSessionFunctions::SessionInfo ClassWatchSessionManager::getSession( int sessionId ) const
{
	if ( m_sessionInfo.contains( sessionId ) ) {
		return m_sessionInfo[sessionId];
	}
	return PlatformSessionFunctions::SessionInfo{};
}

bool ClassWatchSessionManager::hasSession( int sessionId ) const
{
	return m_activeSessions.contains( sessionId );
}

int ClassWatchSessionManager::sessionCount() const
{
	return m_activeSessions.size();
}

QString ClassWatchSessionManager::getSessionType( int sessionId ) const
{
	if ( m_sessionTypes.contains( sessionId ) ) {
		return m_sessionTypes[sessionId];
	}
	return QStringLiteral("unknown");
}

bool ClassWatchSessionManager::isConsoleSession( int sessionId ) const
{
	return getSessionType( sessionId ) == QStringLiteral("console");
}

bool ClassWatchSessionManager::isRdpSession( int sessionId ) const
{
	return getSessionType( sessionId ) == QStringLiteral("rdp");
}

bool ClassWatchSessionManager::isActiveSession( int sessionId ) const
{
	return m_activeSessions.contains( sessionId );
}

void ClassWatchSessionManager::assignStudentToSession( int sessionId, const QString& studentId )
{
	if ( !validateSessionId( sessionId ) ) {
		return;
	}
	
	QString oldStudent = m_studentAssignments.value( sessionId );
	m_studentAssignments[sessionId] = studentId;
	
	QJsonObject event;
	event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString( Qt::ISODate );
	event[QStringLiteral("sessionId")] = sessionId;
	event[QStringLiteral("eventType")] = QStringLiteral("student_assigned");
	event[QStringLiteral("studentId")] = studentId;
	event[QStringLiteral("oldStudentId")] = oldStudent;
	m_sessionHistory.append( event );
	
	Q_EMIT studentAssigned( sessionId, studentId );
	Q_EMIT sessionEvent( sessionId, QStringLiteral("student_assigned"), QString( QStringLiteral("Student %1 assigned to session %2") ).arg( studentId ).arg( sessionId ) );
}

QString ClassWatchSessionManager::getStudentForSession( int sessionId ) const
{
	return m_studentAssignments.value( sessionId );
}

void ClassWatchSessionManager::removeStudentFromSession( int sessionId )
{
	if ( !m_studentAssignments.contains( sessionId ) ) {
		return;
	}
	
	QString studentId = m_studentAssignments[sessionId];
	m_studentAssignments.remove( sessionId );
	
	QJsonObject event;
	event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString( Qt::ISODate );
	event[QStringLiteral("sessionId")] = sessionId;
	event[QStringLiteral("eventType")] = QStringLiteral("student_removed");
	event[QStringLiteral("studentId")] = studentId;
	m_sessionHistory.append( event );
	
	Q_EMIT studentRemoved( sessionId );
	Q_EMIT sessionEvent( sessionId, QStringLiteral("student_removed"), QString( QStringLiteral("Student %1 removed from session %2") ).arg( studentId ).arg( sessionId ) );
}

QMap<int, QString> ClassWatchSessionManager::getAllStudentAssignments() const
{
	return m_studentAssignments;
}

void ClassWatchSessionManager::recordSessionEvent( int sessionId, const QString& eventType, const QString& details )
{
	QJsonObject event;
	event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString( Qt::ISODate );
	event[QStringLiteral("sessionId")] = sessionId;
	event[QStringLiteral("eventType")] = eventType;
	event[QStringLiteral("details")] = details;
	m_sessionHistory.append( event );
	
	// Keep only last 1000 events
	if ( m_sessionHistory.size() > 1000 ) {
		m_sessionHistory.removeFirst();
	}
	
	Q_EMIT sessionEvent( sessionId, eventType, details );
}

QList<QJsonObject> ClassWatchSessionManager::getSessionHistory( int sessionId, int limit ) const
{
	QList<QJsonObject> filteredHistory;
	
	for ( const QJsonObject& event : m_sessionHistory ) {
		if ( sessionId == -1 || event[QStringLiteral("sessionId")].toInt() == sessionId ) {
			filteredHistory.append( event );
			if ( filteredHistory.size() >= limit ) {
				break;
			}
		}
	}
	
	return filteredHistory;
}

void ClassWatchSessionManager::clearSessionHistory()
{
	m_sessionHistory.clear();
}

void ClassWatchSessionManager::setMaxSessions( int maxSessions )
{
	m_maxSessions = maxSessions;
}

int ClassWatchSessionManager::getMaxSessions() const
{
	return m_maxSessions;
}

void ClassWatchSessionManager::setSessionTimeout( int timeoutMs )
{
	m_sessionTimeout = timeoutMs;
}

int ClassWatchSessionManager::getSessionTimeout() const
{
	return m_sessionTimeout;
}

void ClassWatchSessionManager::onSessionCheckTimer()
{
	if ( !m_monitoring ) {
		return;
	}
	
	detectSessions();
	checkSessionTimeouts();
}

void ClassWatchSessionManager::onComputerControlStateChanged( ComputerControlInterface::State state )
{
	// This is called when a computer control interface changes state
	// We can use this to detect session changes
	Q_UNUSED( state );
}

void ClassWatchSessionManager::detectSessions()
{
	// This is a simplified implementation
	// In a real implementation, we'd query the Windows Terminal Services API
	// or use platform-specific session enumeration
	
	// For now, we'll use the current session info from VeyonCore
	PlatformSessionFunctions& sessionFunctions = VeyonCore::platform().sessionFunctions();
	
	int currentSessionId = sessionFunctions.currentSessionId();
	if ( currentSessionId != PlatformSessionFunctions::InvalidSessionId ) {
		PlatformSessionFunctions::SessionInfo info;
		info.id = currentSessionId;
		info.uptime = sessionFunctions.currentSessionUptime();
		info.clientAddress = sessionFunctions.currentSessionClientAddress();
		info.clientName = sessionFunctions.currentSessionClientName();
		info.hostName = sessionFunctions.currentSessionHostName();
		
		updateSessionInfo( currentSessionId, info );
		classifySession( currentSessionId );
	}
}

void ClassWatchSessionManager::updateSessionInfo( int sessionId, const PlatformSessionFunctions::SessionInfo& info )
{
	bool isNewSession = !m_activeSessions.contains( sessionId );
	
	if ( isNewSession ) {
		// Check session limit
		if ( m_activeSessions.size() >= m_maxSessions ) {
			Q_EMIT sessionLimitReached( m_maxSessions );
			return;
		}
		
		m_activeSessions.insert( sessionId );
		Q_EMIT sessionDetected( sessionId, info );
		
		QJsonObject event;
		event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString( Qt::ISODate );
		event[QStringLiteral("sessionId")] = sessionId;
		event[QStringLiteral("eventType")] = QStringLiteral("session_detected");
		event[QStringLiteral("sessionType")] = determineSessionType( sessionId );
		m_sessionHistory.append( event );
	}
	
	m_sessionInfo[sessionId] = info;
}

void ClassWatchSessionManager::classifySession( int sessionId )
{
	QString newType = determineSessionType( sessionId );
	QString oldType = m_sessionTypes.value( sessionId, QStringLiteral("unknown") );
	
	if ( newType != oldType ) {
		m_sessionTypes[sessionId] = newType;
		Q_EMIT sessionTypeChanged( sessionId, oldType, newType );
	}
}

void ClassWatchSessionManager::checkSessionTimeouts()
{
	// Check for sessions that haven't been updated recently
	QDateTime now = QDateTime::currentDateTime();
	QList<int> sessionsToRemove;
	
	for ( auto it = m_sessionInfo.begin(); it != m_sessionInfo.end(); ++it ) {
		int sessionId = it.key();
		// This is a simplified check - in real implementation,
		// we'd track last update time for each session
		Q_UNUSED( now );
		Q_UNUSED( sessionId );
	}
	
	for ( int sessionId : sessionsToRemove ) {
		m_activeSessions.remove( sessionId );
		m_sessionInfo.remove( sessionId );
		m_sessionTypes.remove( sessionId );
		m_studentAssignments.remove( sessionId );
		
		Q_EMIT sessionLost( sessionId );
		
		QJsonObject event;
		event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString( Qt::ISODate );
		event[QStringLiteral("sessionId")] = sessionId;
		event[QStringLiteral("eventType")] = QStringLiteral("session_lost");
		m_sessionHistory.append( event );
	}
}

QString ClassWatchSessionManager::determineSessionType( int sessionId ) const
{
	// This is a simplified implementation
	// In a real implementation, we'd use Windows Terminal Services API
	// to determine if it's console or RDP session
	
	PlatformSessionFunctions& sessionFunctions = VeyonCore::platform().sessionFunctions();
	QString sessionType = sessionFunctions.currentSessionType();
	
	if ( sessionType.contains( QStringLiteral("rdp"), Qt::CaseInsensitive ) ||
		 sessionType.contains( QStringLiteral("remote"), Qt::CaseInsensitive ) ) {
		return QStringLiteral("rdp");
	}
	
	if ( sessionType.contains( QStringLiteral("console"), Qt::CaseInsensitive ) ||
		 sessionType.isEmpty() ) {
		return QStringLiteral("console");
	}
	
	return QStringLiteral("unknown");
}

bool ClassWatchSessionManager::validateSessionId( int sessionId ) const
{
	return sessionId > 0 && m_activeSessions.contains( sessionId );
}
