/*
 * ClassWatchSecurityManager.h - classroom lockdown, configurator lock and
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

#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>

#include "VeyonCore.h"

class VEYON_CORE_EXPORT ClassWatchSecurityManager : public QObject
{
	Q_OBJECT

public:
	explicit ClassWatchSecurityManager( QObject* parent = nullptr );

	// called once by the service at startup - applies lockdown automatically
	static void initialize();
	static ClassWatchSecurityManager& instance();

	// terminal lockdown: disables cmd.exe, PowerShell and Task Manager
	// for all users via Group Policy registry keys (Windows only)
	bool applyTerminalLockdown();
	bool removeTerminalLockdown();
	bool isTerminalLockdownActive() const;

	// administrator override password (SHA-256 hashed, never stored plain)
	void setAdminPassword( const QString& password );
	bool verifyAdminPassword( const QString& password ) const;
	bool hasAdminPassword() const;

	// configurator lock: once set, the Configurator demands the admin
	// password before allowing changes
	bool isConfiguratorLocked() const;
	void setConfiguratorLocked( bool locked );

	// security event reporting - JSON-lines log with local and UTC
	// timestamps; also mirrored into the ClassWatch audit log
	void logSecurityEvent( const QString& kind, const QString& detail,
						   const QString& source = QString() );
	QList<QJsonObject> recentSecurityEvents( int maxEntries = 100 ) const;

	// device-side warning shown on the student machine (picked up by the
	// client tray application which polls the pending-warning file)
	void raiseDeviceWarning( const QString& title, const QString& message );
	bool hasPendingDeviceWarning() const;
	QJsonObject takePendingDeviceWarning();

	// web filtering: forces DNS-level content filtering on all active
	// network interfaces (Windows only, CleanBrowsing family filter by
	// default). Reversible - restores DHCP when disabled.
	bool applyWebFilter();
	bool removeWebFilter();
	bool isWebFilterActive() const;

	// per-domain blocking via the hosts file: suspicious domains (social
	// media, AI chat agents, etc.) resolve to 0.0.0.0 so they cannot be
	// reached. Domains in the allowed list are never blocked - teachers
	// can grant access per site. Runs on Windows and Linux.
	bool applyDomainFilter();
	bool removeDomainFilter();
	bool isDomainFilterActive() const;

	// filesystem locations
	static QString securityDataDirectory();
	static QString securityEventsLogPath();

Q_SIGNALS:
	void securityEventLogged( const QJsonObject& event );

private:
	QString pendingWarningPath() const;
	void appendAuditMirror( const QJsonObject& event ) const;
	bool runNetsh( const QStringList& arguments, int* exitCode = nullptr ) const;
	QStringList activeInterfaceNames() const;
	QString webFilterMarkerPath() const;
	static QString hostsFilePath();
	static QStringList parseDomainList( const QString& raw );
	static bool domainIsAllowed( const QString& domain, const QStringList& allowed );
	static QStringList expandDomainsWithSubdomains( const QStringList& domains );
	void hardenSecurityFiles();
};
