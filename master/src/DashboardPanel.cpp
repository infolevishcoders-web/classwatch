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
#include "ClassWatchTheme.h"
#include "ComputerManager.h"
#include "ComputerControlListModel.h"
#include "ComputerMonitoringModel.h"
#include "ScreenRecorder.h"
#include "VeyonMaster.h"
#include "UserConfig.h"
#include "VncConnection.h"

#include "ui_DashboardPanel.h"

#include <QDateTime>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTimer>

#include "VeyonConfiguration.h"
#include "VeyonCore.h"
#include "Filesystem.h"

DashboardPanel::DashboardPanel( VeyonMaster& master, QWidget *parent ) :
	QWidget(parent),
	ui(new Ui::DashboardPanel),
	m_master(master),
	m_computerManager(master.computerManager())
{
	ui->setupUi(this);

	// compact metric tiles: big accent number over a small caption
	for( const auto* name : { "totalComputersValue", "connectedComputersValue",
							  "offlineComputersValue", "activeSessionsValue" } )
	{
		if( auto* label = findChild<QLabel *>( QString::fromLatin1( name ) ) )
		{
			label->setStyleSheet( QStringLiteral(
				"QLabel { color:#fecaca; }"
				"QLabel QLabel { color:#fca5a5; }" ) );
		}
	}
	setStyleSheet( QStringLiteral(
		"QWidget { background: rgba(28,0,0,0.6); border-radius:10px; }"
		"QLabel { color:#fecaca; font-size:15px; font-weight:700; }"
		"QLineEdit { background:rgba(24,24,27,240); border:1px solid #7f1d1d;"
		" border-radius:6px; padding:3px 6px; color:#f4f4f5; font-size:11px; }"
		"QListWidget { background: rgba(60,10,10,0.15); border:1px solid rgba(220,38,38,0.35);"
		" border-radius:8px; color:#f4f4f5; }"
		"QPushButton { background:#991b1b; color:#fff1f2; border:1px solid rgba(254,202,202,0.3);"
		" border-radius:8px; padding:4px 10px; font-weight:600; font-size:12px; }"
		"QPushButton:hover { background:#dc2626; }"
		"QPushButton:disabled { background:#3f3f46; color:#a1a1aa; }"
		"QComboBox { background:rgba(24,24,27,240); border:1px solid #7f1d1d;"
		" border-radius:7px; padding:3px; color:#f4f4f5; font-size:12px; }"
		"QGroupBox { border:1px solid rgba(220,38,38,0.4); border-radius:10px;"
		" margin-top:10px; padding-top:6px; color:#fca5a5; font-weight:bold; font-size:12px; }"
		"QGroupBox::title { subcontrol-origin:margin; left:8px; padding:0 4px; color:#fca5a5; }" ) );

	// storage locations: recordings + screenshots save folders
	ui->recordingsDirEdit->setText( VeyonCore::filesystem().expandPath(
		VeyonCore::config().recordingsDirectory() ) );
	ui->screenshotsDirEdit->setText( VeyonCore::filesystem().expandPath(
		VeyonCore::config().screenshotDirectory() ) );
	connect( ui->browseRecordingsButton, &QPushButton::clicked, this, [this]() {
		const auto dir = QFileDialog::getExistingDirectory( this, tr( "Select recording save folder" ),
														 ui->recordingsDirEdit->text() );
		if( !dir.isEmpty() )
		{
			VeyonCore::config().setRecordingsDirectory( dir );
			ui->recordingsDirEdit->setText( dir );
		}
	} );
	connect( ui->browseScreenshotsButton, &QPushButton::clicked, this, [this]() {
		const auto dir = QFileDialog::getExistingDirectory( this, tr( "Select screenshot save folder" ),
														 ui->screenshotsDirEdit->text() );
		if( !dir.isEmpty() )
		{
			VeyonCore::config().setScreenshotDirectory( dir );
			ui->screenshotsDirEdit->setText( dir );
		}
	} );

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

	// Screen recorder
	m_recorder = new ScreenRecorder( this );
	connect( ui->recordButton, &QPushButton::clicked,
			 this, &DashboardPanel::onRecordClicked );
	connect( ui->stopRecordButton, &QPushButton::clicked,
			 this, &DashboardPanel::onStopRecordClicked );
	connect( m_recorder, &ScreenRecorder::frameCaptured,
			 this, &DashboardPanel::onRecorderFrame );
	connect( m_recorder, &ScreenRecorder::finished,
			 this, &DashboardPanel::onRecorderFinished );
	connect( m_recorder, &ScreenRecorder::failed,
			 this, &DashboardPanel::onRecorderFailed );

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

	// Update UI labels - compact "number\ncaption" tiles
	ui->totalComputersValue->setText( QStringLiteral("<span style='font-size:20px'>%1</span><br>"
													 "<span style='font-size:10px'>Total</span>")
												  .arg( totalComputers ) );
	ui->connectedComputersValue->setText( QStringLiteral("<span style='font-size:20px'>%1</span><br>"
														 "<span style='font-size:10px'>Online</span>")
													.arg( connectedComputers ) );
	ui->offlineComputersValue->setText( QStringLiteral("<span style='font-size:20px'>%1</span><br>"
													   "<span style='font-size:10px'>Offline</span>")
													  .arg( offlineComputers ) );
	ui->activeSessionsValue->setText( QStringLiteral("<span style='font-size:20px'>%1</span><br>"
														 "<span style='font-size:10px'>Sessions</span>")
													.arg( activeSessions ) );

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

	// refresh the recorder's computer picker with currently connected PCs
	const auto previousSelection = ui->recorderComputerCombo->currentText();
	ui->recorderComputerCombo->clear();

	for (int row = 0; row < model->rowCount(); ++row) {
		QModelIndex index = model->index(row, 0);
		if (index.isValid()) {
			QString computerName = model->data(index, Qt::DisplayRole).toString();
			QString userName = model->data(index, Qt::UserRole + 1).toString(); // Assuming user role is defined
			
			auto controlInterface = model->computerControlInterface(index);
			if (!controlInterface) continue;

			auto state = controlInterface->state();

			if( state == VncConnection::State::Connected &&
				controlInterface->vncConnection() &&
				controlInterface->vncConnection()->hasValidFramebuffer() )
			{
				ui->recorderComputerCombo->addItem( computerName, row );
			}

			QString statusText;
			QIcon statusIcon;

			switch (state) {
			case VncConnection::State::Connected:
				statusText = QStringLiteral("%1 - %2 (Connected)").arg(computerName, userName);
				statusIcon = ClassWatchTheme::darkAwareIcon( QStringLiteral(":/master/host-online.png") );
				break;
			case VncConnection::State::Disconnected:
			case VncConnection::State::HostOffline:
				statusText = QStringLiteral("%1 - Offline").arg(computerName);
				statusIcon = ClassWatchTheme::darkAwareIcon( QStringLiteral(":/master/host-offline.png") );
				break;
			case VncConnection::State::AuthenticationFailed:
			case VncConnection::State::AccessControlFailed:
			case VncConnection::State::ConnectionFailed:
			case VncConnection::State::ServerNotRunning:
				statusText = QStringLiteral("%1 - Error").arg(computerName);
				statusIcon = ClassWatchTheme::darkAwareIcon( QStringLiteral(":/master/host-access-denied.png") );
				break;
			case VncConnection::State::Connecting:
				statusText = QStringLiteral("%1 - Connecting...").arg(computerName);
				statusIcon = ClassWatchTheme::darkAwareIcon( QStringLiteral(":/master/host-dns-error.png") );
				break;
			default:
				statusText = QStringLiteral("%1 - Unknown").arg(computerName);
				statusIcon = ClassWatchTheme::darkAwareIcon( QStringLiteral(":/master/host-dns-error.png") );
				break;
			}

			auto item = new QListWidgetItem(statusIcon, statusText);
			ui->computerStatusList->addItem(item);
		}
	}

	// restore previous recorder selection if that computer is still listed
	const auto idx = ui->recorderComputerCombo->findText( previousSelection );
	if( idx >= 0 )
	{
		ui->recorderComputerCombo->setCurrentIndex( idx );
	}
}


