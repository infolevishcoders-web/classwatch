/*
 * ClassWatchTheme.h - ClassWatch "ember" dark theme (black & red)
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
#include <QIcon>
#include <QImage>
#include <QPalette>
#include <QPixmap>
#include <QTimer>
#include <QtMath>
#include <QWidget>

namespace ClassWatchTheme
{
	// ember palette - deep black surfaces with crimson/red accents
	inline const QColor kEmberDeep( 127, 29, 29 );      // dark red accent text
	inline const QColor kEmber( 220, 38, 38 );          // primary red accent
	inline const QColor kEmberSoft( 252, 165, 165 );    // soft red highlight
	inline const QColor kAsh( 24, 24, 27 );             // near-black panel
	inline const QColor kCoal( 9, 9, 11 );              // deepest background

	inline QString wateryGradient( double t )
	{
		// slowly drifting "ember" gradient - t in [0,1)
		const auto mix = []( const QColor& a, const QColor& b, double f ) {
			return QColor::fromRgbF( a.redF() + ( b.redF() - a.redF() ) * f,
								 a.greenF() + ( b.greenF() - a.greenF() ) * f,
								 a.blueF() + ( b.blueF() - a.blueF() ) * f );
		};
		const double w = 0.5 + 0.5 * qSin( t * 6.28318 );
		const QColor top = mix( kCoal, QColor( 24, 0, 0 ), w * 0.7 );
		const QColor mid = mix( QColor( 40, 4, 4 ), kAsh, w * 0.5 );
		const QColor bottom = mix( kAsh, QColor( 10, 0, 0 ), w * 0.4 );
		return QStringLiteral(
			"qlineargradient(x1:0, y1:0, x2:0.35, y2:1,"
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
		palette.setColor( QPalette::Window, kCoal );
		palette.setColor( QPalette::WindowText, QColor( 0xf4f4f5 ) );
		palette.setColor( QPalette::Base, QColor( 0x18181b ) );
		palette.setColor( QPalette::AlternateBase, QColor( 0x27272a ) );
		palette.setColor( QPalette::Text, QColor( 0xf4f4f5 ) );
		palette.setColor( QPalette::Button, QColor( 0x3f3f46 ) );
		palette.setColor( QPalette::ButtonText, QColor( 0xf4f4f5 ) );
		palette.setColor( QPalette::Highlight, kEmber );
		palette.setColor( QPalette::HighlightedText, Qt::white );
		palette.setColor( QPalette::ToolTipBase, kEmberDeep );
		palette.setColor( QPalette::ToolTipText, Qt::white );
		palette.setColor( QPalette::PlaceholderText, QColor( 0xa1a1aa ) );
		palette.setColor( QPalette::Disabled, QPalette::Text, QColor( 0x71717a ) );
		palette.setColor( QPalette::Disabled, QPalette::ButtonText, QColor( 0x71717a ) );
		app->setPalette( palette );

		app->setStyleSheet( QStringLiteral(
			"QMainWindow, QDialog {"
			"  background: %1; color: #f4f4f5; }"
			"QMenuBar {"
			"  background: rgba(24,0,0,235); color: #fecaca;"
			"  border-bottom: 1px solid #7f1d1d; padding: 2px; }"
			"QMenuBar::item { padding: 6px 12px; border-radius: 6px; }"
			"QMenuBar::item:selected { background: #7f1d1d; color: #fecaca; }"
			"QGroupBox {"
			"  border: 1px solid #7f1d1d; border-radius: 12px;"
			"  margin-top: 14px; padding-top: 10px;"
			"  background: rgba(39,0,0,60); color: #fecaca; }"
			"QGroupBox::title {"
			"  subcontrol-origin: margin; left: 12px; padding: 0 6px;"
			"  color: #fca5a5; font-weight: bold; }"
			"QPushButton {"
			"  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
			"    stop:0 #dc2626, stop:1 #991b1b);"
			"  color: #fff1f2; border: 1px solid rgba(254,202,202,120);"
			"  border-radius: 8px; padding: 7px 16px; font-weight: bold; }"
			"QPushButton:hover {"
			"  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
			"    stop:0 #ef4444, stop:1 #b91c1c);"
			"  border-color: #fca5a5; }"
			"QPushButton:pressed { background: #7f1d1d; color: white; }"
			"QPushButton:disabled { background: #27272a; color: #71717a; border-color: #3f3f46; }"
			"QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateTimeEdit {"
			"  background: rgba(24,24,27,245); border: 1px solid #7f1d1d;"
			"  border-radius: 7px; padding: 5px; color: #f4f4f5;"
			"  selection-background-color: #7f1d1d; }"
			"QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDateTimeEdit:focus {"
			"  border: 1px solid #dc2626; }"
			"QTextEdit, QPlainTextEdit {"
			"  background: rgba(24,24,27,240); border: 1px solid #7f1d1d;"
			"  border-radius: 7px; padding: 5px; color: #f4f4f5; }"
			"QTableWidget, QTreeWidget, QTableView, QListWidget, QUndoView {"
			"  background: rgba(24,24,27,235); alternate-background-color: #1c0a0a;"
			"  border: 1px solid #7f1d1d; border-radius: 8px;"
			"  color: #f4f4f5; gridline-color: #3f1d1d; selection-background-color: #7f1d1d; }"
			"QHeaderView::section {"
			"  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
			"    stop:0 #3f1d1d, stop:1 #7f1d1d);"
			"  color: #fecaca; border: none; border-bottom: 1px solid #991b1b;"
			"  padding: 6px; font-weight: bold; }"
			"QTabBar::tab {"
			"  background: #3f1d1d; color: #fecaca; padding: 9px 20px;"
			"  border-top-left-radius: 8px; border-top-right-radius: 8px;"
			"  margin-right: 3px; }"
			"QTabBar::tab:selected { background: #dc2626; color: white; }"
			"QTabBar::tab:hover:!selected { background: #7f1d1d; }"
			"QToolTip { background: #450a0a; color: #fecaca; border: 1px solid #7f1d1d; padding: 5px; border-radius: 4px; }"
			"QMenu { background: rgba(24,0,0,250); border: 1px solid #7f1d1d;"
			"  border-radius: 8px; padding: 5px; }"
			"QMenu::item { padding: 7px 22px; border-radius: 5px; color: #f4f4f5; }"
			"QMenu::item:selected { background: #7f1d1d; color: #fecaca; }"
			"QMenu::separator { height: 1px; background: #3f1d1d; margin: 4px 8px; }"
			"QStatusBar { background: rgba(24,0,0,225); color: #fca5a5; border-top: 1px solid #7f1d1d; }"
			"QSplitter::handle { background: #7f1d1d; }"
			"QSplitter::handle:hover { background: #dc2626; }"
			"QScrollBar:vertical { background: transparent; width: 11px; margin: 0; }"
			"QScrollBar::handle:vertical { background: #7f1d1d; border-radius: 6px; min-height: 32px; }"
			"QScrollBar::handle:vertical:hover { background: #dc2626; }"
			"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
			"QScrollBar:horizontal { background: transparent; height: 11px; margin: 0; }"
			"QScrollBar::handle:horizontal { background: #7f1d1d; border-radius: 6px; min-width: 32px; }"
			"QScrollBar::handle:horizontal:hover { background: #dc2626; }"
			"QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"
			"QProgressBar { background: #3f1d1d; border: 1px solid #7f1d1d; border-radius: 7px;"
			"  text-align: center; color: #fecaca; }"
			"QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
			"  stop:0 #dc2626, stop:1 #7f1d1d); border-radius: 6px; }"
			"QCheckBox, QRadioButton { color: #f4f4f5; spacing: 7px; }"
			"QCheckBox::indicator, QRadioButton::indicator {"
			"  width: 16px; height: 16px; border: 1px solid #7f1d1d; border-radius: 4px;"
			"  background: rgba(24,24,27,220); }"
			"QCheckBox::indicator:checked, QRadioButton::indicator:checked {"
			"  background: #dc2626; border-color: #fca5a5; }"
			"QToolBar { background: rgba(24,0,0,220); border: none;"
			"  border-bottom: 1px solid #7f1d1d; spacing: 5px; padding: 5px; }"
			"QToolButton { padding: 6px 10px; border-radius: 6px; color: #f4f4f5; }"
			"QToolButton:hover { background: #7f1d1d; color: #fecaca; }"
			"QToolButton:checked { background: #991b1b; color: white; }"
			"QDockWidget::title { background: #3f1d1d; color: #fecaca; padding: 7px; font-weight: bold; }"
			"QListView::item:selected, QTreeView::item:selected { background: #7f1d1d; color: #fecaca; }"
			"QMessageBox { background: %1; }"
			"QMessageBox QLabel { color: #f4f4f5; }"
			"QMessageBox QPushButton {"
			"  background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #dc2626, stop:1 #991b1b);"
			"  color: #fff1f2; border-radius: 8px; padding: 7px 18px; font-weight: bold; min-width: 80px; }"
			"QMessageBox QPushButton:hover { background: #ef4444; }"
		).arg( wateryGradient( 0.0 ) ) );
	}

	// Returns an icon that stays visible on the dark theme: dark glyph pixels
	// are lifted to a light tone while colored/light icons pass through.
	inline QIcon darkAwareIcon( const QString& url )
	{
		const QIcon source( url );
		if( source.isNull() )
		{
			return source;
		}

		QIcon result;
		for( const auto size : { 16, 22, 32, 48, 64 } )
		{
			QImage image = source.pixmap( size, size ).toImage().convertToFormat( QImage::Format_ARGB32 );
			for( int y = 0; y < image.height(); ++y )
			{
				for( int x = 0; x < image.width(); ++x )
				{
					const auto pixel = image.pixel( x, y );
					const auto alpha = qAlpha( pixel );
					if( alpha < 16 )
					{
						continue;
					}
					// perceived luminance of the RGB part
					const auto lum = 0.299 * qRed( pixel ) + 0.587 * qGreen( pixel ) + 0.114 * qBlue( pixel );
					if( lum < 110.0 )
					{
						// dark glyph -> lift to light theme text color, keep alpha
						image.setPixel( x, y, qRgba( 0xf4, 0xf4, 0xf5, alpha ) );
					}
					else if( lum < 170.0 )
					{
						// mid-tone -> brighten so it reads on black
						const auto f = ( 170.0 - lum ) / 170.0 * 0.6 + 0.4;
						image.setPixel( x, y, qRgba(
							qBound( 0, qRound( qRed( pixel ) * f + 80 * ( 1 - f ) ), 255 ),
							qBound( 0, qRound( qGreen( pixel ) * f + 80 * ( 1 - f ) ), 255 ),
							qBound( 0, qRound( qBlue( pixel ) * f + 80 * ( 1 - f ) ), 255 ),
							alpha ) );
					}
				}
			}
			result.addPixmap( QPixmap::fromImage( image ) );
		}
		return result;
	}

	// Slowly drifting "ember" animation for a top-level window.
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
