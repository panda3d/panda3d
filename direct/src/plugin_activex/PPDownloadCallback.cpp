// Filename: PPDownloadCallback.cpp
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
#include "PPDownloadCallback.h"
#include "PPInstance.h"


PPDownloadCallback::PPDownloadCallback( PPDownloadCallbackSync& downloadSync ) 
    : m_downloadSync( downloadSync ), m_dwTotalRead( 0 ), 
      m_dwTotalInStream( 0 ), m_ulObjRefCount( 1 ) 
{
}

PPDownloadCallback::~PPDownloadCallback()
{
	m_spStream.Release();
}

STDMETHODIMP PPDownloadCallback::QueryInterface(REFIID riid, void **ppvObject)
{
    TRACE(_T("IUnknown::QueryInterface\n"));

    *ppvObject = NULL;

    // IUnknown
    if (::IsEqualIID(riid, __uuidof(IUnknown)))
    {
        TRACE(_T("IUnknown::QueryInterface(IUnknown)\n"));

        *ppvObject = this;
    }
    // IBindStatusCallback
    else if (::IsEqualIID(riid, __uuidof(IBindStatusCallback)))
    {
        TRACE(_T("IUnknown::QueryInterface(IBindStatusCallback)\n"));

        *ppvObject = static_cast<IBindStatusCallback *>(this);
    }

    if (*ppvObject)
    {
        (*reinterpret_cast<LPUNKNOWN *>(ppvObject))->AddRef();

        return S_OK;
    }

    return E_NOINTERFACE;
}                                             

STDMETHODIMP_(ULONG) PPDownloadCallback::AddRef()
{
    TRACE(_T("IUnknown::AddRef\n"));

    return ++m_ulObjRefCount;
}

STDMETHODIMP_(ULONG) PPDownloadCallback::Release()
{
    TRACE(_T("IUnknown::Release\n"));

    return --m_ulObjRefCount;
}

STDMETHODIMP PPDownloadCallback::OnStartBinding(DWORD, IBinding *)
{
    TRACE(_T("IBindStatusCallback::OnStartBinding\n"));

    m_downloadSync.Begin();

    return S_OK;
}

STDMETHODIMP PPDownloadCallback::GetPriority(LONG *)
{
    TRACE(_T("IBindStatusCallback::GetPriority\n"));

    return E_NOTIMPL;
}

STDMETHODIMP PPDownloadCallback::OnLowResource(DWORD)
{
    TRACE(_T("IBindStatusCallback::OnLowResource\n"));

    return S_OK;
}

