// Filename: PPInterface.cpp
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

#include "PPInterface.h"
#include "PPPandaObject.h"
#include "PPBrowserObject.h"
#include "PPInstance.h"
#include "P3DActiveXCtrl.h"

#include <strstream>
#include "Mshtml.h"

PPInterface::PPInterface( ) 
{
}

PPInterface::~PPInterface( )
{
}


HRESULT PPInterface::GetIdOfName( IDispatch* pDisp, CString& ptName, DISPID* dispID )
{
    if( !pDisp )
    {
        return E_FAIL;
    }
    OLECHAR* pElementName = ptName.AllocSysString();

    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &pElementName, 1, LOCALE_USER_DEFAULT, dispID);
    SysFreeString( pElementName );

    return hr;
}

HRESULT PPInterface::Invoke(int nType, IDispatch* pDisp, CString& ptName, VARIANT* pvResult, int cArgs, VARIANT* params)
{
    if( !pDisp )
    {
        return E_FAIL;
    }

    DISPPARAMS dp = { NULL, NULL, 0, 0 };
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    DISPID dispID;
    CComPtr<IDispatchEx> pDispEx;

    HRESULT hr = GetIdOfName( pDisp, ptName, &dispID );
    if ( DISP_E_UNKNOWNNAME == hr )
    {
        hr = pDisp->QueryInterface( IID_IDispatchEx, ( void** )&pDispEx );
        if ( SUCCEEDED( hr ) && pDispEx )
        {
            OLECHAR* pElementName = ptName.AllocSysString();
            hr = pDispEx->GetDispID( pElementName, fdexNameEnsure, &dispID );
            SysFreeString( pElementName );
        }
    }
    if ( FAILED( hr ) )
    {
        return hr;
    }

    // Allocate memory for arguments...
    VARIANT *pArgs = new VARIANT[ cArgs + 1 ];

    // Reversing the arguments!!!
    // NOTE: http://msdn.microsoft.com/en-us/library/cc237569(PROT.10).aspx
    // pDispParams: MUST point to a DISPPARAMS structure that defines the arguments passed to the method. 
    // Arguments MUST be stored in pDispParams->rgvarg in reverse order, so that the first argument is 
    // the one with the highest index in the array. Byref arguments MUST be marked in this array 
    // as VT_EMPTY entries, and stored in rgVarRef instead. 

    for( int i = 0; i < cArgs; i++ ) 
    {
        pArgs[i] = params[ cArgs - 1 - i ];
    }

    // Build DISPPARAMS
    dp.cArgs = cArgs;
    dp.rgvarg = pArgs;

    // Handle special-case for property-puts!
    if( nType & DISPATCH_PROPERTYPUT  || nType & DISPATCH_PROPERTYPUTREF ) 
    {
        dp.cNamedArgs = 1;
        dp.rgdispidNamedArgs = &dispidNamed;
    }

    // Make the call!
    if ( pDispEx )
    {
        hr = pDispEx->InvokeEx(dispID, LOCALE_USER_DEFAULT, 
                        nType, &dp, pvResult, NULL, NULL);
        if ( FAILED( hr ) )
        {
            pDispEx->DeleteMemberByDispID( dispID );
        }
    }
    else
    {
        hr = pDisp->Invoke( dispID, IID_NULL, LOCALE_USER_DEFAULT, 
                        nType, &dp, pvResult, NULL, NULL );
    }
    delete [] pArgs;

    return hr;
}

HRESULT PPInterface::GetHtmlDocDispatch( CComPtr<IDispatch>& pDispScript )
{
    HRESULT hr = S_OK; 

    CComPtr<IOleClientSite> pOleClientSite = GetClientSte( ); 
    if (pOleClientSite == NULL) {
      return E_FAIL;
    }

    CComPtr<IOleContainer> pOleContainer; 
    hr = pOleClientSite->GetContainer(& pOleContainer ); 
    ASSERT( SUCCEEDED( hr ) && pOleContainer ); 

    CComPtr<IHTMLDocument> pHtmlDoc; 
    hr = pOleContainer->QueryInterface( IID_IHTMLDocument, ( void** )&pHtmlDoc ); 
    ASSERT( SUCCEEDED( hr ) && pHtmlDoc ); 

    // Get the script object (this returns the script object, NOT the script 
    // element(s) that the get_scripts method does). 
    hr = pHtmlDoc->get_Script( &pDispScript );
    ASSERT( SUCCEEDED( hr ) && pDispScript );

    CComPtr<IDispatchEx> pDispExScript;
    hr = pDispScript->QueryInterface( IID_IDispatchEx, ( void** )&pDispExScript );
    ASSERT( SUCCEEDED( hr ) && pDispExScript );

    pDispScript = pDispExScript;

    CComPtr<ITypeInfo> pTypeInfo;
    hr = pDispScript->GetTypeInfo( 0, 0, &pTypeInfo );

    return hr; 
}

