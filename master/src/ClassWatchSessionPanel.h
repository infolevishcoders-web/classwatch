/*
 * ClassWatchSessionPanel.h - panel for managing ClassWatch sessions
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

#include <QWidget>
#include "ClassWatchSessionManager.h"

namespace Ui {
class ClassWatchSessionPanel;
}

class VeyonMaster;

class ClassWatchSessionPanel : public QWidget
{
	Q_OBJECT

public:
	explicit ClassWatchSessionPanel( VeyonMaster* master, QWidget* parent = nullptr );
	~ClassWatchSessionPanel() override;

private Q_SLOTS:
	void refreshSessionList();
	void onSessionSelectionChanged();
	void onAssignStudent();
	void onRemoveStudent();
	void onClearHistory();
	void onExportHistory();
	void onMonitorToggle();
	void onClose();

private:
	void setupConnections();
	void updateSessionDetails( int sessionId );
	void populateSessionTree();
	void populateSessionHistory();
	QString formatUptime( int seconds ) const;

	Ui::ClassWatchSessionPanel* ui;
	VeyonMaster* m_master;
	ClassWatchSessionManager* m_sessionManager;
};