STDMETHODIMP PPDownloadCallback::OnProgress(ULONG ulProgress,
                                         ULONG ulProgressMax,
                                         ULONG ulStatusCode,
                                         LPCWSTR szStatusText)
{
#ifdef _DEBUG
    static const LPCTSTR plpszStatus[] = 
    {
        _T("BINDSTATUS_FINDINGRESOURCE"),  // 1
        _T("BINDSTATUS_CONNECTING"),
        _T("BINDSTATUS_REDIRECTING"),
        _T("BINDSTATUS_BEGINDOWNLOADDATA"),
        _T("BINDSTATUS_DOWNLOADINGDATA"),
        _T("BINDSTATUS_ENDDOWNLOADDATA"),
        _T("BINDSTATUS_BEGINDOWNLOADCOMPONENTS"),
        _T("BINDSTATUS_INSTALLINGCOMPONENTS"),
        _T("BINDSTATUS_ENDDOWNLOADCOMPONENTS"),
        _T("BINDSTATUS_USINGCACHEDCOPY"),
        _T("BINDSTATUS_SENDINGREQUEST"),
        _T("BINDSTATUS_CLASSIDAVAILABLE"),
        _T("BINDSTATUS_MIMETYPEAVAILABLE"),
        _T("BINDSTATUS_CACHEFILENAMEAVAILABLE"),
        _T("BINDSTATUS_BEGINSYNCOPERATION"),
        _T("BINDSTATUS_ENDSYNCOPERATION"),
        _T("BINDSTATUS_BEGINUPLOADDATA"),
        _T("BINDSTATUS_UPLOADINGDATA"),
        _T("BINDSTATUS_ENDUPLOADINGDATA"),
        _T("BINDSTATUS_PROTOCOLCLASSID"),
        _T("BINDSTATUS_ENCODING"),
        _T("BINDSTATUS_VERFIEDMIMETYPEAVAILABLE"),
        _T("BINDSTATUS_CLASSINSTALLLOCATION"),
        _T("BINDSTATUS_DECODING"),
        _T("BINDSTATUS_LOADINGMIMEHANDLER"),
        _T("BINDSTATUS_CONTENTDISPOSITIONATTACH"),
        _T("BINDSTATUS_FILTERREPORTMIMETYPE"),
        _T("BINDSTATUS_CLSIDCANINSTANTIATE"),
        _T("BINDSTATUS_IUNKNOWNAVAILABLE"),
        _T("BINDSTATUS_DIRECTBIND"),
        _T("BINDSTATUS_RAWMIMETYPE"),
        _T("BINDSTATUS_PROXYDETECTING"),
        _T("BINDSTATUS_ACCEPTRANGES"),
        _T("???")  // unknown
    };
#endif

    TRACE(_T("IBindStatusCallback::OnProgress\n"));

    TRACE(_T("ulProgress: %lu, ulProgressMax: %lu\n"),
          ulProgress, ulProgressMax);

    TRACE(_T("ulStatusCode: %lu "), ulStatusCode);

    if (ulStatusCode < UF_BINDSTATUS_FIRST ||
        ulStatusCode > UF_BINDSTATUS_LAST)
    {
        ulStatusCode = UF_BINDSTATUS_LAST + 1;
    }

    m_dwTotalInStream = ulProgressMax;
    m_downloadSync.ProgressNotify( ulProgress, ulProgressMax );

#ifdef _DEBUG
    TRACE(_T("(%s), szStatusText: %ls\n"),
          plpszStatus[ulStatusCode - UF_BINDSTATUS_FIRST],
          szStatusText);
#endif

    return S_OK;
}

STDMETHODIMP PPDownloadCallback::OnStopBinding(HRESULT, LPCWSTR)
{
    TRACE(_T("IBindStatusCallback::OnStopBinding\n"));

    m_downloadSync.End( );

    return S_OK;
}

STDMETHODIMP PPDownloadCallback::GetBindInfo(DWORD *, BINDINFO *)
{
    TRACE(_T("IBindStatusCallback::GetBindInfo\n"));

    return S_OK;
}

STDMETHODIMP PPDownloadCallback::OnDataAvailable(DWORD grfBSCF, DWORD dwSize,
                                              FORMATETC *, STGMEDIUM *pstgmed)
{
    TRACE(_T("IBindStatusCallback::OnDataAvailable\n"));

    HRESULT hr = S_OK;

    // Get the Stream passed
    if (BSCF_FIRSTDATANOTIFICATION & grfBSCF)
    {
        if (!m_spStream && pstgmed->tymed == TYMED_ISTREAM)
        {
            m_spStream = pstgmed->pstm;
        }
    }

    DWORD dwRead = dwSize - m_dwTotalRead; // Minimum amount available that hasn't been read
    DWORD dwActuallyRead = 0;            // Placeholder for amount read during this pull

    // If there is some data to be read then go ahead and read them
    if (m_spStream)
    {
        if (dwRead > 0)
        {
            BYTE* pBytes = NULL;
            ATLTRY(pBytes = new BYTE[dwRead + 1]);
            if (pBytes == NULL)
                return E_OUTOFMEMORY;
            hr = m_spStream->Read(pBytes, dwRead, &dwActuallyRead);
            if (SUCCEEDED(hr))
            {
                pBytes[dwActuallyRead] = 0;
                if (dwActuallyRead>0)
                {
                    bool ret = m_downloadSync.DataNotify( m_dwTotalInStream, (const void*)pBytes, dwActuallyRead );
                    if (!ret)
                    {
                        hr = E_ABORT;
                    }

                    m_dwTotalRead += dwActuallyRead;
                }
            }
            delete[] pBytes;
        }
    }

    if (BSCF_LASTDATANOTIFICATION & grfBSCF || E_ABORT == hr )
    {
        m_spStream.Release();
    }
    return hr;
}

STDMETHODIMP PPDownloadCallback::OnObjectAvailable(REFIID, IUnknown *)
{
    TRACE(_T("IBindStatusCallback::OnObjectAvailable\n"));

    return S_OK;
}
