/*
 * ClassWatchGuideDialog.h - built-in getting-started guide for
 * ClassWatch Teacher: downloads, installation, classroom setup,
 * monitoring, file transfer and troubleshooting
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

class QTabWidget;
class QLabel;

class ClassWatchGuideDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ClassWatchGuideDialog( QWidget* parent = nullptr );

private:
	QWidget* buildDownloadTab();
	QWidget* buildInstallTab();
	QWidget* buildConnectTab();
	QWidget* buildMonitorTab();
	QWidget* buildFilesTab();
	QWidget* buildTroubleshootingTab();
	QWidget* screenshotLabel( const QString& resource, const QString& caption );

	QTabWidget* m_tabs;
};
