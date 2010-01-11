// P3DActiveXCtrl.cpp : Implementation of the CP3DActiveXCtrl ActiveX Control class.

// Filename: P3DActiveXCtrl.cpp
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
#include "P3DActiveX.h"
#include "P3DActiveXCtrl.h"
#include "P3DActiveXPropPage.h"
#include "PPBrowserObject.h"

#include "Mshtml.h"
#include "atlconv.h"
#include "comutil.h"

#include <strstream>

class CPropbagPropExchange : public CPropExchange
{
public:
    CPropbagPropExchange(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog,
        BOOL bLoading, BOOL bSaveAllProperties=FALSE);
    ~CPropbagPropExchange();

// Operations
    virtual BOOL ExchangeProp(LPCTSTR pszPropName, VARTYPE vtProp,
                void* pvProp, const void* pvDefault = NULL);
    virtual BOOL ExchangeBlobProp(LPCTSTR pszPropName, HGLOBAL* phBlob,
                HGLOBAL hBlobDefault = NULL);
    virtual BOOL ExchangeFontProp(LPCTSTR pszPropName, CFontHolder& font,
                const FONTDESC* pFontDesc, LPFONTDISP pFontDispAmbient);
    virtual BOOL ExchangePersistentProp(LPCTSTR pszPropName,
                LPUNKNOWN* ppUnk, REFIID iid, LPUNKNOWN pUnkDefault);

// Implementation
    LPPROPERTYBAG m_pPropBag;
    LPERRORLOG m_pErrorLog;
    BOOL m_bSaveAllProperties;
};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CP3DActiveXCtrl, COleControl)



// Message map

BEGIN_MESSAGE_MAP(CP3DActiveXCtrl, COleControl)
    ON_MESSAGE(OCM_COMMAND, OnOcmCommand)
    ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
    ON_WM_CREATE()
    ON_MESSAGE(WM_PANDA_NOTIFICATION, OnPandaNotification)
END_MESSAGE_MAP()



// Dispatch map

BEGIN_DISPATCH_MAP(CP3DActiveXCtrl, COleControl)
    DISP_PROPERTY_NOTIFY_ID(CP3DActiveXCtrl, "main", dispidmain, m_pPandaObject, OnmainChanged, VT_DISPATCH)
END_DISPATCH_MAP()



// Event map

BEGIN_EVENT_MAP(CP3DActiveXCtrl, COleControl)
END_EVENT_MAP()



// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CP3DActiveXCtrl, 1)
    PROPPAGEID(CP3DActiveXPropPage::guid)
END_PROPPAGEIDS(CP3DActiveXCtrl)



// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CP3DActiveXCtrl, "P3DACTIVEX.P3DActiveXCtrl.1",
    0x924b4927, 0xd3ba, 0x41ea, 0x9f, 0x7e, 0x8a, 0x89, 0x19, 0x4a, 0xb3, 0xac)



// Type library ID and version

IMPLEMENT_OLETYPELIB(CP3DActiveXCtrl, _tlid, _wVerMajor, _wVerMinor)



// Interface IDs

const IID BASED_CODE IID_DP3DActiveX =
        { 0x76904D54, 0xCC5, 0x4DBB, { 0xB0, 0x22, 0xF4, 0x8B, 0x1E, 0x95, 0x18, 0x3B } };
const IID BASED_CODE IID_DP3DActiveXEvents =
        { 0x1B2413ED, 0x51C8, 0x495E, { 0xB9, 0x17, 0x98, 0x3C, 0x45, 0x9B, 0x8F, 0xC7 } };



// Control type information

static const DWORD BASED_CODE _dwP3DActiveXOleMisc =
    OLEMISC_ACTIVATEWHENVISIBLE |
    OLEMISC_SETCLIENTSITEFIRST |
    OLEMISC_INSIDEOUT |
    OLEMISC_CANTLINKINSIDE |
    OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CP3DActiveXCtrl, IDS_P3DACTIVEX, _dwP3DActiveXOleMisc)



// CP3DActiveXCtrl::CP3DActiveXCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CP3DActiveXCtrl

BOOL CP3DActiveXCtrl::CP3DActiveXCtrlFactory::UpdateRegistry(BOOL bRegister)
{
    // TODO: Verify that your control follows apartment-model threading rules.
    // Refer to MFC TechNote 64 for more information.
    // If your control does not conform to the apartment-model rules, then
    // you must modify the code below, changing the 6th parameter from
    // afxRegApartmentThreading to 0.

    if (bRegister)
        return AfxOleRegisterControlClass(
            AfxGetInstanceHandle(),
            m_clsid,
            m_lpszProgID,
            IDS_P3DACTIVEX,
            IDB_P3DACTIVEX,
            afxRegApartmentThreading,
            _dwP3DActiveXOleMisc,
            _tlid,
            _wVerMajor,
            _wVerMinor);
    else
        return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}



