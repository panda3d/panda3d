// Filename: PPBrowserObject.h
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
#include <string>

class PPInterface;

class PPBrowserObject : public P3D_object 
{
public:
    PPBrowserObject( PPInterface* interfac, IDispatch* pDisp );
    PPBrowserObject( const PPBrowserObject& copy );
    ~PPBrowserObject( );

    int get_repr( char* buffer, int buffer_length ) const;
    P3D_object* get_property( const std::string &property ) const;
    bool set_property( const std::string& property, bool needs_response,
                       P3D_object* value );

    P3D_object* call( const std::string &method_name, 
        P3D_object* params[], int num_params ) const;
    P3D_object* eval( const std::string &expression ) const;

    static void clear_class_definition();

protected:
    PPBrowserObject( );
    static P3D_class_definition* get_class_definition( );

    PPInterface* m_interface;
    static P3D_class_definition* _browser_object_class;

    CComPtr<IDispatch> m_pDispatch;
};
