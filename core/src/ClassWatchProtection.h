/*
 * ClassWatchProtection.h - anti-tamper and anti-debugging guards for
 * ClassWatch applications
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

#include <QObject>

#include "VeyonCore.h"

// Raises the bar against casual cracking: detects attached debuggers,
// common analysis tools and license-file tampering, and can be polled
// periodically. Determined attackers with kernel drivers can still
// bypass user-mode checks - server-side seat limits and license
// expiry remain the real enforcement.
class VEYON_CORE_EXPORT ClassWatchProtection : public QObject
{
	Q_OBJECT
public:
	explicit ClassWatchProtection( QObject* parent = nullptr );

	// one-shot checks - call at startup
	static bool debuggerDetected();
	static bool analysisToolDetected();
	static bool licenseFilesIntact();

	// periodic guard - call once after QApplication exists; checks run
	// on an internal timer and the app exits quietly on detection
	void installPeriodicChecks();

private Q_SLOTS:
	void check();

private:
	int m_checksWithoutHit = 0;
};