HRESULT PPInterface::HasProperty( CComPtr<IDispatch>& pDispatch, CString& name )
{
    HRESULT hr = S_OK;
    if ( pDispatch == NULL )
    {
        hr = GetHtmlDocDispatch( pDispatch );
    }

    DISPID dispID;
    if ( SUCCEEDED( hr ) )
    {
        hr = GetIdOfName( pDispatch, name, &dispID );
    }
    return hr;
}

HRESULT PPInterface::CallMethod( CComPtr<IDispatch>& pDispatch, CString& name, COleVariant& varResult, int numParams, COleVariant* params )
{
    HRESULT hr = S_OK;
    if ( pDispatch == NULL )
    {
        hr = GetHtmlDocDispatch( pDispatch );
    }
    if ( SUCCEEDED( hr ) )
    {
        hr = Invoke( DISPATCH_METHOD, pDispatch, name, &varResult, numParams, params ); 
    }
    return hr;
}

HRESULT PPInterface::GetProperty( CComPtr<IDispatch>& pDispatch, CString& name, COleVariant& varResult )
{
    HRESULT hr = S_OK;
    if ( pDispatch == NULL )
    {
        hr = GetHtmlDocDispatch( pDispatch );
    }
    if ( SUCCEEDED( hr ) )
    {
        hr = Invoke( DISPATCH_PROPERTYGET, pDispatch, name, &varResult, 0, NULL ); 
    }
    return hr;
}

HRESULT PPInterface::SetProperty( CComPtr<IDispatch>& pDispatch, CString& name, COleVariant& varValue, COleVariant& varResult )
{
    HRESULT hr = S_OK;
    if ( pDispatch == NULL )
    {
        hr = GetHtmlDocDispatch( pDispatch );
    }
    if ( SUCCEEDED( hr ) )
    {
        if ( varValue.vt == VT_DISPATCH )
        {
            hr = Invoke( DISPATCH_PROPERTYPUTREF, pDispatch, name, &varResult, 1, &varValue );
        }
        else
        {
            hr = Invoke( DISPATCH_PROPERTYPUT, pDispatch, name, &varResult, 1, &varValue );
        }
    }
    return hr;
}

HRESULT PPInterface::EvalExpression( CComPtr<IDispatch>& pDispatch, CString& expression, COleVariant& varResult )
{
    HRESULT hr = S_OK;
    if ( pDispatch == NULL )
    {
        hr = GetHtmlDocDispatch( pDispatch );
    }
    if ( SUCCEEDED( hr ) )
    {
        COleVariant param( expression );
        hr = Invoke( DISPATCH_METHOD, pDispatch, CString("eval"), &varResult, 1, &param ); 
    }
    return hr;
}

HRESULT PPInterface::P3DHasMethod( P3D_object* p3dObject, CString& name, bool& result )
{
    if ( !p3dObject )
    {
        p3dObject = GetP3DObject( );
        if ( !p3dObject )
        {
            return E_FAIL;
        }
    }
    result = P3D_OBJECT_HAS_METHOD( p3dObject, name );
    return S_OK;
}

HRESULT PPInterface::P3DCallMethod( P3D_object* p3dObject, CString& name, DISPPARAMS FAR* pdispparams, VARIANT FAR* varResult )
{
    if ( !p3dObject )
    {
        p3dObject = GetP3DObject( );
        if ( !p3dObject )
        {
            return E_FAIL;
        }
    }
    P3D_object** params = new P3D_object*[pdispparams->cArgs];
    for ( UINT i = 0; i < pdispparams->cArgs; i++ )
    {
        // Reversing the arguments!!!
        // http://msdn.microsoft.com/en-us/library/cc237569(PROT.10).aspx
        // pDispParams: MUST point to a DISPPARAMS structure that defines the arguments passed to the method. 
        // Arguments MUST be stored in pDispParams->rgvarg in reverse order, so that the first argument is 
        // the one with the highest index in the array. Byref arguments MUST be marked in this array 
        // as VT_EMPTY entries, and stored in rgVarRef instead. 

        COleVariant vaArg( pdispparams->rgvarg[pdispparams->cArgs - 1 - i] );
        params[i] = variant_to_p3dobj( &vaArg );
    }
    bool needResult( false );
    if ( varResult )
    {
        needResult = true;
    }
    P3D_object* p3dObjectResult = P3D_OBJECT_CALL( p3dObject, name, needResult, params, pdispparams->cArgs );
    if ( p3dObjectResult )
    {
        p3dobj_to_variant( varResult, p3dObjectResult );
        P3D_OBJECT_DECREF( p3dObjectResult );
    }
    for ( UINT i = 0; i < pdispparams->cArgs; i++ )
    {
        P3D_OBJECT_DECREF( params[i] );
    }
    delete [] params;

    return S_OK;
}

