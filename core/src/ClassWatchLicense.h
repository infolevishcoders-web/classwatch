/*
 * ClassWatchLicense.h - offline license validation, free trial and
 * seat management for ClassWatch Master
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

#include <QDateTime>
#include <QJsonObject>
#include <QString>

class ClassWatchLicense
{
public:
	enum class Status
	{
		Valid,          // signed license, not expired
		Trial,          // inside the free trial period
		TrialEnded,     // trial expired, activation required
		Expired,        // license expired, renewal required
		Invalid,        // license file/key failed validation
		Missing         // no license at all and no trial record
	};

	struct Info
	{
		Status status = Status::Missing;
		QString licensee;       // email or organisation name
		QString plan;           // trial | monthly | quarterly | yearly ...
		int seats = 0;          // maximum monitored devices
		QDateTime expiresAt;    // invalid for trial/invalid states
		int daysLeft = 0;
		QString message;        // human readable status
	};

	// evaluate the current license/trial state on this machine
	static Info current();

	// activate with a license key received from the ClassWatch website
	// (format: CW1-<payload>.<signature>)
	static bool activate( const QString& licenseKey, QString* errorMessage = nullptr );

	// remove the stored license (danger zone / transfer)
	static void deactivate();

	// paths
	static QString licenseDirectory();
	static QString licenseFilePath();
	static QString trialStateFilePath();

private:
	static QJsonObject loadLicenseFile();
	static void saveLicenseFile( const QJsonObject& license );
	static QJsonObject loadTrialState();
	static void saveTrialState( const QJsonObject& state );
	static Info evaluateTrial();
	static QString signPayload( const QByteArray& payload );
	static bool verifySignature( const QByteArray& payload, const QString& signature );
	static QString licensingSecret();
};