// CP3DActiveXCtrl::CP3DActiveXCtrl - Constructor

CP3DActiveXCtrl::CP3DActiveXCtrl() : m_pPandaObject( NULL ), m_instance( *this )
{
    InitializeIIDs(&IID_DP3DActiveX, &IID_DP3DActiveXEvents);
    // TODO: Initialize your control's instance data here.

}

// CP3DActiveXCtrl::~CP3DActiveXCtrl - Destructor

CP3DActiveXCtrl::~CP3DActiveXCtrl()
{
    // TODO: Cleanup your control's instance data here.
    if ( m_pPandaObject )
    {
        delete m_pPandaObject;
    }
}



// CP3DActiveXCtrl::OnDraw - Drawing function

void CP3DActiveXCtrl::OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
    if (!pdc)
        return;

    if ( !m_instance.IsInit( ) )
    {
        Init( );
    }

    CBrush brBackGnd(TranslateColor(AmbientBackColor()));
    pdc->FillRect(rcBounds, &brBackGnd);

    DoSuperclassPaint(pdc, rcBounds);
}

void CP3DActiveXCtrl::OnClose( DWORD dwSaveOption )
{
	m_instance.Stop();
	COleControl::OnClose( dwSaveOption );
}


// CP3DActiveXCtrl::DoPropExchange - Persistence support

void CP3DActiveXCtrl::DoPropExchange(CPropExchange* pPX)
{
    ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
    COleControl::DoPropExchange(pPX);

    // TODO: Call PX_ functions for each persistent custom property.
    
    ExchangeProperties( pPX );
}



// CP3DActiveXCtrl::GetControlFlags -
// Flags to customize MFC's implementation of ActiveX controls.
//
DWORD CP3DActiveXCtrl::GetControlFlags()
{
    DWORD dwFlags = COleControl::GetControlFlags();


    // The control will not be redrawn when making the transition
    // between the active and inactivate state.
    dwFlags |= noFlickerActivate;
    return dwFlags;
}



// CP3DActiveXCtrl::OnResetState - Reset control to default state

void CP3DActiveXCtrl::OnResetState()
{
    COleControl::OnResetState();  // Resets defaults found in DoPropExchange

    // TODO: Reset any other control state here.
}



// CP3DActiveXCtrl::PreCreateWindow - Modify parameters for CreateWindowEx

BOOL CP3DActiveXCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.lpszClass = _T("STATIC");
    return COleControl::PreCreateWindow(cs);
}



// CP3DActiveXCtrl::IsSubclassedControl - This is a subclassed control

BOOL CP3DActiveXCtrl::IsSubclassedControl()
{
    return TRUE;
}



// CP3DActiveXCtrl::OnOcmCommand - Handle command messages

LRESULT CP3DActiveXCtrl::OnOcmCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
    WORD wNotifyCode = HIWORD(wParam);
#else
    WORD wNotifyCode = HIWORD(lParam);
#endif

    // TODO: Switch on wNotifyCode here.

    return 0;
}

// CP3DActiveXCtrl message handlers

int CP3DActiveXCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (COleControl::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  Add your specialized creation code here

    int error(0);
    m_spClientSite = GetClientSite();
    if ( !m_spClientSite )
    {
        return ( error = -1 );
    }
    CComPtr<IOleContainer> pOleContainer; 
    HRESULT hr = m_spClientSite->GetContainer( &pOleContainer ); 
    if ( FAILED( hr ) || !pOleContainer )
    {
        return ( error = -1 );
    }
    CComPtr<IHTMLDocument2> pHtml2Doc; 
    hr = pOleContainer->QueryInterface( IID_IHTMLDocument, ( void** )&pHtml2Doc ); 
    if ( FAILED( hr ) || !pHtml2Doc ) 
    {
        return ( error = -1 );
    }
    BSTR url;
    hr = pHtml2Doc->get_URL( &url );
    if ( FAILED( hr ) ||  !url ) 
    {
        return ( error = -1 );
    }
    m_hostingPageUrl = url;
    m_hostingPageUrl.Replace( '\\', '/' );

    ::SysFreeString( url );

    CComPtr<IHTMLElementCollection> pHtmlElementCollect;
    hr = pHtml2Doc->get_scripts( &pHtmlElementCollect );
    if ( FAILED( hr ) || !pHtmlElementCollect )
    {
        return ( error = -1 );
    }
    long collectionLength = 0;
    hr = pHtmlElementCollect->get_length( &collectionLength );
    if ( FAILED( hr ) || !pHtmlElementCollect )
    {
        return ( error = -1 );
    }
    if ( collectionLength < 1 )
    {
        // javascript engine was not specified on the page.
        // hence we need to initialize it by infusing javascript 
        // element tags

        CComPtr<IHTMLElement> spHtmlElement; 
        hr = pHtml2Doc->createElement( CComBSTR( "script" ), &spHtmlElement );
        if ( SUCCEEDED( hr ) && spHtmlElement )
        {
            hr = spHtmlElement->put_language( CComBSTR( "javascript" ) );
            if ( SUCCEEDED( hr ) )
            {
                CComPtr<IHTMLDOMNode> spElementDomNode;
                hr = spHtmlElement->QueryInterface( IID_IHTMLDOMNode, ( void** )&spElementDomNode );
                if ( SUCCEEDED( hr ) && spElementDomNode )
                {
                    CComPtr<IHTMLDOMNode> spHtmlDomNode;
                    hr = pHtml2Doc->QueryInterface( IID_IHTMLDOMNode, ( void** )&spHtmlDomNode );
                    if ( SUCCEEDED( hr ) && spHtmlDomNode )
                    {
                        CComPtr<IHTMLDOMNode> newNode;
                        hr = spHtmlDomNode->appendChild( spElementDomNode, &newNode );
                    }
                }                    
            }
        }
    }
    return 0;
}

