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
	DECLARE_OLECTLTYPE(CP3DActiveXCtrl)		// Type name and misc status

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

    int Init( );
    virtual P3D_object* GetP3DObject( );
    virtual IOleClientSite* GetClientSte();

    // ActiveX properties <PARAM NAME="name" VALUE="value">
    std::vector< std::pair < CString, CString > > m_parameters; 

    CString m_hostingPageUrl;

protected:
    HRESULT ExchangeProperties( CPropExchange* pPropBag );

    LRESULT OnPandaNotification(WPARAM wParam, LPARAM lParam);

    PPandaObject* m_pPandaObject;

    CComPtr<IOleClientSite> m_spClientSite;

    void OnmainChanged(void);
};

