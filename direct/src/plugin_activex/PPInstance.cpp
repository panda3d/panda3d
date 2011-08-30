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
#include "parse_color.h"
#include "wstring_encode.h"

// We can include this header file to get the DTOOL_PLATFORM
// definition, even though we don't link with dtool.
#include "dtool_platform.h"
#include "pandaVersion.h"

#define P3D_CONTENTS_FILENAME "contents.xml"
#define P3D_DEFAULT_PLUGIN_FILENAME "p3d_plugin.dll"

static int s_instanceCount = 0;
void P3D_NotificationSync(P3D_instance *instance)
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
    m_parentCtrl( parentCtrl ), m_p3dInstance( NULL ), m_p3dObject( NULL ), m_isInit( false )
{
  // We need the root dir first.
  m_rootDir = find_root_dir( );
  string_to_wstring(m_rootDir_w, m_rootDir);

  // Then open the logfile.
  m_logger.Open( m_rootDir );

  m_pluginLoaded = false;
  _contents_expiration = 0;
  _failed = false;

  _tokens = NULL;
  _num_tokens = 0;

  // Ensure this event is initially in the "set" state, in case we
  // never get a download request before we get a close request.
  m_eventDownloadStopped.SetEvent( );
  m_eventStop.ResetEvent();

  nout << "Plugin is built with " << PANDA_PACKAGE_HOST_URL << "\n";
}

PPInstance::~PPInstance() {
  assert(_tokens == NULL);
}

// This is called at setup time to read the set of web tokens from the
// ActiveX control.
void PPInstance::
read_tokens() {
    assert(_tokens == NULL);
    _num_tokens = (int)m_parentCtrl.m_parameters.size();
    _tokens = new P3D_token[ _num_tokens ];
    for (int i = 0; i < _num_tokens; i++ ) {
      std::pair< CString, CString > keyAndValue = m_parentCtrl.m_parameters[ i ];
      // Make the token lowercase, since HTML is case-insensitive but
      // we're not.
      string keyword;
      for (const char *p = m_parentCtrl.m_parameters[ i ].first; *p; ++p) {
        keyword += tolower(*p);
      }
      
      _tokens[i]._keyword = strdup( keyword.c_str() ); 
      _tokens[i]._value = strdup( m_parentCtrl.m_parameters[ i ].second );
    }
    
    // fgcolor and bgcolor are useful to know here (in case we have to
    // draw a twirling icon).
    
    // The default bgcolor is white.
    _bgcolor_r = _bgcolor_g = _bgcolor_b = 0xff;
    if (has_token("bgcolor")) {
      int r, g, b;
      if (parse_color(r, g, b, lookup_token("bgcolor"))) {
        _bgcolor_r = r;
        _bgcolor_g = g;
        _bgcolor_b = b;
      }
    }
    
    // The default fgcolor is either black or white, according to the
    // brightness of the bgcolor.
    if (_bgcolor_r + _bgcolor_g + _bgcolor_b > 0x80 + 0x80 + 0x80) {
      _fgcolor_r = _fgcolor_g = _fgcolor_b = 0x00;
    } else {
      _fgcolor_r = _fgcolor_g = _fgcolor_b = 0xff;
    }
    if (has_token("fgcolor")) {
      int r, g, b;
      if (parse_color(r, g, b, lookup_token("fgcolor"))) {
        _fgcolor_r = r;
        _fgcolor_g = g;
        _fgcolor_b = b;
      }
    }
}

