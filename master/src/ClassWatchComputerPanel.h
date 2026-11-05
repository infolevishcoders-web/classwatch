/*
 * ClassWatchComputerPanel.h - panel for managing ClassWatch computer identification
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
#include "ClassWatchComputerManagerExtension.h"

namespace Ui {
class ClassWatchComputerPanel;
}

class VeyonMaster;

class ClassWatchComputerPanel : public QWidget
{
	Q_OBJECT

public:
	explicit ClassWatchComputerPanel( VeyonMaster* master, QWidget* parent = nullptr );
	~ClassWatchComputerPanel() override;

private Q_SLOTS:
	void refreshComputerList();
	void onComputerSelectionChanged();
	void onRegisterComputer();
	void onUnregisterComputer();
	void onSaveRegistry();
	void onLoadRegistry();
	void onClose();

private:
	void setupConnections();
	void updateComputerDetails( const QString& classWatchId );
	void populateComputerTree();

	Ui::ClassWatchComputerPanel* ui;
	VeyonMaster* m_master;
	ClassWatchComputerManagerExtension* m_classWatchManager;
};
