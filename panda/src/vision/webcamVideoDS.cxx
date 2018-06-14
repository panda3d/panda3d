/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webcamVideoDS.cxx
 * @author jyelon
 * @date 2007-11-01
 *
 * It goes against Panda3D coding style conventions to hide an
 * entire class in a C++ file and not expose it through header
 * files at all.  However, in this case, these classes are so full
 * of OS-specific junk that I feel it is better to hide them
 * entirely.  - Josh
 *
 * This code was created by studying and adapting the VDOGRAB
 * library by Shu-Kai Yang and the videoInput library by Theodore
 * Watson.  We owe both of them a great deal of thanks for
 * figuring all this out.  Both of their libraries have
 * informal licenses (the "do whatever you want and don't blame
 * me" sort), so I think there's not a problem using their code.
 */

#if defined(HAVE_DIRECTCAM) && !defined(CPPPARSER)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

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
#include <comutil.h>

#include <wchar.h>
#include <string.h>
#include <windows.h>

using std::cerr;
using std::string;

/*
 * This used to work back when qedit.h still existed.  The hacks served to
 * prevent it from including the defunct dxtrans.h.  #pragma include_alias(
 * "dxtrans.h", "qedit.h" ) #define __IDxtCompositor_INTERFACE_DEFINED__
 * #define __IDxtAlphaSetter_INTERFACE_DEFINED__ #define
 * __IDxtJpeg_INTERFACE_DEFINED_ #define __IDxtKey_INTERFACE_DEFINED__ #define
 * IDXEffect IUnknown #include <qedit.h>
 */

// We can use this fugly hack to still access the qedit.h interfaces.  When
// this stops working, we'll have to just copy the relevant definitions to
// this file.
#import "libid:78530B68-61F9-11D2-8CAD-00A024580902" \
  no_namespace named_guids raw_interfaces_only no_implementation \
  exclude("_AMMediaType", "_FilterState", "IReferenceClock", "IMediaFilter", \
          "_PinDirection", "IEnumMediaTypes", "IFilterGraph", "_FilterInfo", \
          "IGraphBuilder", "IBaseFilter", "_PinInfo", "IPin", "IEnumPins", \
          "IEnumFilters", "IEnumMediaTypes", "IAMSetErrorLog","IAMTimelineObj", \
          "IMediaDet", "IMediaSample", "IPersistStream", "IPersist", "IStream", \
          "ISequentialStream", "_LARGE_INTEGER", "_ULARGE_INTEGER", \
          "tagSTATSTG", "_FILETIME", "IPropertyBag", "IErrorLog")

/**
 * The directshow implementation of webcams.
 */

class WebcamVideoDS : public WebcamVideo
{
public:
  static void find_all_webcams_ds();
  friend void find_all_webcams_ds();

private:
  typedef pvector<PT(WebcamVideoDS)> WebcamVideoList;

  static int  media_score(AM_MEDIA_TYPE *media);
  static int  media_x(AM_MEDIA_TYPE *media);
  static int  media_y(AM_MEDIA_TYPE *media);
  static int  media_fps(AM_MEDIA_TYPE *media);
  static void delete_media_type(AM_MEDIA_TYPE *media);
  static string bstr_to_string(const BSTR &source);
  static string get_moniker_name(IMoniker *pMoniker);
  static void add_device(WebcamVideoList &list, IMoniker *pMoniker, AM_MEDIA_TYPE *media);

  virtual PT(MovieVideoCursor) open();

  IMoniker *_moniker;
  AM_MEDIA_TYPE *_media;

  friend class WebcamVideoCursorDS;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WebcamVideo::init_type();
    register_type(_type_handle, "WebcamVideoDS",
                  WebcamVideo::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

TypeHandle WebcamVideoDS::_type_handle;

/**
 * The directshow implementation of webcams.
 */


class WebcamVideoCursorDS : public MovieVideoCursor
{
public:
  WebcamVideoCursorDS(WebcamVideoDS *src);
  virtual ~WebcamVideoCursorDS();
  virtual PT(Buffer) fetch_buffer();

public:
  void cleanup();

  class CSampleGrabberCB : public ISampleGrabberCB
  {
  public:
    WebcamVideoCursorDS *_host;

    ULONG __stdcall AddRef() { return 2; }
    ULONG __stdcall Release() { return 1; }

    HRESULT __stdcall QueryInterface(REFIID riid, void ** ppv);
    HRESULT __stdcall SampleCB(double SampleTime, IMediaSample *pSample);
    HRESULT __stdcall BufferCB(double dblSampleTime, BYTE *pBuffer, long lBufferSize);
  };

  unsigned char *_buffer;
  IMediaSample *_saved;

  IGraphBuilder           *_pGraphBuilder;
  ICaptureGraphBuilder2   *_pCaptureBuilder;
  IBaseFilter             *_pSrcFilter;
  IAMStreamConfig         *_pStreamConfig;
  ISampleGrabber          *_pSampleGrabber;
  IBaseFilter             *_pStreamRenderer;
  IMediaControl           *_pMediaCtrl;
  // IMemAllocator           *_pAllocator;
  CSampleGrabberCB         _sample_cb;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieVideoCursor::init_type();
    register_type(_type_handle, "WebcamVideoCursorDS",
                  MovieVideoCursor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

TypeHandle WebcamVideoCursorDS::_type_handle;

/**
 * Evaluate an AM_MEDIA_TYPE to determine how desirable it is for our
 * purposes.  Lower is better.
 */
int WebcamVideoDS::
media_score(AM_MEDIA_TYPE *media) {
  const GUID &subtype = media->subtype;
  if (subtype == MEDIASUBTYPE_RGB24)  return 1;
  if (subtype == MEDIASUBTYPE_RGB32)  return 2;
  if (subtype == MEDIASUBTYPE_RGB555) return 3;
  if (subtype == MEDIASUBTYPE_RGB565) return 3;
  return 4;
}

/**
 * Returns the x-resolution of the AM_MEDIA_TYPE
 */
int WebcamVideoDS::
media_x(AM_MEDIA_TYPE *media) {
  VIDEOINFOHEADER *header = (VIDEOINFOHEADER*)(media->pbFormat);
  return (header->bmiHeader.biWidth);
}

/**
 * Returns the y-resolution of the AM_MEDIA_TYPE
 */
int WebcamVideoDS::
media_y(AM_MEDIA_TYPE *media) {
  VIDEOINFOHEADER *header = (VIDEOINFOHEADER*)(media->pbFormat);
  return (header->bmiHeader.biHeight);
}

/**
 * Returns the frame-rate of the AM_MEDIA_TYPE
 */
int WebcamVideoDS::
media_fps(AM_MEDIA_TYPE *media) {
  VIDEOINFOHEADER *header = (VIDEOINFOHEADER*)(media->pbFormat);
  return int(10000000.0 / (header->AvgTimePerFrame));
}

/**
 * Free all memory of the AM_MEDIA_TYPE
 */
void WebcamVideoDS::
delete_media_type(AM_MEDIA_TYPE *pmt) {
  if (pmt == nullptr) {
    return;
  }
  if (pmt->cbFormat != 0) {
    CoTaskMemFree((PVOID)pmt->pbFormat);
    pmt->cbFormat = 0;
    pmt->pbFormat = nullptr;
  }
  if (pmt->pUnk != nullptr) {
    // Unecessary because pUnk should not be used, but safest.
    pmt->pUnk->Release();
    pmt->pUnk = nullptr;
  }
  CoTaskMemFree(pmt);
}

/**
 * Converts a visual basic BSTR to a C++ string.
 */
string WebcamVideoDS::
bstr_to_string(const BSTR &source) {
  string res = "";
  int count = 0;
  while( source[count] != 0x00 ) {
    res.push_back(source[count]);
    count++;
  }
  return res;
}

/**
 * Obtains the text name associated with an IMoniker
 */
string WebcamVideoDS::
get_moniker_name(IMoniker *pMoniker) {
  string res = "Unknown Device";
  IPropertyBag *propBag=nullptr;
  VARIANT name; HRESULT hResult;
  pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&propBag);
  VariantInit(&name);
  hResult = propBag->Read(L"FriendlyName", &name, 0);
  if (!hResult != S_OK) {
    res = bstr_to_string(name.bstrVal);
    goto done;
  }
  hResult = propBag->Read(L"Description", &name, 0);
  if (!hResult != S_OK) {
    res = bstr_to_string(name.bstrVal);
    goto done;
  }
 done:
  VariantClear(&name);
  propBag->Release();
  return res;
}

/**
 * Creates a new WebcamVideoDS and adds it to the list, unless there is
 * already a very similar configuration in the list.  If there is already a
 * very similar configuration, this routine will leave one or the other on the
 * list based on a scoring system.
 */
void WebcamVideoDS::
add_device(WebcamVideoList &list, IMoniker *pMoniker, AM_MEDIA_TYPE *media) {
  for (int i=0; i<(int)list.size(); i++) {
    if ((list[i]->_moniker == pMoniker)&&
        (media_x(list[i]->_media) == media_x(media))&&
        (media_y(list[i]->_media) == media_y(media))) {
      int oldscore = media_score(list[i]->_media);
      if (media_score(media) < oldscore) {
        delete_media_type(list[i]->_media);
        list[i]->_media = media;
      } else {
        delete_media_type(media);
      }
      return;
    }
  }
  PT(WebcamVideoDS) wc = new WebcamVideoDS;
  wc->set_name(get_moniker_name(pMoniker));
  wc->_size_x = media_x(media);
  wc->_size_y = media_y(media);
  wc->_fps = media_fps(media);
  wc->_moniker = pMoniker;
  wc->_media = media;
  list.push_back(wc);
  cerr << "Added device: DirectShow: " << wc << "\n";
}


/**
 * Finds all DirectShow webcams and adds them to the global list _all_webcams.
 */
void WebcamVideoDS::
find_all_webcams_ds() {

  pvector <PT(WebcamVideoDS)> list;

  ICreateDevEnum *pCreateDevEnum=nullptr;
  IEnumMoniker *pEnumMoniker=nullptr;
  IGraphBuilder *pGraphBuilder=nullptr;
  ICaptureGraphBuilder2 *pCaptureGraphBuilder2=nullptr;
  IMoniker *pMoniker=nullptr;
  IBaseFilter *pBaseFilter=nullptr;
  IAMStreamConfig *pStreamConfig=nullptr;
  HRESULT hResult;
  ULONG cFetched;

  hResult=CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC,
                           IID_IGraphBuilder,(void**)&pGraphBuilder);
  if (hResult != S_OK) goto cleanup;
  hResult=CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC,
                           IID_ICaptureGraphBuilder2, (void**)&pCaptureGraphBuilder2);
  if (hResult != S_OK) goto cleanup;
  hResult = pCaptureGraphBuilder2->SetFiltergraph(pGraphBuilder);
  if (hResult != S_OK) goto cleanup;
  hResult=CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
                           IID_ICreateDevEnum, (void**)&pCreateDevEnum);
  if (hResult != S_OK) goto cleanup;
  hResult=pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
                                                &pEnumMoniker, 0);
  if(hResult != S_OK) goto cleanup;

  while(1) {
    if (pMoniker)       { pMoniker->Release();  pMoniker=0; }
    if (pBaseFilter)    { pBaseFilter->Release(); pBaseFilter=0; }
    if (pStreamConfig)  { pStreamConfig->Release(); pStreamConfig=0; }

    hResult = pEnumMoniker->Next(1, &pMoniker, &cFetched);
    if (hResult != S_OK) break;

    hResult = pMoniker->BindToObject(nullptr,nullptr,IID_IBaseFilter, (void**)&pBaseFilter);
    if (hResult != S_OK) continue;
    hResult = pCaptureGraphBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pBaseFilter,
                                                   IID_IAMStreamConfig, (void **)&pStreamConfig);
    if (hResult != S_OK) continue;
    int iCount, iSize;
    hResult = pStreamConfig->GetNumberOfCapabilities(&iCount, &iSize);
    if (hResult != S_OK || (iSize != sizeof(VIDEO_STREAM_CONFIG_CAPS))) continue;
    for (int iFormat=0; iFormat<iCount; iFormat++) {
      AM_MEDIA_TYPE *mtype;
      VIDEO_STREAM_CONFIG_CAPS caps;
      hResult = pStreamConfig->GetStreamCaps(iFormat, &mtype, (BYTE*)&caps);
      if (mtype->majortype == MEDIATYPE_Video) {
        VIDEOINFOHEADER *header = (VIDEOINFOHEADER*)(mtype->pbFormat);
        header->bmiHeader.biWidth  = caps.MaxOutputSize.cx;
        header->bmiHeader.biHeight = caps.MaxOutputSize.cy;
        add_device(list, pMoniker, mtype);
      }
    }

    pMoniker = 0;
  }

 cleanup:
  if (pCreateDevEnum) { pCreateDevEnum->Release(); pCreateDevEnum=0; }
  if (pEnumMoniker)   { pEnumMoniker->Release();   pEnumMoniker=0; }
  if (pGraphBuilder)  { pGraphBuilder->Release();  pGraphBuilder=0; }
  if (pCaptureGraphBuilder2) { pCaptureGraphBuilder2->Release(); pCaptureGraphBuilder2=0; }
  if (pMoniker)       { pMoniker->Release();  pMoniker=0; }
  if (pBaseFilter)    { pBaseFilter->Release(); pBaseFilter=0; }
  if (pStreamConfig)  { pStreamConfig->Release(); pStreamConfig=0; }

  for (int i=0; i<(int)list.size(); i++) {
    WebcamVideoDS *obj = list[i];
    _all_webcams.push_back(obj);
  }
}

