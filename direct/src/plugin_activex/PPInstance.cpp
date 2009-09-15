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

#include "wintools/sdk/tinyxml/tinyxml.h"

#define P3D_CONTENTS_FILENAME "contents.xml"

#define P3D_BASE_URL "http://www.ddrose.com/~drose/p3d_7/"
//#define P3D_BASE_URL "file:///C:/p3dstage/"

#define P3D_FILE_BASE_URL "http://www.ddrose.com/~drose/plugin/"
//#define P3D_FILE_BASE_URL "file:///C:/temp/"

#define P3D_DEFAULT_PLUGIN_FILENAME "p3d_plugin.dll"

P3D_initialize_func *P3D_initialize;
P3D_new_instance_func *P3D_new_instance;
P3D_instance_start_func *P3D_instance_start;
P3D_instance_finish_func *P3D_instance_finish;
P3D_instance_setup_window_func *P3D_instance_setup_window;
P3D_instance_get_request_func *P3D_instance_get_request;
P3D_check_request_func *P3D_check_request;
P3D_request_finish_func *P3D_request_finish;
P3D_instance_feed_url_stream_func *P3D_instance_feed_url_stream;

P3D_instance_set_browser_script_object_func *P3D_instance_set_browser_script_object;
P3D_instance_get_panda_script_object_func *P3D_instance_get_panda_script_object;
P3D_make_class_definition_func *P3D_make_class_definition;

P3D_new_undefined_object_func *P3D_new_undefined_object;
P3D_new_none_object_func *P3D_new_none_object;
P3D_new_bool_object_func *P3D_new_bool_object;
P3D_new_int_object_func *P3D_new_int_object;
P3D_new_float_object_func *P3D_new_float_object;
P3D_new_string_object_func *P3D_new_string_object;


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
    m_parentCtrl( parentCtrl ), m_p3dInstance( NULL ), m_p3dObject( NULL ), m_hP3DPluginDll( NULL ), m_handleRequestOnUIThread( true )
{
    TCHAR tempFolderName[ MAX_PATH ];
    DWORD pathLength = ::GetTempPath( MAX_PATH, tempFolderName );

    m_logger.Open( std::string( tempFolderName ), std::string( P3D_DEFAULT_PLUGIN_LOG_FILENAME ) );
}

