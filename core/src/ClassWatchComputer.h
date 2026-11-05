/*
 * ClassWatchComputer.h - enhanced computer class for ClassWatch
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

#include "Computer.h"
#include <QDateTime>

// clazy:excludeall=rule-of-three

class VEYON_CORE_EXPORT ClassWatchComputer : public Computer
{
	Q_GADGET
public:
	explicit ClassWatchComputer( const Computer& computer = {} );

	// ClassWatch ID (CW-001, CW-002, etc.)
	void setClassWatchId( const QString& classWatchId )
	{
		m_classWatchId = classWatchId;
	}

	QString classWatchId() const
	{
		return m_classWatchId;
	}

	// Windows version information
	void setWindowsVersion( const QString& windowsVersion )
	{
		m_windowsVersion = windowsVersion;
	}

	QString windowsVersion() const
	{
		return m_windowsVersion;
	}

	// Logged-in username
	void setLoggedInUser( const QString& userName )
	{
		m_loggedInUser = userName;
	}

	QString loggedInUser() const
	{
		return m_loggedInUser;
	}

	// Domain name (for Windows Server environments)
	void setDomainName( const QString& domainName )
	{
		m_domainName = domainName;
	}

	QString domainName() const
	{
		return m_domainName;
	}

	// Connection status
	void setConnectionStatus( const QString& status )
	{
		m_connectionStatus = status;
	}

	QString connectionStatus() const
	{
		return m_connectionStatus;
	}

	// Last connection time
	void setLastConnectionTime( const QDateTime& time )
	{
		m_lastConnectionTime = time;
	}

	QDateTime lastConnectionTime() const
	{
		return m_lastConnectionTime;
	}

	// Session type (console, RDP, etc.)
	void setSessionType( const QString& sessionType )
	{
		m_sessionType = sessionType;
	}

	QString sessionType() const
	{
		return m_sessionType;
	}

	// Hardware information
	void setCpuInfo( const QString& cpuInfo )
	{
		m_cpuInfo = cpuInfo;
	}

	QString cpuInfo() const
	{
		return m_cpuInfo;
	}

	void setRamInfo( const QString& ramInfo )
	{
		m_ramInfo = ramInfo;
	}

	QString ramInfo() const
	{
		return m_ramInfo;
	}

	void setGpuInfo( const QString& gpuInfo )
	{
		m_gpuInfo = gpuInfo;
	}

	QString gpuInfo() const
	{
		return m_gpuInfo;
	}

private:
	QString m_classWatchId;
	QString m_windowsVersion;
	QString m_loggedInUser;
	QString m_domainName;
	QString m_connectionStatus;
	QDateTime m_lastConnectionTime;
	QString m_sessionType;
	QString m_cpuInfo;
	QString m_ramInfo;
	QString m_gpuInfo;
};

using ClassWatchComputerList = QVector<ClassWatchComputer>;
