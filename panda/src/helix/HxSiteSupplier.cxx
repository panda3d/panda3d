// Filename: hxSiteSupplier.cxx
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

////////////////////////////////////////////////////////////////////
// Helix Main Header Files
////////////////////////////////////////////////////////////////////
#include "mainHelix.h"

////////////////////////////////////////////////////////////////////
// Normal Header Files
////////////////////////////////////////////////////////////////////
#include <iostream>

////////////////////////////////////////////////////////////////////
//  Member: HxSiteSupplier::Constructor
//  Access: Public
//  Purpose: The default constructor of the class which 
//               initializes the member variables, and queries
//               the appropriate Helix Interfaces.
////////////////////////////////////////////////////////////////////
//  Params: IUnknown * - Pointer to an IUnknown COM Interface
//          Texture * tex- Pointer to the Panda texture object.
//  Return: None
////////////////////////////////////////////////////////////////////
HxSiteSupplier::HxSiteSupplier(IUnknown* unknown, Texture* tex)
  : _ref_count(0), 
    _site_manager(0), 
	_ccf(0), 
	_unknown(unknown),
	_dest_buffer(0),
	_texture(tex) {
    
	if(_unknown) {
	  _unknown->QueryInterface(IID_IHXSiteManager, (void**)&_site_manager);
	  _unknown->QueryInterface(IID_IHXCommonClassFactory, (void**)&_ccf);
	  _unknown->AddRef();
    }
}

/////////////////////////////////////////////////////////////////////
//  Member: HxSiteSupplier:Destructor
//  Access: Private
//  Purpose: The default destructor of the class that releases
//               the interface member variables from memory.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: None 
////////////////////////////////////////////////////////////////////
HxSiteSupplier::~HxSiteSupplier() {
  HX_RELEASE(_site_manager);
  HX_RELEASE(_ccf);
  HX_RELEASE(_unknown);
}
////////////////////////////////////////////////////////////////////
//  Member: HxSiteSupplier::QueryInterface
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
STDMETHODIMP HxSiteSupplier::QueryInterface(REFIID _id, void** _interface_obj) {
  if (IsEqualIID(_id, IID_IUnknown)) {
	AddRef();
	*_interface_obj = (IUnknown*)(IHXSiteSupplier*)this;
	return HXR_OK;
  }
  else if (IsEqualIID(_id, IID_IHXSiteSupplier)) {
	AddRef();
	*_interface_obj = (IHXSiteSupplier*)this;
	return HXR_OK;
  }
  *_interface_obj = NULL;
  return HXR_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////
//  Member: HxSiteSupplier::AddRef
//  Access: Public
//  Purpose: Increases the object's reference count by one.
//               Whenever an object is created, it's reference count
//               begins at 1.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: ULONG32 - The new reference count. 
////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG32) HxSiteSupplier::AddRef() {
  return InterlockedIncrement(&_ref_count);
}

////////////////////////////////////////////////////////////////////
//  Member: HxSite::Release
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
STDMETHODIMP_(ULONG32)HxSiteSupplier::Release() {
  if (InterlockedDecrement(&_ref_count) > 0) {
    return _ref_count;
  }
  delete this;
  return 0;
}




