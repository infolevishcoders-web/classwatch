/*
 * ClassWatchComputerManager.h - ClassWatch-specific computer management
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

#include "ComputerManager.h"
#include "ClassWatchComputer.h"
#include "ClassWatchComputerManager.h"
#include <QMap>
#include <QTimer>

class ClassWatchComputerManagerExtension : public QObject
{
	Q_OBJECT
public:
	ClassWatchComputerManagerExtension( ComputerManager* computerManager, QObject* parent = nullptr );
	~ClassWatchComputerManagerExtension() override;

	// Computer identification
	QString getClassWatchIdForComputer( const Computer& computer ) const;
	QString getClassWatchIdForNetworkObjectUid( NetworkObject::Uid uid ) const;
	ClassWatchComputer getClassWatchComputer( const QString& classWatchId ) const;
	ClassWatchComputer getClassWatchComputerByNetworkObjectUid( NetworkObject::Uid uid ) const;

	// Registration and updates
	void registerComputer( const Computer& computer );
	void unregisterComputer( const QString& classWatchId );
	void updateComputerConnectionStatus( const Computer& computer, const QString& status );
	void updateComputerUser( const Computer& computer, const QString& userName );
	void updateComputerSessionInfo( const Computer& computer, const QString& sessionType );

	// Batch operations
	void syncComputersFromComputerManager();
	bool saveClassWatchRegistry( const QString& filePath );
	bool loadClassWatchRegistry( const QString& filePath );

	// Queries
	ClassWatchComputerList getAllClassWatchComputers() const;
	ClassWatchComputerList getConnectedClassWatchComputers() const;
	ClassWatchComputerList getOfflineClassWatchComputers() const;
	QStringList getAllClassWatchIds() const;
	int classWatchComputerCount() const;

Q_SIGNALS:
	void computerRegistered( const QString& classWatchId, const ClassWatchComputer& computer );
	void computerUnregistered( const QString& classWatchId );
	void computerInfoUpdated( const QString& classWatchId );
	void connectionStatusChanged( const QString& classWatchId, const QString& status );

private:
	void connectToComputerManager();
	void onComputerSelectionChanged();
	void onComputerAdded( const Computer& computer );
	void onComputerRemoved( const Computer& computer );
	void scheduleSync();
	void scheduleRegistrySave();
	void persistRegistry();
	ComputerList allDiscoveredComputers() const;
	void collectHostComputers( const QModelIndex& parent, ComputerList& computers ) const;

	ComputerManager* m_computerManager;
	ClassWatchComputerManager* m_classWatchManager;
	QMap<NetworkObject::Uid, QString> m_uidToClassWatchId;
	QTimer* m_syncDebounceTimer;
	QTimer* m_registrySaveTimer;
	QTimer* m_periodicSyncTimer;
};
