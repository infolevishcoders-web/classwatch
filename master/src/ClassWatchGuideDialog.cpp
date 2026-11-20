/*
 * ClassWatchGuideDialog.cpp - built-in getting-started guide for
 * ClassWatch Teacher: downloads, installation, classroom setup,
 * monitoring, file transfer and troubleshooting
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

#include "ClassWatchGuideDialog.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QTabWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
	const QString kDownloadPage = QStringLiteral("https://classwatch.netlify.app/download.html");

	QLabel* makeStepsLabel( const QString& html )
	{
		auto* label = new QLabel( html );
		label->setWordWrap( true );
		label->setTextFormat( Qt::RichText );
		label->setStyleSheet( QStringLiteral("color:#1E293B; font-size:13px;") );
		label->setAlignment( Qt::AlignTop | Qt::AlignLeft );
		label->setMargin( 8 );
		return label;
	}

	QWidget* scrollWrap( QWidget* content )
	{
		auto* scroll = new QScrollArea;
		scroll->setWidgetResizable( true );
		scroll->setFrameShape( QFrame::NoFrame );
		scroll->setWidget( content );
		return scroll;
	}
}


ClassWatchGuideDialog::ClassWatchGuideDialog( QWidget* parent ) :
	QDialog( parent ),
	m_tabs( new QTabWidget( this ) )
{
	setWindowTitle( tr( "ClassWatch Guide" ) );
	setMinimumSize( 720, 560 );

	auto* layout = new QVBoxLayout( this );
	layout->addWidget( m_tabs );

	m_tabs->addTab( buildDownloadTab(), tr( "Download" ) );
	m_tabs->addTab( buildInstallTab(), tr( "Install" ) );
	m_tabs->addTab( buildConnectTab(), tr( "Connect PCs" ) );
	m_tabs->addTab( buildMonitorTab(), tr( "Monitor" ) );
	m_tabs->addTab( buildFilesTab(), tr( "Files & Messages" ) );
	m_tabs->addTab( buildTroubleshootingTab(), tr( "Troubleshooting" ) );

	auto* buttons = new QDialogButtonBox( QDialogButtonBox::Close, this );
	connect( buttons, &QDialogButtonBox::rejected, this, &QDialog::reject );
	layout->addWidget( buttons );
}


QWidget* ClassWatchGuideDialog::screenshotLabel( const QString& resource, const QString& caption )
{
	const QPixmap pixmap( resource );
	if( pixmap.isNull() )
	{
		return nullptr;
	}

	auto* container = new QWidget;
	auto* vbox = new QVBoxLayout( container );
	vbox->setContentsMargins( 0, 8, 0, 0 );

	auto* image = new QLabel( container );
	image->setPixmap( pixmap.scaledToWidth( 640, Qt::SmoothTransformation ) );
	image->setAlignment( Qt::AlignCenter );
	image->setStyleSheet( QStringLiteral(
		"border:1px solid #C7D2FE; border-radius:10px; background:#fff; padding:4px;" ) );
	vbox->addWidget( image );

	if( !caption.isEmpty() )
	{
		auto* cap = new QLabel( caption, container );
		cap->setStyleSheet( QStringLiteral("color:#64748B; font-size:11px;") );
		cap->setAlignment( Qt::AlignCenter );
		cap->setWordWrap( true );
		vbox->addWidget( cap );
	}
	return container;
}


QWidget* ClassWatchGuideDialog::buildDownloadTab()
{
	auto* page = new QWidget;
	auto* vbox = new QVBoxLayout( page );

	auto* intro = makeStepsLabel(
		QStringLiteral(
			"<p><b>Get ClassWatch</b> — choose the installer for each computer:</p>"
			"<ul>"
			"<li><b>Windows</b> — <code>classwatch-1.0.0.0-win64-setup.exe</code> (teacher + student PCs)</li>"
			"<li><b>Linux</b> — <code>classwatch_1.0.0.0_amd64.deb</code></li>"
			"<li><b>macOS</b> — <code>classwatch-1.0.0.0-Darwin.dmg</code></li>"
			"</ul>"
			"<p>Installers are on the ClassWatch download page and in the "
			"<code>ready</code> folder on the USB drive. Copy them to every "
			"computer in the classroom — the same file works on all of them.</p>" ) );
	vbox->addWidget( intro );

	auto* openButton = new QPushButton( tr( "Open download page" ), page );
	connect( openButton, &QPushButton::clicked, this, [this]() {
		QDesktopServices::openUrl( QUrl( kDownloadPage ) );
	} );
	vbox->addWidget( openButton );
	vbox->addStretch();
	return scrollWrap( page );
}


QWidget* ClassWatchGuideDialog::buildInstallTab()
{
	auto* page = new QWidget;
	auto* vbox = new QVBoxLayout( page );

	auto* steps = makeStepsLabel(
		QStringLiteral(
			"<p><b>Install on every computer (about 1 minute each):</b></p>"
			"<ol>"
			"<li>Copy <code>classwatch-1.0.0.0-win64-setup.exe</code> to the computer "
			"(USB drive, shared folder, or the download page).</li>"
			"<li>Double-click it and click <b>Next → Install</b>.</li>"
			"<li>The <b>ClassWatch Service</b> starts automatically — no login needed.</li>"
			"<li>On the <b>teacher computer</b> only: open <b>ClassWatch Teacher</b> and "
			"enter your license key when asked (free trial works too).</li>"
			"</ol>"
			"<p><b>Updating?</b> Just run the new installer — classrooms, student PCs, "
			"keys and settings are kept automatically. You never need to reconfigure.</p>"
			"<p><b>Linux:</b> <code>sudo dpkg -i classwatch_1.0.0.0_amd64.deb</code><br/>"
			"<b>macOS:</b> open the <code>.dmg</code> and drag ClassWatch to Applications.</p>" ) );
	vbox->addWidget( steps );

	if( auto* shot = screenshotLabel( QStringLiteral(":/guide/02-install.png"),
									  tr( "The installer — click Next, then Install." ) ) )
	{
		vbox->addWidget( shot );
	}
	vbox->addStretch();
	return scrollWrap( page );
}


QWidget* ClassWatchGuideDialog::buildConnectTab()
{
	auto* page = new QWidget;
	auto* vbox = new QVBoxLayout( page );

	auto* steps = makeStepsLabel(
		QStringLiteral(
			"<p><b>Connect student computers to the teacher:</b></p>"
			"<ol>"
			"<li>On each student PC open Command Prompt (<code>Win+R → cmd</code>) and "
			"type <code>hostname</code> — write down the name (e.g. "
			"<code>DESKTOP-OEN7S1V</code>).</li>"
			"<li>On the teacher PC open <b>ClassWatch Setup → Classrooms &amp; student PCs</b>.</li>"
			"<li>Click <b>+ Add</b> under Classrooms, name it (e.g. <i>Form 1A</i>) and select it.</li>"
			"<li>For each student click <b>+ Add</b>, type a name (e.g. "
			"<i>Student-01</i>) and paste the <b>hostname</b> into "
			"<b>IP / hostname</b>. Leave MAC empty.</li>"
			"<li>Click <b>Apply</b>, close Setup, and open <b>ClassWatch Teacher</b> — "
			"screens appear automatically.</li>"
			"</ol>"
			"<p><b>Why hostname, not IP?</b> IP addresses change when the router "
			"restarts. Hostnames never change — ClassWatch finds the PC's current "
			"address automatically, on any network, even without internet.</p>" ) );
	vbox->addWidget( steps );

	if( auto* shot = screenshotLabel( QStringLiteral(":/guide/03-classroom.png"),
									  tr( "ClassWatch Setup — classrooms and student PCs." ) ) )
	{
		vbox->addWidget( shot );
	}
	if( auto* shot2 = screenshotLabel( QStringLiteral(":/guide/01-dashboard.png"),
									   tr( "ClassWatch Teacher — student screens in the list." ) ) )
	{
		vbox->addWidget( shot2 );
	}
	vbox->addStretch();
	return scrollWrap( page );
}


QWidget* ClassWatchGuideDialog::buildMonitorTab()
{
	auto* page = new QWidget;
	auto* vbox = new QVBoxLayout( page );

	auto* steps = makeStepsLabel(
		QStringLiteral(
			"<p><b>Live monitoring — works on the school network with or without "
			"internet:</b></p>"
			"<ul>"
			"<li>Screens mirror automatically in the computer list — no clicking needed.</li>"
			"<li>Click a computer for the full live view; double-click for remote control.</li>"
			"<li>If Wi-Fi drops or a PC restarts, the view reconnects by itself.</li>"
			"</ul>"
			"<p><b>Control tools</b> (select computers, then use the feature buttons):</p>"
			"<ul>"
			"<li><b>Text message</b> — sends a sticky announcement banner on top of "
			"everything; students cannot close it.</li>"
			"<li><b>Clear announcement</b> — removes the banner from all screens.</li>"
			"<li><b>Lock / Unlock</b> — blank the screen and freeze keyboard &amp; mouse.</li>"
			"<li><b>Screenshot</b> — capture what a student sees right now.</li>"
			"<li><b>Power</b> — shut down, restart or log off student PCs.</li>"
			"</ul>" ) );
	vbox->addWidget( steps );

	if( auto* shot = screenshotLabel( QStringLiteral(":/guide/04-live.png"),
									  tr( "Live view of a student screen." ) ) )
	{
		vbox->addWidget( shot );
	}
	vbox->addStretch();
	return scrollWrap( page );
}


QWidget* ClassWatchGuideDialog::buildFilesTab()
{
	auto* page = new QWidget;
	auto* vbox = new QVBoxLayout( page );

	auto* steps = makeStepsLabel(
		QStringLiteral(
			"<p><b>Files &amp; messages:</b></p>"
			"<ul>"
			"<li><b>Distribute</b> — select computers, click Distribute, pick files or a "
			"folder → they land on every selected student PC.</li>"
			"<li><b>Collect</b> — students' folders open on your screen → grab their work "
			"back (homework, projects, screenshots).</li>"
			"<li><b>Text message</b> — type an announcement; it appears as a see-through "
			"purple banner pinned to the top of student screens. It has no close button "
			"and clicks pass through it, so it does not disturb the lesson.</li>"
			"<li><b>Clear announcement</b> — one click removes the banner everywhere.</li>"
			"</ul>"
			"<p>Everything runs inside the school network — student data never leaves "
			"the classroom.</p>" ) );
	vbox->addWidget( steps );
	vbox->addStretch();
	return scrollWrap( page );
}


QWidget* ClassWatchGuideDialog::buildTroubleshootingTab()
{
	auto* page = new QWidget;
	auto* vbox = new QVBoxLayout( page );

	auto* steps = makeStepsLabel(
		QString::fromUtf8(
			R"HTML(<p><b>Troubleshooting — what you see → what it means → the fix:</b></p>
<table cellspacing="6" cellpadding="4" style="font-size:12px;">
<tr style="background:#EDE9FE;"><th align="left">You see</th><th align="left">Means</th><th align="left">Fix</th></tr>
<tr><td>PC missing from list</td><td>Service not running or wrong name</td><td>On that PC: <code>hostname</code>, check the name in Setup; run <code>Get-Service ClassWatchService</code></td></tr>
<tr><td>Padlock / locked screen</td><td>Windows or ClassWatch lock is on</td><td>Click the PC → Lock → Unlock; or sign in at the PC</td></tr>
<tr><td>"error" / offline</td><td>Network or firewall blocking port 11100</td><td>Ping the hostname; check Windows Firewall allows ClassWatch; same Wi-Fi network?</td></tr>
<tr><td>Black screen in live view</td><td>PC locked, asleep or capture crashed</td><td>Wake/unlock the PC; restart ClassWatch Service; update to latest installer</td></tr>
<tr><td>Connects then drops</td><td>Wi-Fi roaming or DHCP change</td><td>Hostname entries handle IP changes — make sure you used hostname, not IP</td></tr>
<tr><td>Some PCs never appear</td><td>Different subnet or router AP isolation</td><td>All PCs on same network; disable "AP isolation"/"client isolation" on the router</td></tr>
</table>
<p><b>Useful commands</b> (Command Prompt on any PC):</p>
<ul>
<li><code>hostname</code> — the PC's permanent name (use this in Setup)</li>
<li><code>ipconfig</code> — shows current IP address</li>
<li><code>ping DESKTOP-XXXX</code> — tests reachability</li>
<li>PowerShell: <code>Test-NetConnection DESKTOP-XXXX -Port 11100</code> — tests the ClassWatch port</li>
<li>PowerShell admin: <code>Restart-Service ClassWatchService</code> — restart the agent</li>
</ul>
<p><b>Router warning:</b> some routers have "AP isolation" or "client isolation" which
stops devices talking to each other — turn it OFF for the classroom network.</p>)HTML" );
	vbox->addWidget( steps );
	vbox->addStretch();
	return scrollWrap( page );
}
