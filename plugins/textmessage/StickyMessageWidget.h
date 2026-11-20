/*
 * StickyMessageWidget.h - always-on-top announcement banner shown on
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

#pragma once

#include <QWidget>

class QLabel;

class StickyMessageWidget : public QWidget
{
	Q_OBJECT
public:
	StickyMessageWidget( const QString& title, const QString& html, QWidget* parent = nullptr );

protected:
	void showEvent( QShowEvent* event ) override;

private:
	void positionOnScreen();

	QLabel* m_titleLabel;
	QLabel* m_bodyLabel;
};
