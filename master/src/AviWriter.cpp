/*
 * AviWriter.cpp - minimal Motion-JPEG AVI writer implementation
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

#include "AviWriter.h"

AviWriter::~AviWriter()
{
	if( m_file.isOpen() )
	{
		finish();
	}
}


void AviWriter::putU32( quint32 v )
{
	QByteArray b( 4, '\0' );
	b[0] = static_cast<char>( v & 0xff );
	b[1] = static_cast<char>( ( v >> 8 ) & 0xff );
	b[2] = static_cast<char>( ( v >> 16 ) & 0xff );
	b[3] = static_cast<char>( ( v >> 24 ) & 0xff );
	m_file.write( b );
}


void AviWriter::putU16( quint16 v )
{
	QByteArray b( 2, '\0' );
	b[0] = static_cast<char>( v & 0xff );
	b[1] = static_cast<char>( ( v >> 8 ) & 0xff );
	m_file.write( b );
}


void AviWriter::patchU32( qint64 pos, quint32 v )
{
	if( pos < 0 )
	{
		return;
	}
	const auto cur = m_file.pos();
	m_file.seek( pos );
	putU32( v );
	m_file.seek( cur );
}


bool AviWriter::open( const QString& filePath, int width, int height, int fps )
{
	if( width <= 0 || height <= 0 || fps <= 0 )
	{
		return false;
	}
	if( !m_file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
	{
		return false;
	}

	m_width = width;
	m_height = height;
	m_fps = fps;
	m_frameCount = 0;
	m_maxFrameBytes = 0;
	m_entries.clear();

	QByteArray h;
	h.reserve( 2048 );

	// RIFF header - sizes patched in finish()
	h.append( "RIFF", 4 );
	h.append( QByteArray( 4, '\0' ) );          // RIFF size
	h.append( "AVI ", 4 );

	// LIST hdrl
	h.append( "LIST", 4 );
	h.append( QByteArray( 4, '\0' ) );          // hdrl size
	h.append( "hdrl", 4 );

	// avih - Main AVI Header (56 bytes)
	h.append( "avih", 4 );
	h.append( QByteArray( 4, '\0' ) );          // avih size = 56
	const auto avihPos = h.size();
	auto put32 = [&h]( quint32 v ) {
		h.append( static_cast<char>( v & 0xff ) );
		h.append( static_cast<char>( ( v >> 8 ) & 0xff ) );
		h.append( static_cast<char>( ( v >> 16 ) & 0xff ) );
		h.append( static_cast<char>( ( v >> 24 ) & 0xff ) );
	};
	auto put16 = [&h]( quint16 v ) {
		h.append( static_cast<char>( v & 0xff ) );
		h.append( static_cast<char>( ( v >> 8 ) & 0xff ) );
	};
	put32( 1000000 / fps );                    // dwMicroSecPerFrame
	put32( 0 );                                // dwMaxBytesPerSec
	put32( 0 );                                // dwPaddingGranularity
	put32( 0x10 );                             // dwFlags = AVIF_HASINDEX
	m_avihTotalFramesPos = h.size();
	put32( 0 );                                // dwTotalFrames (patched)
	put32( 0 );                                // dwInitialFrames
	put32( 1 );                                // dwStreams
	put32( 0 );                                // dwSuggestedBufferSize
	put32( static_cast<quint32>( m_width ) );
	put32( static_cast<quint32>( m_height ) );
	put32( 0 ); put32( 0 ); put32( 0 ); put32( 0 ); // dwReserved[4]

	// LIST strl - stream list
	h.append( "LIST", 4 );
	const auto strlSizePos = h.size();
	h.append( QByteArray( 4, '\0' ) );
	h.append( "strl", 4 );
	const auto strlDataStart = h.size();

	// strh - AVI Stream Header (56 bytes)
	h.append( "strh", 4 );
	h.append( QByteArray( 4, '\0' ) );          // strh size = 56
	h.append( "vids", 4 );                      // fccType
	h.append( "MJPG", 4 );                      // fccHandler
	put32( 0 );                                // dwFlags
	put16( 0 ); put16( 0 );                    // priority, language
	put32( 0 );                                // dwInitialFrames
	put32( 1 );                                // dwScale
	put32( static_cast<quint32>( m_fps ) );    // dwRate
	put32( 0 );                                // dwStart
	m_strhLengthPos = h.size();
	put32( 0 );                                // dwLength (patched)
	put32( 0 );                                // dwSuggestedBufferSize
	put32( 0xFFFFFFFF );                       // dwQuality = -1
	put32( 0 );                                // dwSampleSize
	put32( 0 ); put32( 0 );                    // rcFrame left, top
	put32( static_cast<quint32>( m_width ) );  // rcFrame right
	put32( static_cast<quint32>( m_height ) ); // rcFrame bottom

	// strf - BITMAPINFOHEADER (40 bytes)
	h.append( "strf", 4 );
	h.append( QByteArray( 4, '\0' ) );          // strf size = 40
	put32( 40 );                               // biSize
	put32( static_cast<quint32>( m_width ) );
	put32( static_cast<quint32>( m_height ) );
	put16( 1 );                                // biPlanes
	put16( 24 );                               // biBitCount
	h.append( "MJPG", 4 );                      // biCompression
	put32( 0 );                                // biSizeImage
	put32( 0 );                                // biXPelsPerMeter
	put32( 0 );                                // biYPelsPerMeter
	put32( 0 );                                // biClrUsed
	put32( 0 );                                // biClrImportant

	// patch strl list size: bytes after the size field = "strl"(4) + chunks
	const auto strlSize = h.size() - strlDataStart + 4;
	// strlSizePos points at the 4-byte size field; overwrite in h
	const auto absStrlSizePos = strlSizePos;
	for( int i = 0; i < 4; ++i )
	{
		h[absStrlSizePos + i] = static_cast<char>( ( strlSize >> ( 8 * i ) ) & 0xff );
	}

	// patch hdrl list size: from 'hdrl' fourcc to end of h
	// hdrl size field is at offset 4 (after "LIST")
	const auto hdrlSize = h.size() - 12; // bytes after "hdrl" fourcc
	for( int i = 0; i < 4; ++i )
	{
		h[8 + i] = static_cast<char>( ( hdrlSize >> ( 8 * i ) ) & 0xff );
	}

	// LIST movi - placeholder size patched in finish()
	const auto moviPos = h.size();
	h.append( "LIST", 4 );
	m_moviListSizePos = h.size();
	h.append( QByteArray( 4, '\0' ) );
	h.append( "movi", 4 );
	m_moviDataPos = -1; // set after write: file pos of first frame data marker

	m_file.write( h );
	// movi data starts right after the 'movi' fourcc
	m_moviDataPos = m_file.pos();

	// remember where avih/strh patch positions are in the FILE
	// (they were recorded as offsets inside h, which starts at file pos 0)
	return true;
}


bool AviWriter::addFrameJpeg( const QByteArray& jpegData )
{
	if( !m_file.isOpen() || jpegData.isEmpty() )
	{
		return false;
	}

	// chunk: '00dc' + size + data + word-align pad
	QByteArray chunk;
	chunk.append( "00dc", 4 );
	const auto sz = static_cast<quint32>( jpegData.size() );
	chunk.append( static_cast<char>( sz & 0xff ) );
	chunk.append( static_cast<char>( ( sz >> 8 ) & 0xff ) );
	chunk.append( static_cast<char>( ( sz >> 16 ) & 0xff ) );
	chunk.append( static_cast<char>( ( sz >> 24 ) & 0xff ) );
	chunk.append( jpegData );
	if( jpegData.size() % 2 )
	{
		chunk.append( '\0' );
	}

	const auto offset = static_cast<quint32>( m_file.pos() - m_moviDataPos );
	m_file.write( chunk );
	m_entries.append( { offset, sz } );
	m_maxFrameBytes = qMax( m_maxFrameBytes, sz );
	++m_frameCount;
	return true;
}


bool AviWriter::finish()
{
	if( !m_file.isOpen() )
	{
		return false;
	}

	// end of movi data (before idx1) - needed for the movi LIST size patch
	const auto moviEnd = m_file.pos();

	// idx1 index
	const auto idx1Pos = m_file.pos();
	QByteArray idx;
	idx.reserve( m_entries.size() * 16 + 8 );
	idx.append( "idx1", 4 );
	const auto count = static_cast<quint32>( m_entries.size() );
	idx.append( static_cast<char>( count & 0xff ) );
	idx.append( static_cast<char>( ( count >> 8 ) & 0xff ) );
	idx.append( static_cast<char>( ( count >> 16 ) & 0xff ) );
	idx.append( static_cast<char>( ( count >> 24 ) & 0xff ) );
	for( const auto& e : m_entries )
	{
		idx.append( "00dc", 4 );
		// flags 0x10 = keyframe
		idx.append( static_cast<char>( 0x10 ) );
		idx.append( QByteArray( 3, '\0' ) );
		const auto off = e.offset;
		idx.append( static_cast<char>( off & 0xff ) );
		idx.append( static_cast<char>( ( off >> 8 ) & 0xff ) );
		idx.append( static_cast<char>( ( off >> 16 ) & 0xff ) );
		idx.append( static_cast<char>( ( off >> 24 ) & 0xff ) );
		const auto sz = e.size;
		idx.append( static_cast<char>( sz & 0xff ) );
		idx.append( static_cast<char>( ( sz >> 8 ) & 0xff ) );
		idx.append( static_cast<char>( ( sz >> 16 ) & 0xff ) );
		idx.append( static_cast<char>( ( sz >> 24 ) & 0xff ) );
	}
	m_file.write( idx );

	// patch movi LIST size: bytes after its size field = 4 ("movi") + frame data
	const auto moviSize = moviEnd - m_moviDataPos + 4;
	patchU32( m_moviListSizePos, static_cast<quint32>( moviSize ) );

	// patch RIFF size: file size - 8
	patchU32( 4, static_cast<quint32>( m_file.size() - 8 ) );

	// NOTE: avih dwTotalFrames / strh dwLength were recorded as offsets in h;
	// recompute their file positions: header h started at file pos 0, and the
	// recorded positions are absolute offsets within h == file offsets.
	// They were stored as h.size() at write time -> same numeric file offset.
	// We saved them in member vars as h-offsets which equal file offsets.
	// (m_avihTotalFramesPos / m_strhLengthPos are file offsets already.)
	patchU32( m_avihTotalFramesPos, static_cast<quint32>( m_frameCount ) );
	patchU32( m_strhLengthPos, static_cast<quint32>( m_frameCount ) );

	m_file.close();
	return true;
}
