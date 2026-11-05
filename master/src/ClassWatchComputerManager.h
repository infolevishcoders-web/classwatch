/*
 * ClassWatchComputerManager.h - manages ClassWatch computer identification and tracking
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

#include "ClassWatchComputer.h"
#include "ClassWatchIdGenerator.h"
#include <QMap>
#include <QObject>

class ClassWatchComputerManager : public QObject
{
	Q_OBJECT
public:
	explicit ClassWatchComputerManager( QObject* parent = nullptr );
	~ClassWatchComputerManager() override;

	// Computer registration and identification
	QString registerComputer( const Computer& computer );
	void unregisterComputer( const QString& classWatchId );
	ClassWatchComputer getComputer( const QString& classWatchId ) const;
	ClassWatchComputer getComputerByNetworkObjectUid( NetworkObject::Uid uid ) const;

	// Update computer information
	void updateComputerInfo( const QString& classWatchId, const ClassWatchComputer& computer );
	void updateConnectionStatus( const QString& classWatchId, const QString& status );
	void updateLoggedInUser( const QString& classWatchId, const QString& userName );
	void updateLastConnectionTime( const QString& classWatchId );

	// Query computers
	ClassWatchComputerList getAllComputers() const;
	ClassWatchComputerList getConnectedComputers() const;
	ClassWatchComputerList getOfflineComputers() const;
	QStringList getAllClassWatchIds() const;
	int computerCount() const;
	int connectedComputerCount() const;

	// ID management
	bool isIdInUse( const QString& classWatchId ) const;
	QString getNextAvailableId() const;
	bool validateId( const QString& classWatchId ) const;

	// Persistence
	bool saveRegistry( const QString& filePath );
	bool loadRegistry( const QString& filePath );

Q_SIGNALS:
	void computerRegistered( const QString& classWatchId, const ClassWatchComputer& computer );
	void computerUnregistered( const QString& classWatchId );
	void computerInfoUpdated( const QString& classWatchId );
	void connectionStatusChanged( const QString& classWatchId, const QString& status );

private:
	QString generateClassWatchId();
	void migrateFromVeyonFormat( const QString& filePath );

	QMap<QString, ClassWatchComputer> m_computers;
	QMap<NetworkObject::Uid, QString> m_networkObjectToClassWatchId;
	ClassWatchIdGenerator m_idGenerator;
};
