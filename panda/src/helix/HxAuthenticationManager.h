// Filename: hxAuthenticationManager.cxx
// Created by:  jjtaylor (28Jan04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////
#if !defined (_HXAUTHENTICATIONMANAGER_H_)
#define _HXAUTHENTICATIONMANAGER_H_

////////////////////////////////////////////////////////////////////
// Panda Header Files
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

////////////////////////////////////////////////////////////////////
// Helix Main Header Files
////////////////////////////////////////////////////////////////////
#include "mainHelix.h"

////////////////////////////////////////////////////////////////////
// Class: HxAuthenticationManager
// Purpose: This is a derived class from the IHXAuthenticationManager
//          base interface. This class is meant to gain a user name
//          and password for a restricted file. Typically, this 
//          occurs from a popup box in windows, but it would need
//          to be specified in code to use with Panda.
////////////////////////////////////////////////////////////////////
class HxAuthenticationManager : public IHXAuthenticationManager {
  public:
    // Public Class Method Declarations
    HxAuthenticationManager();

////////////////////////////////////////////////////////////////////
// IHXClientAdviseSink Interface Methods Prototypes
// Purpose: Implements the necessary interface methods that is
//          called when a username and password is required.
////////////////////////////////////////////////////////////////////
    STDMETHOD (HandleAuthenticationRequest) (IHXAuthenticationManagerResponse* response);

////////////////////////////////////////////////////////////////////
// IHUnkown Interface Methods Prototypes
// Purpose: Implements the basic COM interface methods for reference
//          coutning and querying of related interfaces.
////////////////////////////////////////////////////////////////////
    STDMETHOD (QueryInterface) (THIS_ REFIID id, void** ppInterfaceObj);
    STDMETHOD_ (UINT32, AddRef) (THIS);
    STDMETHOD_ (UINT32, Release) (THIS);
  private:
    // Private Class Method Declarations
    ~HxAuthenticationManager();

    // Private Class Variable Declarations
    INT32 _ref_count;
    BOOL _sent_password;
};
#endif
