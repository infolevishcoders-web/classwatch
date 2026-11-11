/*
 * ClassWatchComputerManager.cpp - implementation of ClassWatchComputerManager
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

#include "ClassWatchComputerManager.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QUuid>

ClassWatchComputerManager::ClassWatchComputerManager( QObject* parent ) :
	QObject( parent )
{
}

ClassWatchComputerManager::~ClassWatchComputerManager()
{
}

QString ClassWatchComputerManager::registerComputer( const Computer& computer )
{
	// Check if computer is already registered by network object UID
	if ( m_networkObjectToClassWatchId.contains( computer.networkObjectUid() ) ) {
		QString existingId = m_networkObjectToClassWatchId[computer.networkObjectUid()];
		ClassWatchComputer& existing = m_computers[existingId];
		
		// Update computer info with latest data
		existing.setDisplayName( computer.displayName() );
		existing.setHostAddress( computer.hostAddress().toString() );
		existing.setMacAddress( computer.macAddress() );
		existing.setLocation( computer.location() );
		
		Q_EMIT computerInfoUpdated( existingId );
		return existingId;
	}

	// Generate new ClassWatch ID
	QStringList existingIds = getAllClassWatchIds();
	QString classWatchId = ClassWatchIdGenerator::getNextAvailableId( existingIds );

	// Create ClassWatchComputer from Computer
	ClassWatchComputer cwComputer( computer );
	cwComputer.setClassWatchId( classWatchId );
	cwComputer.setConnectionStatus( QStringLiteral("Registered") );
	cwComputer.setLastConnectionTime( QDateTime::currentDateTime() );

	// Store in maps
	m_computers[classWatchId] = cwComputer;
	m_networkObjectToClassWatchId[computer.networkObjectUid()] = classWatchId;

	Q_EMIT computerRegistered( classWatchId, cwComputer );
	return classWatchId;
}

void ClassWatchComputerManager::unregisterComputer( const QString& classWatchId )
{
	if ( !m_computers.contains( classWatchId ) ) {
		return;
	}

	ClassWatchComputer computer = m_computers[classWatchId];
	NetworkObject::Uid uid = computer.networkObjectUid();

	m_computers.remove( classWatchId );
	m_networkObjectToClassWatchId.remove( uid );

	Q_EMIT computerUnregistered( classWatchId );
}

ClassWatchComputer ClassWatchComputerManager::getComputer( const QString& classWatchId ) const
{
	if ( m_computers.contains( classWatchId ) ) {
		return m_computers[classWatchId];
	}
	return ClassWatchComputer();
}

ClassWatchComputer ClassWatchComputerManager::getComputerByNetworkObjectUid( NetworkObject::Uid uid ) const
{
	if ( m_networkObjectToClassWatchId.contains( uid ) ) {
		QString classWatchId = m_networkObjectToClassWatchId[uid];
		return getComputer( classWatchId );
	}
	return ClassWatchComputer();
}

void ClassWatchComputerManager::updateComputerInfo( const QString& classWatchId, const ClassWatchComputer& computer )
{
	if ( !m_computers.contains( classWatchId ) ) {
		return;
	}

	m_computers[classWatchId] = computer;
	Q_EMIT computerInfoUpdated( classWatchId );
}

void ClassWatchComputerManager::updateConnectionStatus( const QString& classWatchId, const QString& status )
{
	if ( !m_computers.contains( classWatchId ) ) {
		return;
	}

	ClassWatchComputer& computer = m_computers[classWatchId];
	QString oldStatus = computer.connectionStatus();
	computer.setConnectionStatus( status );
	
	if ( status == QStringLiteral("Connected") ) {
		computer.setLastConnectionTime( QDateTime::currentDateTime() );
	}

	Q_EMIT connectionStatusChanged( classWatchId, status );
	Q_EMIT computerInfoUpdated( classWatchId );
}

void ClassWatchComputerManager::updateLoggedInUser( const QString& classWatchId, const QString& userName )
{
	if ( !m_computers.contains( classWatchId ) ) {
		return;
	}

	m_computers[classWatchId].setLoggedInUser( userName );
	Q_EMIT computerInfoUpdated( classWatchId );
}

void ClassWatchComputerManager::updateLastConnectionTime( const QString& classWatchId )
{
	if ( !m_computers.contains( classWatchId ) ) {
		return;
	}

	m_computers[classWatchId].setLastConnectionTime( QDateTime::currentDateTime() );
	Q_EMIT computerInfoUpdated( classWatchId );
}

ClassWatchComputerList ClassWatchComputerManager::getAllComputers() const
{
	ClassWatchComputerList list;
	for ( auto it = m_computers.begin(); it != m_computers.end(); ++it ) {
		list.append( it.value() );
	}
	return list;
}

ClassWatchComputerList ClassWatchComputerManager::getConnectedComputers() const
{
	ClassWatchComputerList list;
	for ( auto it = m_computers.begin(); it != m_computers.end(); ++it ) {
		if ( it.value().connectionStatus() == QStringLiteral("Connected") ) {
			list.append( it.value() );
		}
	}
	return list;
}

ClassWatchComputerList ClassWatchComputerManager::getOfflineComputers() const
{
	ClassWatchComputerList list;
	for ( auto it = m_computers.begin(); it != m_computers.end(); ++it ) {
		if ( it.value().connectionStatus() != QStringLiteral("Connected") ) {
			list.append( it.value() );
		}
	}
	return list;
}

QStringList ClassWatchComputerManager::getAllClassWatchIds() const
{
	QStringList ids;
	for ( auto it = m_computers.begin(); it != m_computers.end(); ++it ) {
		ids.append( it.key() );
	}
	return ids;
}

int ClassWatchComputerManager::computerCount() const
{
	return m_computers.size();
}

int ClassWatchComputerManager::connectedComputerCount() const
{
	return getConnectedComputers().size();
}

bool ClassWatchComputerManager::isIdInUse( const QString& classWatchId ) const
{
	return m_computers.contains( classWatchId );
}

QString ClassWatchComputerManager::getNextAvailableId() const
{
	QStringList existingIds = getAllClassWatchIds();
	return ClassWatchIdGenerator::getNextAvailableId( existingIds );
}

bool ClassWatchComputerManager::validateId( const QString& classWatchId ) const
{
	return ClassWatchIdGenerator::isValidId( classWatchId );
}

QString ClassWatchComputerManager::generateClassWatchId()
{
	QStringList existingIds = getAllClassWatchIds();
	return ClassWatchIdGenerator::getNextAvailableId( existingIds );
}

bool ClassWatchComputerManager::saveRegistry( const QString& filePath )
{
	QJsonArray computersArray;
	
	for ( auto it = m_computers.begin(); it != m_computers.end(); ++it ) {
		const ClassWatchComputer& computer = it.value();
		
		QJsonObject computerObj;
		computerObj[QStringLiteral("classWatchId")] = computer.classWatchId();
		computerObj[QStringLiteral("displayName")] = computer.displayName();
		computerObj[QStringLiteral("hostAddress")] = computer.hostAddress().toString();
		computerObj[QStringLiteral("macAddress")] = computer.macAddress();
		computerObj[QStringLiteral("location")] = computer.location();
		computerObj[QStringLiteral("windowsVersion")] = computer.windowsVersion();
		computerObj[QStringLiteral("loggedInUser")] = computer.loggedInUser();
		computerObj[QStringLiteral("domainName")] = computer.domainName();
		computerObj[QStringLiteral("connectionStatus")] = computer.connectionStatus();
		computerObj[QStringLiteral("sessionType")] = computer.sessionType();
		computerObj[QStringLiteral("cpuInfo")] = computer.cpuInfo();
		computerObj[QStringLiteral("ramInfo")] = computer.ramInfo();
		computerObj[QStringLiteral("gpuInfo")] = computer.gpuInfo();
		computerObj[QStringLiteral("lastConnectionTime")] = computer.lastConnectionTime().toString(Qt::ISODate);
		if ( !computer.networkObjectUid().isNull() ) {
			computerObj[QStringLiteral("networkObjectUid")] =
				computer.networkObjectUid().toString( QUuid::WithoutBraces );
		}

		computersArray.append( computerObj );
	}
	
	QJsonObject rootObj;
	rootObj[QStringLiteral("version")] = QStringLiteral("1.0");
	rootObj[QStringLiteral("computers")] = computersArray;
	
	QJsonDocument doc( rootObj );
	QDir().mkpath( QFileInfo( filePath ).absolutePath() );
	QFile file( filePath );
	
	if ( !file.open( QIODevice::WriteOnly ) ) {
		return false;
	}
	
	file.write( doc.toJson() );
	file.close();
	return true;
}

bool ClassWatchComputerManager::loadRegistry( const QString& filePath )
{
	QFile file( filePath );
	
	if ( !file.exists() ) {
		// Try to migrate from Veyon format
		migrateFromVeyonFormat( filePath );
		return false;
	}
	
	if ( !file.open( QIODevice::ReadOnly ) ) {
		return false;
	}
	
	QByteArray data = file.readAll();
	file.close();
	
	QJsonDocument doc = QJsonDocument::fromJson( data );
	if ( !doc.isObject() ) {
		return false;
	}
	
	QJsonObject rootObj = doc.object();
	QJsonArray computersArray = rootObj[QStringLiteral("computers")].toArray();
	
	m_computers.clear();
	m_networkObjectToClassWatchId.clear();
	
	for ( const QJsonValue& value : computersArray ) {
		QJsonObject computerObj = value.toObject();

		const auto uidString = computerObj[QStringLiteral("networkObjectUid")].toString();
		const auto uid = uidString.isEmpty() ? NetworkObject::Uid()
											 : QUuid::fromString( uidString );

		// Build the base Computer first so the persistent network object UID
		// (which has no setter) is carried into the ClassWatchComputer.
		Computer baseComputer( uid,
							  computerObj[QStringLiteral("displayName")].toString(),
							  computerObj[QStringLiteral("hostAddress")].toString(),
							  computerObj[QStringLiteral("macAddress")].toString(),
							  computerObj[QStringLiteral("location")].toString() );
		ClassWatchComputer computer( baseComputer );
		computer.setClassWatchId( computerObj[QStringLiteral("classWatchId")].toString() );
		computer.setWindowsVersion( computerObj[QStringLiteral("windowsVersion")].toString() );
		computer.setLoggedInUser( computerObj[QStringLiteral("loggedInUser")].toString() );
		computer.setDomainName( computerObj[QStringLiteral("domainName")].toString() );
		computer.setConnectionStatus( computerObj[QStringLiteral("connectionStatus")].toString() );
		computer.setSessionType( computerObj[QStringLiteral("sessionType")].toString() );
		computer.setCpuInfo( computerObj[QStringLiteral("cpuInfo")].toString() );
		computer.setRamInfo( computerObj[QStringLiteral("ramInfo")].toString() );
		computer.setGpuInfo( computerObj[QStringLiteral("gpuInfo")].toString() );
		
		QString timeStr = computerObj[QStringLiteral("lastConnectionTime")].toString();
		computer.setLastConnectionTime( QDateTime::fromString( timeStr, Qt::ISODate ) );

		QString classWatchId = computer.classWatchId();
		m_computers[classWatchId] = computer;

		// Restore the network object UID -> ClassWatch ID mapping so the same
		// physical machine keeps its ID across restarts and IP address changes.
		if ( !uid.isNull() && !m_networkObjectToClassWatchId.contains( uid ) ) {
			m_networkObjectToClassWatchId[uid] = classWatchId;
		}
	}
	
	return true;
}

void ClassWatchComputerManager::migrateFromVeyonFormat( const QString& filePath )
{
	// This is a placeholder for migrating from Veyon's computer list format
	// In a real implementation, we'd read the Veyon computer list and
	// convert it to ClassWatch format with generated IDs
	Q_UNUSED( filePath );
}
