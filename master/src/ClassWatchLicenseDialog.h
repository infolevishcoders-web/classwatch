/*
 * ClassWatchLicenseDialog.h - license activation / account login dialog
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

#pragma once

#include <QDialog>

class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;

// Shown when no valid license/trial exists. Teachers sign up on the
// ClassWatch website, receive a CW1-... license key, and sign in here -
// the key is verified online against the licensing backend and the
// returned session JWT is stored for authenticated API calls.
class ClassWatchLicenseDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ClassWatchLicenseDialog( QWidget* parent = nullptr );

private Q_SLOTS:
	void onActivate();
	void refreshStatus();

private:
	void setupUi();
	void verifyOnline();

	QLabel* m_statusLabel;
	QLabel* m_accountLabel;
	QLineEdit* m_emailEdit;
	QLineEdit* m_licenseKeyEdit;
	QPushButton* m_activateButton;
	QDialogButtonBox* m_buttonBox;
};
