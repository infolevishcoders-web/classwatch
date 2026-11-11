/*
 * ClassWatchSecurityManager.cpp - classroom lockdown, configurator lock and
 * security event reporting for ClassWatch
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

#include "ClassWatchSecurityManager.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSettings>
#include <QStandardPaths>
#include <QSysInfo>
#include <QTextStream>
#include <QUuid>

#include "ClassWatchConfigurationProperties.h"
#include "VeyonCore.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace
{
	constexpr auto kSystemPolicyPath = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\System";
	constexpr auto kDisableCMD = "DisableCMD";
	constexpr auto kDisableTaskMgr = "DisableTaskMgr";

	QString hashedPassword( const QString& password )
	{
		return QString::fromLatin1(
			QCryptographicHash::hash( password.toUtf8(),
									  QCryptographicHash::Sha256 ).toHex() );
	}
}


ClassWatchSecurityManager& ClassWatchSecurityManager::instance()
{
	static ClassWatchSecurityManager manager;
	return manager;
}


void ClassWatchSecurityManager::initialize()
{
	auto& config = VeyonCore::config();
	if( config.securityAutoLockdown() )
	{
		instance().applyTerminalLockdown();
		instance().logSecurityEvent( QStringLiteral("lockdown_applied"),
									  QStringLiteral("Terminal lockdown policies applied at service start") );
	}
}


ClassWatchSecurityManager::ClassWatchSecurityManager( QObject* parent ) :
	QObject( parent )
{
}


QString ClassWatchSecurityManager::securityDataDirectory()
{
#ifdef Q_OS_WIN
	return QDir::toNativeSeparators( QStringLiteral("C:/ProgramData/ClassWatch") );
#else
	return QDir::toNativeSeparators(
		QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) );
#endif
}


QString ClassWatchSecurityManager::securityEventsLogPath()
{
	return securityDataDirectory() + QDir::separator() +
		   QStringLiteral("security-events.log");
}


bool ClassWatchSecurityManager::applyTerminalLockdown()
{
#ifdef Q_OS_WIN
	QSettings policies( QString::fromLatin1( kSystemPolicyPath ),
						QSettings::NativeFormat );
	policies.setValue( QString::fromLatin1( kDisableCMD ), 2 ); // cmd + PowerShell
	policies.setValue( QString::fromLatin1( kDisableTaskMgr ), 1 );
	policies.sync();
	return policies.status() == QSettings::NoError;
#else
	return true; // non-Windows: lockdown is a no-op
#endif
}


bool ClassWatchSecurityManager::removeTerminalLockdown()
{
#ifdef Q_OS_WIN
	QSettings policies( QString::fromLatin1( kSystemPolicyPath ),
						QSettings::NativeFormat );
	policies.remove( QString::fromLatin1( kDisableCMD ) );
	policies.remove( QString::fromLatin1( kDisableTaskMgr ) );
	policies.sync();
	return policies.status() == QSettings::NoError;
#else
	return true;
#endif
}


bool ClassWatchSecurityManager::isTerminalLockdownActive() const
{
#ifdef Q_OS_WIN
	QSettings policies( QString::fromLatin1( kSystemPolicyPath ),
						QSettings::NativeFormat );
	return policies.value( QString::fromLatin1( kDisableCMD ) ).toInt() != 0 ||
		   policies.value( QString::fromLatin1( kDisableTaskMgr ) ).toInt() != 0;
#else
	return VeyonCore::config().securityAutoLockdown();
#endif
}


void ClassWatchSecurityManager::setAdminPassword( const QString& password )
{
	VeyonCore::config().setSecurityAdminPasswordHash(
		password.isEmpty() ? QString() : hashedPassword( password ) );
}


bool ClassWatchSecurityManager::verifyAdminPassword( const QString& password ) const
{
	const auto stored = VeyonCore::config().securityAdminPasswordHash();
	return !stored.isEmpty() && stored == hashedPassword( password );
}


bool ClassWatchSecurityManager::hasAdminPassword() const
{
	return !VeyonCore::config().securityAdminPasswordHash().isEmpty();
}


bool ClassWatchSecurityManager::isConfiguratorLocked() const
{
	return VeyonCore::config().securityConfiguratorLocked() && hasAdminPassword();
}


void ClassWatchSecurityManager::setConfiguratorLocked( bool locked )
{
	VeyonCore::config().setSecurityConfiguratorLocked( locked );
	logSecurityEvent( locked ? QStringLiteral("configurator_locked")
							 : QStringLiteral("configurator_unlocked"),
					  QStringLiteral("Configurator lock state changed") );
}


void ClassWatchSecurityManager::logSecurityEvent( const QString& kind,
												  const QString& detail,
												  const QString& source )
{
	QJsonObject event;
	event[QStringLiteral("id")] = QUuid::createUuid().toString( QUuid::WithoutBraces );
	event[QStringLiteral("kind")] = kind;
	event[QStringLiteral("detail")] = detail;
	event[QStringLiteral("source")] = source.isEmpty()
		? QSysInfo::machineHostName() : source;
	event[QStringLiteral("localTime")] =
		QDateTime::currentDateTime().toString( Qt::ISODateWithMs );
	event[QStringLiteral("utcTime")] =
		QDateTime::currentDateTimeUtc().toString( Qt::ISODateWithMs );

	QDir().mkpath( securityDataDirectory() );
	QFile log( securityEventsLogPath() );
	if( log.open( QIODevice::Append | QIODevice::Text ) )
	{
		log.write( QJsonDocument( event ).toJson( QJsonDocument::Compact ) + '\n' );
	}

	if( VeyonCore::config().enableAuditLogging() )
	{
		appendAuditMirror( event );
	}

	emit securityEventLogged( event );
}


QList<QJsonObject> ClassWatchSecurityManager::recentSecurityEvents( int maxEntries ) const
{
	QList<QJsonObject> events;
	QFile log( securityEventsLogPath() );
	if( !log.open( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		return events;
	}
	const auto lines = QString::fromUtf8( log.readAll() ).split( QLatin1Char('\n') );
	for( int i = lines.size() - 1; i >= 0 && events.size() < maxEntries; --i )
	{
		const auto line = lines.at( i ).trimmed();
		if( line.isEmpty() )
		{
			continue;
		}
		QJsonParseError error;
		const auto doc = QJsonDocument::fromJson( line.toUtf8(), &error );
		if( error.error == QJsonParseError::NoError && doc.isObject() )
		{
			events.prepend( doc.object() );
		}
	}
	return events;
}


void ClassWatchSecurityManager::raiseDeviceWarning( const QString& title,
													const QString& message )
{
	QDir().mkpath( securityDataDirectory() );
	QJsonObject warning;
	warning[QStringLiteral("title")] = title;
	warning[QStringLiteral("message")] = message;
	warning[QStringLiteral("localTime")] =
		QDateTime::currentDateTime().toString( Qt::ISODateWithMs );
	warning[QStringLiteral("utcTime")] =
		QDateTime::currentDateTimeUtc().toString( Qt::ISODateWithMs );

	QFile file( pendingWarningPath() );
	if( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
	{
		file.write( QJsonDocument( warning ).toJson( QJsonDocument::Compact ) );
	}
	logSecurityEvent( QStringLiteral("device_warning"),
					  QStringLiteral("%1 - %2").arg( title, message ) );
}


bool ClassWatchSecurityManager::hasPendingDeviceWarning() const
{
	return QFile::exists( pendingWarningPath() );
}


QJsonObject ClassWatchSecurityManager::takePendingDeviceWarning()
{
	QJsonObject warning;
	QFile file( pendingWarningPath() );
	if( !file.open( QIODevice::ReadOnly ) )
	{
		return warning;
	}
	const auto doc = QJsonDocument::fromJson( file.readAll() );
	file.close();
	QFile::remove( pendingWarningPath() );
	return doc.object();
}


QString ClassWatchSecurityManager::pendingWarningPath() const
{
	return securityDataDirectory() + QDir::separator() +
		   QStringLiteral("pending-warning.json");
}


void ClassWatchSecurityManager::appendAuditMirror( const QJsonObject& event ) const
{
	const auto path = VeyonCore::config().auditLogPath();
	QFile file( path );
	if( !file.open( QIODevice::Append | QIODevice::Text ) )
	{
		return;
	}
	file.write( QJsonDocument( event ).toJson( QJsonDocument::Compact ) + '\n' );
}
