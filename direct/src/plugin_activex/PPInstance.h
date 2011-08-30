// Filename: PPInstance.h
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
#include <vector>
#include <math.h>
#include "afxmt.h"

#include "p3d_plugin.h"
#include "PPDownloadCallback.h"
#include "PPLogger.h"
#include "fileSpec.h"
#include "load_plugin.h"

#define WM_PY_LAUNCHED        (WM_USER + 1)
#define WM_PROGRESS           (WM_USER + 2)
#define WM_PANDA_NOTIFICATION (WM_USER + 3)

class CP3DActiveXCtrl;

class PPInstance
{
public:
    PPInstance( CP3DActiveXCtrl& parentCtrl );
    virtual ~PPInstance( );

    void read_tokens();
    int DownloadP3DComponents( std::string& p3dDllFilename );

    int LoadPlugin( const std::string& dllFilename );
    int UnloadPlugin( void );

    static void ref_plugin();
    static void unref_plugin();

    int Start( const std::string& p3dFileName );
    int Stop( );

    std::string GetHostUrl( );
    std::string GetP3DFilename( );

    static void HandleRequestLoop();

    inline bool IsInit() { return m_isInit; }

    HWND m_parentWnd;
    CEvent m_eventStop;
    CEvent m_eventDownloadStopped;

    P3D_object* m_p3dObject;

    // Set from fgcolor & bgcolor.
    int _fgcolor_r, _fgcolor_b, _fgcolor_g;
    int _bgcolor_r, _bgcolor_b, _bgcolor_g;

protected:
    PPInstance( );
    PPInstance( const PPInstance& );

    int DownloadFile( const std::string& from, const std::string& to );
    int CopyFile( const std::string& from, const std::string& to );

    bool read_contents_file(const std::string &contents_filename, bool fresh_download);
    void find_host(TiXmlElement *xcontents);
    void read_xhost(TiXmlElement *xhost);
    void add_mirror(std::string mirror_url);
    void choose_random_mirrors(std::vector<std::string> &result, int num_mirrors);

    bool HandleRequest( P3D_request *request );
    static void HandleRequestGetUrl( void *data );

    static int compare_seq(const string &seq_a, const string &seq_b);
    static int compare_seq_int(const char *&num_a, const char *&num_b);

    void set_failed();

    std::string lookup_token(const std::string &keyword) const;
    bool has_token(const std::string &keyword) const;

    P3D_instance* m_p3dInstance;
    CP3DActiveXCtrl& m_parentCtrl;
    PPLogger m_logger;

    bool m_isInit;
    bool m_pluginLoaded;
    CMutex _load_mutex;

    std::string _download_url_prefix;
    typedef std::vector<std::string> Mirrors;
    Mirrors _mirrors;
    string _coreapi_set_ver;
    FileSpec _coreapi_dll;
    time_t _contents_expiration;
    bool _failed;

    P3D_token *_tokens;
    int _num_tokens;

    std::string m_rootDir;
    std::wstring m_rootDir_w;

    class ThreadedRequestData {
    public:
      PPInstance *_self;
      P3D_request *_request;
      std::string _host_url;
    };
};
