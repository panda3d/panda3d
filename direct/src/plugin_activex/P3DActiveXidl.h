

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Mon Sep 14 17:17:04 2009
 */
/* Compiler settings for .\P3DActiveX.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __P3DActiveXidl_h__
#define __P3DActiveXidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef ___DP3DActiveX_FWD_DEFINED__
#define ___DP3DActiveX_FWD_DEFINED__
typedef interface _DP3DActiveX _DP3DActiveX;
#endif 	/* ___DP3DActiveX_FWD_DEFINED__ */


#ifndef ___DP3DActiveXEvents_FWD_DEFINED__
#define ___DP3DActiveXEvents_FWD_DEFINED__
typedef interface _DP3DActiveXEvents _DP3DActiveXEvents;
#endif 	/* ___DP3DActiveXEvents_FWD_DEFINED__ */


#ifndef __P3DActiveX_FWD_DEFINED__
#define __P3DActiveX_FWD_DEFINED__

#ifdef __cplusplus
typedef class P3DActiveX P3DActiveX;
#else
typedef struct P3DActiveX P3DActiveX;
#endif /* __cplusplus */

#endif 	/* __P3DActiveX_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __P3DActiveXLib_LIBRARY_DEFINED__
#define __P3DActiveXLib_LIBRARY_DEFINED__

/* library P3DActiveXLib */
/* [control][helpstring][helpfile][version][uuid] */ 


EXTERN_C const IID LIBID_P3DActiveXLib;

#ifndef ___DP3DActiveX_DISPINTERFACE_DEFINED__
#define ___DP3DActiveX_DISPINTERFACE_DEFINED__

/* dispinterface _DP3DActiveX */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__DP3DActiveX;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("76904D54-0CC5-4DBB-B022-F48B1E95183B")
    _DP3DActiveX : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DP3DActiveXVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DP3DActiveX * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DP3DActiveX * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DP3DActiveX * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DP3DActiveX * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DP3DActiveX * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DP3DActiveX * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DP3DActiveX * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DP3DActiveXVtbl;

    interface _DP3DActiveX
    {
        CONST_VTBL struct _DP3DActiveXVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DP3DActiveX_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _DP3DActiveX_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _DP3DActiveX_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _DP3DActiveX_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _DP3DActiveX_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _DP3DActiveX_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _DP3DActiveX_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DP3DActiveX_DISPINTERFACE_DEFINED__ */


#ifndef ___DP3DActiveXEvents_DISPINTERFACE_DEFINED__
#define ___DP3DActiveXEvents_DISPINTERFACE_DEFINED__

/* dispinterface _DP3DActiveXEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__DP3DActiveXEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("1B2413ED-51C8-495E-B917-983C459B8FC7")
    _DP3DActiveXEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DP3DActiveXEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DP3DActiveXEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DP3DActiveXEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DP3DActiveXEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DP3DActiveXEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DP3DActiveXEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DP3DActiveXEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DP3DActiveXEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DP3DActiveXEventsVtbl;

    interface _DP3DActiveXEvents
    {
        CONST_VTBL struct _DP3DActiveXEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DP3DActiveXEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _DP3DActiveXEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _DP3DActiveXEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _DP3DActiveXEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _DP3DActiveXEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _DP3DActiveXEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _DP3DActiveXEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DP3DActiveXEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_P3DActiveX;

#ifdef __cplusplus

class DECLSPEC_UUID("924B4927-D3BA-41EA-9F7E-8A89194AB3AC")
P3DActiveX;
#endif
#endif /* __P3DActiveXLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


