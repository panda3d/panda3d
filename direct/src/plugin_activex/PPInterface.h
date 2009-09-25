// Filename: PPInterface.h
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

#include "p3d_plugin.h"

class PPInterface
{
public:
    PPInterface();
    virtual ~PPInterface();

    virtual P3D_object* GetP3DObject() = 0;
    virtual IOleClientSite* GetClientSte() = 0;

    HRESULT HasProperty( CComPtr<IDispatch>& pDispatch, CString& name );
    HRESULT CallMethod( CComPtr<IDispatch>& pDispatch, CString& name, COleVariant& varResult, int num_params, COleVariant* params  );
    HRESULT GetProperty( CComPtr<IDispatch>& pDispatch, CString& name, COleVariant& varResult );
    HRESULT SetProperty( CComPtr<IDispatch>& pDispatch, CString& name, COleVariant& varValue, COleVariant& varResult );
    HRESULT EvalExpression( CComPtr<IDispatch>& pDispatch, CString& expression, COleVariant& varResult );

    HRESULT P3DHasMethod( P3D_object* p3dObject, CString& name, bool& result );
    HRESULT P3DCallMethod( P3D_object* p3dObject, CString& name, DISPPARAMS FAR* pdispparams, VARIANT FAR* varResult );
    HRESULT P3DGetProperty( P3D_object* p3dObject, CString& name, VARIANT FAR* varResult );
    HRESULT P3DSetProperty( P3D_object* p3dObject, CString& name, DISPPARAMS FAR* pdispparams, bool& result );

    void p3dobj_to_variant(VARIANT* result, P3D_object* object); 
    P3D_object* variant_to_p3dobj(COleVariant* variant);
    CString get_repr(COleVariant& variant);

private:
    HRESULT GetIdOfName( IDispatch* pDisp, CString& ptName, DISPID* dispID );
    HRESULT Invoke( int nType, IDispatch* pDisp, CString& ptName, VARIANT* pvResult, int cArgs, VARIANT* params );
    HRESULT GetHtmlDocDispatch( CComPtr<IDispatch>& pHTMLDocDispatch );

};
