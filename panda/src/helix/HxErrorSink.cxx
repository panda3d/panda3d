// Filename: hxErrorSink.cpp
// Created by:  jjtaylor (27Jan04)
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
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Panda Header Files
////////////////////////////////////////////////////////////////////
#include "HxErrorSink.h"
#include "print.h"

////////////////////////////////////////////////////////////////////
// Normal Header Files
////////////////////////////////////////////////////////////////////
#include <stdio.h>

////////////////////////////////////////////////////////////////////
//  Member: HxErrorSink::Constructor
//  Access: Public
//  Purpose: The default constructor of the class which 
//               initializes the member variables.
////////////////////////////////////////////////////////////////////
//  Params: IUnknown * - Pointer to an IUnknown COM Interface
//  Return: None
////////////////////////////////////////////////////////////////////
HxErrorSink::HxErrorSink(IUnknown* unknown) 
  : ref_count(0),
    _player(0) 
{
	IHXClientEngine* pEngine = 0;
    unknown->QueryInterface(IID_IHXClientEngine, (void**)&pEngine );
    if( pEngine != 0 )
    {
        IUnknown* pTemp = 0;
        pEngine->GetPlayer(0, pTemp);
        _player = (IHXPlayer*)pTemp;
    }
    
    HX_RELEASE(pEngine);
    HX_ASSERT(_player);
}

/////////////////////////////////////////////////////////////////////
//  Member: HxErrorSink::Destructor
//  Access: Private
//  Purpose: The default destructor of the class that releases
//               the interface member variables from memory.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: None 
////////////////////////////////////////////////////////////////////
HxErrorSink::~HxErrorSink() {
  HX_RELEASE(_player);
}


////////////////////////////////////////////////////////////////////
//  Member: HxErrorSink::QueryInterface
//  Access:
//  Purpose: Queries this class to determine whether it supports
//               supports a specific interface. If the call succeeds
//               then the methods of that interface are made 
//               available for use.
/////////////////////////////////////////////////////////////////////
//  Params: _id - Indicates the reference identifier of the 
//                 the interface being queried.
//          _interface_obj - Points to an interface pointer that is
//                           filled in if the query succeeds.
//  Return: HX_RESULT - Varies based on wheter the interface is
//                      supported or not.
////////////////////////////////////////////////////////////////////
HX_RESULT HxErrorSink::QueryInterface(REFIID _id, void** _interface_obj) {
  // Determine if the IUnknown and ErrorSink interfaces
  // are supported.
  if (IsEqualIID(_id, IID_IUnknown)) {
   // Increase the reference count, set the Interface Object,
   // and return that the interface is supported within this
   // object.
   AddRef();
   *_interface_obj = (IUnknown*)(IHXErrorSink*)this;
  }
  else if (IsEqualIID(_id, IID_IHXErrorSink)) {
    // Same as above.
    AddRef();
    *_interface_obj = (IHXErrorSink*)this;
  }
  else {
    // This Interface is not supported by this object. Set the
    // Interface Object to the NULL-state and return.
    *_interface_obj = 0;
    return HXR_NOINTERFACE;
  }
  return HXR_OK;
}

////////////////////////////////////////////////////////////////////
//  Member: HxErrorSink::AddRef
//  Access: Public
//  Purpose: Increases the object's reference count by one.
//               Whenever an object is created, it's reference count
//               begins at 1.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: ULONG32 - The new reference count. 
////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG32) HxErrorSink::AddRef() {
  return InterlockedIncrement(&ref_count);
}
















////////////////////////////////////////////////////////////////////
//  Member: HxErrorSink::Release
//  Access: Public
//  Purpose: Decreases the object's reference count by one. 
//               Every call to IUnknown::AddRef, 
//               IUnknown::QueryInterface, or a creation function 
//               such as HXCreateInstance must have a corresponding 
//               call to IUnknown::Release. When the reference count 
//               reaches 0 (zero), the object is destroyed. 
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: ULONG32 - The new reference count.
////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG32) HxErrorSink::Release() {
  // As long as the reference count is greater than 0, then this
  // object is still in "scope." 
  if (InterlockedDecrement(&ref_count) > 0 ) {
    return ref_count;
  }

  // Otherwise, this object is no longer necessary and should be 
  // removed from memory.
  delete this;
  return 0;
}

////////////////////////////////////////////////////////////////////
//  Member: HxErrorSink::ErrorOccurred
//  Access: Public
//  Description: Reports an error, event, or status message
////////////////////////////////////////////////////////////////////
//  Params: severity - Severity or level of the error message.
//          hx_code - Specific Error Code
//          user_code - User-Specific Error Code
//          user_string - Pointer to a user-specific string.
//          more_info_url - Pointer to a specific more information
//                         URL string.
//  Return: None 
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxErrorSink::ErrorOccurred(const UINT8 severity, const UINT32 hx_code,
	                                    const UINT32 user_code, const char* user_string,
										const char* more_info_url) {
  // Initialize Local Variables to the Method.
  char HXDefine[256];

  // Store the error code that was generated.
  convert_to_string(hx_code, HXDefine, 256);

  STDOUT("Report(%d, %ld, \"%s\", %ld, \"%s\", \"%s\")\n", severity,
         hx_code, (user_string && *user_string) ? user_string : "(NULL)",
         user_code, (more_info_url && *more_info_url) ? more_info_url : "(NULL)",
         HXDefine);

  return HXR_OK;
}


										
										
										
										
										
										
										
										
										
										

////////////////////////////////////////////////////////////////////
//  Member: HxErrorSink::ConvertErrorToText
//  Access: Protected
//  Purpose: Converts the Helix error code to text for the 
//               client to display.
////////////////////////////////////////////////////////////////////		
//  Params: hx_code - the error code to be translated to text.
//          buffer - Text buffer that holds the code.
//          buffer_length - The length of the text buffer.
//  Return: None 
////////////////////////////////////////////////////////////////////
void HxErrorSink::convert_to_string(const ULONG32 hx_code, char* buffer, UINT32 buffer_length) {
  // Initialize Local Variables to this method. 
  //IHXErrorMessages * pErrMsg = 0;
  
  // If the buffer is not empty, then there is nothing to do.
  if( !buffer ) {
    return;
  }
  buffer[0] = '\0';

  if (strlen(buffer) == 0) {
    SafeSprintf(buffer, buffer_length, "Can't convert the error code %p", hx_code);      
  }
}