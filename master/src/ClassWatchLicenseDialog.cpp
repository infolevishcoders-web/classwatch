/*
 * ClassWatchLicenseDialog.cpp - license activation dialog for ClassWatch Master
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

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "ClassWatchLicense.h"
#include "VeyonCore.h"


ClassWatchLicenseDialog::ClassWatchLicenseDialog( QWidget* parent ) :
	QDialog( parent )
{
	setWindowTitle( tr( "ClassWatch License Activation" ) );
	setModal( true );
	setupUi();
	refreshStatus();
}


void ClassWatchLicenseDialog::setupUi()
{
	auto* layout = new QVBoxLayout( this );

	auto* intro = new QLabel(
		tr( "ClassWatch Master requires an active license or a free trial.\n"
			"Get your license key from https://classwatch.levishcoders.com" ),
		this );
	intro->setWordWrap( true );
	layout->addWidget( intro );

	m_statusLabel = new QLabel( this );
	m_statusLabel->setWordWrap( true );
	m_statusLabel->setStyleSheet( QStringLiteral("color:#c0392b;font-weight:bold;") );
	layout->addWidget( m_statusLabel );

	auto* keyLabel = new QLabel( tr( "License key:" ), this );
	layout->addWidget( keyLabel );

	m_licenseKeyEdit = new QLineEdit( this );
	m_licenseKeyEdit->setPlaceholderText( QStringLiteral("CW1-...") );
	layout->addWidget( m_licenseKeyEdit );

	m_activateButton = new QPushButton( tr( "Activate License" ), this );
	connect( m_activateButton, &QPushButton::clicked,
			 this, &ClassWatchLicenseDialog::onActivate );
	layout->addWidget( m_activateButton );

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
	QString error;
	if( ClassWatchLicense::activate( m_licenseKeyEdit->text(), &error ) )
	{
		const auto info = ClassWatchLicense::current();
		QMessageBox::information( this, tr( "License activated" ),
								  tr( "License activated successfully!\n%1" ).arg( info.message ) );
		accept();
		return;
	}
	QMessageBox::warning( this, tr( "Activation failed" ), error );
	refreshStatus();
}