void find_all_webcams_ds() {
  WebcamVideoDS::init_type();
  WebcamVideoCursorDS::init_type();
  WebcamVideoDS::find_all_webcams_ds();
}

/**
 * Open this video, returning a MovieVideoCursor.
 */
PT(MovieVideoCursor) WebcamVideoDS::
open() {
  return new WebcamVideoCursorDS(this);
}

/**
 *
 */
WebcamVideoCursorDS::
WebcamVideoCursorDS(WebcamVideoDS *src) :
  MovieVideoCursor(src),
  _pGraphBuilder(nullptr),
  _pCaptureBuilder(nullptr),
  _pSrcFilter(nullptr),
  _pStreamConfig(nullptr),
  _pStreamRenderer(nullptr),
  _pMediaCtrl(nullptr)
{
  AM_MEDIA_TYPE mediaType;
  VIDEOINFOHEADER *pVideoInfo;

  HRESULT hResult;

  hResult=CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC,
                           IID_IGraphBuilder,(void**)&_pGraphBuilder);
  if(hResult != S_OK) { cleanup(); return; }

  hResult=CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC,
                           IID_ICaptureGraphBuilder2, (void**)&_pCaptureBuilder);
  if(hResult != S_OK) { cleanup(); return; }

  _pCaptureBuilder->SetFiltergraph(_pGraphBuilder);
  cerr << "  IID_IGraphBuilder & IID_ICaptureGraphBuilder2 are established.\n";

  hResult=_pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&_pMediaCtrl);
  if(hResult != S_OK)
    {  cerr << "  Can not get the IID_IMediaControl interface!";
    cleanup(); return;  }
  cerr << "  IID_IMediaControl interface is acquired.\n";

  src->_moniker->BindToObject(nullptr,nullptr,IID_IBaseFilter, (void**)&_pSrcFilter);
  if(_pSrcFilter == nullptr)
    {  cerr << "  Such capture device is not found.\n";
    cleanup(); return;  }
  cerr << "  The capture filter is acquired.\n";

  hResult=_pGraphBuilder->AddFilter(_pSrcFilter, L"Capture Filter");
  if(hResult != DD_OK)
    {  cerr << "  The capture filter can not be added to the graph.\n";
    cleanup(); return;  }
  cerr << "  The capture filter has been added to the graph.\n";


  hResult = _pCaptureBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, _pSrcFilter,
                                            IID_IAMStreamConfig, (void **)&_pStreamConfig);
  if (hResult != S_OK) {
    cerr << "Could not get stream config interface.\n";
    cleanup(); return;
  }
  hResult = _pStreamConfig->SetFormat(src->_media);
  if (hResult != S_OK) {
    cerr << "Could not select desired video resolution\n";
    cleanup(); return;
  }

  hResult = CoCreateInstance(CLSID_SampleGrabber, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&_pSampleGrabber));
  if(hResult != S_OK)
    {  cerr << "  Can not create the sample grabber, maybe qedit.dll is not registered?";
    cleanup(); return;  }


  // hResult = CoCreateInstance(CLSID_SampleGrabber,) CComQIPtr< IBaseFilter,
  // &IID_IBaseFilter > pGrabberFilter(_pSampleGrabber);
  IBaseFilter *pGrabberFilter = nullptr;
  hResult = _pSampleGrabber->QueryInterface(IID_PPV_ARGS(&pGrabberFilter));
  cerr << "  IID_IBaseFilter of CLSID_SampleGrabber is acquired.\n";

  ZeroMemory(&mediaType, sizeof(AM_MEDIA_TYPE));
  mediaType.majortype=MEDIATYPE_Video;
  mediaType.subtype=MEDIASUBTYPE_RGB24;
  hResult=_pSampleGrabber->SetMediaType(&mediaType);
  if(hResult != S_OK)
    {  cerr << "  Fail to set the media type!";
    cleanup(); return;  }
  cerr << "  The media type of the sample grabber is set 24-bit RGB.\n";

  hResult=_pGraphBuilder->AddFilter(pGrabberFilter, L"Sample Grabber");
  if(hResult != S_OK)
    {  cerr << "  Fail to add the sample grabber to the graph.";
    cleanup(); return;  }
  cerr << "  The sample grabber has been added to the graph.\n";

  // used to give the video stream somewhere to go to.
  hResult = CoCreateInstance(CLSID_NullRenderer, nullptr, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&_pStreamRenderer);
  if(hResult != S_OK)
    {  cerr << "  Can not create the null renderer.";
    cleanup(); return;  }
  cerr << "  IID_IBaseFilter of CLSID_NullRenderer is acquired.\n";

  hResult=_pGraphBuilder->AddFilter(_pStreamRenderer, L"Stream Renderer");
  if(hResult != S_OK)
    {  cerr << "  Fail to add the Null Renderer to the graph.";
    cleanup(); return;  }
  cerr << "  The Null Renderer has been added to the graph.\n";

  hResult=_pCaptureBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
                                         &MEDIATYPE_Video, _pSrcFilter, pGrabberFilter, _pStreamRenderer);
  if(hResult != S_OK) {
    cerr << "  ICaptureGraphBuilder2::RenderStream() can not connect the pins\n";
    cleanup(); return;
  }

  hResult=_pSampleGrabber->GetConnectedMediaType(&mediaType);
  if(hResult != S_OK) {
    cerr << "  Failed to read the connected media type.";
    cleanup(); return;
  }

