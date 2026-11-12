/*
 * ClassWatchTheme.h - ClassWatch "watery" light-blue application theme
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

#include <QApplication>
#include <QColor>
#include <QPalette>

namespace ClassWatchTheme
{
	inline void apply( QApplication* app )
	{
		if( app == nullptr )
		{
			return;
		}

		QPalette palette = app->palette();
		palette.setColor( QPalette::Window, QColor( 0xf0f9ff ) );
		palette.setColor( QPalette::WindowText, QColor( 0x0c4a6e ) );
		palette.setColor( QPalette::Base, Qt::white );
		palette.setColor( QPalette::AlternateBase, QColor( 0xe0f2fe ) );
		palette.setColor( QPalette::Text, QColor( 0x0c4a6e ) );
		palette.setColor( QPalette::Button, QColor( 0xe0f2fe ) );
		palette.setColor( QPalette::ButtonText, QColor( 0x0c4a6e ) );
		palette.setColor( QPalette::Highlight, QColor( 0x38bdf8 ) );
		palette.setColor( QPalette::HighlightedText, Qt::white );
		palette.setColor( QPalette::ToolTipBase, QColor( 0x0c4a6e ) );
		palette.setColor( QPalette::ToolTipText, Qt::white );
		app->setPalette( palette );

		app->setStyleSheet( QStringLiteral(
			"QMainWindow, QDialog {"
			"  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
			"    stop:0 #dbeafe, stop:0.55 #e0f2fe, stop:1 #f8fafc); }"
			"QWidget#centralWidget, QWidget#contents {"
			"  background: transparent; }"
			"QGroupBox {"
			"  border: 1px solid #bae6fd; border-radius: 10px;"
			"  margin-top: 12px; padding-top: 8px;"
			"  background: rgba(255,255,255,190); }"
			"QGroupBox::title {"
			"  subcontrol-origin: margin; left: 10px; padding: 0 4px;"
			"  color: #0369a1; font-weight: bold; }"
			"QPushButton {"
			"  background: #38bdf8; color: white; border: none;"
			"  border-radius: 7px; padding: 6px 14px; font-weight: bold; }"
			"QPushButton:hover { background: #0ea5e9; }"
			"QPushButton:pressed { background: #0284c7; }"
			"QPushButton:disabled { background: #cbd5e1; color: #64748b; }"
			"QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateTimeEdit, QPlainTextEdit {"
			"  background: white; border: 1px solid #bae6fd; border-radius: 6px;"
			"  padding: 4px; color: #0c4a6e; selection-background-color: #bae6fd; }"
			"QTableWidget, QTreeWidget, QTableView, QListWidget, QUndoView {"
			"  background: rgba(255,255,255,215); alternate-background-color: #f0f9ff;"
			"  border: 1px solid #bae6fd; border-radius: 8px;"
			"  color: #0c4a6e; gridline-color: #e0f2fe; }"
			"QHeaderView::section {"
			"  background: #e0f2fe; color: #0369a1; border: none;"
			"  border-bottom: 1px solid #bae6fd; padding: 5px; font-weight: bold; }"
			"QTabBar::tab {"
			"  background: #e0f2fe; color: #0369a1; padding: 8px 18px;"
			"  border-top-left-radius: 7px; border-top-right-radius: 7px;"
			"  margin-right: 2px; }"
			"QTabBar::tab:selected { background: #38bdf8; color: white; }"
			"QTabBar::tab:hover:!selected { background: #bae6fd; }"
			"QToolTip { background: #0c4a6e; color: white; border: none; padding: 5px; }"
			"QMenu { background: white; border: 1px solid #bae6fd; border-radius: 6px; padding: 4px; }"
			"QMenu::item { padding: 6px 20px; border-radius: 4px; }"
			"QMenu::item:selected { background: #e0f2fe; color: #0369a1; }"
			"QStatusBar { background: #e0f2fe; color: #0369a1; }"
			"QSplitter::handle { background: #bae6fd; }"
			"QSplitter::handle:hover { background: #38bdf8; }"
			"QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }"
			"QScrollBar::handle:vertical { background: #bae6fd; border-radius: 5px; min-height: 30px; }"
			"QScrollBar::handle:vertical:hover { background: #38bdf8; }"
			"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
			"QScrollBar:horizontal { background: transparent; height: 10px; margin: 0; }"
			"QScrollBar::handle:horizontal { background: #bae6fd; border-radius: 5px; min-width: 30px; }"
			"QScrollBar::handle:horizontal:hover { background: #38bdf8; }"
			"QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"
			"QProgressBar { background: #e0f2fe; border: 1px solid #bae6fd; border-radius: 6px; text-align: center; color: #0369a1; }"
			"QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #38bdf8, stop:1 #0284c7); border-radius: 5px; }"
			"QCheckBox, QRadioButton { color: #0c4a6e; spacing: 6px; }"
			"QLabel { color: #0c4a6e; }"
			"QToolBar { background: rgba(224,242,254,220); border: none; border-bottom: 1px solid #bae6fd; spacing: 4px; padding: 4px; }"
			"QDockWidget::title { background: #e0f2fe; color: #0369a1; padding: 6px; font-weight: bold; }"
			"QListView::item:selected, QTreeView::item:selected { background: #bae6fd; color: #0c4a6e; }"
			"QFrame[frameShape=\"4\"] { border: 1px solid #bae6fd; }"
		) );
	}
}