HRESULT PPInterface::P3DGetProperty( P3D_object* p3dObject, CString& name, VARIANT FAR* varResult )
{
    if ( !p3dObject )
    {
        p3dObject = GetP3DObject( );
        if ( !p3dObject )
        {
            return E_FAIL;
        }
    }
    P3D_object* p3dObjectResult = P3D_OBJECT_GET_PROPERTY( p3dObject, name );
    if ( p3dObjectResult )
    {
        p3dobj_to_variant( varResult, p3dObjectResult );
        P3D_OBJECT_DECREF( p3dObjectResult );
    }
    return S_OK;
}

HRESULT PPInterface::P3DSetProperty( P3D_object* p3dObject, CString& name, DISPPARAMS FAR* pdispparams, bool& result )
{
    if ( !p3dObject )
    {
        p3dObject = GetP3DObject( );
        if ( !p3dObject )
        {
            return E_FAIL;
        }
    }
    COleVariant vaArg( pdispparams->rgvarg );
    P3D_object* param = variant_to_p3dobj( &vaArg );
    result = P3D_OBJECT_SET_PROPERTY( p3dObject, name, true, param );
    P3D_OBJECT_DECREF( param );

    if (!result) {
      return E_FAIL;
    }

    return S_OK;
}

void PPInterface::p3dobj_to_variant(VARIANT* result, P3D_object* object) 
{
    if ( !result )
    {
        return;
    }
    switch ( P3D_OBJECT_GET_TYPE( object ) ) 
    {
    case P3D_OT_undefined:
        {
            result->vt = VT_EMPTY;
            break;
        }
    case P3D_OT_none:
        {
            result->vt = VT_EMPTY;
            break;
        }
    case P3D_OT_bool:
        {
            result->vt = VT_BOOL;
            result->bVal = P3D_OBJECT_GET_BOOL( object );
            break;
        }
    case P3D_OT_int:
        {
            result->vt = VT_I4;
            result->lVal = P3D_OBJECT_GET_INT( object );
            break;
        }
    case P3D_OT_float:
        {
            result->vt = VT_R4;
            result->fltVal = P3D_OBJECT_GET_FLOAT( object );
            break;
        }
    case P3D_OT_string:
        {
            int size = P3D_OBJECT_GET_STRING(object, NULL, 0);
            char *buffer = new char[size];
            P3D_OBJECT_GET_STRING(object, buffer, size);

            int wsize = MultiByteToWideChar(CP_UTF8, 0, buffer, size, NULL, 0);
            WCHAR *wbuffer = new WCHAR[wsize + 1];
            MultiByteToWideChar(CP_UTF8, 0, buffer, size, wbuffer, wsize);
            wbuffer[wsize] = 0;

            result->vt = VT_BSTR;
            result->bstrVal = SysAllocString(wbuffer);

            delete[] buffer;
            delete[] wbuffer;
            break;
        }
    case P3D_OT_object:
        {
            PPBrowserObject *ppBrowserObject = (PPBrowserObject *)object;
            PPandaObject* ppPandaObject = new PPandaObject( this, ppBrowserObject );
            result->vt = VT_DISPATCH;
            result->pdispVal = ppPandaObject;
            break;
        }
    default:
        {
            result->vt = VT_EMPTY;
            break;
        }
    }
}

