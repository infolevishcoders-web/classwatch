/*
 * ScreenRecorder.h - records a computer's live screen to a Motion-JPEG AVI
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

#include <QElapsedTimer>
#include <QImage>
#include <QObject>
#include <QPointer>

class AviWriter;
class QTimer;
class VncConnection;

// Captures the framebuffer of a connected computer at a fixed frame rate
// and writes it to a .avi file. Works for remote student machines and for
// the local machine when the ClassWatch service runs on it too.
class ScreenRecorder : public QObject
{
	Q_OBJECT
public:
	explicit ScreenRecorder( QObject* parent = nullptr );
	~ScreenRecorder() override;

	bool isRecording() const { return m_writer != nullptr; }
	QString outputFilePath() const { return m_outputPath; }
	int frameCount() const;
	qint64 elapsedMs() const;

	// starts recording; returns false on error (errorMessage filled)
	bool start( VncConnection* connection, const QString& filePath,
				QString* errorMessage = nullptr );
	QString stop(); // finalizes file, returns path (empty on failure)

Q_SIGNALS:
	void frameCaptured( int frameCount );
	void finished( const QString& filePath, int frames );
	void failed( const QString& reason );

private Q_SLOTS:
	void captureFrame();

private:
	static constexpr int kTargetFps = 5;
	static constexpr int kMaxWidth = 1280;
	static constexpr int kJpegQuality = 70;

	QPointer<VncConnection> m_connection;
	AviWriter* m_writer = nullptr;
	QTimer* m_timer = nullptr;
	QString m_outputPath;
	QElapsedTimer m_elapsed;
	int m_frameWidth = 0;
	int m_frameHeight = 0;
};
