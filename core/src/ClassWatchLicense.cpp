/*
 * ClassWatchLicense.cpp - offline license validation, free trial and
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

#include "ClassWatchLicense.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QStandardPaths>

namespace
{
	constexpr auto kTrialDays = 7;
	constexpr auto kKeyPrefix = "CW1-";

	QByteArray urlEncode( const QByteArray& input )
	{
		// base64url without padding - safe inside a single-line license key
		return input.toBase64( QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals );
	}

	QByteArray urlDecode( const QByteArray& input )
	{
		return QByteArray::fromBase64( input, QByteArray::Base64UrlEncoding );
	}
}


QString ClassWatchLicense::licenseDirectory()
{
	const auto base = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );
	const auto dir = QDir( base ).filePath( QStringLiteral("ClassWatch") );
	QDir().mkpath( dir );
	return dir;
}


QString ClassWatchLicense::licenseFilePath()
{
	return QDir( licenseDirectory() ).filePath( QStringLiteral("license.json") );
}


QString ClassWatchLicense::trialStateFilePath()
{
	return QDir( licenseDirectory() ).filePath( QStringLiteral("trial-state.json") );
}


QString ClassWatchLicense::sessionFilePath()
{
	return QDir( licenseDirectory() ).filePath( QStringLiteral("session.json") );
}


QString ClassWatchLicense::licensingSecret()
{
	// NOTE: keep in sync with CLASSWATCH_LICENSE_SECRET on the licensing
	// backend (Render). Stored split to slow casual extraction - determined
	// attackers can still recover it from the binary, which is why seat
	// limits and renewals are also enforced server-side.
	return QStringLiteral("ClsW@tch-L1c3ns1ng-v1#") +
		   QStringLiteral("4mQ2vL8nP4xK9");
}


QString ClassWatchLicense::signPayload( const QByteArray& payload )
{
	// HMAC-SHA256 implemented via QCryptographicHash so we don't depend on
	// Qt6's QHmac module availability across platforms
	const auto key = licensingSecret().toUtf8();
	QByteArray k = key.left( 64 );
	if( k.size() < 64 )
	{
		k.append( QByteArray( 64 - k.size(), '\0' ) );
	}
	for( auto& c : k )
	{
		c = static_cast<char>( static_cast<unsigned char>( c ) ^ 0x36 );
	}
	QByteArray innerIn = k;
	innerIn.append( payload );
	const auto inner = QCryptographicHash::hash( innerIn, QCryptographicHash::Sha256 );
	for( auto& c : k )
	{
		c = static_cast<char>( static_cast<unsigned char>( c ) ^ ( 0x36 ^ 0x5c ) );
	}
	QByteArray outerIn = k;
	outerIn.append( inner );
	const auto outer = QCryptographicHash::hash( outerIn, QCryptographicHash::Sha256 );
	return QString::fromLatin1( urlEncode( outer ) );
}


bool ClassWatchLicense::verifySignature( const QByteArray& payload, const QString& signature )
{
	return signPayload( payload ) == signature;
}


QJsonObject ClassWatchLicense::loadLicenseFile()
{
	QFile file( licenseFilePath() );
	if( !file.open( QIODevice::ReadOnly ) )
	{
		return {};
	}
	QJsonParseError error;
	const auto doc = QJsonDocument::fromJson( file.readAll(), &error );
	return error.error == QJsonParseError::NoError ? doc.object() : QJsonObject();
}


void ClassWatchLicense::saveLicenseFile( const QJsonObject& license )
{
	QFile file( licenseFilePath() );
	if( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
	{
		file.write( QJsonDocument( license ).toJson( QJsonDocument::Compact ) );
	}
}


QJsonObject ClassWatchLicense::loadTrialState()
{
	QFile file( trialStateFilePath() );
	if( !file.open( QIODevice::ReadOnly ) )
	{
		return {};
	}
	QJsonParseError error;
	const auto doc = QJsonDocument::fromJson( file.readAll(), &error );
	return error.error == QJsonParseError::NoError ? doc.object() : QJsonObject();
}


void ClassWatchLicense::saveTrialState( const QJsonObject& state )
{
	QFile file( trialStateFilePath() );
	if( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
	{
		file.write( QJsonDocument( state ).toJson( QJsonDocument::Compact ) );
	}
}


ClassWatchLicense::Info ClassWatchLicense::evaluateTrial()
{
	Info info;
	auto state = loadTrialState();
	if( state.isEmpty() )
	{
		state[QStringLiteral("firstRunUtc")] =
			QDateTime::currentDateTimeUtc().toString( Qt::ISODate );
		saveTrialState( state );
	}

	const auto firstRun = QDateTime::fromString(
		state.value( QStringLiteral("firstRunUtc") ).toString(), Qt::ISODate );
	const auto trialEnd = firstRun.addDays( kTrialDays );
	const auto now = QDateTime::currentDateTimeUtc();

	info.licensee = state.value( QStringLiteral("email") ).toString();
	info.plan = QStringLiteral("trial");
	info.seats = 3;

	if( now < trialEnd )
	{
		info.status = Status::Trial;
		info.expiresAt = trialEnd;
		info.daysLeft = firstRun.daysTo( trialEnd ) - firstRun.daysTo( now );
		info.message = QStringLiteral("Free trial - %1 day(s) remaining").arg( info.daysLeft );
	}
	else
	{
		info.status = Status::TrialEnded;
		info.daysLeft = 0;
		info.message = QStringLiteral("Free trial ended - activate your license");
	}
	return info;
}


ClassWatchLicense::Info ClassWatchLicense::current()
{
	const auto license = loadLicenseFile();
	if( !license.isEmpty() )
	{
		const auto payload = license.value( QStringLiteral("payload") ).toString().toUtf8();
		const auto signature = license.value( QStringLiteral("signature") ).toString();

		if( !verifySignature( payload, signature ) )
		{
			// A corrupt or tampered license file must not lock the user out of
			// their own machine - discard it and fall back to the free trial.
			QFile::remove( licenseFilePath() );
			return evaluateTrial();
		}

		const auto doc = QJsonDocument::fromJson( urlDecode( payload ) );
		const auto data = doc.object();

		Info info;
		info.licensee = data.value( QStringLiteral("email") ).toString();
		info.plan = data.value( QStringLiteral("plan") ).toString();
		info.seats = data.value( QStringLiteral("seats") ).toInt( 3 );
		info.expiresAt = QDateTime::fromString(
			data.value( QStringLiteral("exp") ).toString(), Qt::ISODate );

		if( QDateTime::currentDateTimeUtc() > info.expiresAt )
		{
			info.status = Status::Expired;
			info.daysLeft = 0;
			info.message = QStringLiteral("License expired on %1 - renew to continue")
							   .arg( info.expiresAt.toString( QStringLiteral("yyyy-MM-dd") ) );
		}
		else
		{
			info.status = Status::Valid;
			info.daysLeft = info.expiresAt.daysTo( QDateTime::currentDateTimeUtc() ) * -1;
			info.message = QStringLiteral("Licensed to %1 (%2) - %3 day(s) remaining")
							   .arg( info.licensee, info.plan ).arg( info.daysLeft );
		}
		return info;
	}

	return evaluateTrial();
}


bool ClassWatchLicense::activate( const QString& licenseKey, QString* errorMessage )
{
	auto fail = [errorMessage]( const QString& msg ) {
		if( errorMessage )
		{
			*errorMessage = msg;
		}
		return false;
	};

	QString key = licenseKey.trimmed();
	if( !key.startsWith( QLatin1String( kKeyPrefix ) ) )
	{
		return fail( QStringLiteral("Invalid license key format") );
	}
	key = key.mid( QString::fromLatin1( kKeyPrefix ).size() );

	const auto parts = key.split( QLatin1Char('.') );
	if( parts.size() != 2 )
	{
		return fail( QStringLiteral("Invalid license key format") );
	}

	const auto payloadEncoded = parts.at( 0 ).toUtf8();
	const auto signature = parts.at( 1 );
	const auto payload = urlDecode( payloadEncoded );

	if( !verifySignature( payload, signature ) )
	{
		return fail( QStringLiteral("License key signature is invalid") );
	}

	const auto doc = QJsonDocument::fromJson( payload );
	const auto data = doc.object();
	if( data.isEmpty() )
	{
		return fail( QStringLiteral("License key payload is corrupt") );
	}

	const auto expiresAt = QDateTime::fromString(
		data.value( QStringLiteral("exp") ).toString(), Qt::ISODate );
	if( expiresAt.isValid() && QDateTime::currentDateTimeUtc() > expiresAt )
	{
		return fail( QStringLiteral("This license key has already expired") );
	}

	QJsonObject stored;
	stored[QStringLiteral("payload")] = QString::fromLatin1( payloadEncoded );
	stored[QStringLiteral("signature")] = signature;
	stored[QStringLiteral("activatedUtc")] =
		QDateTime::currentDateTimeUtc().toString( Qt::ISODate );
	saveLicenseFile( stored );

	// remember the email for support/renewal reminders
	auto state = loadTrialState();
	state[QStringLiteral("email")] = data.value( QStringLiteral("email") ).toString();
	saveTrialState( state );

	return true;
}


void ClassWatchLicense::deactivate()
{
	QFile::remove( licenseFilePath() );
	QFile::remove( sessionFilePath() );
}


void ClassWatchLicense::saveSession( const QString& token, const QJsonObject& profile )
{
	QJsonObject session;
	session[QStringLiteral("token")] = token;
	session[QStringLiteral("profile")] = profile;
	session[QStringLiteral("savedUtc")] =
		QDateTime::currentDateTimeUtc().toString( Qt::ISODate );
	QFile file( sessionFilePath() );
	if( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
	{
		file.write( QJsonDocument( session ).toJson( QJsonDocument::Compact ) );
	}
}


QString ClassWatchLicense::sessionToken()
{
	QFile file( sessionFilePath() );
	if( !file.open( QIODevice::ReadOnly ) )
	{
		return {};
	}
	QJsonParseError error;
	const auto doc = QJsonDocument::fromJson( file.readAll(), &error );
	if( error.error != QJsonParseError::NoError )
	{
		return {};
	}
	return doc.object().value( QStringLiteral("token") ).toString();
}


QJsonObject ClassWatchLicense::sessionProfile()
{
	QFile file( sessionFilePath() );
	if( !file.open( QIODevice::ReadOnly ) )
	{
		return {};
	}
	QJsonParseError error;
	const auto doc = QJsonDocument::fromJson( file.readAll(), &error );
	if( error.error != QJsonParseError::NoError )
	{
		return {};
	}
	return doc.object().value( QStringLiteral("profile") ).toObject();
}
