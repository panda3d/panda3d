// Filename: hxErrorSink.h
// Created by:  jjtaylor (29Jan04)
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
#if !defined (_HXERRORSINK_H_)
#define _HXERRORSINK_H_

////////////////////////////////////////////////////////////////////
// Panda Header Files
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

////////////////////////////////////////////////////////////////////
// Helix Main Header Files
////////////////////////////////////////////////////////////////////
#include "mainHelix.h"

////////////////////////////////////////////////////////////////////
// Prototype/Struct/Class Forward Delcarations
////////////////////////////////////////////////////////////////////
struct IUnknown;
struct IHXErrorMessages;
struct IHXPlayer;

////////////////////////////////////////////////////////////////////
// Class: HxErrorSink
// Purpose: This is a derived class from the IHXErrorSink
//          base interface. This class is meant to receive 
//          error messages from the client engine.
////////////////////////////////////////////////////////////////////
class HxErrorSink : public IHXErrorSink {
public:
  HxErrorSink(IUnknown* unknown);
  ~HxErrorSink();

////////////////////////////////////////////////////////////////////
// IHXErrorSink Interface Methods Prototypes
// Purpose: Implements the necessary interface methods that may
//          be called when an error message is generated.
////////////////////////////////////////////////////////////////////
  STDMETHOD (ErrorOccurred) (THIS_ const UINT8 severity, const UINT32 hx_code,
                                   const UINT32 user_code, const char* user_string,
                                   const char* more_info_url);

////////////////////////////////////////////////////////////////////
// IHUnkown Interface Methods Prototypes
// Purpose: Implements the basic COM interface methods for reference
//          coutning and querying of related interfaces.
////////////////////////////////////////////////////////////////////
  STDMETHOD (QueryInterface) (THIS_ REFIID _id, void** _interface_obj);
  STDMETHOD_ (ULONG32, AddRef) (THIS);
  STDMETHOD_ (ULONG32, Release) (THIS);



protected:
  // Protected Class Method Declarations
  void convert_to_string(const ULONG32 hx_code, char * buffer, UINT32 buffer_length);

  // Protected Class Variable Declarations
  INT32 ref_count;
  IHXPlayer* _player;
};
#endif 