void DashboardPanel::onRecordClicked()
{
	if( m_recorder->isRecording() )
	{
		return;
	}

	auto* model = &m_master.computerControlListModel();
	const auto row = ui->recorderComputerCombo->currentData().toInt();
	if( !model || row < 0 || row >= model->rowCount() )
	{
		ui->recorderStatusLabel->setText( tr( "Select a connected computer first" ) );
		return;
	}

	const auto index = model->index( row, 0 );
	const auto controlInterface = model->computerControlInterface( index );
	auto* connection = controlInterface ? controlInterface->vncConnection() : nullptr;
	if( !connection )
	{
		ui->recorderStatusLabel->setText( tr( "That computer is not connected" ) );
		return;
	}

	const auto computerName = ui->recorderComputerCombo->currentText()
			.replace( QRegularExpression( QStringLiteral("[^A-Za-z0-9_-]") ), QStringLiteral("_") );
	const auto dir = VeyonCore::filesystem().expandPath( VeyonCore::config().recordingsDirectory() );
	VeyonCore::filesystem().ensurePathExists( dir );
	const auto filePath = dir + QStringLiteral("/%1-%2.avi")
			.arg( computerName,
				  QDateTime::currentDateTime().toString( QStringLiteral("yyyyMMdd-HHmmss") ) );

	QString error;
	if( m_recorder->start( connection, filePath, &error ) )
	{
		ui->recordButton->setEnabled( false );
		ui->stopRecordButton->setEnabled( true );
		ui->recorderStatusLabel->setText( tr( "Recording %1 - 0 frames" ).arg( computerName ) );
	}
	else
	{
		ui->recorderStatusLabel->setText( error );
	}
}


void DashboardPanel::onStopRecordClicked()
{
	const auto path = m_recorder->stop();
	ui->recordButton->setEnabled( true );
	ui->stopRecordButton->setEnabled( false );
	ui->recorderStatusLabel->setText( path.isEmpty()
			? tr( "Recording failed - no frames captured" )
			: tr( "Saved: %1" ).arg( path ) );
}


void DashboardPanel::onRecorderFrame( int frameCount )
{
	ui->recorderStatusLabel->setText(
		tr( "Recording… %1 frames (%2s)" )
			.arg( frameCount )
			.arg( m_recorder->elapsedMs() / 1000 ) );
}


void DashboardPanel::onRecorderFinished( const QString& filePath, int frames )
{
	ui->recorderStatusLabel->setText(
		tr( "Saved %1 frames to %2" ).arg( frames ).arg( filePath ) );
}


void DashboardPanel::onRecorderFailed( const QString& reason )
{
	ui->recorderStatusLabel->setText( reason );
}