P3D_object* PPInterface::variant_to_p3dobj(COleVariant* variant) 
{
    if ( !variant )
    {
        return P3D_new_none_object_ptr();
    }
    switch( variant->vt )
    {
    case VT_VOID: 
        {
            return P3D_new_undefined_object_ptr();
            break;
        } 
    case VT_EMPTY:
        {
            // return P3D_new_none_object_ptr();
            // A.T. Panda really expect undefined object here
            return P3D_new_undefined_object_ptr();
            break;
        } 
    case VT_BOOL: 
        {
            return P3D_new_bool_object_ptr( variant->bVal );
            break;
        } 
    case VT_I2: 
        {
            return P3D_new_int_object_ptr( variant->iVal );
            break;
        }
    case VT_I4:
        {
            return P3D_new_int_object_ptr( variant->lVal );
            break;
        }
    case VT_I8:
        {
            return P3D_new_int_object_ptr( variant->llVal );
            break;
        }
    case VT_R4:
        {
            return P3D_new_float_object_ptr( variant->fltVal );
            break;
        }
    case VT_R8:
        {
            return P3D_new_float_object_ptr( variant->dblVal );
            break;
        }
    case VT_BSTR:
        {
            BSTR bstr = variant->bstrVal;
            UINT blen = SysStringLen(bstr);

            int slen = WideCharToMultiByte(CP_UTF8, 0, bstr, blen,
                                           0, 0, NULL, NULL);
            char *sbuffer = new char[slen];
            WideCharToMultiByte(CP_UTF8, 0, bstr, blen,
                                sbuffer, slen, NULL, NULL);

            P3D_object *object = P3D_new_string_object_ptr(sbuffer, slen);
            delete[] sbuffer;
            return object;
            break;
        }
    case VT_DISPATCH:
        {
          // The following commented-out code crashes IE7:
          /*
            CComPtr<IDispatch> pDispObject( variant->pdispVal );
            CComPtr<ITypeInfo> pTypeInfo;
            HRESULT hr = pDispObject->GetTypeInfo( 0, 0, &pTypeInfo );
            if ( SUCCEEDED( hr ) && pTypeInfo )
            {
                TYPEATTR* pTypeAttr;
                hr = pTypeInfo->GetTypeAttr( &pTypeAttr );

                pTypeInfo->ReleaseTypeAttr( pTypeAttr );
            }
          */
            return new PPBrowserObject( this, variant->pdispVal );
            break;
        }
    default:
        {
            return P3D_new_undefined_object_ptr();
            break;
        }
    }
}

CString PPInterface::get_repr(COleVariant& variant) 
{
    std::strstream repr;
    repr << "IDispatch";
    switch( variant.vt )
    {
    case VT_VOID: 
        {
            repr << ":VT_VOID";
            break;
        } 
    case VT_EMPTY:
        {
            repr << ":VT_EMPTY";
            break;
        } 
    case VT_BOOL: 
        {
            repr << ":VT_BOOL:" << variant.bVal;
            break;
        } 
    case VT_I2: 
        {
            repr << ":VT_I2:" << variant.iVal;
            break;
        }
    case VT_I4:
        {
            repr << ":VT_I4:" << variant.lVal;
            break;
        }
    case VT_I8:
        {
            repr << ":VT_I8:" << variant.llVal;
            break;
        }
    case VT_R4: 
        {
            repr << ":VT_R4:" << variant.fltVal;
            break;
        }
    case VT_R8: 
        {
            repr << ":VT_R8:" << variant.dblVal;
            break;
        }
    case VT_BSTR:
        {
            CString str( variant );
            repr << ":VT_BSTR:" << str;
            break;
        }
    case VT_DISPATCH:
        {
            if ( variant.pdispVal )
            {
                CComPtr<IDispatch> pDispObject( variant.pdispVal );
                CComPtr<ITypeInfo> pTypeInfo;
                HRESULT hr = pDispObject->GetTypeInfo( 0, 0, &pTypeInfo );
                if ( SUCCEEDED( hr ) && pTypeInfo )
                {
                    TYPEATTR* pTypeAttr;
                    hr = pTypeInfo->GetTypeAttr( &pTypeAttr );
                    if ( pTypeAttr )
                    {
                        OLECHAR szGuid[40] = { 0 };
                        int nCount = ::StringFromGUID2( pTypeAttr->guid, szGuid, sizeof( szGuid ) );
                        CString guid( szGuid, nCount );

                        repr << ":VT_DISPATCH:GUID:" << guid;
                        pTypeInfo->ReleaseTypeAttr( pTypeAttr );
                    }
                    else
                    {
                        repr << ":VT_DISPATCH:" << variant.pdispVal;
                    }
                }
                else
                {
                    repr << ":VT_DISPATCH:" << variant.pdispVal;
                }
            }
            else
            {
                repr << ":UNKNOWN";
            }

            break;
        }
    default:
        {
            repr << ":UNKNOWN";
            break;
        }
    }
    
    return CString ( repr.str(), repr.pcount() );
}

