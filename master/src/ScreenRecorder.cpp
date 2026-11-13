/*
 * ScreenRecorder.cpp - records a computer's live screen to AVI
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

#include "ScreenRecorder.h"

#include <QBuffer>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QTimer>

#include "AviWriter.h"
#include "VncConnection.h"


ScreenRecorder::ScreenRecorder( QObject* parent ) :
	QObject( parent )
{
}


ScreenRecorder::~ScreenRecorder()
{
	if( m_writer )
	{
		stop();
	}
}


int ScreenRecorder::frameCount() const
{
	return m_writer ? m_writer->frameCount() : 0;
}


qint64 ScreenRecorder::elapsedMs() const
{
	return m_elapsed.isValid() ? m_elapsed.elapsed() : 0;
}


bool ScreenRecorder::start( VncConnection* connection, const QString& filePath,
							QString* errorMessage )
{
	auto fail = [errorMessage]( const QString& msg ) {
		if( errorMessage )
		{
			*errorMessage = msg;
		}
		return false;
	};

	if( isRecording() )
	{
		return fail( QStringLiteral("Already recording") );
	}
	if( !connection )
	{
		return fail( QStringLiteral("No connection") );
	}

	// wait until the framebuffer is actually valid so we know the
	// resolution up-front
	if( !connection->hasValidFramebuffer() )
	{
		return fail( QStringLiteral("Screen not available yet - connect first") );
	}

	const auto source = connection->image();
	if( source.isNull() )
	{
		return fail( QStringLiteral("Screen image is empty") );
	}

	// cap recording resolution so files stay reasonable on big monitors
	QSize outSize = source.size();
	if( outSize.width() > kMaxWidth )
	{
		outSize.setHeight( qRound( outSize.height() * ( kMaxWidth / double( outSize.width() ) ) ) );
		outSize.setWidth( kMaxWidth );
	}
	if( outSize.isEmpty() || outSize.width() < 2 || outSize.height() < 2 )
	{
		return fail( QStringLiteral("Invalid screen size") );
	}

	QDir().mkpath( QFileInfo( filePath ).absolutePath() );

	auto* writer = new AviWriter;
	if( !writer->open( filePath, outSize.width(), outSize.height(), kTargetFps ) )
	{
		delete writer;
		return fail( QStringLiteral("Cannot create file: %1" ).arg( filePath ) );
	}

	m_frameWidth = outSize.width();
	m_frameHeight = outSize.height();
	m_writer = writer;
	m_connection = connection;
	m_outputPath = filePath;
	m_elapsed.start();

	m_timer = new QTimer( this );
	m_timer->setInterval( 1000 / kTargetFps );
	connect( m_timer, &QTimer::timeout, this, &ScreenRecorder::captureFrame );
	m_timer->start();

	captureFrame(); // first frame immediately
	return true;
}


void ScreenRecorder::captureFrame()
{
	if( !m_writer || !m_connection )
	{
		return;
	}
	if( m_connection->state() != VncConnection::State::Connected ||
		!m_connection->hasValidFramebuffer() )
	{
		return; // keep timer running; resume automatically on reconnect
	}

	auto image = m_connection->image();
	if( image.isNull() )
	{
		return;
	}
	if( image.format() != QImage::Format_RGB32 )
	{
		image = image.convertToFormat( QImage::Format_RGB32 );
	}

	// scale every frame to the exact size declared in the AVI header so a
	// mid-recording resolution change cannot corrupt the file
	if( image.size() != QSize( m_frameWidth, m_frameHeight ) )
	{
		image = image.scaled( m_frameWidth, m_frameHeight,
							  Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
	}

	QByteArray jpeg;
	QBuffer buffer( &jpeg );
	buffer.open( QIODevice::WriteOnly );
	if( !image.save( &buffer, "JPG", kJpegQuality ) )
	{
		return;
	}

	if( m_writer->addFrameJpeg( jpeg ) )
	{
		Q_EMIT frameCaptured( m_writer->frameCount() );
	}
}


QString ScreenRecorder::stop()
{
	if( !m_writer )
	{
		return {};
	}

	const auto path = m_outputPath;
	const auto frames = m_writer->frameCount();

	if( m_timer )
	{
		m_timer->stop();
		m_timer->deleteLater();
		m_timer = nullptr;
	}

	auto* writer = m_writer;
	m_writer = nullptr;
	m_connection = nullptr;

	QString result;
	if( frames > 0 && writer->finish() )
	{
		result = path;
		Q_EMIT finished( path, frames );
	}
	else
	{
		QFile::remove( path );
		Q_EMIT failed( QStringLiteral("No frames captured") );
	}
	delete writer;
	return result;
}
