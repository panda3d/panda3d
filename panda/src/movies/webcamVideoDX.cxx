// Filename: webcamVideoDX.cxx
// Created by: jyelon (01Nov2007)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////
//
// This code was created by studying and adapting the VDOGRAB
// library by Shu-Kai Yang.  We owe him a great deal of thanks
// for figuring all this out.
//
// http://shukaiyang.myweb.hinet.net/index.html
//
// The license for VDOGRAB is as follows:
//
// "The library is agreed to be used in your production WITHOUT any
// fee. And the binary file vdograb.dll is agreed to be distributed
// WITHOUT any fee. Any production using the library DOES NOT need to
// mark the library on its logo. Therefore, I, the author or the
// library, AM NOT responsible to any problems possibly caused by the
// library in your production. I have NO DUTY to fixing any damage or
// providing any support."
//
////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN 

#undef Configure

#pragma warning(disable:4100) // warning C4100: unreferenced formal parameter
#pragma warning(disable:4201) // warning C4201: nonstandard extension used : nameless struct/union
#pragma warning(disable:4511) // warning C4511: copy constructor could not be generated
#pragma warning(disable:4512) // warning C4512: assignment operator could not be generated
#pragma warning(disable:4514) // warning C4514: "unreferenced inline function has been removed"

#include <windows.h>
#include <windowsx.h>
#include <olectl.h>
#include <mmsystem.h>
#include <strmif.h>     // Generated IDL header file for streams interfaces
#include <amvideo.h>    // ActiveMovie video interfaces and definitions
#include <amaudio.h>    // ActiveMovie audio interfaces and definitions
#include <control.h>    // generated from control.odl
#include <evcode.h>     // event code definitions
#include <uuids.h>      // declaration of type GUIDs and well-known clsids
#include <errors.h>     // HRESULT status and error definitions
#include <edevdefs.h>   // External device control interface defines
#include <audevcod.h>   // audio filter device error event codes
#include <dvdevcod.h>   // DVD error event codes

#include <wchar.h>
#include <string.h>
#include <windows.h>
#include <qedit.h>
#include <atlbase.h>

////////////////////////////////////////////////////////////////////
//
// I'm hiding the entire definition of class WebcamVideoCursor in
// the OS specific portion of the WebcamVideo CXX file.  This goes
// against Panda3D coding conventions: normally, classes get
// exposed in header files.  However, this class is such a mess
// of OS-specific code and conditional compilation that it's better
// to just hide it entirely.  - Josh
//
////////////////////////////////////////////////////////////////////

class WebcamVideoCursor: public MovieVideoCursor
{
public:
  WebcamVideoCursor(WebcamVideo *src);
  virtual ~WebcamVideoCursor();
  virtual void fetch_into_buffer(double time, unsigned char *block, bool rgba);

public:
  void cleanup();

  class CSampleGrabberCB : public ISampleGrabberCB 
  { 
  public:
    int width;
    int height;
    
    ULONG __stdcall AddRef() { return 2; }
    ULONG __stdcall Release() { return 1; }
    
    HRESULT __stdcall QueryInterface(REFIID riid, void ** ppv);
    HRESULT __stdcall SampleCB(double SampleTime, IMediaSample *pSample);
    HRESULT __stdcall BufferCB(double dblSampleTime, BYTE *pBuffer, long lBufferSize);
  };

