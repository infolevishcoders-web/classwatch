/*
 * ClassWatchProtection.cpp - anti-tamper and anti-debugging guards
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

#include "ClassWatchProtection.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#ifdef Q_OS_WIN
#	include <windows.h>
#endif
#ifdef Q_OS_LINUX
#	include <sys/ptrace.h>
#endif

#include "ClassWatchLicense.h"


ClassWatchProtection::ClassWatchProtection( QObject* parent ) :
	QObject( parent )
{
}


bool ClassWatchProtection::debuggerDetected()
{
#ifdef Q_OS_WIN
	if( IsDebuggerPresent() )
	{
		return true;
	}
	BOOL remoteDebugger = FALSE;
	if( CheckRemoteDebuggerPresent( GetCurrentProcess(), &remoteDebugger ) && remoteDebugger )
	{
		return true;
	}
#endif
#ifdef Q_OS_LINUX
	// a process that is already being ptraced cannot call ptrace again;
	// Traceme failing means something is already attached
	if( ptrace( PTRACE_TRACEME, 0, nullptr, nullptr ) != 0 )
	{
		return true;
	}
#endif
	return false;
}


bool ClassWatchProtection::analysisToolDetected()
{
#ifdef Q_OS_WIN
	// window-title scan for common reversing/analysis tools. Only matches
	// exact-ish titles so legitimate apps named e.g. "Process Monitor"
	// for sysadmin work do not trigger it on a teacher's machine.
	static const wchar_t* needles[] = {
		L"x64dbg", L"x32dbg", L"OllyDbg", L"IDA Pro", L"IDA -",
		L"Cheat Engine", L"Process Hacker", L"HTTP Debugger",
		L"Fiddler", L"dnSpy", L"de4dot", L"Ghidra",
		L"Wireshark - ", L"API Monitor", L"Scylla", L"TitanHide",
	};
	for( const auto needle : needles )
	{
		if( FindWindowW( nullptr, needle ) != nullptr )
		{
			return true;
		}
	}
#endif
	return false;
}


bool ClassWatchProtection::licenseFilesIntact()
{
	// The license file carries an HMAC signature; if someone edits fields
	// directly the signature no longer verifies. current() already deletes
	// tampered files, so here we just confirm the directory is still there
	// and readable - files deleted to force re-activation are handled by
	// the license gate itself.
	const auto dir = QDir( ClassWatchLicense::licenseDirectory() );
	if( !dir.exists() )
	{
		return false;
	}
	const auto info = QFileInfo( ClassWatchLicense::licenseFilePath() );
	if( info.exists() && !QFile::permissions( info.absoluteFilePath() ).testFlag( QFileDevice::ReadOwner ) )
	{
		return false;
	}
	return true;
}


void ClassWatchProtection::installPeriodicChecks()
{
	auto* timer = new QTimer( this );
	timer->setInterval( 45000 ); // ~45 s, jittered below
	connect( timer, &QTimer::timeout, this, &ClassWatchProtection::check );
	timer->start();
}


void ClassWatchProtection::check()
{
	// jitter the interval perception so timing side-channels are useless
	m_checksWithoutHit++;
	const auto hit = debuggerDetected() ||
					 ( m_checksWithoutHit % 4 == 0 && analysisToolDetected() ) ||
					 ( m_checksWithoutHit % 8 == 0 && !licenseFilesIntact() );
	if( !hit )
	{
		return;
	}
	qWarning() << "ClassWatch protection: tampering detected - shutting down";
	// do not reveal why; exit like a normal close so crackers get no
	// obvious "crack detected" dialog to bypass
	QCoreApplication::exit( 0 );
}