/*
 * IPin *iPin; hResult = FindInputPin(pGrabberFilter, &iPin); if ((iPin ==
 * 0)||(hResult != S_OK)) { cerr << "Could not get sampler input pin.\n";
 * cleanup(); return; } CComQIPtr< IMemInputPin, &IID_IMemInputPin >
 * pMemInputPin(iPin); if (pMemInputPin == 0) { cerr << "Could not get sampler
 * meminput pin.\n"; cleanup(); return; } hResult =
 * pMemInputPin->GetAllocator(&_pAllocator); if (hResult != S_OK) { cerr <<
 * "Could not get sample grabber allocator handle.\n"; } ALLOCATOR_PROPERTIES
 * props, aprops; hResult = _pAllocator->GetProperties(&props); if (hResult !=
 * S_OK) { cerr << "Could not get allocator properties.\n"; } cerr <<
 * "Allocator properties: cBuffers=" << props.cBuffers << "\n"; props.cBuffers
 * += 10; hResult = _pAllocator->SetProperties(&props, &aprops); if (hResult
 * != S_OK) { cerr << "Could not set allocator properties.\n"; } cerr <<
 * "Allocator properties (adjusted): cBuffers=" << aprops.cBuffers << "\n";
 */

  pVideoInfo=(VIDEOINFOHEADER*)mediaType.pbFormat;
  _size_x = pVideoInfo->bmiHeader.biWidth;
  _size_y = pVideoInfo->bmiHeader.biHeight;
  cerr << "Connected media type " << _size_x << " x " << _size_y << "\n";

  _sample_cb._host = this;
  _num_components = 3;
  _length = 1.0E10;
  _can_seek = false;
  _can_seek_fast = false;
  _aborted = false;
  _streaming = true;
  _buffer = new unsigned char[_size_x * _size_y * 3];
  _ready = false;

  if(mediaType.cbFormat != 0) {
    CoTaskMemFree((PVOID)mediaType.pbFormat);
    mediaType.cbFormat=0;
    mediaType.pbFormat=nullptr;
  }

  if(mediaType.pUnk != nullptr) {
    mediaType.pUnk->Release();
    mediaType.pUnk=nullptr;
  }

  if(pGrabberFilter != nullptr) {
    pGrabberFilter->Release();
    pGrabberFilter=nullptr;
  }

  _pSampleGrabber->SetBufferSamples(FALSE);
  _pSampleGrabber->SetOneShot(FALSE);

  hResult=_pSampleGrabber->SetCallback(&_sample_cb, 0);
  if(hResult != S_OK) {
    cerr << "  Can not set the callback interface!";
    cleanup(); return;
  }

  _pMediaCtrl->Run();
}

