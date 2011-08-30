// Filename: PPDownloadRequest.cpp
// Created by:  atrestman (14Sept09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PPDownloadRequest.h"
#include "PPInstance.h"
#include "wstring_encode.h"

bool PPDownloadRequest::Begin( ) 
{
	m_instance.m_eventStop.ResetEvent( );
	m_instance.m_eventDownloadStopped.ResetEvent( );
    return true;
}

bool PPDownloadRequest::DataNotify( size_t expectedDataSize, const void* data, size_t dataSize )
{
    bool ret = false;
    if ( m_instance.m_eventStop.m_hObject != NULL )
    {
        if ( ::WaitForSingleObject( m_instance.m_eventStop.m_hObject, 0 ) == WAIT_OBJECT_0 )
        {
            return ret;  // canceled by the user
        }
    }
    switch ( this->m_requestType )
    {
    case ( RequestType::P3DObject ):
        {
            if ( m_p3dRequest )
            {
                ret = P3D_instance_feed_url_stream_ptr( m_p3dRequest->_instance, 
                    m_p3dRequest->_request._get_url._unique_id, 
                    P3D_RC_in_progress, 
                    0, 
                    expectedDataSize, 
                    data, 
                    dataSize );
            }
        }
        break;
    case ( RequestType::File ):
        {
            if ( m_hFile == INVALID_HANDLE_VALUE )
            {
                wstring filename_w;
                string_to_wstring(filename_w, m_fileName);
                m_hFile = ::CreateFileW( filename_w.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
                if ( m_hFile == INVALID_HANDLE_VALUE )
                {
                    return ret;
                }
            }
            DWORD numberOfBytesWritten = 0;
            if ( ::WriteFile( m_hFile, data, dataSize, &numberOfBytesWritten, NULL ) == TRUE )
            {
                ret = true;
            }
        }
        break;
    case ( RequestType::Data ):
        {
            if ( m_data )
            {
                std::string bits( static_cast< const char* >( data ), dataSize );
                *m_data << bits;
                ret = true;
            }
        }
        break;
    }
    return ret;
}

void PPDownloadRequest::ProgressNotify( size_t progress, size_t maxProgress )
{
    if ( ::IsWindow( m_instance.m_parentWnd ) )
    {
        SendMessage( m_instance.m_parentWnd, WM_PROGRESS, (WPARAM)(progress * 100.0 / maxProgress), 0 ); 
    }
}

bool PPDownloadRequest::End( ) 
{
    if ( m_hFile != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle( m_hFile );
        m_hFile = INVALID_HANDLE_VALUE;
    }
	m_instance.m_eventDownloadStopped.SetEvent( );
    return true;
}