PPInstance::~PPInstance(  )
{
    if ( m_p3dObject )
    {
        P3D_OBJECT_DECREF( m_p3dObject );
    }
    if ( m_p3dInstance )
    {
        P3D_instance_finish( m_p3dInstance );
        m_p3dInstance = NULL;
    }
    UnloadPlugin();
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

int PPInstance::ReadContents( const std::string& contentsFilename, std::string& p3dDllFilename )
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
                        p3dDllFilename += xpackage->Attribute( "filename" );
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

int PPInstance::DownloadP3DComponents( std::string& p3dDllFilename, std::string& p3dFilename  )
{
    int error(0);

    TCHAR tempFolderName[ MAX_PATH ];
    DWORD pathLength = ::GetTempPath( MAX_PATH, tempFolderName );

    std::string localContentsFileName( tempFolderName, pathLength );
    localContentsFileName += P3D_CONTENTS_FILENAME;

    std::string remoteContentsFilename( P3D_BASE_URL );
    remoteContentsFilename += P3D_CONTENTS_FILENAME;

    error = DownloadFile( remoteContentsFilename, localContentsFileName );
    if ( !error )
    {
        std::string p3dRemoteModuleFileName( P3D_BASE_URL );
        error = ReadContents( localContentsFileName, p3dRemoteModuleFileName );
        if ( !error )
        {
            std::string p3dLocalModuleFileName( tempFolderName, pathLength );
            p3dLocalModuleFileName += P3D_DEFAULT_PLUGIN_FILENAME;

            // Check for existance
            if ( ::GetFileAttributes( p3dLocalModuleFileName.c_str( ) ) == INVALID_FILE_ATTRIBUTES )
            {
                error = DownloadFile( p3dRemoteModuleFileName, p3dLocalModuleFileName );
            }
            if ( !error )
            {
                p3dDllFilename = p3dLocalModuleFileName;

                std::string p3dLocalFilename( tempFolderName );
                p3dLocalFilename += GetP3DFilename( );

                std::string p3dRemoteFilename( P3D_FILE_BASE_URL );
                p3dRemoteFilename += GetP3DFilename( );

                // Check for existance
                if ( ::GetFileAttributes( p3dLocalFilename.c_str( ) ) == INVALID_FILE_ATTRIBUTES )
                {
                    error = DownloadFile( p3dRemoteFilename, p3dLocalFilename );
                }
                p3dFilename = p3dLocalFilename;
            }
        }
    }
    return error;
}

int PPInstance::LoadPlugin( const std::string& dllFilename ) 
{
    int error(0);
    std::string filename( dllFilename );

    if ( filename.empty() ) 
    {
        // Look for the plugin along the path.
        filename = P3D_DEFAULT_PLUGIN_FILENAME;
        filename += ".dll";
    }

    nout << "Loading " << filename << "\n";
    m_hP3DPluginDll = LoadLibrary( filename.c_str() );
    if ( m_hP3DPluginDll == NULL ) 
    {
        // Couldn't load the DLL.
        nout << "Error loading " << filename << " :" << GetLastError() << "\n";
        return false;
    }

    char buffer[MAX_PATH];
    if ( GetModuleFileName( m_hP3DPluginDll, buffer, MAX_PATH ) != 0 ) 
    {
        if ( GetLastError() != 0 ) 
        {
            filename = buffer;
        }
    }

    // Now get all of the function pointers.
    P3D_initialize = (P3D_initialize_func *)GetProcAddress(m_hP3DPluginDll, "P3D_initialize");  
    P3D_new_instance = (P3D_new_instance_func *)GetProcAddress(m_hP3DPluginDll, "P3D_new_instance");  
    P3D_instance_start = (P3D_instance_start_func *)GetProcAddress(m_hP3DPluginDll, "P3D_instance_start");
    P3D_instance_finish = (P3D_instance_finish_func *)GetProcAddress(m_hP3DPluginDll, "P3D_instance_finish");  
    P3D_instance_setup_window = (P3D_instance_setup_window_func *)GetProcAddress(m_hP3DPluginDll, "P3D_instance_setup_window");
    P3D_instance_get_request = (P3D_instance_get_request_func *)GetProcAddress(m_hP3DPluginDll, "P3D_instance_get_request");  
    P3D_check_request = (P3D_check_request_func *)GetProcAddress(m_hP3DPluginDll, "P3D_check_request");  
    P3D_request_finish = (P3D_request_finish_func *)GetProcAddress(m_hP3DPluginDll, "P3D_request_finish");  
    P3D_instance_feed_url_stream = (P3D_instance_feed_url_stream_func *)GetProcAddress(m_hP3DPluginDll, "P3D_instance_feed_url_stream");  

    P3D_instance_set_browser_script_object = (P3D_instance_set_browser_script_object_func *)GetProcAddress(m_hP3DPluginDll, "P3D_instance_set_browser_script_object");
    P3D_instance_get_panda_script_object = (P3D_instance_get_panda_script_object_func *)GetProcAddress(m_hP3DPluginDll, "P3D_instance_get_panda_script_object");
    P3D_make_class_definition = (P3D_make_class_definition_func *)GetProcAddress(m_hP3DPluginDll, "P3D_make_class_definition");

    P3D_new_undefined_object = (P3D_new_undefined_object_func *)GetProcAddress(m_hP3DPluginDll, "P3D_new_undefined_object");
    P3D_new_none_object = (P3D_new_none_object_func *)GetProcAddress(m_hP3DPluginDll, "P3D_new_none_object");
    P3D_new_bool_object = (P3D_new_bool_object_func *)GetProcAddress(m_hP3DPluginDll, "P3D_new_bool_object");
    P3D_new_int_object = (P3D_new_int_object_func *)GetProcAddress(m_hP3DPluginDll, "P3D_new_int_object");
    P3D_new_float_object = (P3D_new_float_object_func *)GetProcAddress(m_hP3DPluginDll, "P3D_new_float_object");
    P3D_new_string_object = (P3D_new_string_object_func *)GetProcAddress(m_hP3DPluginDll, "P3D_new_string_object");

    // Ensure that all of the function pointers have been found.
    if (P3D_initialize == NULL ||
        P3D_new_instance == NULL ||
        P3D_instance_finish == NULL ||
        P3D_instance_get_request == NULL ||
        P3D_check_request == NULL ||
        P3D_request_finish == NULL ||
        P3D_instance_get_panda_script_object == NULL ||
        P3D_instance_set_browser_script_object == NULL ||
        P3D_instance_feed_url_stream == NULL ||
        P3D_make_class_definition == NULL ||
        P3D_new_none_object == NULL ||
        P3D_new_bool_object == NULL ||
        P3D_new_int_object == NULL ||
        P3D_new_float_object == NULL ||
        P3D_new_string_object == NULL ) 
    {
            return ( error = 1 );
    }

    // Successfully loaded.
    nout << "Initializing P3D P3D_API_VERSION=" << P3D_API_VERSION << "\n";
    if ( !P3D_initialize( P3D_API_VERSION, "", "", true, "", "", "", false ) ) 
    {
        // Oops, failure to initialize.
        nout << "Error initializing P3D: " << GetLastError() << "\n"; 
        return ( error = 1 );
    }

    return error ;
}

int PPInstance::UnloadPlugin()
{
    int error( 0 );

    if ( m_hP3DPluginDll )
    {
        nout << "Unloading P3D dll \n";  
        if ( !::FreeLibrary( m_hP3DPluginDll ) )
        {
            nout << "Error unloading P3D dll :" << GetLastError << "\n";
            error = 1;
        }
        else
        {  
            m_hP3DPluginDll = NULL;
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
        return 0;
    }
    CComPtr<IDispatch> pDispatch;
    PPBrowserObject *pobj = new PPBrowserObject( &m_parentCtrl, pDispatch );
    P3D_instance_set_browser_script_object( m_p3dInstance, pobj );
    
    m_p3dObject = P3D_instance_get_panda_script_object( m_p3dInstance );
    P3D_OBJECT_INCREF( m_p3dObject );
    
    P3D_instance_setup_window( m_p3dInstance, P3D_WT_embedded, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, parent_window );

    nout << "Starting new P3D instance " << p3dFilename << "\n";
    if ( !P3D_instance_start( m_p3dInstance, true, p3dFilename.c_str() ) )
    {
        nout << "Error starting P3D instance: " << GetLastError() << "\n"; 
    }

    return 1;
}

CString PPInstance::GetP3DFilename( )
{
    CString p3dFilename;
    for ( UINT i = 0; i < m_parentCtrl.m_parameters.size(); i++ )
    {
        std::pair< CString, CString > keyAndValue = m_parentCtrl.m_parameters[ i ];
        if ( keyAndValue.first == "src" )
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



