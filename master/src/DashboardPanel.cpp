/*
 * DashboardPanel.cpp - classroom dashboard overview panel implementation
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

#include "DashboardPanel.h"
#include "ComputerManager.h"
#include "ComputerMonitoringModel.h"
#include "VeyonMaster.h"
#include "UserConfig.h"
#include "VncConnection.h"

#include "ui_DashboardPanel.h"

#include <QListWidgetItem>
#include <QTimer>

DashboardPanel::DashboardPanel( VeyonMaster& master, QWidget *parent ) :
	QWidget(parent),
	ui(new Ui::DashboardPanel),
	m_master(master),
	m_computerManager(master.computerManager())
{
	ui->setupUi(this);

	// Connect quick action buttons
	connect(ui->monitorAllButton, &QPushButton::clicked, this, []() {
		// TODO: Implement monitor all computers feature
	});

	connect(ui->lockAllButton, &QPushButton::clicked, this, []() {
		// TODO: Implement lock all screens feature
	});

	connect(ui->messageAllButton, &QPushButton::clicked, this, []() {
		// TODO: Implement send message to all feature
	});

	connect(ui->powerOffButton, &QPushButton::clicked, this, []() {
		// TODO: Implement power off selected computers
	});

	// Set up refresh timer
	auto refreshTimer = new QTimer(this);
	refreshTimer->setInterval(5000); // Refresh every 5 seconds
	connect(refreshTimer, &QTimer::timeout, this, &DashboardPanel::refreshStatistics);
	refreshTimer->start();

	// Initial refresh
	refreshStatistics();
}

DashboardPanel::~DashboardPanel()
{
	delete ui;
}

void DashboardPanel::showEvent( QShowEvent* event )
{
	QWidget::showEvent(event);
	refreshStatistics();
}

void DashboardPanel::refreshStatistics()
{
	// Get computer statistics from the monitoring model
	auto* model = &m_master.computerControlListModel();
	if (!model) {
		return;
	}

	int totalComputers = 0;
	int connectedComputers = 0;
	int offlineComputers = 0;
	int activeSessions = 0;

	// Iterate through all computers to count statistics
	for (int row = 0; row < model->rowCount(); ++row) {
		QModelIndex index = model->index(row, 0);
		if (index.isValid()) {
			totalComputers++;
			
			// Get computer state from the control interface
			auto controlInterface = model->computerControlInterface(index);
			if (controlInterface) {
				auto state = controlInterface->state();
				if (state == VncConnection::State::Connected) {
					connectedComputers++;
					activeSessions++;
				} else if (state == VncConnection::State::Disconnected || 
						   state == VncConnection::State::HostOffline ||
						   state == VncConnection::State::ConnectionFailed ||
						   state == VncConnection::State::AuthenticationFailed ||
						   state == VncConnection::State::AccessControlFailed) {
					offlineComputers++;
				}
			}
		}
	}

	// Update UI labels
	ui->totalComputersValue->setText(QString::number(totalComputers));
	ui->connectedComputersValue->setText(QString::number(connectedComputers));
	ui->offlineComputersValue->setText(QString::number(offlineComputers));
	ui->activeSessionsValue->setText(QString::number(activeSessions));

	// Update computer status list
	updateComputerList();
}

void DashboardPanel::updateComputerList()
{
	ui->computerStatusList->clear();

	auto* model = &m_master.computerControlListModel();
	if (!model) {
		return;
	}

	for (int row = 0; row < model->rowCount(); ++row) {
		QModelIndex index = model->index(row, 0);
		if (index.isValid()) {
			QString computerName = model->data(index, Qt::DisplayRole).toString();
			QString userName = model->data(index, Qt::UserRole + 1).toString(); // Assuming user role is defined
			
			auto controlInterface = model->computerControlInterface(index);
			if (!controlInterface) continue;
			
			auto state = controlInterface->state();

			QString statusText;
			QIcon statusIcon;

			switch (state) {
			case VncConnection::State::Connected:
				statusText = QStringLiteral("%1 - %2 (Connected)").arg(computerName, userName);
				statusIcon = QIcon(QStringLiteral(":/master/host-online.png"));
				break;
			case VncConnection::State::Disconnected:
			case VncConnection::State::HostOffline:
				statusText = QStringLiteral("%1 - Offline").arg(computerName);
				statusIcon = QIcon(QStringLiteral(":/master/host-offline.png"));
				break;
			case VncConnection::State::AuthenticationFailed:
			case VncConnection::State::AccessControlFailed:
			case VncConnection::State::ConnectionFailed:
			case VncConnection::State::ServerNotRunning:
				statusText = QStringLiteral("%1 - Error").arg(computerName);
				statusIcon = QIcon(QStringLiteral(":/master/host-access-denied.png"));
				break;
			case VncConnection::State::Connecting:
				statusText = QStringLiteral("%1 - Connecting...").arg(computerName);
				statusIcon = QIcon(QStringLiteral(":/master/host-dns-error.png"));
				break;
			default:
				statusText = QStringLiteral("%1 - Unknown").arg(computerName);
				statusIcon = QIcon(QStringLiteral(":/master/host-dns-error.png"));
				break;
			}

			auto item = new QListWidgetItem(statusIcon, statusText);
			ui->computerStatusList->addItem(item);
		}
	}
}
