/*
 * ClassWatchLicenseDialog.cpp - license activation / account login dialog
 * for ClassWatch Master
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

#include "ClassWatchLicenseDialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>

#include "ClassWatchLicense.h"
#include "VeyonCore.h"

namespace
{
	const QString kApiBase = QStringLiteral("https://classwatch-website.onrender.com");
}


ClassWatchLicenseDialog::ClassWatchLicenseDialog( QWidget* parent ) :
	QDialog( parent )
{
	setWindowTitle( tr( "ClassWatch Sign In" ) );
	setModal( true );
	setupUi();
	refreshStatus();
}


void ClassWatchLicenseDialog::setupUi()
{
	auto* layout = new QVBoxLayout( this );

	auto* intro = new QLabel(
		tr( "Sign in with the account you created on the ClassWatch website.\n"
			"New here? Sign up free at classwatch.netlify.app - you'll get a "
			"license key by email/dashboard to paste below." ),
		this );
	intro->setWordWrap( true );
	layout->addWidget( intro );

	m_statusLabel = new QLabel( this );
	m_statusLabel->setWordWrap( true );
	m_statusLabel->setStyleSheet( QStringLiteral("color:#c0392b;font-weight:bold;") );
	layout->addWidget( m_statusLabel );

	auto* emailLabel = new QLabel( tr( "Account email:" ), this );
	layout->addWidget( emailLabel );

	m_emailEdit = new QLineEdit( this );
	m_emailEdit->setPlaceholderText( QStringLiteral("you@example.com") );
	// prefill with the email stored during a previous activation/trial
	const auto storedEmail = ClassWatchLicense::sessionProfile()
			.value( QStringLiteral("email") ).toString();
	if( storedEmail.isEmpty() )
	{
		QFile trialFile( QDir( ClassWatchLicense::licenseDirectory() )
							 .filePath( QStringLiteral("trial-state.json") ) );
		if( trialFile.open( QIODevice::ReadOnly ) )
		{
			const auto obj = QJsonDocument::fromJson( trialFile.readAll() ).object();
			m_emailEdit->setText( obj.value( QStringLiteral("email") ).toString() );
		}
	}
	else
	{
		m_emailEdit->setText( storedEmail );
	}
	layout->addWidget( m_emailEdit );

	auto* keyLabel = new QLabel( tr( "License key:" ), this );
	layout->addWidget( keyLabel );

	m_licenseKeyEdit = new QLineEdit( this );
	m_licenseKeyEdit->setPlaceholderText( QStringLiteral("CW1-...") );
	layout->addWidget( m_licenseKeyEdit );

	m_accountLabel = new QLabel( this );
	m_accountLabel->setWordWrap( true );
	m_accountLabel->setStyleSheet( QStringLiteral("color:#0e7490;font-weight:bold;") );
	layout->addWidget( m_accountLabel );

	m_activateButton = new QPushButton( tr( "Sign In" ), this );
	connect( m_activateButton, &QPushButton::clicked,
			 this, &ClassWatchLicenseDialog::onActivate );
	layout->addWidget( m_activateButton );

	auto* hint = new QLabel(
		tr( "Your license key is on the ClassWatch dashboard after sign-up. "
			"The free trial on the website works without a key on first launch." ),
		this );
	hint->setWordWrap( true );
	hint->setStyleSheet( QStringLiteral("color:#64748b;font-size:11px;") );
	layout->addWidget( hint );

	m_buttonBox = new QDialogButtonBox( QDialogButtonBox::Close, this );
	connect( m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
	layout->addWidget( m_buttonBox );
}


void ClassWatchLicenseDialog::refreshStatus()
{
	const auto info = ClassWatchLicense::current();
	m_statusLabel->setText( info.message );
}


void ClassWatchLicenseDialog::onActivate()
{
	const auto key = m_licenseKeyEdit->text().trimmed();
	if( key.isEmpty() )
	{
		QMessageBox::warning( this, tr( "Sign in" ), tr( "Please enter your license key." ) );
		return;
	}

	m_activateButton->setEnabled( false );
	m_activateButton->setText( tr( "Signing in…" ) );
	QApplication::processEvents();

	verifyOnline();

	m_activateButton->setEnabled( true );
	m_activateButton->setText( tr( "Sign In" ) );
}


void ClassWatchLicenseDialog::verifyOnline()
{
	const auto key = m_licenseKeyEdit->text().trimmed();
	const auto email = m_emailEdit->text().trimmed();

	QJsonObject body;
	body[QStringLiteral("license_key")] = key;
	if( !email.isEmpty() )
	{
		body[QStringLiteral("email")] = email;
	}

	QNetworkAccessManager nam;
	QNetworkRequest req( QUrl( kApiBase + QStringLiteral("/api/license/verify") ) );
	req.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json") );
	req.setRawHeader( "User-Agent", "ClassWatch-Master/1.0" );

	auto* reply = nam.post( req, QJsonDocument( body ).toJson( QJsonDocument::Compact ) );

	QEventLoop loop;
	QTimer timeout;
	timeout.setSingleShot( true );
	connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
	connect( &timeout, &QTimer::timeout, &loop, &QEventLoop::quit );
	timeout.start( 15000 );
	loop.exec();

	QJsonObject resp;
	int status = 0;
	if( reply->isFinished() && reply->error() == QNetworkReply::NoError )
	{
		status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
		resp = QJsonDocument::fromJson( reply->readAll() ).object();
	}
	reply->deleteLater();

	const auto serverError = resp.value( QStringLiteral("error") ).toString();

	if( status == 200 && !resp.value( QStringLiteral("token") ).toString().isEmpty() )
	{
		// server confirmed the license + subscription: activate locally and
		// store the session for authenticated API calls
		QString localError;
		if( !ClassWatchLicense::activate( key, &localError ) )
		{
			QMessageBox::warning( this, tr( "Sign in" ), localError );
			refreshStatus();
			return;
		}
		const auto profileObj = resp.value( QStringLiteral("profile") ).toObject();
		ClassWatchLicense::saveSession( resp.value( QStringLiteral("token") ).toString(), profileObj );

		const auto lic = resp.value( QStringLiteral("license") ).toObject();
		const auto name = profileObj.value( QStringLiteral("display_name") ).toString();
		const auto mail = profileObj.value( QStringLiteral("email") ).toString();
		m_accountLabel->setText(
			tr( "Signed in as %1 (%2) — plan %3, %4 device(s), expires %5" )
				.arg( name.isEmpty() ? mail : name,
					  mail,
					  lic.value( QStringLiteral("plan") ).toString(),
					  QString::number( lic.value( QStringLiteral("seats") ).toInt() ),
					  lic.value( QStringLiteral("expires_at") ).toString().left( 10 ) ) );

		const auto info = ClassWatchLicense::current();
		QMessageBox::information( this, tr( "Signed in" ),
								  tr( "Welcome back!\n%1" ).arg( info.message ) );
		accept();
		return;
	}

	if( !serverError.isEmpty() )
	{
		// server explicitly rejected: show its reason
		QMessageBox::warning( this, tr( "Sign in failed" ), serverError );
		refreshStatus();
		return;
	}

	// network unreachable / timeout: fall back to offline activation so a
	// teacher in a classroom without internet is not locked out - the
	// license signature is still verified locally and seat limits remain
	// enforced by the key itself
	QString localError;
	if( ClassWatchLicense::activate( key, &localError ) )
	{
		QMessageBox::information( this, tr( "Offline mode" ),
								  tr( "Could not reach the ClassWatch server - signed in offline.\n"
									  "Reconnect soon so your subscription stays verified." ) );
		accept();
		return;
	}
	QMessageBox::warning( this, tr( "Sign in failed" ),
						  localError.isEmpty() ? tr( "Invalid license key" ) : localError );
	refreshStatus();
}
