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

#include <QAbstractSocket>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QSysInfo>
#include <QTextStream>
#include <QUuid>

#include <algorithm>

#include "ClassWatchConfigurationProperties.h"
#include "Filesystem.h"
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
	auto& manager = instance();
	if( config.securityAutoLockdown() )
	{
		manager.applyTerminalLockdown();
		manager.logSecurityEvent( QStringLiteral("lockdown_applied"),
								  QStringLiteral("Terminal lockdown policies applied at service start") );
	}
	// Web filtering and file hardening spawn netsh/icacls processes which can
	// take tens of seconds. Running them here would block the service before
	// it reports RUNNING to the Service Control Manager and Windows would kill
	// it with error 1053. Defer them until the event loop is up.
	QTimer::singleShot( 2000, &manager, [&manager]() {
		auto& deferredConfig = VeyonCore::config();
		if( deferredConfig.enableWebFiltering() )
		{
			if( manager.applyWebFilter() )
			{
				manager.logSecurityEvent( QStringLiteral("web_filter_applied"),
									   QStringLiteral("DNS-level web filtering enabled on all active interfaces") );
			}
		}
		else
		{
			// restores DHCP only on interfaces we previously filtered
			manager.removeWebFilter();
		}
		manager.hardenSecurityFiles();
		// per-domain blocking via hosts file - runs as SYSTEM so it can
		// write to the protected hosts file location
		if( deferredConfig.enableWebFiltering() )
		{
			manager.applyDomainFilter();
		}
		else
		{
			manager.removeDomainFilter();
		}
	} );
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

	Q_EMIT securityEventLogged( event );
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
	const auto path = VeyonCore::filesystem().expandPath(
		VeyonCore::config().auditLogPath() );
	QFile file( path );
	if( !file.open( QIODevice::Append | QIODevice::Text ) )
	{
		return;
	}
	file.write( QJsonDocument( event ).toJson( QJsonDocument::Compact ) + '\n' );
}


bool ClassWatchSecurityManager::runNetsh( const QStringList& arguments, int* exitCode ) const
{
#ifdef Q_OS_WIN
	QProcess netsh;
	netsh.start( QStringLiteral("netsh"), arguments );
	if( !netsh.waitForFinished( 15000 ) )
	{
		netsh.kill();
		return false;
	}
	if( exitCode )
	{
		*exitCode = netsh.exitCode();
	}
	return netsh.exitCode() == 0;
#else
	Q_UNUSED( arguments )
	if( exitCode )
	{
		*exitCode = -1;
	}
	return false;
#endif
}


QStringList ClassWatchSecurityManager::activeInterfaceNames() const
{
	QStringList names;
#ifdef Q_OS_WIN
	QProcess netsh;
	netsh.start( QStringLiteral("netsh"),
				 { QStringLiteral("interface"), QStringLiteral("ipv4"),
				   QStringLiteral("show"), QStringLiteral("interfaces") } );
	if( !netsh.waitForFinished( 15000 ) )
	{
		return names;
	}
	// lines look like: "  8   25   1500  connected     Ethernet"
	const QRegularExpression lineRe(
		QStringLiteral("^\\s*\\d+\\s+\\d+\\s+\\d+\\s+connected\\s+(.+?)\\s*$") );
	const auto lines = QString::fromLocal8Bit( netsh.readAllStandardOutput() )
						  .split( QLatin1Char('\n') );
	for( const auto& line : lines )
	{
		const auto match = lineRe.match( line );
		if( match.hasMatch() )
		{
			names.append( match.captured( 1 ) );
		}
	}
#endif
	return names;
}


QString ClassWatchSecurityManager::webFilterMarkerPath() const
{
	return securityDataDirectory() + QDir::separator() +
		   QStringLiteral("webfilter.active.json");
}


