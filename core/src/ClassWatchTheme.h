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
#include <QTimer>
#include <QtMath>
#include <QWidget>

namespace ClassWatchTheme
{
	// soft watery palette - readable, no harsh contrast
	inline const QColor kDeepWater( 14, 116, 144 );     // dark teal-blue accent text
	inline const QColor kWater( 14, 165, 233 );         // sky blue accent
	inline const QColor kPaleWater( 186, 230, 253 );    // pale blue
	inline const QColor kMist( 240, 249, 255 );         // near-white blue

	inline QString wateryGradient( double t )
	{
		// slowly drifting "water" gradient - t in [0,1)
		const auto mix = []( const QColor& a, const QColor& b, double f ) {
			return QColor::fromRgbF( a.redF() + ( b.redF() - a.redF() ) * f,
								 a.greenF() + ( b.greenF() - a.greenF() ) * f,
								 a.blueF() + ( b.blueF() - a.blueF() ) * f );
		};
		const double w = 0.5 + 0.5 * qSin( t * 6.28318 );
		const QColor top = mix( kPaleWater, kMist, w * 0.6 );
		const QColor mid = mix( QColor( 224, 242, 254 ), kPaleWater, w * 0.5 );
		const QColor bottom = mix( kMist, QColor( 255, 255, 255 ), w * 0.4 );
		return QStringLiteral(
			"qlineargradient(x1:0, y1:0, x2:0.3, y2:1,"
			" stop:0 %1, stop:0.5 %2, stop:1 %3)" )
			.arg( top.name(), mid.name(), bottom.name() );
	}

