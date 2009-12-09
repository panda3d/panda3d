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
#include <algorithm>

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

// We can include this header file to get the DTOOL_PLATFORM
// definition, even though we don't link with dtool.
#include "dtool_platform.h"
#include "pandaVersion.h"

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
  // We need the root dir first.
  m_rootDir = find_root_dir( );

  // Then open the logfile.
  m_logger.Open( m_rootDir );

  m_pluginLoaded = false;
  _failed = false;
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
    PPDownloadRequest p3dFileDownloadRequest( *this, to ); 
    PPDownloadCallback dcForFile( p3dFileDownloadRequest );

    nout << "Downloading " << from << " into " << to << "\n";
    HRESULT hr = ::URLOpenStream( m_parentCtrl.GetControllingUnknown(), from.c_str(), 0, &dcForFile );
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

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::read_contents_file
//       Access: Private
//  Description: Reads the contents.xml file and starts the core API
//               DLL downloading, if necessary.
////////////////////////////////////////////////////////////////////
bool PPInstance::
read_contents_file(const string &contents_filename) {
  TiXmlDocument doc(contents_filename.c_str());
  if (!doc.LoadFile()) {
    return false;
  }

  TiXmlElement *xcontents = doc.FirstChildElement("contents");
  if (xcontents != NULL) {
    // Look for the <host> entry; it might point us at a different
    // download URL, and it might mention some mirrors.
    find_host(xcontents);

    // Now look for the core API package.
    TiXmlElement *xpackage = xcontents->FirstChildElement("package");
    while (xpackage != NULL) {
      const char *name = xpackage->Attribute("name");
      if (name != NULL && strcmp(name, "coreapi") == 0) {
        const char *platform = xpackage->Attribute("platform");
        if (platform != NULL && strcmp(platform, DTOOL_PLATFORM) == 0) {
          _core_api_dll.load_xml(xpackage);
          return true;
        }
      }
    
      xpackage = xpackage->NextSiblingElement("package");
    }
  }

  // Couldn't find the coreapi package description.
  nout << "No coreapi package defined in contents file for "
       << DTOOL_PLATFORM << "\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::find_host
//       Access: Private
//  Description: Scans the <contents> element for the matching <host>
//               element.
////////////////////////////////////////////////////////////////////
void PPInstance::
find_host(TiXmlElement *xcontents) {
  string host_url = PANDA_PACKAGE_HOST_URL;
  TiXmlElement *xhost = xcontents->FirstChildElement("host");
  if (xhost != NULL) {
    const char *url = xhost->Attribute("url");
    if (url != NULL && host_url == string(url)) {
      // We're the primary host.  This is the normal case.
      read_xhost(xhost);
      return;

    } else {
      // We're not the primary host; perhaps we're an alternate host.
      TiXmlElement *xalthost = xhost->FirstChildElement("alt_host");
      while (xalthost != NULL) {
        const char *url = xalthost->Attribute("url");
        if (url != NULL && host_url == string(url)) {
          // Yep, we're this alternate host.
          read_xhost(xhost);
          return;
        }
        xalthost = xalthost->NextSiblingElement("alt_host");
      }
    }

    // Hmm, didn't find the URL we used mentioned.  Assume we're the
    // primary host.
    read_xhost(xhost);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::read_xhost
//       Access: Private
//  Description: Reads the host data from the <host> (or <alt_host>)
//               entry in the contents.xml file.
////////////////////////////////////////////////////////////////////
void PPInstance::
read_xhost(TiXmlElement *xhost) {
  // Get the "download" URL, which is the source from which we
  // download everything other than the contents.xml file.
  const char *download_url = xhost->Attribute("download_url");
  if (download_url != NULL) {
    _download_url_prefix = download_url;
  } else {
    _download_url_prefix = PANDA_PACKAGE_HOST_URL;
  }
  if (!_download_url_prefix.empty()) {
    if (_download_url_prefix[_download_url_prefix.size() - 1] != '/') {
      _download_url_prefix += "/";
    }
  }
        
  TiXmlElement *xmirror = xhost->FirstChildElement("mirror");
  while (xmirror != NULL) {
    const char *url = xmirror->Attribute("url");
    if (url != NULL) {
      add_mirror(url);
    }
    xmirror = xmirror->NextSiblingElement("mirror");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::add_mirror
//       Access: Private
//  Description: Adds a new URL to serve as a mirror for this host.
//               The mirrors will be consulted first, before
//               consulting the host directly.
////////////////////////////////////////////////////////////////////
void PPInstance::
add_mirror(std::string mirror_url) {
  // Ensure the URL ends in a slash.
  if (!mirror_url.empty() && mirror_url[mirror_url.size() - 1] != '/') {
    mirror_url += '/';
  }
  
  // Add it to the _mirrors list, but only if it's not already
  // there.
  if (std::find(_mirrors.begin(), _mirrors.end(), mirror_url) == _mirrors.end()) {
    _mirrors.push_back(mirror_url);
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: PPInstance::choose_random_mirrors
//       Access: Public
//  Description: Selects num_mirrors elements, chosen at random, from
//               the _mirrors list.  Adds the selected mirrors to
//               result.  If there are fewer than num_mirrors elements
//               in the list, adds only as many mirrors as we can get.
////////////////////////////////////////////////////////////////////
void PPInstance::
choose_random_mirrors(std::vector<std::string> &result, int num_mirrors) {
  std::vector<size_t> selected;

  size_t num_to_select = min(_mirrors.size(), (size_t)num_mirrors);
  while (num_to_select > 0) {
    size_t i = (size_t)(((double)rand() / (double)RAND_MAX) * _mirrors.size());
    while (std::find(selected.begin(), selected.end(), i) != selected.end()) {
      // Already found this i, find a new one.
      i = (size_t)(((double)rand() / (double)RAND_MAX) * _mirrors.size());
    }
    selected.push_back(i);
    result.push_back(_mirrors[i]);
    --num_to_select;
  }
}

int PPInstance::DownloadP3DComponents( std::string& p3dDllFilename )
{
    int error(0);

    // Start off by downloading contents.xml into a local temporary
    // file.  We get a unique temporary filename each time; this is a
    // small file and it's very important that we get the most current
    // version, not an old cached version.
    TCHAR tempFileName[ MAX_PATH ];
    if (!::GetTempFileName( m_rootDir.c_str(), "p3d", 0, tempFileName )) {
      nout << "GetTempFileName failed (folder is " << m_rootDir << ")\n";
      return 1;
    }
      
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

    error = DownloadFile( remoteContentsUrl, localContentsFileName );
    if ( !error )
    {
      if ( !read_contents_file( localContentsFileName ) )
        error = 1;
    }

    if ( error ) {
      // If we couldn't download or read the contents.xml file, check
      // to see if there's a good one on disk already, as a fallback.
      if ( !read_contents_file( finalContentsFileName ) )
        error = 1;

    } else {
      // If we have successfully read the downloaded version,
      // then move the downloaded version into the final location.
      mkfile_complete( finalContentsFileName, nout );
      CopyFile( localContentsFileName, finalContentsFileName );
    }

    // We don't need the temporary file any more.
    ::DeleteFile( localContentsFileName.c_str() );

    if (!error) {
      // OK, at this point we have successfully read contents.xml,
      // and we have a good file spec in _core_api_dll.
      if (_core_api_dll.quick_verify(m_rootDir)) {
        // The DLL is already on-disk, and is good.
        p3dDllFilename = _core_api_dll.get_pathname(m_rootDir);
      } else {
        // The DLL is not already on-disk, or it's stale.  Go get it.
        std::string p3dLocalModuleFileName(_core_api_dll.get_pathname(m_rootDir));
        mkfile_complete(p3dLocalModuleFileName, nout);

        // Try one of the mirrors first.
        std::vector<std::string> mirrors;
        choose_random_mirrors(mirrors, 2);

        error = 1;
        for (std::vector<std::string>::iterator si = mirrors.begin();
             si != mirrors.end() && error; 
             ++si) {
          std::string url = (*si) + _core_api_dll.get_filename();
          error = DownloadFile(url, p3dLocalModuleFileName);
          if (!error && !_core_api_dll.full_verify(m_rootDir)) {
            // If it's not right after downloading, it's an error.
            error = 1;
          }
        }

        // If that failed, go get it from the authoritative host.
        if (error) {
          std::string url = _download_url_prefix + _core_api_dll.get_filename();
          error = DownloadFile(url, p3dLocalModuleFileName);
          if (!error && !_core_api_dll.full_verify(m_rootDir)) {
            error = 1;
          }
        }

        // If *that* failed, go get it again from the same URL, this
        // time with a query prefix to bust through any caches.
        if (error) {
          std::ostringstream strm;
          strm << _download_url_prefix << _core_api_dll.get_filename();
          strm << "?" << time(NULL);

          std::string url = strm.str();
          error = DownloadFile(url, p3dLocalModuleFileName);
          if (!error && !_core_api_dll.full_verify(m_rootDir)) {
            nout << "After download, " << _core_api_dll.get_filename()
                 << " is no good.\n";
            error = 1;
          }
        }

        if (!error) {
          // Downloaded successfully.
          p3dDllFilename = _core_api_dll.get_pathname(m_rootDir);
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
      if (!load_plugin(pathname, "", "", true, "", "", "", false, false, nout)) {
        nout << "Unable to launch core API in " << pathname << "\n";
        set_failed();
        error = 1;
      } else {
#ifdef PANDA_OFFICIAL_VERSION
        static const bool official = true;
#else
        static const bool official = false;
#endif
        P3D_set_plugin_version(P3D_PLUGIN_MAJOR_VERSION, P3D_PLUGIN_MINOR_VERSION,
                               P3D_PLUGIN_SEQUENCE_VERSION, official,
                               PANDA_DISTRIBUTOR,
                               PANDA_PACKAGE_HOST_URL, _core_api_dll.get_timestamp());

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
    memset(&parent_window, 0, sizeof(parent_window));
    parent_window._window_handle_type = P3D_WHT_win_hwnd;
    parent_window._handle._win_hwnd._hwnd = m_parentCtrl.m_hWnd;

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
    CComPtr<IDispatchEx> pDispatch;
    PPBrowserObject *pobj = new PPBrowserObject( &m_parentCtrl, pDispatch );
    P3D_instance_set_browser_script_object( m_p3dInstance, pobj );
    P3D_OBJECT_DECREF( pobj );
    
    m_p3dObject = P3D_instance_get_panda_script_object( m_p3dInstance );
    P3D_OBJECT_INCREF( m_p3dObject );
    
    P3D_instance_setup_window( m_p3dInstance, P3D_WT_embedded, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, &parent_window );

    nout << "Starting new P3D instance " << p3dFilename << "\n";

    if ( !P3D_instance_start( m_p3dInstance, false, p3dFilename.c_str(), 0 ) )
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

    P3D_result_code result_code = P3D_RC_done;
    if ( FAILED( hr ) )
    {
        nout << "Error handling P3D_RT_get_url request" << " :" << hr << "\n"; 
        result_code = P3D_RC_generic_error;
    }

    P3D_instance_feed_url_stream( 
        request->_instance, 
        request->_request._get_url._unique_id, 
        result_code, 
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

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::set_failed
//       Access: Private
//  Description: Called when something has gone wrong that prevents
//               the plugin instance from running.  Specifically, this
//               means it failed to load the core API.
////////////////////////////////////////////////////////////////////
void PPInstance::
set_failed() {
  if (!_failed) {
    _failed = true;

    nout << "Plugin failed.\n";

    string expression;
    // Look for the "onpluginfail" token.
    
    for (UINT i = 0; i < m_parentCtrl.m_parameters.size(); i++) {
      std::pair<CString, CString> keyAndValue = m_parentCtrl.m_parameters[i];
      // Make the token lowercase, since HTML is case-insensitive but
      // we're not.
      const CString &orig_keyword = m_parentCtrl.m_parameters[i].first;
      string keyword;
      for (const char *p = orig_keyword; *p; ++p) {
        keyword += tolower(*p);
      }
      
      if (keyword == "onpluginfail") {
        expression = m_parentCtrl.m_parameters[i].second;
        break;
      }
    }

    if (!expression.empty()) {
      // Now attempt to evaluate the expression.
      COleVariant varResult;
      CComPtr<IDispatch> pDispatch;

      CString evalExpression( expression.c_str() );
      HRESULT hr = m_parentCtrl.EvalExpression( pDispatch, evalExpression , varResult ); 
      if (FAILED(hr)) {
        nout << "Unable to eval " << expression << "\n";
      } else {
        nout << "Eval " << expression << "\n";
      }
    }
  }
}



