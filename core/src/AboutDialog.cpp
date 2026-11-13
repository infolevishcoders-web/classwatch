/*
 * AboutDialog.cpp - implementation of AboutDialog
 *
 * Copyright (c) 2011-2026 Tobias Junghans <tobydox@veyon.io>
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

#include <QDesktopServices>
#include <QFile>

#include "AboutDialog.h"
#include "VeyonCore.h"

#include "ui_AboutDialog.h"


AboutDialog::AboutDialog( QWidget *parent ) :
	QDialog( parent ),
	ui( new Ui::AboutDialog )
{
	ui->setupUi( this );

	// ClassWatch dark theme styling
	setStyleSheet( QStringLiteral(
		"QDialog { background:#101014; }"
		"QLabel { color:#f4f4f5; }"
		"QLabel#label_2 { color:#fecaca; }"
		"QTabWidget::pane { border:1px solid #7f1d1d; border-radius:8px; }"
		"QTabBar::tab { background:#1c0a0a; color:#fca5a5; padding:8px 18px;"
		" border:1px solid #7f1d1d; border-bottom:none; border-top-left-radius:8px;"
		" border-top-right-radius:8px; margin-right:2px; }"
		"QTabBar::tab:selected { background:#7f1d1d; color:#fff1f2; font-weight:bold; }"
		"QTabBar::tab:hover:!selected { background:#450a0a; color:#fecaca; }"
		"QPlainTextEdit { background:#0a0a0c; color:#d4d4d8; border:1px solid #3f3f46;"
		" border-radius:8px; padding:8px; }"
		"QPushButton { background:#991b1b; color:#fff1f2; border:1px solid rgba(254,202,202,0.3);"
		" border-radius:8px; padding:8px 18px; font-weight:600; }"
		"QPushButton:hover { background:#dc2626; }"
		"QDialogButtonBox QPushButton { background:#27272a; color:#f4f4f5;"
		" border:1px solid #52525b; border-radius:8px; padding:6px 20px; }"
		"QDialogButtonBox QPushButton:hover { background:#3f3f46; }" ) );

	setWindowTitle(tr("About ClassWatch %1").arg(VeyonCore::versionString()));

	ui->versionLabel->setText( VeyonCore::versionString() );

	QFile authors( QStringLiteral( ":/CONTRIBUTORS" ) );
	if (authors.open(QFile::ReadOnly))
	{
		ui->authors->setPlainText(QString::fromUtf8(authors.readAll()));
	}

	// ClassWatch by Levish Coders - product license notice.
	// ClassWatch is built on the open-source Veyon project (GPLv2+);
	// upstream license text remains in the COPYING file shipped with the app.
	ui->license->setPlainText( QStringLiteral(
		"CLASSWATCH\n"
		"Classroom Monitoring & Management System\n"
		"Copyright (c) 2026 Levish Coders - https://levishcoders.com\n\n"
		"ClassWatch is a commercial classroom management product developed and\n"
		"supported by Levish Coders. It is built on the open-source Veyon\n"
		"project (https://veyon.io), which is licensed under the GNU General\n"
		"Public License v2 or later. The full GPL license text is included in\n"
		"the COPYING file distributed with this software, in accordance with\n"
		"the terms of that license.\n\n"
		"For licensing, support and enterprise inquiries contact:\n"
		"info.levishcoders@gmail.com | +256 743 199 780\n"
		"https://classwatch.netlify.app" ) );
}



AboutDialog::~AboutDialog()
{
	delete ui;
}



void AboutDialog::openDonationWebsite()
{
	  QDesktopServices::openUrl( QUrl( QStringLiteral( "https://levishcoders.com" ) ) );
}
