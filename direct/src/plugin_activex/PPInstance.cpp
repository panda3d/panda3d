// Filename: PPInstance.cpp
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

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <windows.h>
#include <process.h>

#include "resource.h"
#include "PPInstance.h"
#include "P3DActiveXCtrl.h"
#include "PPBrowserObject.h"
#include "PPDownloadRequest.h"

#include "p3d_plugin_config.h"
#include "get_tinyxml.h"
#include "load_plugin.h"
#include "find_root_dir.h"
#include "mkdir_complete.h"

#define P3D_CONTENTS_FILENAME "contents.xml"
#define P3D_DEFAULT_PLUGIN_FILENAME "p3d_plugin.dll"

static int s_instanceCount = 0;

void P3D_NofificationSync(P3D_instance *instance)
{
    static bool handleRequestOnUIThread = true;
    if ( instance )
    {
        CP3DActiveXCtrl* parent = static_cast<CP3DActiveXCtrl*>(instance->_user_data);
        if ( parent )
        {
            if ( ::IsWindow( parent->m_hWnd ) )
            {
                ::PostMessage( parent->m_hWnd, WM_PANDA_NOTIFICATION, 0, 0 );
            }
            else
            {
                nout << "Can handle P3D_Notification on UI thread. Controls window is invalid\n";
            }
        }
        else
        {
            nout << "Can handle P3D_Notification on UI thread. Instance's user data is not a Control\n";
        }
    }
}

PPInstance::PPInstance( CP3DActiveXCtrl& parentCtrl ) : 
    m_parentCtrl( parentCtrl ), m_p3dInstance( NULL ), m_p3dObject( NULL ), m_handleRequestOnUIThread( true ), m_isInit( false )
{
  // Open the logfile first.
  m_logger.Open( );

  m_rootDir = find_root_dir( nout );
  m_pluginLoaded = false;
}

PPInstance::~PPInstance(  )
{
    if ( m_p3dInstance )
    {
        P3D_instance_finish( m_p3dInstance );
        m_p3dInstance = NULL;
    }
    if ( m_p3dObject )
    {
        P3D_OBJECT_DECREF( m_p3dObject );
        m_p3dObject = NULL;
    }
    if ( m_pluginLoaded )
    {
        UnloadPlugin();
    }
}

int PPInstance::DownloadFile( const std::string& from, const std::string& to )
{
    int error( 0 );
    PPDownloadRequest p3dContentsDownloadRequest( *this, to ); 
    PPDownloadCallback dcForContents( p3dContentsDownloadRequest );

    nout << "Downloading " << from << " into " << to << "\n";
    HRESULT hr = ::URLOpenStream( m_parentCtrl.GetControllingUnknown(), from.c_str(), 0, &dcForContents );
    if ( FAILED( hr ) )
    {   
        error = 1;
        nout << "Error downloading " << from << " :" << hr << "\n";
    }
    return error;
}

int PPInstance::CopyFile( const std::string& from, const std::string& to )
{
  ifstream in(from.c_str(), ios::in | ios::binary);
  ofstream out(to.c_str(), ios::out | ios::binary);
        
  static const size_t buffer_size = 4096;
  char buffer[buffer_size];
  
  in.read(buffer, buffer_size);
  size_t count = in.gcount();
  while (count != 0) {
    out.write(buffer, count);
    if (out.fail()) {
      return 1;
    }
    in.read(buffer, buffer_size);
    count = in.gcount();
  }

  if (!in.eof()) {
    return 1;
  }

  return 0;
}

