/*
 * ClassWatchClientStatus.h - client status display for ClassWatch
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
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include "VeyonCore.h"
#include "VeyonConfiguration.h"

class ClassWatchClientStatus : public QDialog
{
	Q_OBJECT
public:
	explicit ClassWatchClientStatus( QWidget* parent = nullptr );
	~ClassWatchClientStatus() override;

private Q_SLOTS:
	void updateStatus();
	void onClose();

private:
	void setupUI();
	QString getClassWatchId();
	QString getConnectionStatus();

	QLabel* m_idLabel;
	QLabel* m_statusLabel;
	QLabel* m_userLabel;
	QLabel* m_computerLabel;
	QPushButton* m_closeButton;
	QTimer* m_updateTimer;
};