  IGraphBuilder           *_pGraphBuilder;
  ICaptureGraphBuilder2   *_pCaptureBuilder;
  IBaseFilter             *_pSrcFilter;
  CComPtr<ISampleGrabber>  _pSampleGrabber; 
  IBaseFilter             *_pVMR9;
  IMediaControl           *_pMediaCtrl;
  IVMRWindowlessControl   *_pWindowssCtrl;   
  CSampleGrabberCB         _sample_grabber_cb;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieVideoCursor::init_type();
    register_type(_type_handle, "WebcamVideoCursor",
                  MovieVideoCursor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

TypeHandle WebcamVideoCursor::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideo::get_num_devices
//       Access: Static, Published
//  Description: 
////////////////////////////////////////////////////////////////////
int WebcamVideo::
get_num_devices() {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideo::get_device_name
//       Access: Static, Published
//  Description: 
////////////////////////////////////////////////////////////////////
string WebcamVideo::
get_device_name(int n) {
  return "";
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideo::open
//       Access: Published, Virtual
//  Description: Open this video, returning a MovieVideoCursor.
////////////////////////////////////////////////////////////////////
PT(MovieVideoCursor) WebcamVideo::
open() {
  return new WebcamVideoCursor(this);
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideo::init_cursor_type
//       Access: Static, Public
//  Description: Calls WebcamVideoCursor::init_type
////////////////////////////////////////////////////////////////////
void WebcamVideo::
init_cursor_type() {
  WebcamVideoCursor::init_type();
  CoInitialize(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideoCursor::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
WebcamVideoCursor::
WebcamVideoCursor(WebcamVideo *src) :
  MovieVideoCursor(src),
  _pGraphBuilder(NULL),
  _pCaptureBuilder(NULL),
  _pSrcFilter(NULL),
  _pVMR9(NULL),
  _pMediaCtrl(NULL),
  _pWindowssCtrl(NULL)
{

  wchar_t *pFriendlyName = 0; // Put the device name here.

  AM_MEDIA_TYPE mediaType;
  VIDEOINFOHEADER *pVideoInfo;
  RECT srcRect, destRect;
  
  ICreateDevEnum *pCreateDevEnum=NULL;
  IEnumMoniker *pEnumMoniker=NULL;
  IPropertyBag *property=NULL;
  IMoniker *pMoniker=NULL;
  IVMRFilterConfig *pFilterConfig;
  
  HRESULT hResult;
  ULONG cFetched;
  VARIANT name;
  
  static wchar_t deviceName[256];
  
  hResult=CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                           IID_IGraphBuilder,(void**)&_pGraphBuilder);
  if(hResult != S_OK) { cleanup(); return; }
  
  
  hResult=CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
                           IID_ICaptureGraphBuilder2, (void**)&_pCaptureBuilder);
  if(hResult != S_OK) { cleanup(); return; }
  
  _pCaptureBuilder->SetFiltergraph(_pGraphBuilder);
  cerr << "  IID_IGraphBuilder & IID_ICaptureGraphBuilder2 are established.\n";
  
  hResult=_pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&_pMediaCtrl);
  if(FAILED(hResult))
    {  cerr << "  Can not get the IID_IMediaControl interface!";
    cleanup(); return;  }
  cerr << "  IID_IMediaControl interface is acquired.\n";
  
  
  
  
  
  hResult=CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                           IID_ICreateDevEnum, (void**)&pCreateDevEnum);
  if(hResult != S_OK) {  cleanup(); return;  }
  cerr << "  IID_ICreateDevEnum of CLSID_SystemDeviceEnum is acquired.\n";
  
   
  hResult=pCreateDevEnum->CreateClassEnumerator
    (CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);
  if(hResult != DD_OK)
    {  cleanup();  pCreateDevEnum->Release();  return;  }
  cerr << "  Moniker of CLSID_VideoInputDeviceCategory is acquired.\n";
  
  
  while(pEnumMoniker->Next(1, &pMoniker, &cFetched) == S_OK)
    {  if(pFriendlyName)
      {  
        pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&property);
        VariantInit(&name);
        hResult=property->Read(L"FriendlyName", &name, 0);
        wcscpy(deviceName, name.bstrVal);
        VariantClear(&name);
        property->Release();
        cerr << "  Enumerated device: " << deviceName << "\n";
        
        if(wcscmp(deviceName, pFriendlyName) == 0)
          {  pMoniker->BindToObject
               (NULL, NULL, IID_IBaseFilter, (void**)&_pSrcFilter);  }
      }
    else
      {  
        pMoniker->BindToObject
          (NULL, NULL, IID_IBaseFilter, (void**)&_pSrcFilter);
      }
    
    pMoniker->Release();
    if(_pSrcFilter != NULL) {  break;  }
    }
  
  
  pCreateDevEnum->Release();
  pEnumMoniker->Release();
  
  if(_pSrcFilter == NULL)
    {  cerr << "  Such capture device is not found.\n";
    cleanup(); return;  }
  cerr << "  The capture filter is acquired.\n";
  
  
  hResult=_pGraphBuilder->AddFilter(_pSrcFilter, L"Capture Filter");
  if(hResult != DD_OK)
    {  cerr << "  The capture filter can not be added to the graph.\n";
    cleanup(); return;  }
  cerr << "  The capture filter has been added to the graph.\n";
  
   
  _pSampleGrabber.CoCreateInstance(CLSID_SampleGrabber);
  if(!_pSampleGrabber)
    {  cerr << "  Can not create the sample grabber, maybe qedit.dll is not registered?";
    cleanup(); return;  }
  
  
  CComQIPtr< IBaseFilter, &IID_IBaseFilter > pGrabberFilter(_pSampleGrabber);
  cerr << "  IID_IBaseFilter of CLSID_SampleGrabber is acquired.\n";
  
  
  ZeroMemory(&mediaType, sizeof(AM_MEDIA_TYPE));
  mediaType.majortype=MEDIATYPE_Video;
  mediaType.subtype=MEDIASUBTYPE_RGB24;
  hResult=_pSampleGrabber->SetMediaType(&mediaType);
  if(FAILED(hResult))
    {  cerr << "  Fail to set the media type!";
    cleanup(); return;  }
  cerr << "  The media type of the sample grabber is set 24-bit RGB.\n";
  
  
  hResult=_pGraphBuilder->AddFilter(pGrabberFilter, L"Sample Grabber");
  if(FAILED(hResult))
    {  cerr << "  Fail to add the sample grabber to the graph.";
    cleanup(); return;  }
  cerr << "  The sample grabber has been added to the graph.\n";
   
  hResult=CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC,
                           IID_IBaseFilter, (void**)&_pVMR9);
  if(FAILED(hResult))
    {  cerr << "  Can not create the video mixing renderer.";
    cleanup(); return;  }
  cerr << "  IID_IBaseFilter of CLSID_VideoMixingRenderer is acquired.\n";
  
  
  hResult=_pGraphBuilder->AddFilter(_pVMR9, L"Video Mixing Renderer");
  if(FAILED(hResult))
    {  cerr << "  Fail to add the VMR to the graph.";
    cleanup(); return;  }
  cerr << "  The VMR has been added to the graph.\n";
  
  
  hResult=_pVMR9->QueryInterface(IID_IVMRFilterConfig, (void**)&pFilterConfig);
  if(FAILED(hResult))
    {  cerr << "  Can not get IVMRFilterConfig interface of VMR.";
    cleanup(); return;  }
  
  hResult=pFilterConfig->SetRenderingMode(VMRMode_Windowless);
  pFilterConfig->Release();
  
  if(FAILED(hResult))
    {  cerr << "  Can not set VMR in windowless mode.";
    cleanup(); return;  }
  cerr << "  VMR is set in windowless mode by IVMRFilterConfig interface.\n";
  
  hResult=_pVMR9->QueryInterface(IID_IVMRWindowlessControl, (void**)&_pWindowssCtrl);
  if(FAILED(hResult))
    {  cerr << "  Can not get the IVMRWindowlessControl interface.";
    cleanup(); return;  }
  
  cerr << "  IID_IVMRWindowlessControl interface is acquired.\n";
  _pWindowssCtrl->SetVideoClippingWindow(GetDesktopWindow());
  
  
  
  
  
  hResult=_pCaptureBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
                                          &MEDIATYPE_Video, _pSrcFilter, pGrabberFilter, _pVMR9);
  if(FAILED(hResult))
    {  hResult=_pCaptureBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
                                               &MEDIATYPE_Video, _pSrcFilter, pGrabberFilter, _pVMR9);
    
