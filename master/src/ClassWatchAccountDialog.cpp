/*
 * ClassWatchAccountDialog.cpp - account profile, subscription status and
 * mobile-money renewal for ClassWatch Teacher
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

#include "ClassWatchAccountDialog.h"

#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QLocale>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include "ClassWatchLicense.h"
#include "VeyonCore.h"

namespace
{
	const QString kAccountApiBase = QStringLiteral("https://classwatch-website.onrender.com");
	const QString kDashboardUrl = QStringLiteral("https://classwatch.netlify.app/dashboard.html");

	QString fmtUGX( qint64 cents )
	{
		// prices are stored in UGX (whole shillings) on the API
		return QStringLiteral("UGX %1").arg( QLocale().toString( cents ) );
	}
}


ClassWatchAccountDialog::ClassWatchAccountDialog( QWidget* parent ) :
	QDialog( parent ),
	m_pollTimer( new QTimer( this ) ),
	m_pendingDepositId(),
	m_pollAttempts( 0 )
{
	setWindowTitle( tr( "ClassWatch Account" ) );
	setMinimumWidth( 460 );
	setupUi();
	loadStoredInfo();
	refresh();
}


void ClassWatchAccountDialog::setupUi()
{
	auto* layout = new QVBoxLayout( this );

	m_profileLabel = new QLabel( this );
	m_profileLabel->setStyleSheet( QStringLiteral("font-size:15px;font-weight:bold;") );
	layout->addWidget( m_profileLabel );

	m_planLabel = new QLabel( this );
	m_planLabel->setStyleSheet( QStringLiteral("color:#a1a1aa;") );
	layout->addWidget( m_planLabel );

	m_daysLabel = new QLabel( this );
	m_daysLabel->setStyleSheet(
		QStringLiteral("font-size:26px;font-weight:bold;color:#dc2626;") );
	layout->addWidget( m_daysLabel );

	m_detailLabel = new QLabel( this );
	m_detailLabel->setWordWrap( true );
	m_detailLabel->setStyleSheet( QStringLiteral("color:#a1a1aa;") );
	layout->addWidget( m_detailLabel );

	m_statusLabel = new QLabel( this );
	m_statusLabel->setWordWrap( true );
	m_statusLabel->setStyleSheet( QStringLiteral("color:#fca5a5;") );
	layout->addWidget( m_statusLabel );

	m_paymentLabel = new QLabel( this );
	m_paymentLabel->setWordWrap( true );
	m_paymentLabel->setStyleSheet(
		QStringLiteral("background:#052e16;color:#4ade80;padding:8px;border-radius:6px;") );
	m_paymentLabel->setVisible( false );
	layout->addWidget( m_paymentLabel );

	// renewal section
	auto* renewLabel = new QLabel( tr( "Renew or upgrade (mobile money):" ), this );
	renewLabel->setStyleSheet( QStringLiteral("font-weight:bold;margin-top:8px;") );
	layout->addWidget( renewLabel );

	m_planCombo = new QComboBox( this );
	layout->addWidget( m_planCombo );

	auto* phoneRow = new QHBoxLayout;
	m_phoneEdit = new QLineEdit( this );
	m_phoneEdit->setPlaceholderText( tr( "Mobile money number e.g. 743199780" ) );
	phoneRow->addWidget( m_phoneEdit );
	layout->addLayout( phoneRow );

	m_renewButton = new QPushButton( tr( "Pay with Mobile Money" ), this );
	connect( m_renewButton, &QPushButton::clicked,
			 this, &ClassWatchAccountDialog::startRenewal );
	layout->addWidget( m_renewButton );

	m_refreshButton = new QPushButton( tr( "Refresh status" ), this );
	connect( m_refreshButton, &QPushButton::clicked,
			 this, &ClassWatchAccountDialog::refresh );
	layout->addWidget( m_refreshButton );

	auto* dashButton = new QPushButton( tr( "Open web dashboard" ), this );
	dashButton->setFlat( true );
	connect( dashButton, &QPushButton::clicked,
			 this, &ClassWatchAccountDialog::openDashboard );
	layout->addWidget( dashButton );

	auto* buttons = new QDialogButtonBox( QDialogButtonBox::Close, this );
	connect( buttons, &QDialogButtonBox::rejected, this, &QDialog::reject );
	layout->addWidget( buttons );

	connect( m_pollTimer, &QTimer::timeout, this, [this]() {
		if( !m_pendingDepositId.isEmpty() )
		{
			pollPayment( m_pendingDepositId, m_pollAttempts );
		}
	} );
}


void ClassWatchAccountDialog::loadStoredInfo()
{
	const auto profile = ClassWatchLicense::sessionProfile();
	const auto name = profile.value( QStringLiteral("display_name") ).toString();
	const auto email = profile.value( QStringLiteral("email") ).toString();
	m_profileLabel->setText(
		tr( "%1 — %2" ).arg( name.isEmpty() ? email : name, email ) );

	// stored subscription from the last online sign-in
	const auto sub = ClassWatchLicense::sessionSubscription();
	if( !sub.isEmpty() )
	{
		renderSubscription( sub );
	}
	else
	{
		const auto lic = ClassWatchLicense::sessionLicense();
		if( !lic.isEmpty() )
		{
			m_planLabel->setText(
				tr( "Plan: %1 · %2 device(s)" )
					.arg( lic.value( QStringLiteral("plan") ).toString(),
						  QString::number( lic.value( QStringLiteral("seats") ).toInt() ) ) );
		}
	}
}


void ClassWatchAccountDialog::refresh()
{
	m_statusLabel->setText( {} );
	loadStoredInfo();
	fetchOnlineStatus();
}


void ClassWatchAccountDialog::fetchOnlineStatus()
{
	const auto token = ClassWatchLicense::sessionToken();
	if( token.isEmpty() )
	{
		m_statusLabel->setText( tr( "Not signed in online — sign in again to refresh." ) );
		return;
	}

	QJsonObject result;
	QString error;
	if( !apiCall( QStringLiteral("GET"), QStringLiteral("/api/subscription/status"),
				  {}, &result, &error ) )
	{
		m_statusLabel->setText( tr( "Offline — showing last known info. (%1)" ).arg( error ) );
		return;
	}

	// refresh plans for the renewal combo
	QJsonObject plansResult;
	if( apiCall( QStringLiteral("GET"), QStringLiteral("/api/plans"), {}, &plansResult, nullptr ) )
	{
		m_planCombo->clear();
		const auto plans = plansResult.value( QStringLiteral("plans") ).toArray();
		for( const auto& v : plans )
		{
			const auto plan = v.toObject();
			m_planCombo->addItem(
				QStringLiteral("%1 — %2 / %3 month(s), %4 devices" )
					.arg( plan.value( QStringLiteral("name") ).toString(),
						  fmtUGX( plan.value( QStringLiteral("price_ugx") ).toVariant().toLongLong() ),
						  QString::number( plan.value( QStringLiteral("duration_months") ).toInt() ),
						  QString::number( plan.value( QStringLiteral("device_limit") ).toInt() ) ),
				plan.value( QStringLiteral("code") ).toString() );
		}
	}

	const auto active = result.value( QStringLiteral("active") ).toObject();
	if( active.isEmpty() )
	{
		m_planLabel->setText( tr( "No active subscription" ) );
		m_daysLabel->setText( tr( "—" ) );
		m_detailLabel->setText( tr( "Start a trial or choose a plan below." ) );
		return;
	}
	renderSubscription( active );
}


void ClassWatchAccountDialog::renderSubscription( const QJsonObject& sub )
{
	const auto planName = sub.value( QStringLiteral("plans") ).toObject()
							  .value( QStringLiteral("name") ).toString();
	const auto planCode = sub.value( QStringLiteral("plan_code") ).toString();
	const auto status = sub.value( QStringLiteral("status") ).toString();
	const auto seats = sub.value( QStringLiteral("seats") ).toInt();
	const auto expiresStr = sub.value( QStringLiteral("expires_at") ).toString();

	m_planLabel->setText(
		tr( "Plan: %1 (%2) · status: %3 · %4 device(s)" )
			.arg( planName.isEmpty() ? planCode : planName, planCode, status,
				  QString::number( seats ) ) );

	const auto expires = QDateTime::fromString( expiresStr, Qt::ISODate );
	if( expires.isValid() )
	{
		const int daysLeft = qMax( 0, QDate::currentDate().daysTo( expires.date() ) );
		m_daysLabel->setText( QString::number( daysLeft ) +
							  ( daysLeft == 1 ? tr( " day left" ) : tr( " days left" ) ) );
		m_daysLabel->setStyleSheet(
			QStringLiteral("font-size:26px;font-weight:bold;color:%1;")
				.arg( daysLeft <= 3 ? QStringLiteral("#dc2626")
									 : daysLeft <= 7 ? QStringLiteral("#d97706")
													: QStringLiteral("#4ade80") ) );
		m_detailLabel->setText(
			tr( "Expires: %1" ).arg( expires.toString( QStringLiteral("dd MMM yyyy, HH:mm") ) ) +
			( daysLeft <= 7
				  ? QStringLiteral("\n") + tr( "Your subscription is ending soon — renew below to keep monitoring." )
				  : QString() ) );
	}
	else
	{
		m_daysLabel->setText( {} );
	}
}


void ClassWatchAccountDialog::startRenewal()
{
	const auto planCode = m_planCombo->currentData().toString();
	const auto phone = m_phoneEdit->text().trimmed();
	if( planCode.isEmpty() )
	{
		QMessageBox::warning( this, tr( "Renew" ), tr( "Choose a plan first." ) );
		return;
	}
	if( phone.isEmpty() )
	{
		QMessageBox::warning( this, tr( "Renew" ),
							  tr( "Enter your mobile money number (MTN or Airtel)." ) );
		return;
	}

	m_renewButton->setEnabled( false );
	m_paymentLabel->setVisible( true );
	m_paymentLabel->setText( tr( "Starting payment…" ) );
	QApplication::processEvents();

	QJsonObject body;
	body[QStringLiteral("plan_code")] = planCode;
	body[QStringLiteral("phone")] = phone;

	QJsonObject result;
	QString error;
	if( !apiCall( QStringLiteral("POST"), QStringLiteral("/api/subscription/checkout"),
				  body, &result, &error ) )
	{
		m_paymentLabel->setText( tr( "Payment error: %1" ).arg( error ) );
		m_renewButton->setEnabled( true );
		return;
	}

	if( result.value( QStringLiteral("mode") ).toString() == QStringLiteral("pawapay") )
	{
		m_pendingDepositId = result.value( QStringLiteral("deposit_id") ).toString();
		m_pollAttempts = 120; // ~10 minutes at 5s intervals
		m_paymentLabel->setText(
			tr( "Approve the payment on your phone — %1 · %2\nAmount: %3\nWaiting for approval…" )
				.arg( result.value( QStringLiteral("provider") ).toString(),
					  result.value( QStringLiteral("phone") ).toString(),
					  fmtUGX( result.value( QStringLiteral("amount_ugx") ).toVariant().toLongLong() ) ) );
		m_pollTimer->start( 5000 );
		return;
	}

	// manual fallback (no payment provider configured)
	m_paymentLabel->setText(
		tr( "Reference: %1 — %2\nPay via mobile money and quote this reference; "
			"your license activates after confirmation." )
			.arg( result.value( QStringLiteral("reference") ).toString(),
				  fmtUGX( result.value( QStringLiteral("amount_ugx") ).toVariant().toLongLong() ) ) );
	m_renewButton->setEnabled( true );
}


void ClassWatchAccountDialog::pollPayment( const QString& depositId, int attemptsLeft )
{
	if( attemptsLeft <= 0 )
	{
		m_pollTimer->stop();
		m_pendingDepositId.clear();
		m_paymentLabel->setText( tr( "Payment timed out — try again or pay on the website." ) );
		m_renewButton->setEnabled( true );
		return;
	}
	m_pollAttempts = attemptsLeft - 1;

	QJsonObject result;
	QString error;
	if( !apiCall( QStringLiteral("GET"),
				  QStringLiteral("/api/payments/") + depositId, {}, &result, &error ) )
	{
		m_paymentLabel->setText( tr( "Checking payment… (%1)").arg( error ) );
		return;
	}

	const auto status = result.value( QStringLiteral("status") ).toString();
	if( status == QStringLiteral("completed") )
	{
		m_pollTimer->stop();
		m_pendingDepositId.clear();
		finishRenewal( result.value( QStringLiteral("license_key") ).toString() );
		return;
	}
	if( status == QStringLiteral("failed") )
	{
		m_pollTimer->stop();
		m_pendingDepositId.clear();
		m_paymentLabel->setText( tr( "Payment failed or was rejected on the phone." ) );
		m_renewButton->setEnabled( true );
		return;
	}
	m_paymentLabel->setText( tr( "Waiting for approval on your phone… (%1s)")
								.arg( ( attemptsLeft * 5 ) ) );
}


void ClassWatchAccountDialog::finishRenewal( const QString& licenseKey )
{
	if( licenseKey.isEmpty() )
	{
		m_paymentLabel->setText( tr( "Payment confirmed — sign in again with your new license key." ) );
		m_renewButton->setEnabled( true );
		return;
	}

	QString localError;
	if( ClassWatchLicense::activate( licenseKey, &localError ) )
	{
		m_paymentLabel->setText( tr( "Payment confirmed — license activated! "
									  "Your subscription is renewed." ) );
		QMessageBox::information( this, tr( "Subscription renewed" ),
								  tr( "Payment received and your ClassWatch license "
									  "has been activated automatically.\n\n"
									  "New license key: %1" ).arg( licenseKey ) );
		loadStoredInfo();
		fetchOnlineStatus();
	}
	else
	{
		m_paymentLabel->setText( tr( "Payment confirmed. New license key: %1 — "
									  "sign out and in again to activate it." ).arg( licenseKey ) );
	}
	m_renewButton->setEnabled( true );
}


void ClassWatchAccountDialog::openDashboard()
{
	QDesktopServices::openUrl( QUrl( kDashboardUrl ) );
}


bool ClassWatchAccountDialog::apiCall( const QString& method, const QString& path,
									   const QJsonObject& body, QJsonObject* result,
									   QString* error )
{
	const auto token = ClassWatchLicense::sessionToken();
	if( token.isEmpty() )
	{
		if( error ) *error = tr( "not signed in" );
		return false;
	}

	QNetworkAccessManager nam;
	QNetworkRequest req( QUrl( kAccountApiBase + path ) );
	req.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json") );
	req.setRawHeader( "Authorization", QByteArray( "Bearer " ) + token.toUtf8() );
	req.setRawHeader( "User-Agent", "ClassWatch-Teacher/1.0" );

	QNetworkReply* reply = nullptr;
	if( method == QStringLiteral("POST") )
	{
		reply = nam.post( req, QJsonDocument( body ).toJson( QJsonDocument::Compact ) );
	}
	else
	{
		reply = nam.get( req );
	}

	QEventLoop loop;
	QTimer timeout;
	timeout.setSingleShot( true );
	connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
	connect( &timeout, &QTimer::timeout, &loop, &QEventLoop::quit );
	timeout.start( 20000 );
	loop.exec();

	QJsonObject resp;
	const bool finished = reply->isFinished() && reply->error() == QNetworkReply::NoError;
	if( finished )
	{
		resp = QJsonDocument::fromJson( reply->readAll() ).object();
	}
	const auto httpError = resp.value( QStringLiteral("error") ).toString();
	reply->deleteLater();

	if( !finished )
	{
		if( error ) *error = tr( "server unreachable" );
		return false;
	}
	if( !httpError.isEmpty() )
	{
		if( error ) *error = httpError;
		return false;
	}
	if( result ) *result = resp;
	return true;
}
