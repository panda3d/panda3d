// Filename: PPPandaObject.cpp
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
#include "PPPandaObject.h"
#include "PPInstance.h"
#include "load_plugin.h"

PPandaObject::PPandaObject( PPInterface* interfac, P3D_object* p3dObject ) : 
    m_interface( interfac ), m_p3dObject( p3dObject ), m_refs( 0 ), m_ptinfo( NULL )
{
    if ( m_p3dObject )
    {
        P3D_OBJECT_INCREF( m_p3dObject );
    }
    AddRef();
    PPInstance::ref_plugin();
}

PPandaObject::~PPandaObject()
{
    if( m_ptinfo )
    {
        m_ptinfo->Release();
    }

    // Clean up the p3d_object, but only if we haven't already
    // unloaded the plugin.
    if ( m_p3dObject )
    {
        P3D_OBJECT_DECREF( m_p3dObject );
    }

    PPInstance::unref_plugin();
}

// Dispatch Methods

STDMETHODIMP_(unsigned long) PPandaObject::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(unsigned long) PPandaObject::Release()
{
    --m_refs;
    if( m_refs <= 0 )
    {
        delete this;
        return 0;
    }
    return m_refs;
}

STDMETHODIMP PPandaObject::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(!IsEqualIID(riid, IID_IUnknown))
    {
        if(!IsEqualIID(riid, IID_IDispatch)) 
        {
            *ppv = NULL;      
            return E_NOINTERFACE;
        }
    }
    *ppv = this;
    AddRef();
    return NOERROR;
}

STDMETHODIMP PPandaObject::GetIDsOfNames(
    REFIID riid,
    OLECHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    LCID lcid,
    DISPID FAR* rgdispid)
{
    UNUSED(lcid);

    if(!IsEqualIID(riid, IID_NULL))
    {
      return DISP_E_UNKNOWNINTERFACE;
    }

    for ( UINT i = 0; i < cNames; i++ )
    {
        UINT j = 0;
        for ( j = 0; j < m_idsOfNames.size(); j++ )
        {
            if ( m_idsOfNames[j] == rgszNames[i] )
            {
                rgdispid[i] = j;
                break;
            }
        }
        if ( j >= m_idsOfNames.size() )
        {
            CString name( rgszNames[i] );
            m_idsOfNames.push_back( name );
            rgdispid[i] = j;
        }
    }
    return S_OK;
}

STDMETHODIMP PPandaObject::GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo)
{
    UNUSED(lcid);

    if ( ppTInfo == NULL )
    {
        return E_INVALIDARG;
    }
    *ppTInfo = NULL;

    if( iTInfo != 0 )
    {
        return DISP_E_BADINDEX;
    }
    if ( m_ptinfo == NULL )
    {
        return E_FAIL;
    }
    m_ptinfo->AddRef();      // AddRef and return pointer to cached
    // typeinfo for this object.
    *ppTInfo = m_ptinfo;

    return NOERROR;
}

STDMETHODIMP PPandaObject::GetTypeInfoCount(unsigned int FAR* pctinfo)
{
    // This object has a single *introduced* interface
    //
    *pctinfo = 1;

    return NOERROR;
}

STDMETHODIMP PPandaObject::Invoke(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr)
{
    UNUSED(lcid);
    HRESULT hr( S_OK );


    if(!IsEqualIID(riid, IID_NULL))
    {
        return DISP_E_UNKNOWNINTERFACE;
    }
    if ( !m_interface )
    {
        return E_FAIL;
    }

    if ( dispidMember >= m_idsOfNames.size( ) ) 
    {
        return E_FAIL;
    }

    CString& name( m_idsOfNames[dispidMember] );

    switch ( wFlags )
    {
    case ( DISPATCH_METHOD ):
    case ( DISPATCH_METHOD | DISPATCH_PROPERTYGET ):
        {
            // NOTE: http://msdn.microsoft.com/en-us/library/ms221479.aspx
            // The member is invoked as a method. If a property has the same name, 
            // both the DISPATCH_METHOD and the DISPATCH_PROPERTYGET flag may be set.

            bool hasMethod( false );
            hr = m_interface->P3DHasMethod( m_p3dObject, name, hasMethod );
            if ( SUCCEEDED( hr ) && hasMethod )
            {
                hr = m_interface->P3DCallMethod( m_p3dObject, name, pdispparams, pvarResult );
            }
        }
        break;
    case ( DISPATCH_PROPERTYGET ):
        {
            hr = m_interface->P3DGetProperty( m_p3dObject, name, pvarResult );
        }
        break;
    case ( DISPATCH_PROPERTYPUT ):
        {
            bool result( false );
            hr = m_interface->P3DSetProperty( m_p3dObject, name, pdispparams, result );
        }
        break;
    }
    return hr;
}