////////////////////////////////////////////////////////////////////
//  Member: HxSiteSupplier::SitesNeeded
//  Access: Public
//  Purpose: Informs the site supplier that a site with a
//               particular set of characteristics is needed. If the
//               supplier can fulfill the requestion, it should call
//               the site manager and add a new site to it.
////////////////////////////////////////////////////////////////////
//  Params: UINT32 request_id - ID used to map between corresponding
//                              "sites needed" and "sites not needed"
//                              calls.
//          IHXValues * pProps - The properties of the requested site
//  Return: None
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxSiteSupplier::SitesNeeded(UINT32	request_id, IHXValues*	pProps) {
  // Determine if there are valid properties. If not then a site
  // can not properly be created.
  if (!pProps) {
	return HXR_INVALID_PARAMETER;
  }

  // Local Variable Declaration and Intialization
  HRESULT hres = HXR_OK;
  IHXValues* site_props	= NULL;
  IHXSiteWindowed* pSiteWindowed	= NULL;
  IHXBuffer* pValue	= NULL;
  UINT32 style = 0;
  IHXSite* pSite = NULL;

  // Just let the Helix client core create a windowed site for us.
  hres = _ccf->CreateInstance(CLSID_IHXSiteWindowed,(void**)&pSiteWindowed);
  if (HXR_OK != hres) {
	goto exit;
  }

  hres = pSiteWindowed->QueryInterface(IID_IHXSite,(void**)&pSite);
  if (HXR_OK != hres) {
	goto exit;
  }

  hres = pSiteWindowed->QueryInterface(IID_IHXValues,(void**)&site_props);
  if (HXR_OK != hres) {
	goto exit;
  }
	
  // Figures out what type of site must be created.
  hres = pProps->GetPropertyCString("playto",pValue);
  if (HXR_OK == hres) {
	site_props->SetPropertyCString("channel",pValue);
	HX_RELEASE(pValue);
  }
  else {
	hres = pProps->GetPropertyCString("name",pValue);
	if (HXR_OK == hres) {
	    site_props->SetPropertyCString("LayoutGroup",pValue);
    	    HX_RELEASE(pValue);
	}
  }

#ifdef _WINDOWS
  style = WS_OVERLAPPED | WS_VISIBLE | WS_CLIPCHILDREN;
#endif
  
  // Determine if there is a valid Panda buffer available
  // so that it may be sent down into the sites video
  // surface object.
  if( _texture->has_ram_image() ) {
	pSiteWindowed->SetDstBuffer(_texture->_pbuffer->_image, 
								_texture->_pbuffer->get_xsize(),
								_texture->_pbuffer->get_ysize());
  }
  else {
    cout << "--- {{ HxSiteSupplier.cxx, RELOADED RAM IMAGE!!! }}---" << endl;
	PixelBuffer* fake = _texture->get_ram_image();
	if( fake ) {
		pSiteWindowed->SetDstBuffer(fake->_image, 
									_texture->_pbuffer->get_xsize(),
									_texture->_pbuffer->get_ysize());
	}
	else {
		cout << "--- {{ HxSiteSupplier.cxx, NO RAM IMAGE PRESENT!!! }} ---" << endl;
	}
  }

  // Create the window. Not necessary later on.
  hres = pSiteWindowed->Create(NULL, style);
  //hres = pSiteWindowed->AttachWindow(NULL);
  if (HXR_OK != hres) {
  	goto exit;
  }

  // Add the site to the site manager.
  hres = _site_manager->AddSite(pSite);
  if (HXR_OK != hres) {
	goto exit;
  }

  _created_sites.SetAt((void*)request_id,pSite);
  //pair<UINT32, IHXSite*> site(request_id, pSite);
  //_created_sites.insert(site);

  pSite->AddRef();

  exit:
    HX_RELEASE(site_props);
    HX_RELEASE(pSiteWindowed);
    HX_RELEASE(pSite);

  return hres;
}

////////////////////////////////////////////////////////////////////
//  Member: HxSiteSupplier::SitesNotNeeded
//  Access: Public
//  Purpose: Informs the site supplier that all sites from a 
//               previous site request are no longer needed. 
////////////////////////////////////////////////////////////////////
//  Params: UINT32 request_id - ID used to map between corresponding
//                              "sites needed" and "sites not needed"
//                              calls.
//  Return: None
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxSiteSupplier::SitesNotNeeded(UINT32 request_id) {
  // Local Variable Declaration and Initialization
  IHXSite*		pSite = NULL;
  IHXSiteWindowed*	pSiteWindowed = NULL;
  void*	pVoid = NULL;

  // search for the site id in the map
  if (!_created_sites.Lookup((void*)request_id,pVoid)) {
  	return HXR_INVALID_PARAMETER;
  }
  //SITES::iterator iter = _created_sites.find(request_id);
  //if(iter == _created_sites.end()) {
  //  return HXR_INVALID_PARAMETER;
  //}
  pSite = (IHXSite*)pVoid;

  // Remove the site from the site manager.
  _site_manager->RemoveSite(pSite);

  // Need to actually do the work on destroying the window
  // and all that jazz.
  pSite->QueryInterface(IID_IHXSiteWindowed,(void**)&pSiteWindowed);

  pSiteWindowed->Destroy();

  // ref count = 2
  pSiteWindowed->Release();

  // ref count = 1; deleted from this object's view!
  pSite->Release();

  // Remove the site from the map.
  _created_sites.RemoveKey((void*)request_id);
  //_created_sites.erase(request_id);

  return HXR_OK;
}

////////////////////////////////////////////////////////////////////
//  Member: HxSiteSupplier::BeginChangeLayout
//  Access: Public
//  Purpose: Informs the site supplier that a layout change
//               has begun and that it can expect to receive calls
//               to the SitesNeeded and SitesNotNeeded method. 
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: None
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxSiteSupplier::BeginChangeLayout() {
  return HXR_OK;
}

////////////////////////////////////////////////////////////////////
//  Member: HxSiteSupplier::DoneChangeLayout
//  Access: Public
//  Purpose: Informs the site supplier that the layout changes
//               are completed.
////////////////////////////////////////////////////////////////////
//  Params: None
//  Return: None
////////////////////////////////////////////////////////////////////
STDMETHODIMP HxSiteSupplier::DoneChangeLayout() {
  return HXR_OK;
}