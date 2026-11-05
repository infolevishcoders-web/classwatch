/*
 * DashboardPanel.h - classroom dashboard overview panel
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

class ComputerManager;
class VeyonMaster;

namespace Ui {
class DashboardPanel;
}

class DashboardPanel : public QWidget
{
	Q_OBJECT
public:
	explicit DashboardPanel( VeyonMaster& master, QWidget *parent = nullptr );
	~DashboardPanel() override;

protected:
	void showEvent( QShowEvent* event ) override;

private Q_SLOTS:
	void refreshStatistics();
	void updateComputerList();

private:
	Ui::DashboardPanel *ui;
	VeyonMaster& m_master;
	ComputerManager& m_computerManager;

};
