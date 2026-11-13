/*
 * AviWriter.h - minimal Motion-JPEG AVI (Audio Video Interleave) writer
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

#include <QFile>
#include <QList>

// Writes a baseline MJPEG .avi playable in Windows Media Player, VLC and
// most editors. Frames are appended as JPEG-compressed '00dc' chunks -
// no audio track. File sizes are patched on finish().
class AviWriter
{
public:
	AviWriter() = default;
	~AviWriter();

	bool open( const QString& filePath, int width, int height, int fps );
	bool addFrameJpeg( const QByteArray& jpegData );
	bool finish();
	bool isOpen() const { return m_file.isOpen(); }
	int frameCount() const { return m_frameCount; }

private:
	void putU32( quint32 v );
	void putU16( quint16 v );
	void patchU32( qint64 pos, quint32 v );

	QFile m_file;
	qint64 m_avihTotalFramesPos = -1;
	qint64 m_strhLengthPos = -1;
	qint64 m_moviListSizePos = -1;
	qint64 m_moviDataPos = -1;
	struct IndexEntry { quint32 offset; quint32 size; };
	QList<IndexEntry> m_entries;
	quint32 m_maxFrameBytes = 0;
	int m_frameCount = 0;
	int m_width = 0;
	int m_height = 0;
	int m_fps = 10;
};
