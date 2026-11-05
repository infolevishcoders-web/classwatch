/*
 * ClassWatchComputer.cpp - implementation of ClassWatchComputer class
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

#include "ClassWatchComputer.h"

ClassWatchComputer::ClassWatchComputer( const Computer& computer ) :
	Computer( computer ),
	m_classWatchId(),
	m_windowsVersion(),
	m_loggedInUser(),
	m_domainName(),
	m_connectionStatus(),
	m_lastConnectionTime(),
	m_sessionType(),
	m_cpuInfo(),
	m_ramInfo(),
	m_gpuInfo()
{
}