/**
 *
 */
void WebcamVideoCursorDS::
cleanup() {
  if (_buffer) {
    delete[] _buffer;
    _buffer = 0;
  }

  if (_pMediaCtrl) {
    _pMediaCtrl->Stop();
  }

  if(_pMediaCtrl)       {  _pMediaCtrl->Release();  _pMediaCtrl=nullptr;  }
  if(_pCaptureBuilder)  {  _pCaptureBuilder->Release();  _pCaptureBuilder=nullptr;  }
  if(_pGraphBuilder)    {  _pGraphBuilder->Release();  _pGraphBuilder=nullptr;  }
  if(_pSampleGrabber)   {  _pSampleGrabber->Release();  _pSampleGrabber=nullptr;  }
  if(_pStreamRenderer)  {  _pStreamRenderer->Release();  _pStreamRenderer=nullptr;  }
  if(_pSrcFilter)       {  _pSrcFilter->Release();  _pSrcFilter=nullptr;  }
  if(_pStreamConfig)    {  _pStreamConfig->Release();  _pStreamConfig=nullptr;  }
}

/**
 *
 */
WebcamVideoCursorDS::
~WebcamVideoCursorDS() {
  cleanup();
}

/**
 *
 */
PT(MovieVideoCursor::Buffer) WebcamVideoCursorDS::
fetch_buffer() {
  if (!_ready) {
    return nullptr;
  }

  Buffer *buffer = get_standard_buffer();
  unsigned char *block = buffer->_block;
#ifdef LOCKING_MODE
  unsigned char *ptr;
  int pixels = _size_x * _size_y;
  HRESULT res = _saved->GetPointer(&ptr);
  if (res == S_OK) {
    int size = _saved->GetActualDataLength();
    if (size == pixels * 3) {
      memcpy(block, ptr, pixels * 3);
    }
  }
  _saved->Release();
#else
  int pixels = _size_x * _size_y;
  memcpy(block, _buffer, pixels * 3);
#endif

  _ready = false;
  return buffer;
}


