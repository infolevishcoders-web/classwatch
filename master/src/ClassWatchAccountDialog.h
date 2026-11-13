/*
 * ClassWatchAccountDialog.h - account profile, subscription status and
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

#pragma once

#include <QDialog>
#include <QJsonObject>

class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
class QTimer;

// Shows the signed-in ClassWatch account: profile, plan, seat count and a
// days-left counter. The "Renew / Upgrade" flow creates a PawaPay mobile
// money deposit through the ClassWatch API and activates the new license
// automatically once the payment is approved on the teacher's phone.
class ClassWatchAccountDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ClassWatchAccountDialog( QWidget* parent = nullptr );

private Q_SLOTS:
	void refresh();
	void startRenewal();
	void openDashboard();

private:
	void setupUi();
	void loadStoredInfo();
	void fetchOnlineStatus();
	void renderSubscription( const QJsonObject& subscription );
	bool apiCall( const QString& method, const QString& path,
				  const QJsonObject& body, QJsonObject* result, QString* error );
	void pollPayment( const QString& depositId, int attemptsLeft );
	void finishRenewal( const QString& licenseKey );

	QLabel* m_profileLabel;
	QLabel* m_planLabel;
	QLabel* m_daysLabel;
	QLabel* m_detailLabel;
	QLabel* m_statusLabel;
	QLabel* m_paymentLabel;
	QComboBox* m_planCombo;
	QLineEdit* m_phoneEdit;
	QPushButton* m_renewButton;
	QPushButton* m_refreshButton;
	QTimer* m_pollTimer;
	QString m_pendingDepositId;
	int m_pollAttempts;
};
