// Filename: PPDownloadCallback.h
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

#include <atlbase.h>

enum
{
    UF_BINDSTATUS_FIRST = BINDSTATUS_FINDINGRESOURCE,
    UF_BINDSTATUS_LAST = BINDSTATUS_ACCEPTRANGES
};

class PPDownloadCallbackSync
{
public:
    virtual bool Begin( ) = 0;
    virtual bool DataNotify( size_t expectedDataSize, const void* data, size_t dataSize ) = 0;
    virtual void ProgressNotify( size_t progress, size_t maxProgress ) = 0;
    virtual bool End( ) = 0;
};

class PPDownloadCallback : public IBindStatusCallback
{
public:
    PPDownloadCallback( PPDownloadCallbackSync& downloadSync );
    virtual ~PPDownloadCallback();

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IBindStatusCallback methods
    STDMETHOD(OnStartBinding)(DWORD, IBinding *);
    STDMETHOD(GetPriority)(LONG *);
    STDMETHOD(OnLowResource)(DWORD);
    STDMETHOD(OnProgress)(ULONG ulProgress,
                          ULONG ulProgressMax,
                          ULONG ulStatusCode,
                          LPCWSTR szStatusText);
    STDMETHOD(OnStopBinding)(HRESULT, LPCWSTR);
    STDMETHOD(GetBindInfo)(DWORD *, BINDINFO *);
    STDMETHOD(OnDataAvailable)(DWORD, DWORD, FORMATETC *, STGMEDIUM *);
    STDMETHOD(OnObjectAvailable)(REFIID, IUnknown *);

protected:
    ULONG m_ulObjRefCount;
    CComPtr<IStream> m_spStream;

private:
    PPDownloadCallbackSync& m_downloadSync;

    DWORD m_dwTotalRead;
    DWORD m_dwTotalInStream;

};
