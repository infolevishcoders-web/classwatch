/*
 * ClassWatchIdGenerator.cpp - implementation of ClassWatchIdGenerator
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

#include "ClassWatchIdGenerator.h"

#include "VeyonConfiguration.h"

#include <QRegularExpression>

namespace
{
	QString configuredPrefix()
	{
		QString prefix = VeyonCore::config().classWatchIdPrefix().trimmed();
		prefix.remove( QLatin1Char('-') );
		return prefix.isEmpty() ? QStringLiteral("CW") : prefix.toUpper();
	}

	int configuredStartNumber()
	{
		const int start = VeyonCore::config().classWatchIdStartNumber();
		return start > 0 ? start : 1;
	}
}

ClassWatchIdGenerator::ClassWatchIdGenerator( QObject* parent ) :
	QObject( parent ),
	m_currentNumber( configuredStartNumber() )
{
}

QString ClassWatchIdGenerator::generateId()
{
	QString id = formatId( m_currentNumber );
	m_currentNumber++;
	return id;
}

QString ClassWatchIdGenerator::generateId( const QStringList& existingIds )
{
	return getNextAvailableId( existingIds );
}

bool ClassWatchIdGenerator::isValidId( const QString& id )
{
	QRegularExpression regex( QStringLiteral("^[A-Za-z][A-Za-z0-9]{0,15}-\\d{3,}$") );
	return regex.match( id ).hasMatch();
}

int ClassWatchIdGenerator::parseIdNumber( const QString& id )
{
	if ( !isValidId( id ) ) {
		return -1;
	}

	const int separator = id.lastIndexOf( QLatin1Char('-') );
	return id.mid( separator + 1 ).toInt();
}

QString ClassWatchIdGenerator::formatId( int number )
{
	return QString( QStringLiteral("%1-%2") )
		.arg( configuredPrefix() )
		.arg( number, 3, 10, QLatin1Char('0') );
}

QString ClassWatchIdGenerator::getNextAvailableId( const QStringList& existingIds )
{
	int maxNumber = configuredStartNumber() - 1;

	for ( const QString& id : existingIds ) {
		const int number = parseIdNumber( id );
		if ( number > maxNumber ) {
			maxNumber = number;
		}
	}

	return formatId( maxNumber + 1 );
}