int PPInstance::DownloadFile( const std::string& from, const std::string& to )
{
    int error( 0 );
	HRESULT hr( S_OK ); 

    nout << "Downloading " << from << " into " << to << "\n";
	{
		PPDownloadRequest p3dFileDownloadRequest( *this, to ); 
		PPDownloadCallback dcForFile( p3dFileDownloadRequest );
		hr = ::URLOpenStream( m_parentCtrl.GetControllingUnknown(), from.c_str(), 0, &dcForFile );
	}
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
//  Description: Attempts to open and read the contents.xml file on
//               disk.  Copies the file to its standard location
//               on success.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool PPInstance::
read_contents_file(const string &contents_filename, bool fresh_download) {
  TiXmlDocument doc(contents_filename.c_str());
  if (!doc.LoadFile()) {
    return false;
  }

  bool found_core_package = false;

  TiXmlElement *xcontents = doc.FirstChildElement("contents");
  if (xcontents != NULL) {
    int max_age = P3D_CONTENTS_DEFAULT_MAX_AGE;
    xcontents->Attribute("max_age", &max_age);

    // Get the latest possible expiration time, based on the max_age
    // indication.  Any expiration time later than this is in error.
    time_t now = time(NULL);
    _contents_expiration = now + (time_t)max_age;

    if (fresh_download) {
      // Update the XML with the new download information.
      TiXmlElement *xorig = xcontents->FirstChildElement("orig");
      while (xorig != NULL) {
        xcontents->RemoveChild(xorig);
        xorig = xcontents->FirstChildElement("orig");
      }

      xorig = new TiXmlElement("orig");
      xcontents->LinkEndChild(xorig);
      
      xorig->SetAttribute("expiration", (int)_contents_expiration);

    } else {
      // Read the expiration time from the XML.
      int expiration = 0;
      TiXmlElement *xorig = xcontents->FirstChildElement("orig");
      if (xorig != NULL) {
        xorig->Attribute("expiration", &expiration);
      }
      
      _contents_expiration = min(_contents_expiration, (time_t)expiration);
    }

    nout << "read contents.xml, max_age = " << max_age
         << ", expires in " << max(_contents_expiration, now) - now
         << " s\n";

    // Look for the <host> entry; it might point us at a different
    // download URL, and it might mention some mirrors.
    find_host(xcontents);

    // Now look for the core API package.
    _coreapi_set_ver = "";
    TiXmlElement *xpackage = xcontents->FirstChildElement("package");
    while (xpackage != NULL) {
      const char *name = xpackage->Attribute("name");
      if (name != NULL && strcmp(name, "coreapi") == 0) {
        const char *platform = xpackage->Attribute("platform");
        if (platform != NULL && strcmp(platform, DTOOL_PLATFORM) == 0) {
          _coreapi_dll.load_xml(xpackage);
          const char *set_ver = xpackage->Attribute("set_ver");
          if (set_ver != NULL) {
            _coreapi_set_ver = set_ver;
          }
          found_core_package = true;
          break;
        }
      }
    
      xpackage = xpackage->NextSiblingElement("package");
    }
  }

  if (!found_core_package) {
    // Couldn't find the coreapi package description.
    nout << "No coreapi package defined in contents file for "
         << DTOOL_PLATFORM << "\n";
    return false;
  }

  // Check the coreapi_set_ver token.  If it is given, it specifies a
  // minimum Core API version number we expect to find.  If we didn't
  // find that number, perhaps our contents.xml is out of date.
  string coreapi_set_ver = lookup_token("coreapi_set_ver");
  if (!coreapi_set_ver.empty()) {
    nout << "Instance asked for Core API set_ver " << coreapi_set_ver
         << ", we found " << _coreapi_set_ver << "\n";
    // But don't bother if we just freshly downloaded it.
    if (!fresh_download) {
      if (compare_seq(coreapi_set_ver, _coreapi_set_ver) > 0) {
        // The requested set_ver value is higher than the one we have on
        // file; our contents.xml file must be out of date after all.
        nout << "expiring contents.xml\n";
        _contents_expiration = 0;
      }
    }
  }

  // Success.  Now save the file in its proper place.
  string standard_filename = m_rootDir + "/contents.xml";

  mkfile_complete(standard_filename, nout);
  if (!doc.SaveFile(standard_filename.c_str())) {
    nout << "Couldn't rewrite " << standard_filename << "\n";
    return false;
  }
  
  return true;
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

    // Get the pathname of the local copy of the contents.xml file.
    std::string finalContentsFileName( m_rootDir );
    finalContentsFileName += "/";
    finalContentsFileName += P3D_CONTENTS_FILENAME;

    // Check to see if the version on disk is already current enough.
    bool already_got = false;
    if (read_contents_file(finalContentsFileName, false)) {
      if (time(NULL) < _contents_expiration) {
        // Got the file, and it's good.
        already_got = true;
      }
    }

    if (!already_got) {
      // OK, we need to download a new contents.xml file.  Start off
      // by downloading it into a local temporary file.
      WCHAR local_filename_w[ MAX_PATH ];
      if (!::GetTempFileNameW( m_rootDir_w.c_str(), L"p3d", 0, local_filename_w )) {
        nout << "GetTempFileName failed (folder is " << m_rootDir << ")\n";
        return 1;
      }
      
      std::string local_filename;
      wstring_to_string(local_filename, local_filename_w);

      std::string hostUrl( PANDA_PACKAGE_HOST_URL );
      if (!hostUrl.empty() && hostUrl[hostUrl.size() - 1] != '/') {
        hostUrl += '/';
      }
      
      // Append a query string to the contents.xml URL to uniquify it
      // and ensure we don't get a cached version.
      std::ostringstream strm;
      strm << hostUrl << P3D_CONTENTS_FILENAME << "?" << time(NULL);
      std::string remoteContentsUrl( strm.str() );
      
      error = DownloadFile( remoteContentsUrl, local_filename );
      if ( !error ) {
        if ( !read_contents_file( local_filename, true ) )
          error = 1;
      }

      if ( error ) {
        // If we couldn't download or read the contents.xml file, check
        // to see if there's a good one on disk already, as a fallback.
        if ( !read_contents_file( finalContentsFileName, false ) )
          error = 1;
      }

      // We don't need the temporary file any more.
      ::DeleteFileW( local_filename_w );
    }
      
    if (!error) {
      // OK, at this point we have successfully read contents.xml,
      // and we have a good file spec in _coreapi_dll.
      if (_coreapi_dll.quick_verify(m_rootDir)) {
        // The DLL is already on-disk, and is good.
        p3dDllFilename = _coreapi_dll.get_pathname(m_rootDir);
      } else {
        // The DLL is not already on-disk, or it's stale.  Go get it.
        std::string p3dLocalModuleFileName(_coreapi_dll.get_pathname(m_rootDir));
        mkfile_complete(p3dLocalModuleFileName, nout);

        // Try one of the mirrors first.
        std::vector<std::string> mirrors;
        choose_random_mirrors(mirrors, 2);

        error = 1;
        for (std::vector<std::string>::iterator si = mirrors.begin();
             si != mirrors.end() && error; 
             ++si) {
          std::string url = (*si) + _coreapi_dll.get_filename();
          error = DownloadFile(url, p3dLocalModuleFileName);
          if (!error && !_coreapi_dll.full_verify(m_rootDir)) {
            // If it's not right after downloading, it's an error.
            error = 1;
          }
        }

        // If that failed, go get it from the authoritative host.
        if (error) {
          std::string url = _download_url_prefix + _coreapi_dll.get_filename();
          error = DownloadFile(url, p3dLocalModuleFileName);
          if (!error && !_coreapi_dll.full_verify(m_rootDir)) {
            error = 1;
          }
        }

        // If *that* failed, go get it again from the same URL, this
        // time with a query prefix to bust through any caches.
        if (error) {
          std::ostringstream strm;
          strm << _download_url_prefix << _coreapi_dll.get_filename();
          strm << "?" << time(NULL);

          std::string url = strm.str();
          error = DownloadFile(url, p3dLocalModuleFileName);
          if (!error && !_coreapi_dll.full_verify(m_rootDir)) {
            nout << "After download, " << _coreapi_dll.get_filename()
                 << " is no good.\n";
            error = 1;
          }
        }

        if (!error) {
          // Downloaded successfully.
          p3dDllFilename = _coreapi_dll.get_pathname(m_rootDir);
        }
      }
    }

    if (error) {
      set_failed();
    }
    return error;
}

int PPInstance::LoadPlugin( const std::string& dllFilename ) 
{
    CSingleLock lock(&_load_mutex);
    lock.Lock();
    if ( !m_pluginLoaded )
    { 
        ref_plugin();
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
      string contents_filename = m_rootDir + "/contents.xml";
      if (!load_plugin(pathname, contents_filename, PANDA_PACKAGE_HOST_URL,
                       P3D_VC_normal, "", "", "", false, false, 
                       m_rootDir, "", nout)) {
        nout << "Unable to launch core API in " << pathname << "\n";
        error = 1;
      } else {

        // Format the coreapi_timestamp as a string, for passing as a
        // parameter.
        ostringstream stream;
        stream << _coreapi_dll.get_timestamp();
        string coreapi_timestamp = stream.str();

#ifdef PANDA_OFFICIAL_VERSION
        static const bool official = true;
#else
        static const bool official = false;
#endif
        P3D_set_plugin_version_ptr(P3D_PLUGIN_MAJOR_VERSION, P3D_PLUGIN_MINOR_VERSION,
                                   P3D_PLUGIN_SEQUENCE_VERSION, official,
                                   PANDA_DISTRIBUTOR,
                                   PANDA_PACKAGE_HOST_URL, coreapi_timestamp.c_str(),
                                   _coreapi_set_ver.c_str());

      }
    }

    if (error) {
      set_failed();
    }
    return error ;
}

int PPInstance::UnloadPlugin()
{
    CSingleLock lock(&_load_mutex);
    lock.Lock();

    int error( 0 );

    if ( m_pluginLoaded )
    { 
        m_pluginLoaded = false;
        m_isInit = false;
        unref_plugin();
    }
    return error;
}

// Increments the reference count on the "plugin" library (i.e. the
// core API).  Call unref_plugin() later to decrement this count.
void PPInstance::
ref_plugin() {
  s_instanceCount += 1;
}

// Decrements the reference count on the "plugin" library.  This must
// correspond to an earlier call to ref_plugin().  When the last
// reference is removed, the plugin will be unloaded.
void PPInstance::
unref_plugin() {
  assert( s_instanceCount > 0 );
  s_instanceCount -= 1;
  
  if ( s_instanceCount == 0 && is_plugin_loaded() ) {
    nout << "Unloading core API\n";
    unload_plugin(nout);
    
    // This pointer is no longer valid and must be reset for next
    // time.
    PPBrowserObject::clear_class_definition();
  }
}

int PPInstance::Start( const std::string& p3dFilename  )
{
    {
      CSingleLock lock(&_load_mutex);
      lock.Lock();
      
      assert(!m_isInit);
      m_isInit = true;
    }

    P3D_window_handle parent_window;
    memset(&parent_window, 0, sizeof(parent_window));
    parent_window._window_handle_type = P3D_WHT_win_hwnd;
    parent_window._handle._win_hwnd._hwnd = m_parentCtrl.m_hWnd;

    RECT rect;
    GetClientRect( m_parentCtrl.m_hWnd, &rect );

    nout << "Creating new P3D instance object for " << p3dFilename << "\n";
    m_p3dInstance = P3D_new_instance_ptr( &P3D_NotificationSync, _tokens, _num_tokens, 0, NULL, (void*)&m_parentCtrl );

    if ( !m_p3dInstance )
    {
        nout << "Error creating P3D instance: " << GetLastError() << "\n"; 
        return 1;
    }
    CComPtr<IDispatchEx> pDispatch;
    PPBrowserObject *pobj = new PPBrowserObject( &m_parentCtrl, pDispatch );
    P3D_instance_set_browser_script_object_ptr( m_p3dInstance, pobj );
    P3D_OBJECT_DECREF( pobj );
    
    m_p3dObject = P3D_instance_get_panda_script_object_ptr( m_p3dInstance );
    P3D_OBJECT_INCREF( m_p3dObject );
    
    P3D_instance_setup_window_ptr( m_p3dInstance, P3D_WT_embedded, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, &parent_window );

    nout << "Starting new P3D instance " << p3dFilename << "\n";
    if ( !P3D_instance_start_ptr( m_p3dInstance, false, p3dFilename.c_str(), 0 ) )
    {
        nout << "Error starting P3D instance: " << GetLastError() << "\n";
        return 1;
    }

    return 0;
}

int PPInstance::Stop( )
{
	m_eventStop.SetEvent( );
	::WaitForSingleObject( m_eventDownloadStopped.m_hObject, INFINITE ); 
    if ( m_p3dInstance )
    {
        P3D_instance_finish_ptr( m_p3dInstance );
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

    if (_tokens != NULL) {
      for ( int j = 0; j < _num_tokens; ++j) {
        delete [] _tokens[j]._keyword;
        delete [] _tokens[j]._value;
      }
      delete [] _tokens;
      _tokens = NULL;
      _num_tokens = 0;
    }

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

void PPInstance::HandleRequestLoop() {
  P3D_instance *p3d_inst = P3D_check_request_ptr(0.0);
  while ( p3d_inst != NULL ) {
    P3D_request *request = P3D_instance_get_request_ptr(p3d_inst);
    if ( request != NULL ) {
      CP3DActiveXCtrl* parent = ( CP3DActiveXCtrl* )(p3d_inst->_user_data);
      if ( parent ) {
        if (!parent->m_instance.HandleRequest( request )) {
          // If handling the request is meant to yield control
          // temporarily to JavaScript (e.g. P3D_RT_callback), then do
          // so now.
          return;
        }
          
      } else {
        nout << "Error handling P3D request. Instance's user data is not a Control \n";
      }
    }
    p3d_inst = P3D_check_request_ptr(0.0);
  }
}

void PPInstance::HandleRequestGetUrl( void* data )
{
	HRESULT hr( S_OK );
    ThreadedRequestData *trdata = static_cast<ThreadedRequestData*>( data );
    PPInstance *self = trdata->_self;
    P3D_request *request = trdata->_request;
    std::string host_url = trdata->_host_url;
    delete trdata;

    if ( !request )
    {
        return;
    }
    int unique_id = request->_request._get_url._unique_id;
    std::string url = request->_request._get_url._url;
    CP3DActiveXCtrl* parent = static_cast<CP3DActiveXCtrl*> ( request->_instance->_user_data );

    if ( !parent )
    {
        return;
    }

    nout << "Handling P3D_RT_get_url request from " << url << "\n";

    // Convert a relative URL into a full URL.
    size_t colon = url.find(':');
    size_t slash = url.find('/');
    if (colon == std::string::npos || colon > slash) {
      // Not a full URL, so it's a relative URL.  Prepend the current
      // URL.
      if (url.empty() || url[0] == '/') {
        // It starts with a slash, so go back to the root of this
        // particular host.
        colon = host_url.find(':');
        if (colon != std::string::npos && 
            colon + 2 < host_url.size() && 
            host_url[colon + 1] == '/' && host_url[colon + 2] == '/') {
          slash = host_url.find('/', colon + 3);
          url = host_url.substr(0, slash) + url;
        }
      } else {
        // It doesn't start with a slash, so it's relative to this
        // page.
        url = host_url + url;
      }
      nout << "Made fullpath: " << url << "\n";
    }

	{
		PPDownloadRequest p3dObjectDownloadRequest( parent->m_instance, request ); 
		PPDownloadCallback bsc( p3dObjectDownloadRequest );
		hr = ::URLOpenStream( parent->GetControllingUnknown(), url.c_str(), 0, &bsc );
	}
    P3D_result_code result_code = P3D_RC_done;
    if ( FAILED( hr ) )
    {
        nout << "Error handling P3D_RT_get_url request" << " :" << hr << "\n"; 
        result_code = P3D_RC_generic_error;
    }

    P3D_instance_feed_url_stream_ptr( 
        request->_instance, 
        request->_request._get_url._unique_id, 
        result_code, 
        0, 
        0, 
        (const void*)NULL, 
        0 
        );
    P3D_request_finish_ptr( request, true );
}

bool PPInstance::
HandleRequest( P3D_request *request ) {
  if ( !request ) {
    return false;
  }
  bool handled = false;
  bool continue_loop = true;
  
  switch ( request->_request_type ) {
  case P3D_RT_stop:
    {
      P3D_instance_finish_ptr( request->_instance );
      handled = true;
      break;
    }
  case P3D_RT_get_url:
    {
      // We always handle url requests on a thread.
      ThreadedRequestData *trdata = new ThreadedRequestData;
      trdata->_self = this;
      trdata->_request = request;
      trdata->_host_url = GetHostUrl();

      _beginthread( HandleRequestGetUrl, 0, trdata );
      handled = true;
      return true;
    }
  case P3D_RT_callback:
    {
      // In the case of a callback, yield control temporarily to JavaScript.
      continue_loop = false;
      break;
    }
  default:
    {
      // Some request types are not handled.
      break;
    }
  };

  P3D_request_finish_ptr( request, handled );
  return continue_loop;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::compare_seq
//       Access: Private, Static
//  Description: Compares the two dotted-integer sequence values
//               numerically.  Returns -1 if seq_a sorts first, 1 if
//               seq_b sorts first, 0 if they are equivalent.
////////////////////////////////////////////////////////////////////
int PPInstance::
compare_seq(const string &seq_a, const string &seq_b) {
  const char *num_a = seq_a.c_str();
  const char *num_b = seq_b.c_str();
  int comp = compare_seq_int(num_a, num_b);
  while (comp == 0) {
    if (*num_a != '.') {
      if (*num_b != '.') {
        // Both strings ran out together.
        return 0;
      }
      // a ran out first.
      return -1;
    } else if (*num_b != '.') {
      // b ran out first.
      return 1;
    }

    // Increment past the dot.
    ++num_a;
    ++num_b;
    comp = compare_seq(num_a, num_b);
  }

  return comp;
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::compare_seq_int
//       Access: Private, Static
//  Description: Numerically compares the formatted integer value at
//               num_a with num_b.  Increments both num_a and num_b to
//               the next character following the valid integer.
////////////////////////////////////////////////////////////////////
int PPInstance::
compare_seq_int(const char *&num_a, const char *&num_b) {
  long int a;
  char *next_a;
  long int b;
  char *next_b;

  a = strtol((char *)num_a, &next_a, 10);
  b = strtol((char *)num_b, &next_b, 10);

  num_a = next_a;
  num_b = next_b;

  if (a < b) {
    return -1;
  } else if (b < a) {
    return 1;
  } else {
    return 0;
  }
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

    // Look for the "onpluginfail" token.
    string expression = lookup_token("onpluginfail");

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


////////////////////////////////////////////////////////////////////
//     Function: PPInstance::lookup_token
//       Access: Private
//  Description: Returns the value associated with the first
//               appearance of the named token, or empty string if the
//               token does not appear.
////////////////////////////////////////////////////////////////////
std::string PPInstance::
lookup_token(const std::string &keyword) const {
  for (int i = 0; i < _num_tokens; ++i) {
    if (strcmp(_tokens[i]._keyword, keyword.c_str()) == 0) {
      return _tokens[i]._value;
    }
  }

  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: PPInstance::has_token
//       Access: Private
//  Description: Returns true if the named token appears in the list,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool PPInstance::
has_token(const std::string &keyword) const {
  for (int i = 0; i < _num_tokens; ++i) {
    if (strcmp(_tokens[i]._keyword, keyword.c_str()) == 0) {
      return true;
    }
  }

  return false;
}