LRESULT CP3DActiveXCtrl::OnPandaNotification(WPARAM wParam, LPARAM lParam)
{
    PPInstance::HandleRequestLoop();

    return 0;
}

int CP3DActiveXCtrl::Init( )
{
    int error( 0 );
    std::string p3dDllFilename;

    error = m_instance.DownloadP3DComponents( p3dDllFilename );
    if ( !error && !( p3dDllFilename.empty() ) )
    {
        error = m_instance.LoadPlugin( p3dDllFilename );
        if ( !error )
        {
            m_pPandaObject = new PPandaObject( this, NULL );
            m_instance.Start( m_instance.GetP3DFilename( ) );
        }
    }
    return error;
}


HRESULT CP3DActiveXCtrl::ExchangeProperties( CPropExchange*  pPX )
{
    USES_CONVERSION;
    HRESULT hr = E_FAIL;

    if ( !pPX )
    {
        return hr;
    }
    CPropbagPropExchange* pPropExchange = dynamic_cast<CPropbagPropExchange*>( pPX );
    if ( !pPropExchange || !pPropExchange->m_pPropBag )
    {
        return hr;
    }

    // get the property bag version 2
    IPropertyBag2 * pPropBag2;
    hr = pPropExchange->m_pPropBag->QueryInterface( IID_IPropertyBag2, (void**)&pPropBag2);
    if (! SUCCEEDED (hr)) {
        return hr;
    }

    // get the number of parameters
    unsigned long aNum;
    hr = pPropBag2->CountProperties (& aNum);
    ATLASSERT (hr >= 0);
    if (! SUCCEEDED (hr)) {
        return hr;
    }
    if (aNum == 0) return S_OK;

    // allocate the parameters names
    PROPBAG2 * aPropNames = new PROPBAG2[aNum];
    unsigned long aReaded;

    // get the param names
    hr = pPropBag2->GetPropertyInfo (0, aNum, aPropNames, & aReaded);
    ATLASSERT (hr >= 0);
    if (! SUCCEEDED (hr)) {
        delete[] aPropNames;
        return hr;
    }
    // allocate the variants and result array
    CComVariant * aVal = new CComVariant[aNum];
    HRESULT * hvs = new HRESULT[aNum];
    hr = pPropBag2->Read (aNum, aPropNames, NULL, aVal, hvs);
    ATLASSERT (hr >= 0);
    if (! SUCCEEDED (hr)) {
        delete[] hvs;
        delete[] aVal;
        delete[] aPropNames;
        return hr;
    }

    // get the params ; put them in the ParamList
    for (unsigned long ind = 0; ind < aNum; ind++) {
        std::pair< CString, CString> p( CString( aPropNames[ind].pstrName ), CString( aVal[ind] ) );
        m_parameters.push_back( p );
//        do_what_you_want_with (OLE2T (aPropNames[ind].pstrName),
//            aVal[ind]);
    }
    // delete the unused arrays
    delete[] hvs;
    delete[] aVal;
    delete[] aPropNames;

    return hr;
}

P3D_object* CP3DActiveXCtrl::GetP3DObject( )
{
    return m_instance.m_p3dObject;
}

IOleClientSite* CP3DActiveXCtrl::GetClientSte( )
{
    return GetClientSite();
}

void CP3DActiveXCtrl::OnmainChanged(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    // TODO: Add your property handler code here

    SetModifiedFlag();
}
