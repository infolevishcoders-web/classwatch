/*
 * BuiltinDirectoryConfigurationPage.cpp - implementation of BuiltinDirectoryConfigurationPage
 *
 * Copyright (c) 2017-2026 Tobias Junghans <tobydox@veyon.io>
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

#include <QHostAddress>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QTimer>
#include <QTcpSocket>

#include "BuiltinDirectoryConfiguration.h"
#include "BuiltinDirectoryConfigurationPage.h"
#include "Configuration/UiMapping.h"
#include "NetworkObjectModel.h"
#include "ObjectManager.h"
#include "VeyonCore.h"

#include "ui_BuiltinDirectoryConfigurationPage.h"

BuiltinDirectoryConfigurationPage::BuiltinDirectoryConfigurationPage( BuiltinDirectoryConfiguration& configuration, QWidget* parent ) :
	ConfigurationPage( parent ),
	ui(new Ui::BuiltinDirectoryConfigurationPage),
	m_configuration( configuration )
{
	ui->setupUi(this);

	populateLocations();

	connect( ui->locationTableWidget, &QTableWidget::currentItemChanged,
			 this, &BuiltinDirectoryConfigurationPage::populateComputers );
}



BuiltinDirectoryConfigurationPage::~BuiltinDirectoryConfigurationPage()
{
	delete ui;
}



void BuiltinDirectoryConfigurationPage::resetWidgets()
{
	populateLocations();

	ui->locationTableWidget->setCurrentCell( 0, 0 );
}



void BuiltinDirectoryConfigurationPage::connectWidgetsToProperties()
{
}



void BuiltinDirectoryConfigurationPage::applyConfiguration()
{
}



void BuiltinDirectoryConfigurationPage::addLocation()
{
	ObjectManager<NetworkObject> objectManager( m_configuration.networkObjects() );
	objectManager.add(NetworkObject(NetworkObject::Type::Location,
									objectManager.generateUniqueName(tr("New classroom")),
									{}, {}, {}, QUuid::createUuid()));
	m_configuration.setNetworkObjects( objectManager.objects() );

	populateLocations();

	ui->locationTableWidget->setCurrentCell( ui->locationTableWidget->rowCount()-1, 0 );
}



void BuiltinDirectoryConfigurationPage::updateLocation()
{
	auto currentLocationIndex = ui->locationTableWidget->currentIndex();
	if( currentLocationIndex.isValid() == false )
	{
		return;
	}

	ObjectManager<NetworkObject> objectManager( m_configuration.networkObjects() );
	objectManager.update( currentLocationObject() );
	m_configuration.setNetworkObjects( objectManager.objects() );

	populateLocations();

	ui->locationTableWidget->setCurrentIndex( currentLocationIndex );
}



void BuiltinDirectoryConfigurationPage::removeLocation()
{
	auto currentRow = ui->locationTableWidget->currentRow();

	ObjectManager<NetworkObject> objectManager( m_configuration.networkObjects() );
	objectManager.remove( currentLocationObject().uid(), true );
	m_configuration.setNetworkObjects( objectManager.objects() );

	populateLocations();

	if( currentRow > 0 )
	{
		ui->locationTableWidget->setCurrentCell( currentRow-1, 0 );
	}
	else if ( ui->locationTableWidget->rowCount() > 0 )
	{
		ui->locationTableWidget->setCurrentCell( currentRow, 0 );
	}
}



void BuiltinDirectoryConfigurationPage::moveLocationUp()
{
	const int row = ui->locationTableWidget->currentRow();

	if( row <= 0 )
	{
		return;
	}

	ObjectManager<NetworkObject> objectManager( m_configuration.networkObjects() );
	objectManager.moveUp( currentLocationObject().uid() );
	m_configuration.setNetworkObjects( objectManager.objects() );

	populateLocations();
	ui->locationTableWidget->setCurrentCell( row - 1, 0 );
}



void BuiltinDirectoryConfigurationPage::moveLocationDown()
{
	const int row = ui->locationTableWidget->currentRow();

	if( row < 0 || row >= ui->locationTableWidget->rowCount() - 1 )
	{
		return;
	}

	ObjectManager<NetworkObject> objectManager( m_configuration.networkObjects() );
	objectManager.moveDown( currentLocationObject().uid() );
	m_configuration.setNetworkObjects( objectManager.objects() );

	populateLocations();
	ui->locationTableWidget->setCurrentCell( row + 1, 0 );
}



void BuiltinDirectoryConfigurationPage::addComputer()
{
	auto currentLocationUid = currentLocationObject().uid();
	if( currentLocationUid.isNull() )
	{
		return;
	}

	ObjectManager<NetworkObject> objectManager( m_configuration.networkObjects() );
	objectManager.add(NetworkObject(NetworkObject::Type::Host,
									objectManager.generateUniqueName(tr("New student PC")),
									{}, {}, {},
									QUuid::createUuid(),
									currentLocationUid));
	m_configuration.setNetworkObjects( objectManager.objects() );

	populateComputers();

	ui->computerTableWidget->setCurrentCell( ui->computerTableWidget->rowCount()-1, 0 );
}



void BuiltinDirectoryConfigurationPage::updateComputer()
{
	auto currentComputerIndex = ui->computerTableWidget->currentIndex();
	if( currentComputerIndex.isValid() == false )
	{
		return;
	}

	ObjectManager<NetworkObject> objectManager( m_configuration.networkObjects() );
	objectManager.update( currentComputerObject() );
	m_configuration.setNetworkObjects( objectManager.objects() );

	populateComputers();

	ui->computerTableWidget->setCurrentIndex( currentComputerIndex );
}



void BuiltinDirectoryConfigurationPage::removeComputer()
{
	ObjectManager<NetworkObject> objectManager( m_configuration.networkObjects() );
	objectManager.remove( currentComputerObject().uid() );
	m_configuration.setNetworkObjects( objectManager.objects() );

	populateComputers();
}



void BuiltinDirectoryConfigurationPage::moveComputerUp()
{
	const int row = ui->computerTableWidget->currentRow();

	if( row <= 0 )
	{
		return;
	}

	ObjectManager<NetworkObject> objectManager( m_configuration.networkObjects() );
	objectManager.moveUp( currentComputerObject().uid() );
	m_configuration.setNetworkObjects( objectManager.objects() );

	populateComputers();
	ui->computerTableWidget->setCurrentCell( row - 1, 0 );
}



void BuiltinDirectoryConfigurationPage::moveComputerDown()
{
	const int row = ui->computerTableWidget->currentRow();

	if( row < 0 || row >= ui->computerTableWidget->rowCount() - 1 )
	{
		return;
	}

	ObjectManager<NetworkObject> objectManager( m_configuration.networkObjects() );
	objectManager.moveDown( currentComputerObject().uid() );
	m_configuration.setNetworkObjects( objectManager.objects() );

	populateComputers();
	ui->computerTableWidget->setCurrentCell( row + 1, 0 );
}



void BuiltinDirectoryConfigurationPage::populateLocations()
{
	ui->locationTableWidget->setUpdatesEnabled( false );
	ui->locationTableWidget->clear();
	ui->locationTableWidget->setRowCount( 0 );

	ui->addComputerButton->setEnabled( false );

	int rowCount = 0;

	const auto networkObjects = m_configuration.networkObjects();
	for( const auto& networkObjectValue : networkObjects )
	{
		const NetworkObject networkObject( networkObjectValue.toObject() );
		if( networkObject.type() == NetworkObject::Type::Location )
		{
			auto item = new QTableWidgetItem( networkObject.name() );
			item->setData( NetworkObjectModel::UidRole, networkObject.uid() );
			ui->locationTableWidget->setRowCount( ++rowCount );
			ui->locationTableWidget->setItem( rowCount-1, 0, item );
		}
	}

	ui->locationTableWidget->setUpdatesEnabled( true );

	if( rowCount > 0 )
		ui->addComputerButton->setEnabled( true );
}



void BuiltinDirectoryConfigurationPage::populateComputers()
{
	auto parentUid = currentLocationObject().uid();

	ui->computerTableWidget->setUpdatesEnabled( false );
	ui->computerTableWidget->setRowCount( 0 );

	int rowCount = 0;

	const auto networkObjects = m_configuration.networkObjects();
	for( const auto& networkObjectValue : networkObjects )
	{
		const NetworkObject networkObject( networkObjectValue.toObject() );

		if( networkObject.type() == NetworkObject::Type::Host &&
			networkObject.parentUid() == parentUid )
		{
			auto nameItem = new QTableWidgetItem( networkObject.name() );
			nameItem->setData( NetworkObjectModel::UidRole, networkObject.uid() );
			nameItem->setData( NetworkObjectModel::ParentUidRole, networkObject.parentUid() );

			ui->computerTableWidget->setRowCount( rowCount+1 );
			ui->computerTableWidget->setItem( rowCount, 0, nameItem );
			ui->computerTableWidget->setItem( rowCount, 1, new QTableWidgetItem( networkObject.hostAddress() ) );
			ui->computerTableWidget->setItem( rowCount, 2, new QTableWidgetItem( networkObject.macAddress() ) );
			++rowCount;
		}
	}

	ui->computerTableWidget->setUpdatesEnabled( true );

	ui->addComputerButton->setEnabled( currentLocationObject().uid().isNull() == false );
}



NetworkObject BuiltinDirectoryConfigurationPage::currentLocationObject() const
{
	const auto selectedLocation = ui->locationTableWidget->currentItem();
	if( selectedLocation )
	{
		return NetworkObject( NetworkObject::Type::Location,
							  selectedLocation->text(),
							  {},
							  {},
							  {},
							  selectedLocation->data( NetworkObjectModel::UidRole ).toUuid(),
							  selectedLocation->data( NetworkObjectModel::ParentUidRole ).toUuid() );
	}

	return NetworkObject();
}



NetworkObject BuiltinDirectoryConfigurationPage::currentComputerObject() const
{
	const int row = ui->computerTableWidget->currentRow();
	if( row >= 0 )
	{
		auto nameItem = ui->computerTableWidget->item( row, 0 );
		auto hostAddressItem = ui->computerTableWidget->item( row, 1 );
		auto macAddressItem = ui->computerTableWidget->item( row, 2 );

		return NetworkObject( NetworkObject::Type::Host,
							  nameItem->text(),
							  hostAddressItem->text().trimmed(),
							  macAddressItem->text().trimmed(),
							  {},
							  nameItem->data( NetworkObjectModel::UidRole ).toUuid(),
							  nameItem->data( NetworkObjectModel::ParentUidRole ).toUuid() );
	}

	return NetworkObject();
}



void BuiltinDirectoryConfigurationPage::scanLan()
{
	if( m_scanSockets.isEmpty() == false )
	{
		return; // scan already running
	}

	m_scanFound.clear();
	m_scanTargets.clear();
	m_scanPort = VeyonCore::config().veyonServerPort();

	// collect every IPv4 /24 subnet this machine is attached to
	QSet<quint32> subnets;
	const auto interfaces = QNetworkInterface::allInterfaces();
	for( const auto& netInterface : interfaces )
	{
		if( netInterface.flags().testFlag( QNetworkInterface::IsUp ) == false ||
			netInterface.flags().testFlag( QNetworkInterface::IsLoopBack ) )
		{
			continue;
		}
		for( const auto& entry : netInterface.addressEntries() )
		{
			const auto ip = entry.ip();
			if( ip.protocol() == QAbstractSocket::IPv4Protocol && ip.isLoopback() == false )
			{
				subnets.insert( ip.toIPv4Address() & 0xFFFFFF00U );
			}
		}
	}

	for( const auto subnet : subnets )
	{
		for( int host = 1; host < 255; ++host )
		{
			m_scanTargets.append( QHostAddress( subnet | static_cast<quint32>( host ) ) );
		}
	}

	if( m_scanTargets.isEmpty() )
	{
		return;
	}

	ui->scanStatusLabel->setText( tr( "Scanning local network for ClassWatch devices…" ) );
	pumpScan();
}



void BuiltinDirectoryConfigurationPage::pumpScan()
{
	constexpr int kMaxConcurrent = 64;
	constexpr int kTimeoutMs = 350;

	while( m_scanSockets.size() < kMaxConcurrent && m_scanTargets.isEmpty() == false )
	{
		const auto target = m_scanTargets.takeFirst();

		auto* socket = new QTcpSocket( this );
		m_scanSockets.insert( socket );

		connect( socket, &QTcpSocket::connected, this, [this, socket]() {
			// the ClassWatch service answered - record the responder
			m_scanFound.append( socket->peerAddress().toString() );
			socket->disconnectFromHost();
		} );

		connect( socket, &QTcpSocket::disconnected, this, [this, socket]() {
			m_scanSockets.remove( socket );
			socket->deleteLater();
			pumpScan();
		} );

		socket->connectToHost( target, static_cast<quint16>( m_scanPort ) );

		QTimer::singleShot( kTimeoutMs, socket, [socket]() {
			if( socket->state() != QAbstractSocket::ConnectedState )
			{
				socket->abort();
			}
		} );
	}

	if( m_scanSockets.isEmpty() )
	{
		finishScan();
	}
}



void BuiltinDirectoryConfigurationPage::finishScan()
{
	ui->scanStatusLabel->setText( tr( "Found %1 ClassWatch device(s) on the local network." )
								  .arg( m_scanFound.size() ) );

	if( m_scanFound.isEmpty() == false )
	{
		registerScannedComputers();
	}
}



void BuiltinDirectoryConfigurationPage::registerScannedComputers()
{
	// register every responder under the currently selected classroom;
	// when nothing is selected, create a "Discovered" classroom first
	NetworkObject::Uid parentUid = currentLocationObject().uid();

	ObjectManager<NetworkObject> objectManager( m_configuration.networkObjects() );

	if( parentUid.isNull() )
	{
		const auto objects = objectManager.objects();
		for( const auto& objectValue : objects )
		{
			const NetworkObject object( objectValue.toObject() );
			if( object.type() == NetworkObject::Type::Location && object.name() == QLatin1String( "Discovered" ) )
			{
				parentUid = object.uid();
				break;
			}
		}

		if( parentUid.isNull() )
		{
			const auto discovered = NetworkObject( NetworkObject::Type::Location,
												   objectManager.generateUniqueName( tr( "Discovered" ) ),
												   {}, {}, {}, QUuid::createUuid() );
			objectManager.add( discovered );
			parentUid = discovered.uid();
		}
	}

	// skip addresses that are already registered
	QStringList existing;
	const auto objects = objectManager.objects();
	for( const auto& objectValue : objects )
	{
		const NetworkObject object( objectValue.toObject() );
		if( object.type() == NetworkObject::Type::Host )
		{
			existing.append( object.hostAddress() );
		}
	}

	int added = 0;
	for( const auto& address : m_scanFound )
	{
		if( existing.contains( address ) )
		{
			continue;
		}
		objectManager.add( NetworkObject( NetworkObject::Type::Host,
										  objectManager.generateUniqueName(
											  tr( "Student PC %1" ).arg( added + 1 ) ),
										  address, {}, {},
										  QUuid::createUuid(), parentUid ) );
		++added;
	}

	if( added > 0 )
	{
		m_configuration.setNetworkObjects( objectManager.objects() );
		populateLocations();
		populateComputers();
	}
}