int PPInstance::ReadContents( const std::string& contentsFilename, FileSpec& p3dDllFile )
{
    int error(1);

    TiXmlDocument doc( contentsFilename.c_str( ) );
    if ( doc.LoadFile( ) ) 
    {
        TiXmlElement *xcontents = doc.FirstChildElement( "contents" );
        if ( xcontents != NULL ) 
        {
            TiXmlElement *xpackage = xcontents->FirstChildElement( "package" );
            while ( xpackage != NULL ) 
            {
                const char *name = xpackage->Attribute( "name" );
                if ( name != NULL && strcmp( name, "coreapi" ) == 0 ) 
                {
                    const char *platform = xpackage->Attribute( "platform" );
                    if ( platform != NULL && !strcmp(platform, "win32") ) 
                    {
                        p3dDllFile.load_xml(xpackage);
                        error = 0;
                        break;
                    }
                }
                xpackage = xpackage->NextSiblingElement( "package" );
            }
        }
    }
    return error;
}

int PPInstance::DownloadP3DComponents( std::string& p3dDllFilename )
{
    int error(0);

    // Start off by downloading contents.xml into a local temporary
    // file.  We get a unique temporary filename each time; this is a
    // small file and it's very important that we get the most current
    // version, not an old cached version.
    TCHAR tempFolderName[ MAX_PATH ];
    ::GetTempPath( MAX_PATH, tempFolderName );
    TCHAR tempFileName[ MAX_PATH ];
    ::GetTempFileName( tempFolderName, "p3d", 0, tempFileName );
    std::string localContentsFileName( tempFileName );

    // We'll also get the final installation path of the contents.xml
    // file.
    std::string finalContentsFileName( m_rootDir );
    finalContentsFileName += "/";
    finalContentsFileName += P3D_CONTENTS_FILENAME;

    std::string hostUrl( PANDA_PACKAGE_HOST_URL );
    if (!hostUrl.empty() && hostUrl[hostUrl.size() - 1] != '/') {
      hostUrl += '/';
    }

    // Append a query string to the contents.xml URL to uniquify it
    // and ensure we don't get a cached version.
    std::ostringstream strm;
    strm << hostUrl << P3D_CONTENTS_FILENAME << "?" << time(NULL);
    std::string remoteContentsUrl( strm.str() );

    FileSpec p3dDllFile;
    error = DownloadFile( remoteContentsUrl, localContentsFileName );
    if ( !error )
    {
        error = ReadContents( localContentsFileName, p3dDllFile );
    }

    if ( error ) {
      // If we couldn't download or read the contents.xml file, check
      // to see if there's a good one on disk already, as a fallback.
      error = ReadContents( finalContentsFileName, p3dDllFile );

    } else {
      // If we have successfully read the downloaded version,
      // then move the downloaded version into the final location.
      mkfile_complete( finalContentsFileName, nout );
      CopyFile( localContentsFileName, finalContentsFileName );
    }

    // We don't need the temporary file any more.
    ::DeleteFile( localContentsFileName.c_str() );

    if ( !error )
    {
        // OK, at this point we have successfully read contents.xml,
        // and we have a good file spec in p3dDllFile.
        if ( p3dDllFile.quick_verify( m_rootDir ) )
        {
            // The DLL is already on-disk, and is good.
            p3dDllFilename = p3dDllFile.get_pathname( m_rootDir );
        }
        else
        {
            // The DLL is not already on-disk, or it's stale.
            std::string p3dLocalModuleFileName( p3dDllFile.get_pathname( m_rootDir ) );
            mkfile_complete( p3dLocalModuleFileName, nout );
            std::string p3dRemoteModuleUrl( hostUrl );
            p3dRemoteModuleUrl += p3dDllFile.get_filename();
            error = DownloadFile( p3dRemoteModuleUrl, p3dLocalModuleFileName );
            if ( !error )
            {
                error = 1;
                if ( p3dDllFile.full_verify( m_rootDir ) )
                {
                    // Downloaded successfully.
                    p3dDllFilename = p3dDllFile.get_pathname( m_rootDir );
                    error = 0;
                }
            }
        }
    }
    
    return error;
}

