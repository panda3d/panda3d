// Filename: P3DActiveXCtrl.h
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

// P3DActiveXCtrl.h : Declaration of the CP3DActiveXCtrl ActiveX Control class.
#include "PPInstance.h"
#include "PPPandaObject.h"
#include "PPInterface.h"
#include "get_twirl_data.h"
#include "Mshtml.h"

#include <vector>

// CP3DActiveXCtrl : See P3DActiveXCtrl.cpp for implementation.

class CP3DActiveXCtrl : public COleControl,
                        public PPInterface
{
    DECLARE_DYNCREATE(CP3DActiveXCtrl)
    
// Constructor
public:
    CP3DActiveXCtrl();

// Overrides
public:
    virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
    virtual void OnClose( DWORD dwSaveOption );
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void DoPropExchange(CPropExchange* pPX);
    virtual void OnResetState();
    virtual DWORD GetControlFlags();

    int BeginDownload(CString& url);

    PPInstance m_instance;

// Implementation
protected:
    ~CP3DActiveXCtrl();

    DECLARE_OLECREATE_EX(CP3DActiveXCtrl)    // Class factory and guid
    DECLARE_OLETYPELIB(CP3DActiveXCtrl)      // GetTypeInfo
    DECLARE_PROPPAGEIDS(CP3DActiveXCtrl)     // Property page IDs
    DECLARE_OLECTLTYPE(CP3DActiveXCtrl)     // Type name and misc status

    // Subclassed control support
    BOOL IsSubclassedControl();
    LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);

// Message maps
    DECLARE_MESSAGE_MAP()

// Dispatch maps
    DECLARE_DISPATCH_MAP()

// Event maps
    DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
    enum {
        dispidmain = 1
    };
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

    static void st_init(void *data);
    int init();
    virtual P3D_object* GetP3DObject( );
    virtual IOleClientSite* GetClientSte();

    // ActiveX properties <PARAM NAME="name" VALUE="value">
    std::vector< std::pair < CString, CString > > m_parameters; 

    CString m_hostingPageUrl;

protected:
    HRESULT ExchangeProperties( CPropExchange* pPropBag );

    LRESULT OnPandaNotification(WPARAM wParam, LPARAM lParam);
    void OnmainChanged(void);

    void get_twirl_bitmaps();
    static void CALLBACK timer_callback(HWND hwnd, UINT msg, UINT_PTR id, DWORD time);

    PPandaObject* m_pPandaObject;

    CComPtr<IOleClientSite> m_spClientSite;

    CBitmap _twirl_bitmaps[twirl_num_steps + 1];

    enum State {
      S_init,     // before starting the download
      S_loading,  // the instance is downloading
      // From S_loading, only the "init" thread may change the state.

      S_ready,    // the instance is ready to run
      S_started,  // the instance has successfully started
      S_failed,   // something went wrong
    };
    State _state;
    DWORD _init_time;
    CEvent _init_not_running;  // set when the init thread has finished, or before it has started.
};

