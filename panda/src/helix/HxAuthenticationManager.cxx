// Filename: HxAuthenticationManager.cxx
// Created by:  jjtaylor (10Feb04)
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

////////////////////////////////////////////////////////////////////
// Panda Header Files
////////////////////////////////////////////////////////////////////
#include "HxAuthenticationManager.h"
#include "print.h"

////////////////////////////////////////////////////////////////////
// Normal Header Files
////////////////////////////////////////////////////////////////////
#include <ctype.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////
//  Member: HxAuthenticationManager::HxAuthenticationManager
//  Access: Public
//  Purpose: A default constructor which initializes the member
//               variables for the class.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: None
////////////////////////////////////////////////////////////////////
HxAuthenticationManager::HxAuthenticationManager()
  : _ref_count(0),
    _sent_password(FALSE) {
}

////////////////////////////////////////////////////////////////////
//  Member: HxAuthenticationManager::Destructor
//  Access: Public
//  Purpose: The default destructor which removes existing
//               COM interfaces that have not been freed from
//               memory.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: None 
////////////////////////////////////////////////////////////////////
HxAuthenticationManager::~HxAuthenticationManager() {
}












////////////////////////////////////////////////////////////////////
//  Member: HxAuthenticationManager::QueryInterface
//  Access: Public
//  Purpose: Queries this class to determine whether it supports
//               supports a specific interface. If the call succeeds
//               then the methods of that interface are made 
//               available for use.
////////////////////////////////////////////////////////////////////
//  Params: id - Indicates the reference identifier of the 
//                 the interface being queried.
//          ppInterfaceObj - Points to an interface pointer that is
//                           filled in if the query succeeds.
//  Return: HX_RESULT - Varies based on wheter the interface is
//                      supported or not.
////////////////////////////////////////////////////////////////////
HX_RESULT HxAuthenticationManager::QueryInterface(THIS_ REFIID id, void** ppInterfaceObj) {
  // Determine if the IUnknown and AuthenticationManager interfaces
  // are supported.
  if (IsEqualIID(id, IID_IUnknown)) {
    // Increase the reference count, set the Interface Object,
    // and return that the interface is supported within this
    // object.
    AddRef();
    *ppInterfaceObj = (IUnknown*)(IHXAuthenticationManager *)this;
  }
  else if (IsEqualIID(id, IID_IHXAuthenticationManager)) {
    // Same as above.
    AddRef();
    *ppInterfaceObj = (IHXAuthenticationManager *)this;
  }
  else {
    // This Interface is not supported by this object. Set the
    // Interface Object to the NULL-state and return.
    *ppInterfaceObj = 0;
    return HXR_NOINTERFACE;
  }
  return HXR_OK;
}

////////////////////////////////////////////////////////////////////
//  Member: HxAuthenticationManager::AddRef
//  Access: Public
//  Purpose: Increases the object's reference count by one. 
//               Every call to IUnknown::AddRef, 
//               IUnknown::QueryInterface, or a creation function 
//               such as HXCreateInstance must have a corresponding 
//               call to IUnknown::Release. When the reference count 
//               reaches 0 (zero), the object is destroyed.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: ULONG32 - The new reference count. 
////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG32) HxAuthenticationManager::AddRef(THIS) {
    return InterlockedIncrement(&_ref_count);
}













////////////////////////////////////////////////////////////////////
//  Member: HxAuthenticationManager::Release
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
STDMETHODIMP_(ULONG32) HxAuthenticationManager::Release(THIS) {
  // As long as the reference count is greater than 0, then this
  // object is still in "scope." 
  if (InterlockedDecrement(&_ref_count) > 0 ) {
    return _ref_count;
  }

  // Otherwise, this object is no longer necessary and should be 
  // removed from memory.
  delete this;
  return 0;
}

////////////////////////////////////////////////////////////////////
//  Member: HxAuthenticationManager:HandleAuthenticationRequest
//  Access: Public
//  Purpose: Retrieves the username and password which is
//               needed to access a specific file or URL. 
////////////////////////////////////////////////////////////////////
//  Params: pResponse - Pointer to a response interface that
//                      manages the response of the authentication
//                      of the username and password.
//  Return: HX_RESULT - Based on the authentication process.
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxAuthenticationManager::HandleAuthenticationRequest(IHXAuthenticationManagerResponse* pResponse)
{
    char      username[1024] = ""; /* Flawfinder: ignore */
    char      password[1024] = ""; /* Flawfinder: ignore */
    HX_RESULT res = HXR_FAIL;
        
    if( !_sent_password )
    {
        res = HXR_OK;
        
        STDOUT("\nSending Username and Password...\n");

        SafeStrCpy(username, "", 1024);
       SafeStrCpy(password, "", 1024);

        //strip trailing whitespace
        char* c;
        for(c = username + strlen(username) - 1; 
            c > username && isspace(*c);
            c--)
            ;
        *(c+1) = 0;
    
        for(c = password + strlen(password) - 1; 
            c > password && isspace(*c);
            c--)
            ;
        *(c+1) = 0;
        
        _sent_password = TRUE;
    }
    if (FAILED(res))
        STDOUT("\nInvalid Username and/or Password.\n");
    
    pResponse->AuthenticationRequestDone(res, username, password);
    return res;
}
