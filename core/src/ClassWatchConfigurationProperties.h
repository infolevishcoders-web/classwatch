/*
 * ClassWatchConfigurationProperties.h - ClassWatch-specific configuration properties
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

#include <QDir>

#define FOREACH_CLASSWATCH_CONFIG_PROPERTIES(OP) \
	OP( VeyonConfiguration, VeyonCore::config(), QString, schoolName, setSchoolName, "SchoolName", "ClassWatch", QString(), Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), QString, laboratoryName, setLaboratoryName, "LaboratoryName", "ClassWatch", QString(), Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), QString, classWatchIdPrefix, setClassWatchIdPrefix, "ClassWatchIdPrefix", "ClassWatch", QStringLiteral("CW"), Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), int, classWatchIdStartNumber, setClassWatchIdStartNumber, "ClassWatchIdStartNumber", "ClassWatch", 1, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), QString, classWatchRegistryPath, setClassWatchRegistryPath, "ClassWatchRegistryPath", "ClassWatch", QDir::toNativeSeparators(QStringLiteral("%APPDATA%/ClassWatch/classwatch_computers.json")), Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), bool, autoRegisterComputers, setAutoRegisterComputers, "AutoRegisterComputers", "ClassWatch", true, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), bool, showClassWatchIdsInMonitoring, setShowClassWatchIdsInMonitoring, "ShowClassWatchIdsInMonitoring", "ClassWatch", true, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), QString, defaultStudentFolder, setDefaultStudentFolder, "DefaultStudentFolder", "ClassWatch", QDir::toNativeSeparators(QStringLiteral("%USERPROFILE%/ClassWatch")), Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), bool, enableStudentNotifications, setEnableStudentNotifications, "EnableStudentNotifications", "ClassWatch", true, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), bool, enableFileDistribution, setEnableFileDistribution, "EnableFileDistribution", "ClassWatch", true, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), bool, enableAudioClassroom, setEnableAudioClassroom, "EnableAudioClassroom", "ClassWatch", false, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), bool, enableWebFiltering, setEnableWebFiltering, "EnableWebFiltering", "ClassWatch", true, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), QString, webFilterDnsServers, setWebFilterDnsServers, "WebFilterDnsServers", "ClassWatch", QStringLiteral("185.228.168.168,185.228.169.168"), Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), QString, webFilterBlockedDomains, setWebFilterBlockedDomains, "WebFilterBlockedDomains", "ClassWatch", QStringLiteral("tiktok.com,www.tiktok.com,vm.tiktok.com,m.tiktok.com,tiktokv.com,likee.video,kwai.com,snapchat.com,chatgpt.com,chat.openai.com,claude.ai,character.ai,c.ai,grok.com,x.ai,perplexity.ai"), Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), QString, webFilterAllowedDomains, setWebFilterAllowedDomains, "WebFilterAllowedDomains", "ClassWatch", QString(), Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), bool, enableTimetable, setEnableTimetable, "EnableTimetable", "ClassWatch", false, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), int, dashboardRefreshInterval, setDashboardRefreshInterval, "DashboardRefreshInterval", "ClassWatch", 5000, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), bool, enableAuditLogging, setEnableAuditLogging, "EnableAuditLogging", "ClassWatch", true, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), QString, auditLogPath, setAuditLogPath, "AuditLogPath", "ClassWatch", QDir::toNativeSeparators(QStringLiteral("%APPDATA%/ClassWatch/audit.log")), Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), bool, enableMultiSessionSupport, setEnableMultiSessionSupport, "EnableMultiSessionSupport", "ClassWatch", false, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), QString, windowsServerSessionPolicy, setWindowsServerSessionPolicy, "WindowsServerSessionPolicy", "ClassWatch", QString(), Configuration::Property::Flag::Advanced ) \
	OP( VeyonConfiguration, VeyonCore::config(), bool, securityAutoLockdown, setSecurityAutoLockdown, "SecurityAutoLockdown", "Security", true, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), QString, securityAdminPasswordHash, setSecurityAdminPasswordHash, "SecurityAdminPasswordHash", "Security", QString(), Configuration::Property::Flag::Advanced ) \
	OP( VeyonConfiguration, VeyonCore::config(), bool, securityConfiguratorLocked, setSecurityConfiguratorLocked, "SecurityConfiguratorLocked", "Security", false, Configuration::Property::Flag::Standard ) \
	OP( VeyonConfiguration, VeyonCore::config(), bool, clientShowTrayIcon, setClientShowTrayIcon, "ClientShowTrayIcon", "Client", false, Configuration::Property::Flag::Standard )
