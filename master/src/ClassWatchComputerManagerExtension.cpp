/*
 * ClassWatchComputerManagerExtension.cpp - implementation of ClassWatchComputerManagerExtension
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

#include "ClassWatchComputerManagerExtension.h"
#include "ComputerManager.h"
#include "NetworkObject.h"
#include <QTimer>

ClassWatchComputerManagerExtension::ClassWatchComputerManagerExtension( ComputerManager* computerManager, QObject* parent ) :
	QObject( parent ),
	m_computerManager( computerManager ),
	m_classWatchManager( new ClassWatchComputerManager( this ) )
{
	connectToComputerManager();
	
	// Sync computers after a short delay to ensure models are ready
	QTimer::singleShot( 100, this, [this]() {
		syncComputersFromComputerManager();
	} );
}

ClassWatchComputerManagerExtension::~ClassWatchComputerManagerExtension()
{
}

void ClassWatchComputerManagerExtension::connectToComputerManager()
{
	if ( !m_computerManager ) {
		return;
	}

	// Connect to computer selection changes
	connect( m_computerManager, &ComputerManager::computerSelectionChanged,
			 this, &ClassWatchComputerManagerExtension::onComputerSelectionChanged );

	// Connect to computer selection reset
	connect( m_computerManager, &ComputerManager::computerSelectionReset,
			 this, &ClassWatchComputerManagerExtension::onComputerSelectionChanged );
}

QString ClassWatchComputerManagerExtension::getClassWatchIdForComputer( const Computer& computer ) const
{
	NetworkObject::Uid uid = computer.networkObjectUid();
	if ( m_uidToClassWatchId.contains( uid ) ) {
		return m_uidToClassWatchId[uid];
	}
	return QString();
}

QString ClassWatchComputerManagerExtension::getClassWatchIdForNetworkObjectUid( NetworkObject::Uid uid ) const
{
	if ( m_uidToClassWatchId.contains( uid ) ) {
		return m_uidToClassWatchId[uid];
	}
	return QString();
}

ClassWatchComputer ClassWatchComputerManagerExtension::getClassWatchComputer( const QString& classWatchId ) const
{
	return m_classWatchManager->getComputer( classWatchId );
}

ClassWatchComputer ClassWatchComputerManagerExtension::getClassWatchComputerByNetworkObjectUid( NetworkObject::Uid uid ) const
{
	QString classWatchId = getClassWatchIdForNetworkObjectUid( uid );
	if ( classWatchId.isEmpty() ) {
		return ClassWatchComputer();
	}
	return m_classWatchManager->getComputer( classWatchId );
}

void ClassWatchComputerManagerExtension::registerComputer( const Computer& computer )
{
	QString classWatchId = m_classWatchManager->registerComputer( computer );
	m_uidToClassWatchId[computer.networkObjectUid()] = classWatchId;
	
	Q_EMIT computerRegistered( classWatchId, m_classWatchManager->getComputer( classWatchId ) );
}

void ClassWatchComputerManagerExtension::unregisterComputer( const QString& classWatchId )
{
	ClassWatchComputer computer = m_classWatchManager->getComputer( classWatchId );
	if ( computer.isValid() ) {
		m_uidToClassWatchId.remove( computer.networkObjectUid() );
	}
	
	m_classWatchManager->unregisterComputer( classWatchId );
	Q_EMIT computerUnregistered( classWatchId );
}

void ClassWatchComputerManagerExtension::updateComputerConnectionStatus( const Computer& computer, const QString& status )
{
	QString classWatchId = getClassWatchIdForComputer( computer );
	if ( classWatchId.isEmpty() ) {
		// Computer not registered, register it first
		registerComputer( computer );
		classWatchId = getClassWatchIdForComputer( computer );
	}
	
	m_classWatchManager->updateConnectionStatus( classWatchId, status );
	Q_EMIT connectionStatusChanged( classWatchId, status );
	Q_EMIT computerInfoUpdated( classWatchId );
}

void ClassWatchComputerManagerExtension::updateComputerUser( const Computer& computer, const QString& userName )
{
	QString classWatchId = getClassWatchIdForComputer( computer );
	if ( classWatchId.isEmpty() ) {
		registerComputer( computer );
		classWatchId = getClassWatchIdForComputer( computer );
	}
	
	m_classWatchManager->updateLoggedInUser( classWatchId, userName );
	Q_EMIT computerInfoUpdated( classWatchId );
}

void ClassWatchComputerManagerExtension::updateComputerSessionInfo( const Computer& computer, const QString& sessionType )
{
	QString classWatchId = getClassWatchIdForComputer( computer );
	if ( classWatchId.isEmpty() ) {
		registerComputer( computer );
		classWatchId = getClassWatchIdForComputer( computer );
	}
	
	ClassWatchComputer cwComputer = m_classWatchManager->getComputer( classWatchId );
	cwComputer.setSessionType( sessionType );
	m_classWatchManager->updateComputerInfo( classWatchId, cwComputer );
	Q_EMIT computerInfoUpdated( classWatchId );
}

void ClassWatchComputerManagerExtension::syncComputersFromComputerManager()
{
	if ( !m_computerManager ) {
		return;
	}

	// Get all computers from the ComputerManager
	QModelIndex rootIndex;
	ComputerList computers = m_computerManager->selectedComputers( rootIndex );
	
	// For now, we'll register all available computers
	// In a real implementation, we'd iterate through the model
	for ( const Computer& computer : computers ) {
		registerComputer( computer );
	}
}

void ClassWatchComputerManagerExtension::saveClassWatchRegistry( const QString& filePath )
{
	m_classWatchManager->saveRegistry( filePath );
}

void ClassWatchComputerManagerExtension::loadClassWatchRegistry( const QString& filePath )
{
	m_classWatchManager->loadRegistry( filePath );
	
	// Rebuild UID mapping
	m_uidToClassWatchId.clear();
	ClassWatchComputerList computers = m_classWatchManager->getAllComputers();
	for ( const ClassWatchComputer& computer : computers ) {
		m_uidToClassWatchId[computer.networkObjectUid()] = computer.classWatchId();
	}
}

ClassWatchComputerList ClassWatchComputerManagerExtension::getAllClassWatchComputers() const
{
	return m_classWatchManager->getAllComputers();
}

ClassWatchComputerList ClassWatchComputerManagerExtension::getConnectedClassWatchComputers() const
{
	return m_classWatchManager->getConnectedComputers();
}

ClassWatchComputerList ClassWatchComputerManagerExtension::getOfflineClassWatchComputers() const
{
	return m_classWatchManager->getOfflineComputers();
}

QStringList ClassWatchComputerManagerExtension::getAllClassWatchIds() const
{
	return m_classWatchManager->getAllClassWatchIds();
}

int ClassWatchComputerManagerExtension::classWatchComputerCount() const
{
	return m_classWatchManager->computerCount();
}

void ClassWatchComputerManagerExtension::onComputerSelectionChanged()
{
	// When computer selection changes, we might want to update status
	// For now, this is a placeholder for future enhancements
}

void ClassWatchComputerManagerExtension::onComputerAdded( const Computer& computer )
{
	registerComputer( computer );
}

void ClassWatchComputerManagerExtension::onComputerRemoved( const Computer& computer )
{
	QString classWatchId = getClassWatchIdForComputer( computer );
	if ( !classWatchId.isEmpty() ) {
		unregisterComputer( classWatchId );
	}
}