/**
 *
 */
HRESULT __stdcall WebcamVideoCursorDS::CSampleGrabberCB::QueryInterface(REFIID riid, void **ppv)
{
  if((riid == IID_ISampleGrabberCB) || (riid == IID_IUnknown)) {
    *ppv=(void *)static_cast<ISampleGrabberCB *> (this);
    return NOERROR;
  }
  return E_NOINTERFACE;
}


/**
 *
 */
HRESULT __stdcall WebcamVideoCursorDS::CSampleGrabberCB::SampleCB(double SampleTime, IMediaSample *pSample)
{
  if (_host->_ready) {
    return 0;
  }

#ifdef LOCKING_MODE
  pSample->AddRef();
  _host->_saved = pSample;
#else
  unsigned char *ptr;
  int pixels = _host->_size_x * _host->_size_y;
  HRESULT res = pSample->GetPointer(&ptr);
  if (res == S_OK) {
    int size = pSample->GetActualDataLength();
    if (size == pixels * 3) {
      memcpy(_host->_buffer, ptr, size);
    }
  }
#endif

  _host->_ready = true;
  return 0;
}

/**
 *
 */
HRESULT __stdcall WebcamVideoCursorDS::CSampleGrabberCB::BufferCB(double dblSampleTime, BYTE *pBuffer, long lBufferSize)
{
  // Not used.
  return 0;
}

/*
 * HRESULT FindInputPin(IBaseFilter *pFilter, IPin **ppPin) { if (!pFilter ||
 * ! ppPin) return E_POINTER; *ppPin = 0; HRESULT hr; Find the output pin of
 * the Source Filter IEnumPins *pPinEnum; hr = pFilter->EnumPins(&pPinEnum);
 * if (FAILED(hr)) return E_FAIL; IPin *pSearchPin; while (pPinEnum->Next(1,
 * &pSearchPin, NULL) == S_OK) { PIN_DIRECTION pPinDir; hr =
 * pSearchPin->QueryDirection(&pPinDir); if (FAILED(hr)) return E_FAIL; if
 * (pPinDir == PINDIR_INPUT) { Found out pin *ppPin = pSearchPin; break; } }
 * pPinEnum->Release(); return hr; }
 */


#endif // HAVE_DIRECTSHOW