int PPInstance::LoadPlugin( const std::string& dllFilename ) 
{
    if ( !m_pluginLoaded )
    { 
        s_instanceCount += 1;
        m_pluginLoaded = true;
    }

    int error = 0;
    if (!is_plugin_loaded()) {

      std::string pathname = dllFilename;
#ifdef P3D_PLUGIN_P3D_PLUGIN
      // This is a convenience macro for development.  If defined and
      // nonempty, it indicates the name of the plugin DLL that we will
      // actually run, even after downloading a possibly different
      // (presumably older) version.  Its purpose is to simplify iteration
      // on the plugin DLL.
      string override_filename = P3D_PLUGIN_P3D_PLUGIN;
      if (!override_filename.empty()) {
        pathname = override_filename;
      }
#endif  // P3D_PLUGIN_P3D_PLUGIN
      
      nout << "Attempting to load core API from " << pathname << "\n";
      if (!load_plugin(pathname, "", "", true, "", "", "", false, nout)) {
        nout << "Unable to launch core API in " << pathname << "\n";
        error = 1;
      }
    }

    return error ;
}

int PPInstance::UnloadPlugin()
{
    int error( 0 );

    if ( m_pluginLoaded )
    { 
        m_pluginLoaded = false;
        assert( s_instanceCount > 0 );
        s_instanceCount -= 1;

        if ( s_instanceCount == 0 && is_plugin_loaded() ) 
        {
            unload_plugin();
            m_isInit = false;
            
            // This pointer is no longer valid and must be reset for next
            // time.
            PPBrowserObject::clear_class_definition();
        }
    }
    return error;
}

int PPInstance::Start( const std::string& p3dFilename  )
{
    m_eventStop.ResetEvent();

    P3D_window_handle parent_window;
    parent_window._hwnd = m_parentCtrl.m_hWnd;

    RECT rect;
    GetClientRect( m_parentCtrl.m_hWnd, &rect );

    P3D_token* p3dTokens = new P3D_token[ m_parentCtrl.m_parameters.size() ];
    for ( UINT i = 0; i < m_parentCtrl.m_parameters.size(); i++ )
    {
        std::pair< CString, CString > keyAndValue = m_parentCtrl.m_parameters[ i ];
        p3dTokens[i]._keyword = strdup( m_parentCtrl.m_parameters[ i ].first ); 
        p3dTokens[i]._value = strdup( m_parentCtrl.m_parameters[ i ].second );
    }
    nout << "Creating new P3D instance object \n";
    m_p3dInstance = P3D_new_instance( &P3D_NofificationSync, p3dTokens, m_parentCtrl.m_parameters.size(), 0, NULL, (void*)&m_parentCtrl );

    for ( UINT j = 0; j < m_parentCtrl.m_parameters.size(); j++ )
    {
        delete [] p3dTokens[j]._keyword;
        delete [] p3dTokens[j]._value;
    }
    delete [] p3dTokens;

    if ( !m_p3dInstance )
    {
        nout << "Error creating P3D instance: " << GetLastError() << "\n"; 
        return 1;
    }
    CComPtr<IDispatch> pDispatch;
    PPBrowserObject *pobj = new PPBrowserObject( &m_parentCtrl, pDispatch );
    P3D_instance_set_browser_script_object( m_p3dInstance, pobj );
    P3D_OBJECT_DECREF( pobj );
    
    m_p3dObject = P3D_instance_get_panda_script_object( m_p3dInstance );
    P3D_OBJECT_INCREF( m_p3dObject );
    
    P3D_instance_setup_window( m_p3dInstance, P3D_WT_embedded, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, parent_window );

    std::string p3dRemoteFilename( GetHostUrl() );
    p3dRemoteFilename += p3dFilename;

    nout << "Starting new P3D instance " << p3dRemoteFilename << "\n";

    if ( !P3D_instance_start( m_p3dInstance, false, p3dRemoteFilename.c_str() ) )
    {
        nout << "Error starting P3D instance: " << GetLastError() << "\n";
        return 1;
    }

    m_isInit = true;

    return 0;
}

