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

////////////////////////////////////////////////////////////////////
// Helix Main Header Files
////////////////////////////////////////////////////////////////////
#include "mainHelix.h"

////////////////////////////////////////////////////////////////////
//  Member: HxClientContext::Constructor
//  Access: Public
//  Purpose: The constructor simply initializes all of the 
//               member variables.
////////////////////////////////////////////////////////////////////
//  Params: LONG32 client_index - the player/client count
//  Return: None
////////////////////////////////////////////////////////////////////
HxClientContext::HxClientContext(LONG32 client_index)
  : _ref_count(0),
    _client_index(client_index),
	_client_sink(0),
	_error_sink(0),
	_auth_mgr(0),
	_site_supplier(0),
	_default_prefs(0) {
}

////////////////////////////////////////////////////////////////////
//  Member: HxClientContext::Destructor
//  Access: Public
//  Purpose: The destructor simply calls the close member
//               function.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: None
////////////////////////////////////////////////////////////////////
HxClientContext::~HxClientContext(){
  close();
}














////////////////////////////////////////////////////////////////////
//  Member: HxClientContext::init
//  Access: Public
//  Purpose: This member performs all of the dirty work for 
//               initializing the the client context for a player
//               as well as instantiating all of the necessary
//               Helix interface objects that must be declared so
//               the engine can properly communicate with the player
////////////////////////////////////////////////////////////////////
//  Params: IUnknown* unknown - a Helix Interface Object
//          IHXPreferences* prefs - 
//          char* guid - a GUID, if applicable.
//          bool sink_on - True if the AdviseSink should broadcast
//                         presentation statistics.
//          Texture* tex - a Panda Texture Object which will be
//                         updated with the video each frame.
//  Return: None
////////////////////////////////////////////////////////////////////
void HxClientContext::init(IUnknown* unknown, IHXPreferences* prefs, char* guid, bool sink_on, Texture* tex) {
  char * cipher = 0;

  _client_sink = new HxAdviseSink(unknown, _client_index, sink_on);
  _error_sink = new HxErrorSink(unknown);
  _auth_mgr = new HxAuthenticationManager();
#if defined(HELIX_FEATURE_VIDEO)
  // From this point, we need to create a site supplier object since we are dealing with video.
  // In addtion, we must send the Panda Texture buffer down to the site supplier, where it will
  // inevitably be sent to the video surface object so that it can be updated.
  //cout << "Buffer: " << buffer << endl;
  _site_supplier = new HxSiteSupplier(unknown, tex);
#endif

  if (_client_sink != 0) {
	_client_sink->AddRef();
  }

  if (_error_sink != 0) {
	_error_sink->AddRef();
  }

  if (_auth_mgr != 0) {
    _auth_mgr->AddRef();
  }

  if (_site_supplier != 0) {
    _site_supplier->AddRef();
  }

  if (prefs != 0) {
    _default_prefs = prefs;
	_default_prefs->AddRef();
  }

  if (guid && *guid) {
	  // Encode GUID
	  cipher = Cipher(guid);
	  SafeStrCpy(_guid, cipher, 256);
  }
  else {
    _guid[0] = '\0';
  }
}






