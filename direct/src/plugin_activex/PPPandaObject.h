// Filename: PPPandaObject.h
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

#include <vector>
#include <string>

#include "p3d_plugin.h"
#include "PPInterface.h"

class PPandaObject  : public IDispatch
{
public:

    // Methods
    PPandaObject( PPInterface* interfac, P3D_object* p3dObject );
    virtual ~PPandaObject();

    // IUnknown methods 
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    // IDispatch methods 
    STDMETHOD(GetTypeInfoCount)(unsigned int FAR* pcTypeInfo);

    STDMETHOD(GetTypeInfo)(
      unsigned int iTypeInfo,
      LCID lcid,
      ITypeInfo FAR* FAR* ppTypeInfo);

    STDMETHOD(GetIDsOfNames)(
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      unsigned int cNames,
      LCID lcid,
      DISPID FAR* rgdispid);

    STDMETHOD(Invoke)(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      unsigned short wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      unsigned int FAR* puArgErr);

private:
     PPandaObject();
    
    // Props - Dispatch related
    unsigned long m_refs;
    ITypeInfo FAR* m_ptinfo;
    std::vector<CString> m_idsOfNames;
    PPInterface* m_interface;
    P3D_object *m_p3dObject;
};
