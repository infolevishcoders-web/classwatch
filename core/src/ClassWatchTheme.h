/*
 * ClassWatchTheme.h - ClassWatch purple theme (light surfaces, violet accents)
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
	// ClassWatch palette - light surfaces with violet/purple accents
	//   primary #6D28D9 - secondary #8B5CF6 - background #F8FAFC - text #1E293B
	inline const QColor kVioletDeep( 76, 29, 149 );       // dark violet accent text
	inline const QColor kViolet( 109, 40, 217 );          // primary violet accent
	inline const QColor kVioletSoft( 196, 181, 253 );    // soft violet highlight
	inline const QColor kPanel( 255, 255, 255 );          // panel surface
	inline const QColor kCanvas( 248, 250, 252 );         // app background
	inline const QColor kInk( 30, 41, 59 );               // primary text

	inline QString wateryGradient( double t )
	{
		// slowly drifting "aurora" gradient - t in [0,1)
		const auto mix = []( const QColor& a, const QColor& b, double f ) {
			return QColor::fromRgbF( a.redF() + ( b.redF() - a.redF() ) * f,
								 a.greenF() + ( b.greenF() - a.greenF() ) * f,
								 a.blueF() + ( b.blueF() - a.blueF() ) * f );
		};
		const double w = 0.5 + 0.5 * qSin( t * 6.28318 );
		const QColor top = mix( kCanvas, QColor( 245, 243, 255 ), w * 0.8 );
		const QColor mid = mix( QColor( 237, 233, 254 ), kPanel, w * 0.5 );
		const QColor bottom = mix( kPanel, QColor( 250, 245, 255 ), w * 0.4 );
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
		palette.setColor( QPalette::Window, kCanvas );
		palette.setColor( QPalette::WindowText, kInk );
		palette.setColor( QPalette::Base, kPanel );
		palette.setColor( QPalette::AlternateBase, QColor( 0xf1f5f9 ) );
		palette.setColor( QPalette::Text, kInk );
		palette.setColor( QPalette::Button, QColor( 0xe2e8f0 ) );
		palette.setColor( QPalette::ButtonText, kInk );
		palette.setColor( QPalette::Highlight, kViolet );
		palette.setColor( QPalette::HighlightedText, Qt::white );
		palette.setColor( QPalette::ToolTipBase, kInk );
		palette.setColor( QPalette::ToolTipText, Qt::white );
		palette.setColor( QPalette::PlaceholderText, QColor( 0x94a3b8 ) );
		palette.setColor( QPalette::Disabled, QPalette::Text, QColor( 0x94a3b8 ) );
		palette.setColor( QPalette::Disabled, QPalette::ButtonText, QColor( 0x94a3b8 ) );
		app->setPalette( palette );

		app->setStyleSheet( QStringLiteral(
			"QWidget { background: #F8FAFC; color: #1E293B; }"
			"QMainWindow, QDialog { background: #F8FAFC; color: #1E293B; }"
			"QLabel { color: #1E293B; background: transparent; }"
			"QMenuBar {"
			"  background: #FFFFFF; color: #1E293B;"
			"  border-bottom: 1px solid #DDD6FE; padding: 2px; }"
			"QMenuBar::item { padding: 6px 12px; border-radius: 6px; color: #1E293B; }"
			"QMenuBar::item:selected { background: #EDE9FE; color: #4C1D95; }"
			"QGroupBox {"
			"  border: 1px solid #DDD6FE; border-radius: 12px;"
			"  margin-top: 14px; padding-top: 10px;"
			"  background: #FFFFFF; color: #4C1D95; }"
			"QGroupBox::title {"
			"  subcontrol-origin: margin; left: 12px; padding: 0 6px;"
			"  color: #4C1D95; font-weight: bold; }"
			"QPushButton {"
			"  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
			"    stop:0 #8B5CF6, stop:1 #6D28D9);"
			"  color: #FFFFFF; border: none;"
			"  border-radius: 8px; padding: 7px 16px; font-weight: 600; }"
			"QPushButton:hover { background: #7C3AED; }"
			"QPushButton:pressed { background: #5B21B6; color: white; }"
			"QPushButton:disabled { background: #CBD5E1; color: #64748B; }"
			"QPushButton:flat { background: transparent; color: #6D28D9; }"
			"QPushButton:flat:hover { background: #EDE9FE; }"
			"QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateTimeEdit {"
			"  background: #FFFFFF; border: 1px solid #C7D2FE;"
			"  border-radius: 8px; padding: 6px 8px; color: #1E293B;"
			"  selection-background-color: #8B5CF6; }"
			"QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDateTimeEdit:focus {"
			"  border: 2px solid #8B5CF6; }"
			"QTextEdit, QPlainTextEdit {"
			"  background: #FFFFFF; border: 1px solid #C7D2FE;"
			"  border-radius: 8px; padding: 6px 8px; color: #1E293B;"
			"  selection-background-color: #8B5CF6; }"
			"QTableWidget, QTreeWidget, QTableView, QListWidget, QUndoView {"
			"  background: #FFFFFF; alternate-background-color: #F8FAFC;"
			"  border: 1px solid #DDD6FE; border-radius: 8px;"
			"  color: #1E293B; gridline-color: #E2E8F0; selection-background-color: #EDE9FE;"
			"  selection-color: #4C1D95; }"
			"QHeaderView::section {"
			"  background: #EDE9FE; color: #4C1D95; border: none;"
			"  border-bottom: 1px solid #DDD6FE;"
			"  padding: 6px; font-weight: 600; }"
			"QTabBar::tab {"
			"  background: #EDE9FE; color: #4C1D95; padding: 9px 20px;"
			"  border-top-left-radius: 8px; border-top-right-radius: 8px;"
			"  margin-right: 3px; }"
			"QTabBar::tab:selected { background: #6D28D9; color: #FFFFFF; }"
			"QTabBar::tab:hover:!selected { background: #DDD6FE; }"
			"QToolTip { background: #1E293B; color: #FFFFFF; border: none; padding: 5px; border-radius: 4px; }"
			"QMenu { background: #FFFFFF; border: 1px solid #DDD6FE;"
			"  border-radius: 8px; padding: 5px; }"
			"QMenu::item { padding: 7px 22px; border-radius: 5px; color: #1E293B; }"
			"QMenu::item:selected { background: #EDE9FE; color: #4C1D95; }"
			"QMenu::separator { height: 1px; background: #DDD6FE; margin: 4px 8px; }"
			"QStatusBar { background: #F8FAFC; color: #1E293B; border-top: 1px solid #DDD6FE; }"
			"QSplitter::handle { background: #DDD6FE; }"
			"QSplitter::handle:hover { background: #8B5CF6; }"
			"QScrollBar:vertical { background: transparent; width: 11px; margin: 0; }"
			"QScrollBar::handle:vertical { background: #C4B5FD; border-radius: 6px; min-height: 32px; }"
			"QScrollBar::handle:vertical:hover { background: #8B5CF6; }"
			"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
			"QScrollBar:horizontal { background: transparent; height: 11px; margin: 0; }"
			"QScrollBar::handle:horizontal { background: #C4B5FD; border-radius: 6px; min-width: 32px; }"
			"QScrollBar::handle:horizontal:hover { background: #8B5CF6; }"
			"QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"
			"QProgressBar { background: #E2E8F0; border: none; border-radius: 7px;"
			"  text-align: center; color: #1E293B; }"
			"QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
			"  stop:0 #8B5CF6, stop:1 #6D28D9); border-radius: 6px; }"
			"QCheckBox, QRadioButton { color: #1E293B; spacing: 7px; }"
			"QCheckBox::indicator, QRadioButton::indicator {"
			"  width: 16px; height: 16px; border: 1px solid #C7D2FE; border-radius: 4px;"
			"  background: #FFFFFF; }"
			"QCheckBox::indicator:checked, QRadioButton::indicator:checked {"
			"  background: #6D28D9; border-color: #6D28D9; }"
			"QToolBar { background: #F1F5F9; border: none;"
			"  border-bottom: 1px solid #DDD6FE; spacing: 5px; padding: 5px; }"
			"QToolButton { padding: 6px 10px; border-radius: 6px; color: #1E293B; }"
			"QToolButton:hover { background: #EDE9FE; color: #4C1D95; }"
			"QToolButton:checked { background: #6D28D9; color: white; }"
			"QDockWidget::title { background: #EDE9FE; color: #4C1D95; padding: 7px; font-weight: bold; }"
			"QListView::item:selected, QTreeView::item:selected { background: #EDE9FE; color: #4C1D95; }"
			"QMessageBox { background: #F8FAFC; }"
			"QMessageBox QLabel { color: #1E293B; }"
			"QMessageBox QPushButton {"
			"  background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #8B5CF6, stop:1 #6D28D9);"
			"  color: #FFFFFF; border-radius: 8px; padding: 7px 18px; font-weight: 600; min-width: 80px; }"
			"QMessageBox QPushButton:hover { background: #7C3AED; }"
		).arg( wateryGradient( 0.0 ) ) );
	}

	// Returns an icon that stays visible on the light theme: near-white
	// grayscale glyph pixels are darkened to the text color while colored
	// icons pass through unchanged.
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
					const auto r = qRed( pixel );
					const auto g = qGreen( pixel );
					const auto b = qBlue( pixel );
					const auto lum = 0.299 * r + 0.587 * g + 0.114 * b;
					const auto spread = qMax( r, qMax( g, b ) ) - qMin( r, qMin( g, b ) );
					if( lum > 200.0 && spread < 30 )
					{
						// near-white gray glyph -> darken to theme text color
						image.setPixel( x, y, qRgba( 0x1e, 0x29, 0x3b, alpha ) );
					}
				}
			}
			result.addPixmap( QPixmap::fromImage( image ) );
		}
		return result;
	}

	// Slowly drifting "aurora" animation for a top-level window.
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
