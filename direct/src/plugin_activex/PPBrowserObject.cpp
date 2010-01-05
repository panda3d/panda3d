// Filename: PPBrowserObject.cpp
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

#include "PPBrowserObject.h"
#include "PPInstance.h"
#include "P3DActiveXCtrl.h"

#include <assert.h>

// The following functions are C-style wrappers around the above
// PPBrowserObject methods; they are defined to allow us to create the
// C-style P3D_class_definition method table to store in the
// P3D_object structure.
static void object_finish(P3D_object *object) 
{
    delete ((PPBrowserObject *)object);
}

static int object_get_repr(P3D_object *object, char *buffer, int buffer_length) 
{
    return ((const PPBrowserObject *)object)->get_repr(buffer, buffer_length);
}

static P3D_object* object_get_property(P3D_object *object, const char *property) 
{
    return ((const PPBrowserObject *)object)->get_property(property);
}

static bool object_set_property(P3D_object* object, const char* property,
                                bool needs_response, P3D_object *value) 
{
    return ((PPBrowserObject *)object)->set_property(property, needs_response, value);
}

static P3D_object* object_call(P3D_object* object, const char* method_name, 
                               bool needs_response,
                               P3D_object *params[], int num_params) 
{
    if (method_name == NULL)
    {
        method_name = "";
    }
    P3D_object *response = ((const PPBrowserObject *)object)->call(method_name, params, num_params);
    if (!needs_response) 
    {
        // No response was expected.  Throw away the response we received,
        // so we can be consistent with defined semantics.
        P3D_OBJECT_XDECREF(response);
        response = NULL;
    }
    return response;
}

static P3D_object* object_eval( P3D_object *object, const char *expression ) 
{
    return ( ( const PPBrowserObject* )object )->eval( expression );
}

P3D_class_definition* PPBrowserObject::_browser_object_class;

PPBrowserObject::PPBrowserObject( PPInterface* interfac, IDispatch* pDispatch ) : 
    m_interface( interfac ), m_pDispatch( pDispatch ) 
{
    _class = get_class_definition( );
    _ref_count = 1;
}

PPBrowserObject::PPBrowserObject( const PPBrowserObject& copy ) :
    m_interface( copy.m_interface ), m_pDispatch( copy.m_pDispatch )
{
    _class = get_class_definition( );
    _ref_count = 1;
}

PPBrowserObject::~PPBrowserObject( ) 
{
    assert( _ref_count == 0 );
}

P3D_object* PPBrowserObject::get_property( const std::string &property ) const
{
    assert( m_interface );

    COleVariant varResult;
    CComPtr<IDispatch> pDispatch( m_pDispatch );
    HRESULT hr = m_interface->HasProperty( pDispatch, CString( property.c_str() ) );
    if( FAILED( hr ) )
    {
        return NULL;
    }
    
    hr = m_interface->GetProperty( pDispatch, CString( property.c_str() ), varResult );
    if ( FAILED( hr ) )
    {
        return NULL;
    }

    return m_interface->variant_to_p3dobj( &varResult );
}

bool PPBrowserObject::set_property( const std::string& property, bool needs_response,
                                    P3D_object* value )
{   
    assert( m_interface );

    CComPtr<IDispatch> pDispatch( m_pDispatch );
    COleVariant varValue, varResult;
    m_interface->p3dobj_to_variant( &varValue, value);

    HRESULT hr = m_interface->SetProperty( pDispatch, CString( property.c_str() ), varValue, varResult );
    if ( SUCCEEDED( hr ) )
    {
        return true;
    }
    return false;
}

P3D_object* PPBrowserObject::call( const std::string &method_name, P3D_object* params[], int num_params ) const
{
    assert( m_interface );

    COleVariant* varParams = new COleVariant[ num_params ];
    COleVariant varResult;

    // First, convert all of the parameters.
    for ( int i = 0; i < num_params; ++i ) 
    {
        m_interface->p3dobj_to_variant( &varParams[i], params[i] );
    }

    CComPtr<IDispatch> pDispatch( m_pDispatch );
    HRESULT hr = m_interface->CallMethod( pDispatch, CString( method_name.c_str() ), varResult,  num_params, varParams ); 

    delete [] varParams;
    if ( FAILED( hr ) )
    {
        return NULL;    }

    return m_interface->variant_to_p3dobj( &varResult );
}

P3D_object* PPBrowserObject::eval( const std::string &expression ) const
{
    assert( m_interface );

    COleVariant varResult;
    CComPtr<IDispatch> pDispatch( m_pDispatch );

    CString evalExpression( expression.c_str() );
    HRESULT hr = m_interface->EvalExpression( pDispatch, evalExpression , varResult ); 
    if ( FAILED( hr ) )
    {
        return NULL;
    }

    return m_interface->variant_to_p3dobj( &varResult );
}

int PPBrowserObject::get_repr( char* buffer, int buffer_length ) const
{
    assert( m_interface );

    VARIANT var;
    var.vt = VT_DISPATCH;
    var.pdispVal = m_pDispatch;

    COleVariant varDispatch( var );

    CString result( m_interface->get_repr( varDispatch ) );
    strncpy( buffer, result, buffer_length );
    return (int)result.GetLength();
}

void PPBrowserObject::clear_class_definition() 
{
    _browser_object_class = NULL;
}

P3D_class_definition* PPBrowserObject::get_class_definition() 
{
    if ( _browser_object_class == NULL ) 
    {
        // Create a default class_definition object, and fill in the
        // appropriate pointers.
        _browser_object_class = P3D_make_class_definition_ptr();
        _browser_object_class->_finish = &object_finish;

        _browser_object_class->_get_repr = &object_get_repr;
        _browser_object_class->_get_property = &object_get_property;
        _browser_object_class->_set_property = &object_set_property;
        _browser_object_class->_call = &object_call;
        _browser_object_class->_eval = &object_eval;
    }
    return _browser_object_class;
}
