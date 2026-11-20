/*
 * StickyMessageWidget.cpp - always-on-top announcement banner shown on
 * student screens; only the teacher can remove it
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

#include "StickyMessageWidget.h"

#include <QCursor>
#include <QGuiApplication>
#include <QLabel>
#include <QScreen>
#include <QVBoxLayout>

#include "PlatformCoreFunctions.h"
#include "VeyonCore.h"


StickyMessageWidget::StickyMessageWidget( const QString& title, const QString& html,
										  QWidget* parent ) :
	QWidget( parent,
			 Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool |
			 Qt::WindowTransparentForInput ),
	m_titleLabel( new QLabel( this ) ),
	m_bodyLabel( new QLabel( this ) )
{
	setAttribute( Qt::WA_TranslucentBackground );
	setWindowTitle( {} );
	setStyleSheet( QStringLiteral(
		"StickyMessageWidget {"
		"  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
		"    stop:0 rgba(109,40,217,235), stop:1 rgba(76,29,149,235));"
		"  border: 1px solid rgba(196,181,253,200);"
		"  border-radius: 16px;"
		"}"
		"QLabel#cwTitle { color: #ddd6fe; font-size: 12px; font-weight: bold;"
		"  letter-spacing: 2px; }"
		"QLabel#cwBody { color: #ffffff; font-size: 15px; }" ) );

	m_titleLabel->setObjectName( QStringLiteral("cwTitle") );
	m_titleLabel->setText( title.toUpper() );
	m_titleLabel->setAlignment( Qt::AlignCenter );

	m_bodyLabel->setObjectName( QStringLiteral("cwBody") );
	m_bodyLabel->setTextFormat( Qt::RichText );
	m_bodyLabel->setTextInteractionFlags( Qt::NoTextInteraction );
	m_bodyLabel->setWordWrap( true );
	m_bodyLabel->setAlignment( Qt::AlignCenter );
	m_bodyLabel->setText( html );

	auto* layout = new QVBoxLayout( this );
	layout->setContentsMargins( 22, 14, 22, 16 );
	layout->setSpacing( 6 );
	layout->addWidget( m_titleLabel );
	layout->addWidget( m_bodyLabel );

	adjustSize();
}


void StickyMessageWidget::positionOnScreen()
{
	auto* screen = QGuiApplication::screenAt( QCursor::pos() );
	if( screen == nullptr )
	{
		screen = QGuiApplication::primaryScreen();
	}
	if( screen == nullptr )
	{
		return;
	}

	const auto geometry = screen->geometry();
	const int maxWidth = qMin( 720, geometry.width() - 80 );
	if( width() > maxWidth )
	{
		setFixedWidth( maxWidth );
		adjustSize();
	}

	move( geometry.center().x() - width() / 2, geometry.top() + 28 );
}


void StickyMessageWidget::showEvent( QShowEvent* event )
{
	QWidget::showEvent( event );
	positionOnScreen();
	VeyonCore::platform().coreFunctions().raiseWindow( this, true );
}