std::string PPInstance::GetHostUrl( )
{
    CString hostingPageLocation = m_parentCtrl.m_hostingPageUrl.Left( m_parentCtrl.m_hostingPageUrl.ReverseFind( '/' ) );;
    std::string p3dRemoteFilename( hostingPageLocation );
    p3dRemoteFilename += "/"; 
    return p3dRemoteFilename; 
}

std::string PPInstance::GetP3DFilename( )
{
    std::string p3dFilename;
    for ( UINT i = 0; i < m_parentCtrl.m_parameters.size(); i++ )
    {
        std::pair< CString, CString > keyAndValue = m_parentCtrl.m_parameters[ i ];
        if ( keyAndValue.first == "data" )
        {
            p3dFilename = keyAndValue.second;
        }
    }
    return p3dFilename;
}

void PPInstance::HandleRequestLoop() 
{
    P3D_instance *p3d_inst = P3D_check_request(false);
    while ( p3d_inst != NULL ) 
    {
        P3D_request *request = P3D_instance_get_request(p3d_inst);
        if ( request != NULL ) 
        {
            CP3DActiveXCtrl* parent = ( CP3DActiveXCtrl* )(p3d_inst->_user_data);
            if ( parent )
            {
                parent->m_instance.HandleRequest( request );
            }
            else
            {
                nout << "Error handling P3D request. Instance's user data is not a Control \n";
            }
        }
        p3d_inst = P3D_check_request( false );
    }
}

void PPInstance::HandleRequestGetUrl( void* data )
{
    P3D_request *request = static_cast<P3D_request*>( data );
    if ( !request )
    {
        return;
    }
    int unique_id = request->_request._get_url._unique_id;
    const std::string &url = request->_request._get_url._url;
    CP3DActiveXCtrl* parent = static_cast<CP3DActiveXCtrl*> ( request->_instance->_user_data );

    if ( !parent )
    {
        return;
    }

    nout << "Handling P3D_RT_get_url request from " << url << "\n";
    PPDownloadRequest p3dObjectDownloadRequest( parent->m_instance, request ); 
    PPDownloadCallback bsc( p3dObjectDownloadRequest );
    HRESULT hr = ::URLOpenStream( parent->GetControllingUnknown(), url.c_str(), 0, &bsc );
    if ( FAILED( hr ) )
    {
        nout << "Error handling P3D_RT_get_url request" << " :" << hr << "\n"; 
        return;
    }
    //inet_InternetSession inet(parent->m_pythonEmbed.m_threadData.m_parent);
    //std::string outdata;
    //if ( !inet.getURLMemory( url, outdata, request ) )
    //{
    //    handled = false;
    //}

    P3D_instance_feed_url_stream( 
        request->_instance, 
        request->_request._get_url._unique_id, 
        P3D_RC_done, 
        0, 
        0, 
        (const void*)NULL, 
        0 
        );
    P3D_request_finish( request, true );
}

void PPInstance::HandleRequest( P3D_request *request ) 
{
    if ( !request ) 
    {
        return;
    }
    bool handled = false;

    switch ( request->_request_type ) 
    {
        case P3D_RT_stop:
        {
            P3D_instance_finish( request->_instance );
            handled = true;
            break;
        }
        case P3D_RT_get_url:
        {
            if ( !m_handleRequestOnUIThread )
            {
                _beginthread( HandleRequestGetUrl, 0, request );
            }
            else
            {
                HandleRequestGetUrl( request );
            }
            handled = true;

            return;
        }
        case P3D_RT_notify:
        {
            CString notification = request->_request._notify._message;
            if ( notification == "ondownloadbegin" )
            {
                m_handleRequestOnUIThread = false;
            }
            else if ( notification == "ondownloadcomplete" )
            {
                m_handleRequestOnUIThread = true;
            }
            handled = true;
            break;
        }
        default:
        {
            // Some request types are not handled.
            break;
        }
    };
    P3D_request_finish( request, handled );
}