    if(FAILED(hResult))
      {  cerr << "  ICaptureGraphBuilder2::RenderStream() can not connect the pins.\n";
      cleanup(); return;  }
    }
  
  
  
  
  hResult=_pSampleGrabber->GetConnectedMediaType(&mediaType);
  if(FAILED(hResult))
    {  cerr << "  Failed to read the connected media type.";
    cleanup(); return;  }
  
  
  pVideoInfo=(VIDEOINFOHEADER*)mediaType.pbFormat;
  _sample_grabber_cb.width=pVideoInfo->bmiHeader.biWidth;
  _sample_grabber_cb.height=pVideoInfo->bmiHeader.biHeight;
  
  
  if(mediaType.cbFormat != 0)
    {  CoTaskMemFree((PVOID)mediaType.pbFormat);
    mediaType.cbFormat=0;
    mediaType.pbFormat=NULL;  }
  
  if(mediaType.pUnk != NULL)
    {  mediaType.pUnk->Release();
    mediaType.pUnk=NULL;  }
  
  
  _pSampleGrabber->SetBufferSamples(FALSE);
  _pSampleGrabber->SetOneShot(FALSE);
  hResult=_pSampleGrabber->SetCallback(&_sample_grabber_cb, 1);
  if(FAILED(hResult))
    {  cerr << "  Can not set the callback interface!";
    cleanup(); return; }
  
  
  SetRect(&srcRect,  0, 0, _sample_grabber_cb.width, _sample_grabber_cb.height);
  SetRect(&destRect, 0, 0, _sample_grabber_cb.width, _sample_grabber_cb.height);
  _pWindowssCtrl->SetVideoPosition(&srcRect, &destRect); 

  if(_pMediaCtrl) {
    _pMediaCtrl->Run();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideoCursor::cleanup
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void WebcamVideoCursor::
cleanup() {
  if (_pMediaCtrl) {
    _pMediaCtrl->Stop();
  }
  
  if(_pWindowssCtrl)    {  _pWindowssCtrl->Release();  _pWindowssCtrl=NULL;  }
  if(_pMediaCtrl)       {  _pMediaCtrl->Release();  _pMediaCtrl=NULL;  }
  if(_pCaptureBuilder)  {  _pCaptureBuilder->Release();  _pCaptureBuilder=NULL;  }
  if(_pGraphBuilder)    {  _pGraphBuilder->Release();  _pGraphBuilder=NULL;  }
  if(_pSampleGrabber.p) {  _pSampleGrabber.Release();  }
  if(_pVMR9)            {  _pVMR9->Release();  _pVMR9=NULL;  }
  if(_pSrcFilter)       {  _pSrcFilter->Release();  _pSrcFilter=NULL;  }
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideoCursor::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
WebcamVideoCursor::
~WebcamVideoCursor() {
  cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideoCursor::fetch_into_buffer
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void WebcamVideoCursor::
fetch_into_buffer(double time, unsigned char *block, bool rgba) {
  MovieVideoCursor::fetch_into_buffer(time, block, rgba);
}


////////////////////////////////////////////////////////////////////
//     Function: WebcamVideoCursor::CSampleGrabberCB::QueryInterface
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
HRESULT __stdcall WebcamVideoCursor::CSampleGrabberCB::QueryInterface(REFIID riid, void **ppv)
{
  if((riid == IID_ISampleGrabberCB) || (riid == IID_IUnknown)) {
    *ppv=(void *)static_cast<ISampleGrabberCB *> (this);
    return NOERROR;
  } 

  return E_NOINTERFACE;
}


////////////////////////////////////////////////////////////////////
//     Function: WebcamVideoCursor::CSampleGrabberCB::SampleCB
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
HRESULT __stdcall WebcamVideoCursor::CSampleGrabberCB::SampleCB(double SampleTime, IMediaSample *pSample)
{
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideoCursor::CSampleGrabberCB::BufferCB
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
HRESULT __stdcall WebcamVideoCursor::CSampleGrabberCB::BufferCB(double dblSampleTime, BYTE *pBuffer, long lBufferSize)
{ 
  // Store the data!
  return 0;
}