bool ClassWatchSecurityManager::applyWebFilter()
{
#ifdef Q_OS_WIN
	// DNS servers from configuration (comma/space separated); default is the
	// CleanBrowsing family filter which blocks adult, malware and phishing
	// content while keeping educational sites reachable.
	const auto configured = VeyonCore::config().webFilterDnsServers();
	QStringList servers = configured.split( QRegularExpression( QStringLiteral("[,;\\s]+") ),
											Qt::SkipEmptyParts );
	servers.erase( std::remove_if( servers.begin(), servers.end(),
								   []( const QString& s ) {
									   return QHostAddress( s ).protocol() != QAbstractSocket::IPv4Protocol;
								   } ),
				   servers.end() );
	if( servers.isEmpty() )
	{
		return false;
	}
	servers = servers.mid( 0, 2 );

	const auto interfaces = activeInterfaceNames();
	if( interfaces.isEmpty() )
	{
		return false;
	}

	for( const auto& name : interfaces )
	{
		if( !runNetsh( { QStringLiteral("interface"), QStringLiteral("ipv4"),
						 QStringLiteral("set"), QStringLiteral("dnsservers"),
						 name, QStringLiteral("static"), servers.first(),
						 QStringLiteral("primary") } ) )
		{
			return false;
		}
		if( servers.size() > 1 )
		{
			runNetsh( { QStringLiteral("interface"), QStringLiteral("ipv4"),
						QStringLiteral("add"), QStringLiteral("dnsservers"),
						name, servers.at( 1 ), QStringLiteral("index=2") } );
		}
	}

	// remember which interfaces we filtered so removal restores DHCP exactly there
	QDir().mkpath( securityDataDirectory() );
	QJsonObject marker;
	marker[QStringLiteral("interfaces")] = QJsonArray::fromStringList( interfaces );
	marker[QStringLiteral("dns")] = QJsonArray::fromStringList( servers );
	QFile file( webFilterMarkerPath() );
	if( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
	{
		file.write( QJsonDocument( marker ).toJson( QJsonDocument::Compact ) );
	}
	return true;
#else
	return false;
#endif
}


bool ClassWatchSecurityManager::removeWebFilter()
{
#ifdef Q_OS_WIN
	QFile file( webFilterMarkerPath() );
	if( !file.open( QIODevice::ReadOnly ) )
	{
		return false; // we never applied a filter - leave DNS alone
	}
	const auto doc = QJsonDocument::fromJson( file.readAll() );
	file.close();
	const auto interfaces = doc.object()
							.value( QStringLiteral("interfaces") )
							.toArray().toVariantList();

	for( const auto& entry : interfaces )
	{
		runNetsh( { QStringLiteral("interface"), QStringLiteral("ipv4"),
					QStringLiteral("set"), QStringLiteral("dnsservers"),
					entry.toString(), QStringLiteral("dhcp") } );
	}
	QFile::remove( webFilterMarkerPath() );
	return true;
#else
	return false;
#endif
}


bool ClassWatchSecurityManager::isWebFilterActive() const
{
	return QFile::exists( webFilterMarkerPath() );
}


QString ClassWatchSecurityManager::hostsFilePath()
{
#ifdef Q_OS_WIN
	return QStringLiteral("C:/Windows/System32/drivers/etc/hosts");
#else
	return QStringLiteral("/etc/hosts");
#endif
}


QStringList ClassWatchSecurityManager::parseDomainList( const QString& raw )
{
	QStringList result;
	const auto parts = raw.split( QRegularExpression( QStringLiteral("[,;\\s]+") ),
								   Qt::SkipEmptyParts );
	for( auto part : parts )
	{
		part = part.trimmed().toLower();
		// strip scheme, path, port and leading wildcard
		part.remove( QRegularExpression( QStringLiteral("^[a-z]+://") ) );
		part = part.section( QLatin1Char('/'), 0, 0 );
		part = part.section( QLatin1Char(':'), 0, 0 );
		while( part.startsWith( QLatin1Char('*') ) || part.startsWith( QLatin1Char('.') ) )
		{
			part.remove( 0, 1 );
		}
		if( !part.isEmpty() && part.contains( QLatin1Char('.') ) &&
			QRegularExpression( QStringLiteral("^[a-z0-9.-]+$") ).match( part ).hasMatch() )
		{
			result.append( part );
		}
	}
	result.removeDuplicates();
	return result;
}


bool ClassWatchSecurityManager::domainIsAllowed( const QString& domain,
												 const QStringList& allowed )
{
	for( const auto& entry : allowed )
	{
		if( domain == entry || domain.endsWith( QLatin1Char('.') + entry ) )
		{
			return true;
		}
	}
	return false;
}


QStringList ClassWatchSecurityManager::expandDomainsWithSubdomains( const QStringList& domains )
{
	QStringList expanded = domains;
	static const QStringList prefixes = {
		QStringLiteral("www."), QStringLiteral("m."),
		QStringLiteral("mobile."), QStringLiteral("web.")
	};
	for( const auto& domain : domains )
	{
		for( const auto& prefix : prefixes )
		{
			expanded.append( prefix + domain );
		}
	}
	expanded.removeDuplicates();
	return expanded;
}


bool ClassWatchSecurityManager::applyDomainFilter()
{
	const auto blocked = parseDomainList( VeyonCore::config().webFilterBlockedDomains() );
	const auto allowed = parseDomainList( VeyonCore::config().webFilterAllowedDomains() );

	QStringList effective;
	for( const auto& domain : expandDomainsWithSubdomains( blocked ) )
	{
		// skip when the domain or any parent is on the allow list
		if( !domainIsAllowed( domain, allowed ) )
		{
			effective.append( domain );
		}
	}
	effective.removeDuplicates();

	const auto path = hostsFilePath();
	QFile file( path );
	if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		return false;
	}
	QString content = QString::fromUtf8( file.readAll() );
	file.close();

	// strip any previous ClassWatch block so re-applying is idempotent
	const QRegularExpression blockRe(
		QStringLiteral("(?s)[ \\t]*# CLASSWATCH-BLOCK-START.*?# CLASSWATCH-BLOCK-END[^\\n]*\\n?") );
	content.remove( blockRe );

	if( effective.isEmpty() )
	{
		// nothing to block - still write the file back without the block
		if( !file.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text ) )
		{
			return false;
		}
		file.write( content.toUtf8() );
		return true;
	}

	if( !content.endsWith( QLatin1Char('\n') ) && !content.isEmpty() )
	{
		content += QLatin1Char('\n');
	}
	content += QStringLiteral("# CLASSWATCH-BLOCK-START - managed by ClassWatch, do not edit\n");
	for( const auto& domain : effective )
	{
		content += QStringLiteral("0.0.0.0 %1\n127.0.0.1 %1\n").arg( domain );
	}
	content += QStringLiteral("# CLASSWatch blocked %1 domain(s) - admin can grant access via WebFilterAllowedDomains\n")
				   .arg( effective.size() );
	content += QStringLiteral("# CLASSWATCH-BLOCK-END\n");

	if( !file.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text ) )
	{
		return false;
	}
	file.write( content.toUtf8() );
	return true;
}


