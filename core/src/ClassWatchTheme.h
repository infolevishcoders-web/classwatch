/*
 * ClassWatchTheme.h - shared ClassWatch purple theme stylesheet
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

#include <QString>

namespace ClassWatchTheme
{
	// ClassWatch brand palette:
	//   primary   #6D28D9   secondary #8B5CF6
	//   background #F8FAFC  text      #1E293B
	inline QString styleSheet()
	{
		return QStringLiteral(
			"QWidget { background: #F8FAFC; color: #1E293B; }"
			"QMainWindow, QDialog { background: #F8FAFC; }"
			"QLabel { color: #1E293B; background: transparent; }"
			"QPushButton {"
			"  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
			"    stop:0 #8B5CF6, stop:1 #6D28D9);"
			"  color: #FFFFFF; border: none; border-radius: 8px;"
			"  padding: 8px 18px; font-weight: 600; }"
			"QPushButton:hover { background: #7C3AED; }"
			"QPushButton:pressed { background: #5B21B6; }"
			"QPushButton:disabled { background: #CBD5E1; color: #64748B; }"
			"QPushButton:flat { background: transparent; color: #6D28D9; }"
			"QPushButton:flat:hover { background: #EDE9FE; }"
			"QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QTimeEdit, QPlainTextEdit, QTextEdit {"
			"  background: #FFFFFF; border: 1px solid #C7D2FE; border-radius: 8px;"
			"  padding: 6px 8px; color: #1E293B; selection-background-color: #8B5CF6; }"
			"QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QPlainTextEdit:focus, QTextEdit:focus {"
			"  border: 2px solid #8B5CF6; }"
			"QComboBox::drop-down { border: none; width: 24px; }"
			"QComboBox QAbstractItemView { background: #FFFFFF; color: #1E293B;"
			"  selection-background-color: #8B5CF6; selection-color: #FFFFFF;"
			"  border: 1px solid #C7D2FE; }"
			"QHeaderView::section { background: #EDE9FE; color: #4C1D95; border: none;"
			"  padding: 6px; font-weight: 600; }"
			"QTabWidget::pane { border: 1px solid #DDD6FE; border-radius: 8px;"
			"  background: #FFFFFF; top: -1px; }"
			"QTabBar::tab { background: #EDE9FE; color: #4C1D95; padding: 8px 18px;"
			"  border-top-left-radius: 8px; border-top-right-radius: 8px;"
			"  margin-right: 2px; }"
			"QTabBar::tab:selected { background: #6D28D9; color: #FFFFFF; }"
			"QTabBar::tab:hover:!selected { background: #DDD6FE; }"
			"QToolTip { background: #1E293B; color: #FFFFFF; border: none; padding: 6px; }"
			"QMenu { background: #FFFFFF; border: 1px solid #DDD6FE; padding: 4px; }"
			"QMenu::item { padding: 6px 24px; color: #1E293B; border-radius: 4px; }"
			"QMenu::item:selected { background: #EDE9FE; color: #4C1D95; }"
			"QMenu::separator { height: 1px; background: #DDD6FE; margin: 4px 8px; }"
			"QStatusBar { background: #F8FAFC; color: #1E293B; }"
			"QStatusBar::item { border: none; }"
			"QToolBar { background: #F1F5F9; border-bottom: 1px solid #DDD6FE; spacing: 4px; padding: 4px; }"
			"QToolButton { color: #1E293B; padding: 6px 10px; border-radius: 6px; }"
			"QToolButton:hover { background: #EDE9FE; }"
			"QToolButton:checked { background: #6D28D9; color: #FFFFFF; }"
			"QGroupBox { border: 1px solid #DDD6FE; border-radius: 10px; margin-top: 14px;"
			"  padding-top: 18px; font-weight: 600; color: #4C1D95; }"
			"QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }"
			"QCheckBox, QRadioButton { color: #1E293B; spacing: 8px; }"
			"QCheckBox::indicator, QRadioButton::indicator { width: 16px; height: 16px; }"
			"QCheckBox::indicator:checked, QRadioButton::indicator:checked {"
			"  background: #6D28D9; border: 1px solid #6D28D9; border-radius: 4px; }"
			"QProgressBar { background: #E2E8F0; border: none; border-radius: 6px;"
			"  text-align: center; color: #1E293B; }"
			"QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
			"  stop:0 #8B5CF6, stop:1 #6D28D9); border-radius: 6px; }"
			"QScrollBar:vertical { background: #F8FAFC; width: 10px; margin: 0; }"
			"QScrollBar::handle:vertical { background: #C4B5FD; border-radius: 5px;"
			"  min-height: 30px; }"
			"QScrollBar::handle:vertical:hover { background: #8B5CF6; }"
			"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
			"QScrollBar:horizontal { background: #F8FAFC; height: 10px; margin: 0; }"
			"QScrollBar::handle:horizontal { background: #C4B5FD; border-radius: 5px;"
			"  min-width: 30px; }"
			"QScrollBar::handle:horizontal:hover { background: #8B5CF6; }"
			"QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"
			"QListWidget, QTreeWidget, QTableWidget { background: #FFFFFF;"
			"  border: 1px solid #DDD6FE; border-radius: 8px; color: #1E293B; }"
			"QListWidget::item:selected, QTreeWidget::item:selected, QTableWidget::item:selected {"
			"  background: #EDE9FE; color: #4C1D95; }"
			"QSplitter::handle { background: #DDD6FE; width: 4px; height: 4px; }"
			"QDialogButtonBox QPushButton { min-width: 80px; }"
			"QMessageBox QLabel { color: #1E293B; }" );
	}
}
