/*
 * ClassWatchIdGenerator.h - generates unique ClassWatch IDs for computers
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

#include <QObject>
#include <QString>
#include <QMap>

class ClassWatchIdGenerator : public QObject
{
	Q_OBJECT
public:
	explicit ClassWatchIdGenerator( QObject* parent = nullptr );

	// Generate a ClassWatch ID (CW-001, CW-002, etc.)
	QString generateId();

	// Generate ID based on existing computers
	QString generateId( const QStringList& existingIds );

	// Check if an ID is valid format
	static bool isValidId( const QString& id );

	// Parse ID number from CW-XXX format
	static int parseIdNumber( const QString& id );

	// Format ID from number
	static QString formatId( int number );

	// Get next available ID based on existing IDs
	static QString getNextAvailableId( const QStringList& existingIds );

private:
	int m_currentNumber;
	QMap<QString, int> m_usedIds;
};
