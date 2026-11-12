/*
 * ClassWatchClientInfo.h - stable per-machine identity and real service
 * status helpers for the ClassWatch client
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

#include "VeyonCore.h"
#include "VeyonConfiguration.h"
#include "VeyonServiceControl.h"

#include <QHostInfo>
#include <QRegularExpression>

namespace ClassWatchClientInfo
{
	// Stable label for this machine. The authoritative ClassWatch ID
	// (e.g. CW-007) is assigned by the Master at registration time and is
	// not visible to the client, so we derive a deterministic label from the
	// hostname: it survives reboots and IP address changes.
	inline QString machineLabel()
	{
		QString host = QHostInfo::localHostName().toUpper();
		host.remove( QRegularExpression( QStringLiteral("[^A-Z0-9]") ) );
		if ( host.isEmpty() ) {
			host = QStringLiteral("PC");
		}
		if ( host.length() > 12 ) {
			host = host.left( 12 );
		}

		QString prefix = QStringLiteral("CW");
		if( VeyonCore::instance() )
		{
			prefix = VeyonCore::config().classWatchIdPrefix().trimmed();
			prefix.remove( QLatin1Char('-') );
		}
		if( prefix.isEmpty() )
		{
			prefix = QStringLiteral("CW");
		}
		return QStringLiteral("%1-%2").arg( prefix, host );
	}

	// Real status: whether the ClassWatch service/worker is installed and
	// actually running on this machine.
	inline QString connectionStatus()
	{
		VeyonServiceControl serviceControl;
		if ( !serviceControl.isServiceRegistered() ) {
			return QStringLiteral("Service Not Installed");
		}
		return serviceControl.isServiceRunning()
				? QStringLiteral("Monitoring Active")
				: QStringLiteral("Service Stopped");
	}
}