////////////////////////////////////////////////////////////////////
//  Member: HxClientContext::close
//  Access: Public
//  Purpose: This method simply releases each of the helix
//               interface objects that are associated with the 
//               player. For instance, the site supplier and the
//               error sink.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: None
////////////////////////////////////////////////////////////////////
void HxClientContext::close() {
  HX_RELEASE(_client_sink);
  HX_RELEASE(_error_sink);
  HX_RELEASE(_auth_mgr);
  HX_RELEASE(_site_supplier);
  HX_RELEASE(_default_prefs);
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientContext::QueryInterface
//  Access: Public
//  Purpose: Queries this class to determine whether it supports
//               supports a specific interface. If the call succeeds
//               then the methods of that interface are made 
//               available for use.
////////////////////////////////////////////////////////////////////
//  Params: id - Indicates the reference identifier of the 
//                 the interface being queried.
//          interface_obj - Points to an interface pointer that is
//                           filled in if the query succeeds.
//  Return: HX_RESULT - Varies based on wheter the interface is
//                      supported or not.
////////////////////////////////////////////////////////////////////
HX_RESULT HxClientContext::QueryInterface(THIS_ REFIID id, void** interface_obj) {
  HX_RESULT result = HXR_NOINTERFACE;
  // Determine if the IUnknown and ClientAdviseSink interfaces
  // are supported.
	if (IsEqualIID(id, IID_IUnknown)) {
	  // Increase the reference count, set the Interface Object,
	  // and return that the interface is supported within this
	  // object.
	  AddRef();
	  *interface_obj = (IUnknown*)(IHXClientAdviseSink*)this;
	  result = HXR_OK;
	}
	else if (IsEqualIID(id, IID_IHXPreferences)) {
	  // Same as above.
	  AddRef();
	  *interface_obj = (IHXPreferences*)this;
	  result = HXR_OK;
	}
	else if ((_client_sink != 0) && 
		     (_client_sink->QueryInterface(id, interface_obj) == HXR_OK)) {
	  result = HXR_OK;
	}
	else if ((_error_sink != 0) && 
		     (_error_sink->QueryInterface(id, interface_obj) == HXR_OK)) {
	  result = HXR_OK;
	}
	else if ((_auth_mgr != 0) &&
		     (_auth_mgr->QueryInterface(id, interface_obj) == HXR_OK)) {
	  result = HXR_OK;
	}
	else if ((_site_supplier != 0) &&
		     (_site_supplier->QueryInterface(id, interface_obj) == HXR_OK)) {
      result = HXR_OK;
	}
	else {
	  // This Interface is not supported by this object. Set the
	  // Interface Object to the NULL-state and return.
	  *interface_obj = 0;
	}
  return result;
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientContex::AddRef
//  Access: Public
//  Purpose: Increases the classes reference count by one. 
//               Whenever an object is created, it's reference count 
//               begins at 1. If an application calls IUnknown::AddRef, 
//               queries an interface belonging to a specific object, 
//               or uses a creation function like HXCreateInstance, 
//               the reference count is incremented by 1. 
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: Returns the new reference count.
////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG32) HxClientContext::AddRef(THIS) {
  return InterlockedIncrement(&_ref_count);
}

////////////////////////////////////////////////////////////////////
//  Method: HxClientContext::Release
//  Access: Public
//  Purpose: Decreases the object's reference count by one. 
//               Every call to IUnknown::AddRef, 
//               IUnknown::QueryInterface, or a creation function 
//               such as HXCreateInstance must have a corresponding 
//               call to IUnknown::Release. When the reference count 
//               reaches 0 (zero), the object is destroyed. 
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: Returns the new reference count.
////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG32) HxClientContext::Release(THIS) {
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
//  Member: HxClientContext::ReadPref
//  Access: Public
//  Purpose: This method reads a preference from the client
//               registry.
////////////////////////////////////////////////////////////////////
//  Params: const char* pref_key - preference key to add to the
//                                 registry.
//          IHXBuffer* buffer - a buffer that manages the value of
//                              the preference.
//  Return: IHXBuffer* buffer - returns a buffer that manages
//                              the value of the preference.
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxClientContext::ReadPref(const char* pref_key, IHXBuffer*& buffer) {
  HX_RESULT result = HXR_OK;
  char * cipher = 0;

  if ((stricmp(pref_key, CLIENT_GUID_REGNAME) == 0) && 
	  (*_guid != 0 )) {
    // Create a buffer
    buffer = new CHXBuffer();
	buffer->AddRef();

	buffer->Set((UCHAR*)_guid, strlen(_guid) + 1);
  }
  else if (_default_prefs != 0) {
    result = _default_prefs->ReadPref(pref_key, buffer);
  }
  else {
	result = HXR_NOTIMPL;
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//  Member: HxClientContext::WritePref
//  Access: Public
//  Purpose: This method writes a preference to the helix
//               client registry.
////////////////////////////////////////////////////////////////////
//  Params: const char* pref_key - preference key to add to the
//                                 registry.
//          IHXBuffer* buffer - a buffer that manages the value of
//                              the preference.
//  Return: IHXBuffer* buffer - returns a buffer that manages
//                              the value of the preference.
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxClientContext::WritePref(const char* pref_key, IHXBuffer *buffer) {
  if (_default_prefs != 0) {
    return _default_prefs->WritePref(pref_key, buffer);
  }
  else {
	return HXR_OK;
  }
}