bool ClassWatchSecurityManager::removeDomainFilter()
{
	const auto path = hostsFilePath();
	QFile file( path );
	if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		return false;
	}
	QString content = QString::fromUtf8( file.readAll() );
	file.close();

	const QRegularExpression blockRe(
		QStringLiteral("(?s)[ \\t]*# CLASSWATCH-BLOCK-START.*?# CLASSWATCH-BLOCK-END[^\\n]*\\n?") );
	if( !content.contains( blockRe ) )
	{
		return false;
	}
	content.remove( blockRe );

	if( !file.open( QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text ) )
	{
		return false;
	}
	file.write( content.toUtf8() );
	return true;
}


bool ClassWatchSecurityManager::isDomainFilterActive() const
{
	QFile file( hostsFilePath() );
	if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		return false;
	}
	return QString::fromUtf8( file.readAll() ).contains(
		QStringLiteral("# CLASSWATCH-BLOCK-START") );
}


void ClassWatchSecurityManager::hardenSecurityFiles()
{
#ifdef Q_OS_WIN
	QDir().mkpath( securityDataDirectory() );
	// Restrict the security data directory to SYSTEM and Administrators so
	// students cannot read the event log, forge warnings or tamper with the
	// pending-warning file.
	const auto dir = QDir::toNativeSeparators( securityDataDirectory() );
	QProcess::execute( QStringLiteral("icacls"),
					   { dir, QStringLiteral("/inheritance:r"),
						 QStringLiteral("/grant:r"),
						 QStringLiteral("NT AUTHORITY\\SYSTEM:(OI)(CI)F"),
						 QStringLiteral("BUILTIN\\Administrators:(OI)(CI)F"),
						 QStringLiteral("/T"), QStringLiteral("/C") } );
#endif
}