	inline void apply( QApplication* app )
	{
		if( app == nullptr )
		{
			return;
		}

		QPalette palette = app->palette();
		palette.setColor( QPalette::Window, QColor( 0xf0f9ff ) );
		palette.setColor( QPalette::WindowText, kDeepWater );
		palette.setColor( QPalette::Base, Qt::white );
		palette.setColor( QPalette::AlternateBase, QColor( 0xe0f2fe ) );
		palette.setColor( QPalette::Text, kDeepWater );
		palette.setColor( QPalette::Button, QColor( 0xbae6fd ) );
		palette.setColor( QPalette::ButtonText, QColor( 0x164e63 ) );
		palette.setColor( QPalette::Highlight, QColor( 0x38bdf8 ) );
		palette.setColor( QPalette::HighlightedText, Qt::white );
		palette.setColor( QPalette::ToolTipBase, kDeepWater );
		palette.setColor( QPalette::ToolTipText, Qt::white );
		palette.setColor( QPalette::PlaceholderText, QColor( 0x64748b ) );
		app->setPalette( palette );

		app->setStyleSheet( QStringLiteral(
			"QMainWindow, QDialog {"
			"  background: %1; }"
			"QMenuBar {"
			"  background: rgba(224,242,254,235); color: #164e63;"
			"  border-bottom: 1px solid #bae6fd; padding: 2px; }"
			"QMenuBar::item { padding: 6px 12px; border-radius: 6px; }"
			"QMenuBar::item:selected { background: #bae6fd; color: #164e63; }"
			"QGroupBox {"
			"  border: 1px solid #bae6fd; border-radius: 12px;"
			"  margin-top: 14px; padding-top: 10px;"
			"  background: rgba(255,255,255,225); }"
			"QGroupBox::title {"
			"  subcontrol-origin: margin; left: 12px; padding: 0 6px;"
			"  color: #0369a1; font-weight: bold; }"
			"QPushButton {"
			"  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
			"    stop:0 #7dd3fc, stop:1 #38bdf8);"
			"  color: #082f49; border: 1px solid rgba(255,255,255,160);"
			"  border-radius: 8px; padding: 7px 16px; font-weight: bold; }"
			"QPushButton:hover {"
			"  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
			"    stop:0 #38bdf8, stop:1 #0ea5e9);"
			"  border-color: #0284c7; }"
			"QPushButton:pressed { background: #0284c7; color: white; }"
			"QPushButton:disabled { background: #e2e8f0; color: #94a3b8; border-color: #cbd5e1; }"
			"QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateTimeEdit {"
			"  background: rgba(255,255,255,240); border: 1px solid #bae6fd;"
			"  border-radius: 7px; padding: 5px; color: #164e63;"
			"  selection-background-color: #bae6fd; }"
			"QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDateTimeEdit:focus {"
			"  border: 1px solid #38bdf8; }"
			"QTextEdit, QPlainTextEdit {"
			"  background: rgba(255,255,255,235); border: 1px solid #bae6fd;"
			"  border-radius: 7px; padding: 5px; color: #164e63; }"
			"QTableWidget, QTreeWidget, QTableView, QListWidget, QUndoView {"
			"  background: rgba(255,255,255,235); alternate-background-color: #f0f9ff;"
			"  border: 1px solid #bae6fd; border-radius: 8px;"
			"  color: #164e63; gridline-color: #e0f2fe; selection-background-color: #bae6fd; }"
			"QHeaderView::section {"
			"  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
			"    stop:0 #e0f2fe, stop:1 #bae6fd);"
			"  color: #0369a1; border: none; border-bottom: 1px solid #bae6fd;"
			"  padding: 6px; font-weight: bold; }"
			"QTabBar::tab {"
			"  background: #e0f2fe; color: #0369a1; padding: 9px 20px;"
			"  border-top-left-radius: 8px; border-top-right-radius: 8px;"
			"  margin-right: 3px; }"
			"QTabBar::tab:selected { background: #38bdf8; color: white; }"
			"QTabBar::tab:hover:!selected { background: #bae6fd; }"
			"QToolTip { background: #164e63; color: white; border: none; padding: 5px; border-radius: 4px; }"
			"QMenu { background: rgba(255,255,255,245); border: 1px solid #bae6fd;"
			"  border-radius: 8px; padding: 5px; }"
			"QMenu::item { padding: 7px 22px; border-radius: 5px; color: #164e63; }"
			"QMenu::item:selected { background: #e0f2fe; color: #0369a1; }"
			"QMenu::separator { height: 1px; background: #e0f2fe; margin: 4px 8px; }"
			"QStatusBar { background: rgba(224,242,254,220); color: #0369a1; border-top: 1px solid #bae6fd; }"
			"QSplitter::handle { background: #bae6fd; }"
			"QSplitter::handle:hover { background: #38bdf8; }"
			"QScrollBar:vertical { background: transparent; width: 11px; margin: 0; }"
			"QScrollBar::handle:vertical { background: #bae6fd; border-radius: 6px; min-height: 32px; }"
			"QScrollBar::handle:vertical:hover { background: #38bdf8; }"
			"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
			"QScrollBar:horizontal { background: transparent; height: 11px; margin: 0; }"
			"QScrollBar::handle:horizontal { background: #bae6fd; border-radius: 6px; min-width: 32px; }"
			"QScrollBar::handle:horizontal:hover { background: #38bdf8; }"
			"QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"
			"QProgressBar { background: #e0f2fe; border: 1px solid #bae6fd; border-radius: 7px;"
			"  text-align: center; color: #0369a1; }"
			"QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
			"  stop:0 #38bdf8, stop:1 #0284c7); border-radius: 6px; }"
			"QCheckBox, QRadioButton { color: #164e63; spacing: 7px; }"
			"QCheckBox::indicator, QRadioButton::indicator {"
			"  width: 16px; height: 16px; border: 1px solid #bae6fd; border-radius: 4px;"
			"  background: rgba(255,255,255,220); }"
			"QCheckBox::indicator:checked, QRadioButton::indicator:checked {"
			"  background: #38bdf8; border-color: #0284c7; }"
			"QToolBar { background: rgba(224,242,254,220); border: none;"
			"  border-bottom: 1px solid #bae6fd; spacing: 5px; padding: 5px; }"
			"QToolButton { padding: 6px 10px; border-radius: 6px; color: #164e63; }"
			"QToolButton:hover { background: #bae6fd; }"
			"QDockWidget::title { background: #e0f2fe; color: #0369a1; padding: 7px; font-weight: bold; }"
			"QListView::item:selected, QTreeView::item:selected { background: #bae6fd; color: #164e63; }"
			"QMessageBox { background: %1; }"
			"QMessageBox QLabel { color: #164e63; }"
			"QMessageBox QPushButton {"
			"  background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #7dd3fc, stop:1 #38bdf8);"
			"  color: #082f49; border-radius: 8px; padding: 7px 18px; font-weight: bold; min-width: 80px; }"
			"QMessageBox QPushButton:hover { background: #38bdf8; }"
		).arg( wateryGradient( 0.0 ) ) );
	}

	// Slowly drifting "watery" animation for a top-level window.
	// Call after the window's central widget has been made transparent so the
	// animated gradient shows through the whole client area.
	inline void startWateryAnimation( QWidget* window, int intervalMs = 90 )
	{
		if( window == nullptr )
		{
			return;
		}

		auto* phase = new double( 0.0 );
		auto* timer = new QTimer( window );
		timer->setInterval( intervalMs );

		if( window->objectName().isEmpty() )
		{
			window->setObjectName( QStringLiteral( "ClassWatchWateryWindow" ) );
		}
		const auto selector = QStringLiteral( "#%1" ).arg( window->objectName() );

		QObject::connect( timer, &QTimer::timeout, window, [window, phase, selector]() {
			*phase += 0.012;
			if( *phase >= 1.0 )
			{
				*phase -= 1.0;
			}
			window->setStyleSheet( QStringLiteral( "%1 { background: %2; }" )
				.arg( selector, wateryGradient( *phase ) ) );
		} );

		timer->start();
	}
}
