// Filename: HxClientContext.h
// Created by:  jjtaylor (01Feb04)
//
////////////////////////////////////////////////////////////////////
//
//
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
#ifndef HXCLIENTCONTEXT_H
#define HXCLIENTCONTEXT_H

////////////////////////////////////////////////////////////////////
// Panda Header Files
////////////////////////////////////////////////////////////////////
#include "pandabase.h"
#include "texture.h"

////////////////////////////////////////////////////////////////////
// Helix Main Header Files
////////////////////////////////////////////////////////////////////
#include "mainHelix.h"

////////////////////////////////////////////////////////////////////
// Prototype/Struct/Class Forward Delcarations
////////////////////////////////////////////////////////////////////
struct IUnknown;
struct IHXPreferences;
class HxAdviseSink;
class HxErrorSink;
class HxAuthenticationManager;

////////////////////////////////////////////////////////////////////
// Class: HxClientContext
// Purpose: This is a derived class from the IHXPreferences
//          base interface. This class is meant to set the 
//          default preferences for a player. It contains the
//          necessary interface objects such as the Advise Sink, 
//          Error Sink, and Site Supplier.
////////////////////////////////////////////////////////////////////
class HxClientContext : public IHXPreferences {
public:
  HxClientContext(LONG32 client_index);
  ~HxClientContext();
  void init(IUnknown* unknown, IHXPreferences* prefs, char* guid, bool sink_on, Texture* tex);
  void close();

////////////////////////////////////////////////////////////////////
// IHUnkown Interface Methods Prototypes
// Purpose: Implements the basic COM interface methods for reference
//          coutning and querying of related interfaces.
////////////////////////////////////////////////////////////////////
  STDMETHOD (QueryInterface) (THIS_ REFIID id, void** interface_obj);
  STDMETHOD_ (ULONG32, AddRef) (THIS);
  STDMETHOD_ (ULONG32, Release) (THIS);


  
  
  
////////////////////////////////////////////////////////////////////
// IHXPreference Interface Methods Prototypes
// Purpose: Implements the necessary interface methods that are 
//          called to read or write from the Helix client registry.
////////////////////////////////////////////////////////////////////
  STDMETHOD(ReadPref) (THIS_ const char* pref_key, IHXBuffer*& buffer);
  STDMETHOD(WritePref) (THIS_ const char* pref_key, IHXBuffer* buffer);
private:
  // Private Class Variable Declarations
  LONG32 _ref_count;
  LONG32 _client_index;
  char _guid[256];

  HxAdviseSink* _client_sink;
  HxErrorSink* _error_sink;
  HxAuthenticationManager* _auth_mgr;
  HxSiteSupplier * _site_supplier;
  IHXPreferences * _default_prefs;
};
#endif
