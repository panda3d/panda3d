// Filename: PPDownloadRequest.h
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

#pragma once

#include <string>
#include <strstream>

#include "PPDownloadCallback.h"
#include "PPInstance.h"

class PPDownloadRequest : public PPDownloadCallbackSync
{
public:
    enum RequestType
    {
        File,
        Data,
        P3DObject
    };

    PPDownloadRequest( PPInstance& instance, P3D_request* p3dRequest ) :
        m_instance( instance ), m_p3dRequest( p3dRequest ), m_data( NULL ), 
        m_requestType( RequestType::P3DObject ), m_hFile( INVALID_HANDLE_VALUE )
    {
    }

    PPDownloadRequest( PPInstance& instance, const std::string& fileName ) :
        m_instance( instance ), m_p3dRequest( NULL ), m_fileName( fileName ), 
        m_data( NULL ), m_requestType( RequestType::File ), m_hFile( INVALID_HANDLE_VALUE )
    {
    }

    PPDownloadRequest( PPInstance& instance, std::strstream* data ) :
        m_instance( instance ), m_p3dRequest ( NULL ), m_data( data ), 
        m_requestType( RequestType::Data ), m_hFile( INVALID_HANDLE_VALUE )
    {
    }

    virtual ~PPDownloadRequest( )
    {
        End();
    }

    // PPDownloadCallbackSync interface
    virtual bool Begin( );
    virtual bool DataNotify( size_t expectedDataSize, const void* data, size_t size );
    virtual void ProgressNotify( size_t progress, size_t maxProgress );
    virtual bool End( );

    PPInstance&  m_instance;
    RequestType  m_requestType;

protected:
    P3D_request*    m_p3dRequest;
    std::string     m_fileName;
    std::strstream* m_data;

    HANDLE  m_hFile;

private:
    PPDownloadRequest();
};
