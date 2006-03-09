////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
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
#include <Carbon/Carbon.h>

#include "osxGraphicsWindow.h"
#include "config_osxdisplay.h"
#include "osxGraphicsPipe.h"
#include "pStatTimer.h"
#include "glgsg.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "osxGraphicsStateGuardian.h"

#include <OpenGL/gl.h>
#include <AGL/agl.h>
#include <ApplicationServices/ApplicationServices.h>

TypeHandle osxGraphicsWindow::_type_handle;

ButtonHandle OSX_TranslateKey( UInt32 key,  EventRef event );

EventHandlerUPP gEvtHandler;			// main event handler
EventHandlerUPP gWinEvtHandler;			// window event handler
//AbsoluteTime gStartTime;

char gErrorMessage[256] = ""; // buffer for error message output
float gErrorTime = 0.0;

static osxGraphicsWindow  * FullScreenWindow = NULL;

osxGraphicsWindow * GetCurrentOSxWindow (WindowRef window)
{
	if(FullScreenWindow != NULL)
	   return FullScreenWindow;

	if (NULL == window)  // HID use this path
		window = FrontWindow ();
	if (window)
		return (osxGraphicsWindow *) GetWRefCon (window);
	else
		return NULL;
}



OSStatus aglReportError (void)
{
	GLenum err = aglGetError();
	if (AGL_NO_ERROR != err) 
		 osxdisplay_cat.error()<<"AGL Error " <<  aglErrorString(err) << "\n";

	if (err == AGL_NO_ERROR)
		return noErr;
	else
		return (OSStatus) err;
}

void InvertGLImage( char *imageData, size_t imageSize, size_t rowBytes )
{
	size_t i, j;
	char *tBuffer = (char*) malloc (rowBytes);
	if (NULL == tBuffer) return;
		
	// Copy by rows through temp buffer
	for (i = 0, j = imageSize - rowBytes; i < imageSize >> 1; i += rowBytes, j -= rowBytes) {
		memcpy( tBuffer, &imageData[i], rowBytes );
		memcpy( &imageData[i], &imageData[j], rowBytes );
		memcpy( &imageData[j], tBuffer, rowBytes );
	}
	free(tBuffer);
}


void CompositeGLBufferIntoWindow (AGLContext ctx, Rect *bufferRect, GrafPtr out_port)
{
	GWorldPtr pGWorld;
	QDErr err;
	// blit OpenGL content into window backing store
	// allocate buffer to hold pane image
	long width  = (bufferRect->right - bufferRect->left);
	long height  = (bufferRect->bottom - bufferRect->top);
	
	
	
	Rect src_rect = {0, 0, height, width};
	Rect ddrc_rect = {0, 0, height, width};
	long rowBytes = width * 4;
	long imageSize = rowBytes * height;
	char *image = (char *) NewPtr (imageSize);
	if (!image) {
		osxdisplay_cat.error() << "Out of memory in CompositeGLBufferIntoWindow()!\n";
		return;		// no harm in continuing
	}
	
//	printf(" Reading aa Conte Data %d %d\n",height,width);
	
	// pull GL content down to our image buffer
	aglSetCurrentContext( ctx );
	glReadPixels (0, 0, width, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image);

	// GL buffers are upside-down relative to QD buffers, so we need to flip it
	InvertGLImage( image, imageSize, rowBytes );

	// create a GWorld containing our image
	err = NewGWorldFromPtr (&pGWorld, k32ARGBPixelFormat, &src_rect, 0, 0, 0, image, rowBytes);
	if (err != noErr) {
		osxdisplay_cat.error() << " error in NewGWorldFromPtr, called from CompositeGLBufferIntoWindow()\n";
		free( image );
		return;
	}
        GrafPtr portSave = NULL;
        GetPort (&portSave);
	
	//SetPort( GetWindowPort (win));
	SetPort(out_port);
	CopyBits( GetPortBitMapForCopyBits (pGWorld), GetPortBitMapForCopyBits (out_port), &src_rect, &ddrc_rect, srcCopy, 0 );
	
	SetPort(portSave);
	DisposeGWorld( pGWorld );
	DisposePtr ( image );
}

void osxGraphicsWindow::SystemCloseWindow()
{	
	osxdisplay_cat.debug() << "System Closing Window \n";

	ReleaseSystemResources();
	
};

// window event handler
static pascal OSStatus windowEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
#pragma unused (userData)
    OSStatus			result = eventNotHandledErr;
    UInt32 				the_class = GetEventClass (event);
    UInt32 				kind = GetEventKind (event);
 
	switch (the_class) {
		case kEventClassWindow:
		
		    WindowRef window = NULL;		
			GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(WindowRef), NULL, &window);
			osxGraphicsWindow * osx_win = GetCurrentOSxWindow(window);
			if(osx_win == NULL)
				return result;
			 
			switch (kind) {
				case kEventWindowCollapsing:
					Rect r;
					GetWindowPortBounds (window, &r);					
					CompositeGLBufferIntoWindow( osx_win->get_context(), &r, GetWindowPort (window));
					result = UpdateCollapsedWindowDockTile (window);
					osx_win->SystemSetWindowForground(false);
					break;
				case kEventWindowActivated: // called on click activation and initially
						osx_win->SystemSetWindowForground(true);
						osx_win->DoResize();
					break;
				case kEventWindowClose: // called when window is being closed (close box)
						osx_win->SystemCloseWindow();
					break;
				case kEventWindowShown: // called on initial show (not on un-minimize)
					if (window == FrontWindow ())
						SetUserFocusWindow (window);
					break;
				case kEventWindowBoundsChanged: // called for resize and moves (drag)
						osx_win->DoResize();
					break;
				case kEventWindowZoomed: // called when user clicks on zoom button (occurs after the window has been zoomed)
					// use this if you need to some special here as you always get a kEventWindowBoundsChanged event
					break;
			}
			break;
	}
    return result;
}
	
void     osxGraphicsWindow::DoResize(void)
{
    if(_osx_window != NULL)
	{
	Rect				rectPort = {0,0,0,0};
	CGRect 				viewRect = {{0.0f, 0.0f}, {0.0f, 0.0f}};
	
	GetWindowPortBounds (_osx_window, &rectPort); 
	viewRect.size.width = (float) (rectPort.right - rectPort.left);
	viewRect.size.height = (float) (rectPort.bottom - rectPort.top);
	
	  WindowProperties properties;
	  properties.set_size((int)viewRect.size.width,(int)viewRect.size.height);
	  properties.set_origin((int) rectPort.left,(int)rectPort.top);
	  system_changed_properties(properties);
	  osxdisplay_cat.debug() << " Resizing Window " << viewRect.size.width << " " << viewRect.size.height << "\n";
	  
	  aglUpdateContext (aglGetCurrentContext());
	  aglReportError();
	}						
  
	
	//printf(" Setting Window Size to %d %d \n",(int)viewRect.size.width,(int)viewRect.size.height);
};

// application level event handler
static pascal OSStatus appEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
#pragma unused (myHandler)
    OSStatus result = eventNotHandledErr;
    osxGraphicsWindow *osx_win = NULL;
    WindowRef window = NULL;	
    UInt32 the_class = GetEventClass (event);
    UInt32 kind = GetEventKind (event);

	GetEventParameter(event, kEventParamWindowRef,
                        typeWindowRef, NULL, sizeof(WindowRef),
                        NULL, (void*) &window);

     osx_win = GetCurrentOSxWindow(window);

    if(osx_win == NULL)
	  return eventNotHandledErr;
	  
	
	switch (the_class) {
        case kEventClassTextInput:
			  if(kind == kEventTextInputUnicodeForKeyEvent)
				osx_win->handleTextInput(myHandler, event);
				//
				// can not report handled .. the os will not sent the raw key strokes then 
				//	if(osx_win->handleTextInput(myHandler, event) == noErr)
				//	      result = noErr;
			  break;
		case kEventClassKeyboard:
			switch (kind) {
			   printf(" Key Event \n");
				case kEventRawKeyDown:
					result = osx_win->handleKeyInput  (myHandler, event, true);
					break;
				case kEventRawKeyUp:
					result = osx_win->handleKeyInput  (myHandler, event, false);
					break;
				case kEventRawKeyModifiersChanged:
					{
						UInt32 newModifiers;
						OSStatus error = GetEventParameter(event, kEventParamKeyModifiers,typeUInt32, NULL,sizeof(UInt32), NULL, &newModifiers);
						if(error == noErr)						   
							osx_win->HandleModifireDeleta(newModifiers);
					}
					break;
			}
			break;

		case kEventClassMouse:
			osx_win->handleWindowMouseEvents (myHandler, event);
			break;
	}
    return result;
}


OSStatus osxGraphicsWindow::handleTextInput (EventHandlerCallRef myHandler, EventRef theTextEvent)
{
	UniChar      *text = NULL;
	UInt32       actualSize = 0; 
 
	OSStatus ret = GetEventParameter (theTextEvent, kEventParamTextInputSendText, typeUnicodeText, NULL, 0, &actualSize, NULL);
	if(ret != noErr)
	   return ret;
 
	text = (UniChar*) NewPtr(actualSize);
	if(text!= NULL)
	{
 
		ret = GetEventParameter (theTextEvent, kEventParamTextInputSendText,typeUnicodeText, NULL, actualSize, NULL, text);
		if(ret != noErr)
			return ret;

		for(unsigned int x = 0; x < actualSize/sizeof(UniChar); ++x)
		{
			printf(" Push KetStroke %x\n",(int)text[x]);
			_input_devices[0].keystroke(text[x]);	
		}
		DisposePtr((char *)text);
	
	}
	
	return ret;
}

 void   osxGraphicsWindow::ReleaseSystemResources()
 {
		
	if(_is_fullsreen)
	{
		if(_originalMode != NULL)
			CGDisplaySwitchToMode( kCGDirectMainDisplay, _originalMode );

		CGDisplayRelease( kCGDirectMainDisplay );
		aglSetDrawable (get_context(), NULL);
		_is_fullsreen = false;
		FullScreenWindow = NULL;
		_originalMode = NULL;
	}
	
	
	if(_osx_window != NULL && GetWindowPort (_osx_window) == (GrafPtr)aglGetDrawable(get_context()))
		aglSetDrawable (get_context(),NULL);
	
			
	if(aglGetCurrentContext() == get_context())			
		aglSetCurrentContext (NULL);


 	if(_osx_window != NULL)  
	{
		HideWindow (_osx_window);
		SetWRefCon (_osx_window, (long int) NULL);
		DisposeWindow(_osx_window);
		_osx_window = NULL;
	}
		

				
#ifdef HACK_SCREEN_HASH_CONTEXT  		 
	if (_aglcontext)
	 {
		aglDestroyContext (_aglcontext);		
		_aglcontext = NULL;
	}
#endif 		
	
	WindowProperties properties;
    properties.set_foreground(false);
	properties.set_open(false);
	system_changed_properties(properties);
				
	_is_fullsreen = false;	
	_osx_window = NULL;
 }


////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsWindow::osxGraphicsWindow(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                  const string &name) :
  GraphicsWindow(pipe, gsg, name) ,
  _osx_window(NULL),
  _is_fullsreen(false),
#ifdef HACK_SCREEN_HASH_CONTEXT  
   _aglPixFmt(NULL),
  _aglcontext(NULL),
#endif  
	_originalMode(NULL)
{
    printf("Create osxGraphicsWindow \n");

  GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard("keyboard/mouse");
  _input_devices.push_back(device);
  _input_devices[0].set_pointer_in_window(0, 0);
  _last_key_modifiers = 0;
  
  }

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsWindow::~osxGraphicsWindow()
{

	ReleaseSystemResources();
	

  cerr << " osxGraphicsWindow::~osxGraphicsWindow() \n";
}

void osxGraphicsWindow::make_current()
{

}

AGLContext  osxGraphicsWindow::get_context(void)
{
	if(_aglcontext != NULL)
	  return _aglcontext;

    return get_ggs_context();
 }

AGLContext  osxGraphicsWindow::get_ggs_context(void)
{
	if(_gsg != NULL)
	{
		osxGraphicsStateGuardian *osxgsg = NULL;
		osxgsg = DCAST(osxGraphicsStateGuardian, _gsg);	
		return osxgsg->get_context();
	}
	return NULL;
 }


OSStatus osxGraphicsWindow::buildGL (void)
{
	// make sure the ggs is up and runnig..
		osxGraphicsStateGuardian *osxgsg = NULL;
		osxgsg = DCAST(osxGraphicsStateGuardian, _gsg);
		osxgsg->buildGL(*this);

	OSStatus err = noErr;
	
#ifdef HACK_SCREEN_HASH_CONTEXT  
	
//	GLint attrib[] = { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_DEPTH_SIZE, 16, AGL_NONE,AGL_NONE,AGL_NONE,AGL_NONE };
	GLint attrib[] = { AGL_RGBA, AGL_NO_RECOVERY,  AGL_DOUBLEBUFFER, AGL_DEPTH_SIZE, 32, AGL_SAMPLE_BUFFERS_ARB, 1, AGL_SAMPLES_ARB, 0, 0 };
  
	if(_properties.has_fullscreen() && _properties.get_fullscreen() == true)
	{		
		short i = 0;
		while (attrib[i++] != AGL_NONE) {}
		i--; // point to AGL_NONE
		attrib [i++] = AGL_FULLSCREEN;
		attrib [i++] = AGL_NONE;		
		
		printf(" Adding Full Screen To Pixel Format \n");
	}
	
				
	if (_aglcontext)
		return noErr; // already built
		
	// build context
	_aglcontext = NULL;
	_aglPixFmt = aglChoosePixelFormat(NULL, 0, attrib);
	aglReportError ();
	if (_aglPixFmt)
	 {
		_aglcontext = aglCreateContext(_aglPixFmt, get_ggs_context());
		aglReportError ();
	}
	
#endif  // HACK_SCREEN_HASH_CONTEXT
    return err;
}





////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool osxGraphicsWindow::begin_frame(FrameMode mode) {
  PStatTimer timer(_make_current_pcollector);
   // printf("Do begin frame \n");

  begin_frame_spam();
  if (_gsg == (GraphicsStateGuardian *)NULL || (_osx_window == NULL && _is_fullsreen != true)) 
  {
		return false;
  }


  if(_is_fullsreen)
  {
		if (!aglSetCurrentContext(get_context()))
				aglReportError ();	
  
//	  printf(" In Full Screen begin_frame\n");
  
  }
  else
  {
	GrafPtr OtherWin = (GrafPtr)aglGetDrawable(get_context());
	aglReportError();
	WindowPtr other  = GetWindowFromPort(OtherWin);
		
	if(OtherWin != NULL && OtherWin != (GrafPtr)GetWindowPort (_osx_window))
	{
		Rect r;
		GetWindowPortBounds (other, &r);
	 //   printf(" Doint Composite %d  %d\n",(int)OtherWin,(int)GetWindowPort (_osx_window));
		CompositeGLBufferIntoWindow(get_context(),& r,OtherWin);
	}

		aglSetDrawable (get_context(),GetWindowPort (_osx_window));
		aglReportError();
		
		
		if (!aglSetCurrentContext(get_context()))
				aglReportError ();	
  	
  }
 
 
	
_gsg->reset_if_new();

  return _gsg->begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::end_frame(FrameMode mode) 
{
  end_frame_spam();

  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  _gsg->end_frame();
  trigger_flip();
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::release_gsg
//       Access: Public, Virtual
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  The window will be
//               permanently unable to render; this is normally called
//               only just before destroying the window.  This should
//               only be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::release_gsg() 
{
	ReleaseSystemResources();
		
  GraphicsWindow::release_gsg();
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::begin_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after end_frame() has been called on all windows, to
//               initiate the exchange of the front and back buffers.
//
//               This should instruct the window to prepare for the
//               flip at the next video sync, but it should not wait.
//
//               We have the two separate functions, begin_flip() and
//               end_flip(), to make it easier to flip all of the
//               windows at the same time.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::begin_flip() 
{
	aglSwapBuffers (get_context());
	aglReportError();
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::close_window() {

	ReleaseSystemResources();
	GraphicsWindow::close_window();

}

//////////////////////////////////////////////////////////
// HACK ALLERT ************ Undocumented OSX calls...
// I can not find any other way to get the mouse focus to a window in OSX..
//
extern "C" {
 struct  CPSProcessSerNum
{
UInt32 lo;
UInt32 hi;
};

extern OSErr CPSGetCurrentProcess(CPSProcessSerNum *psn);
extern OSErr CPSEnableForegroundOperation( struct CPSProcessSerNum *psn);
extern OSErr CPSSetProcessName ( struct CPSProcessSerNum *psn, char *processname);
extern OSErr CPSSetFrontProcess( struct CPSProcessSerNum *psn);
};

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool osxGraphicsWindow::open_window() {
OSErr err;
	printf(" In Open Window \n");
	
	
	static bool GlobalInits = false;
	if(GlobalInits != true)
	{
		EventHandlerRef	ref1;
		EventTypeSpec	list1[] = { 
					           //{ kEventClassCommand,  kEventProcessCommand },
							   //{ kEventClassCommand,  kEventCommandUpdateStatus },
							   { kEventClassMouse, kEventMouseDown },// handle trackball functionality globaly because there is only a single user
							   { kEventClassMouse, kEventMouseUp }, 
							   { kEventClassMouse, kEventMouseMoved },
							   { kEventClassMouse, kEventMouseDragged },
							   { kEventClassMouse, kEventMouseWheelMoved } ,
                               { kEventClassKeyboard, kEventRawKeyDown },
                               { kEventClassKeyboard, kEventRawKeyUp } ,
	  		 				   { kEventClassKeyboard, kEventRawKeyModifiersChanged }	,
							   {kEventClassTextInput,	kEventTextInputUnicodeForKeyEvent},				   
							   };


		gEvtHandler = NewEventHandlerUPP(appEvtHndlr);
		err = InstallApplicationEventHandler (gEvtHandler, GetEventTypeCount (list1) , list1, this, &ref1 );
		GlobalInits = true;
		
		struct CPSProcessSerNum PSN;
		GetCurrentProcess((ProcessSerialNumber *)&PSN);
		err = CPSGetCurrentProcess(&PSN);
		
		if(_properties.has_title())
		{
			err = CPSSetProcessName(&PSN,(char *)_properties.get_title().c_str());
		}
		else		
			err = CPSSetProcessName(&PSN,"Panda3D");
		
		err = CPSEnableForegroundOperation(&PSN);
		err = CPSSetFrontProcess(&PSN);
		
		
		
	}

  

    EventHandlerRef		ref;
    EventTypeSpec list[] = { { kEventClassWindow, kEventWindowCollapsing },
							 { kEventClassWindow, kEventWindowShown },
                             { kEventClassWindow, kEventWindowActivated },
                             { kEventClassWindow, kEventWindowClose },
                             { kEventClassWindow, kEventWindowBoundsChanged },
                             { kEventClassWindow, kEventWindowZoomed },
    //                         { kEventClassKeyboard, kEventRawKeyDown },
      //                       { kEventClassKeyboard, kEventRawKeyUp } ,
		//					 { kEventClassKeyboard, kEventRawKeyModifiersChanged }
							 };


	Rect r;
	if(_properties.has_origin())
	{	
		r.top = _properties.get_y_origin();
		r.left =_properties.get_x_origin();
	}
	else
	{
	     r.top = 50;
		 r.left = 10;
	}
		
	if(_properties.has_size())
	{
		r.right = r.left + _properties.get_x_size();
		r.bottom = r.top + _properties.get_y_size();
	}
	else
	{			
		r.right = r.left + 512;
		r.bottom = r.top + 512;
	}		

if(_properties.has_fullscreen() && _properties.get_fullscreen() == true)
{
	// capture the main display
	CGDisplayCapture( kCGDirectMainDisplay );
	// if sized try and switch it..
	if(_properties.has_size())
	{
		_originalMode = CGDisplayCurrentMode( kCGDirectMainDisplay );	
		CGDisplaySwitchToMode( kCGDirectMainDisplay,
                CGDisplayBestModeForParameters( kCGDirectMainDisplay, 32,  _properties.get_x_size(), _properties.get_y_size(), 0 ) );
	}

	buildGL();	
		if (!aglSetCurrentContext(get_context()))
			err = aglReportError ();
	aglSetFullScreen(get_context(),0,0,0,0);
	aglReportError ();
		
		// VBL SYNC
		GLint swap = 1;	
		if (!aglSetInteger (get_context(), AGL_SWAP_INTERVAL, &swap))
			aglReportError ();
	
	
 
	
// CreateNewWindow(//
// kOverlayWindowClass,
//	kWindowStandardHandlerAttribute, 
//	&r, &_osx_window); 
	_is_fullsreen	=true;	
	FullScreenWindow = this;
}
else
{

 CreateNewWindow(//
// kUtilityWindowClass,
	kDocumentWindowClass,   
//	kWindowLiveResizeAttribute |
	kWindowStandardDocumentAttributes |  
	kWindowStandardHandlerAttribute, 
	&r, &_osx_window);


	if (_osx_window)
	{
	
		SetWRefCon (_osx_window, (long) this); // point to the window record in the ref con of the window
		gWinEvtHandler = NewEventHandlerUPP(windowEvtHndlr); 
		InstallWindowEventHandler (_osx_window, gWinEvtHandler, GetEventTypeCount (list), list, (void*)this, &ref); // add event handler
		ShowWindow (_osx_window);
			
		buildGL();	

        GrafPtr portSave = NULL;
        GetPort (&portSave);
        SetPort ((GrafPtr) GetWindowPort (_osx_window));

		if(!aglSetDrawable(get_context(), GetWindowPort (_osx_window)))
			err = aglReportError ();
			
		if (!aglSetCurrentContext(get_context()))
			err = aglReportError ();
		
		// VBL SYNC
		GLint swap = 1;	
		if (!aglSetInteger (get_context(), AGL_SWAP_INTERVAL, &swap))
			aglReportError ();

        SetPort (portSave);		
	}
}

//RunApplicationEventLoop();
   WindowProperties properties;
   
   _properties.set_foreground(true);
   _properties.set_minimized(false);
   _properties.set_open(true);
   	Rect				rectPort = {0,0,0,0};
	if(_is_fullsreen)
	{
		CGDirectDisplayID display =   CGMainDisplayID ();

		osxdisplay_cat.debug() << "Full Screen Size ["<< 	CGDisplayPixelsWide (display) <<","<< CGDisplayPixelsHigh (display) << "\n";
//	  _properties.set_size((int)800,(int) 600);
	_properties.set_size((int)CGDisplayPixelsWide (display),(int) CGDisplayPixelsHigh (display));
	  _properties.set_origin((int) 0,(int)0);
	}
	else
	{
		GetWindowPortBounds (_osx_window, &rectPort); 	
	  _properties.set_size((int)(rectPort.right - rectPort.left),(int) (rectPort.bottom - rectPort.top));
	  _properties.set_origin((int) rectPort.left,(int)rectPort.top);
	
	}
	return true;
}

void osxGraphicsWindow::process_events()
{
	GraphicsWindow::process_events();
	EventRef theEvent;
	EventTargetRef theTarget;
	theTarget = GetEventDispatcherTarget();
 
//    while  (ReceiveNextEvent(0, NULL,kEventDurationForever,true, &theEvent)== noErr)
    while  (ReceiveNextEvent(0, NULL,kEventDurationNoWait,true, &theEvent)== noErr)
        {
            SendEventToEventTarget (theEvent, theTarget);
            ReleaseEvent(theEvent);
        }

};
// ---------------------------------

// handle display config changes meaing we need to update the GL context via the resize function and check for windwo dimension changes
// also note we redraw the content here as it could be lost in a display config change
void handleWindowDMEvent (void *userData, short theMessage, void *notifyData)
{

	if (kDMNotifyEvent == theMessage) { // post change notifications only
		osxGraphicsWindow * osxwin = NULL;
		WindowRef window = (WindowRef) userData;
		if (window)
			osxwin = GetCurrentOSxWindow (window);
		if (osxwin) { // have a valid OpenGl window
			Rect rectPort;
			CGRect viewRect = {{0.0f, 0.0f}, {0.0f, 0.0f}};
			GetWindowPortBounds (window, &rectPort);
			viewRect.size.width = (float) (rectPort.right - rectPort.left);
			viewRect.size.height = (float) (rectPort.bottom - rectPort.top);
			//resizeGL (pContextInfo->aglContext, &pContextInfo->camera, pContextInfo->shapeSize, viewRect);
			InvalWindowRect (window,  &rectPort); // force redrow
		}
	}
}
/*
	

EventRef     theTextEvent;
UniChar      *text;
UInt32       actualSize; 
 
GetEventParameter (theTextEvent, kEventParamTextInputSendText,
                typeUnicodeText, NULL, 0, &actualSize, NULL);
 
text = (UniChar*) NewPtr(actualSize);
 
 
GetEventParameter (theTextEvent, kEventParamTextInputSendText,
                typeUnicodeText, NULL, actualSize, NULL, text);

*/

// key input handler
OSStatus osxGraphicsWindow::handleKeyInput (EventHandlerCallRef myHandler, EventRef event, Boolean keyDown)
{
	OSStatus result = eventNotHandledErr;

	result = CallNextEventHandler(myHandler, event);	
	if (eventNotHandledErr == result) 
	{ 
			UInt32 newModifiers = 0;
			OSStatus error = GetEventParameter(event, kEventParamKeyModifiers,typeUInt32, NULL,sizeof(UInt32), NULL, &newModifiers);
			if(error == noErr)						   
				HandleModifireDeleta(newModifiers);
	
			UInt32 keyCode;
			GetEventParameter (event, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &keyCode);
			if(keyDown)
			{
			  SendKeyEvent(OSX_TranslateKey( keyCode , event ),true);
			}
			else
			{
			  SendKeyEvent(OSX_TranslateKey( keyCode , event ),false);
			   
			}
	}	
			
	return result;
}


void osxGraphicsWindow::SystemSetWindowForground(bool forground)
{
	  WindowProperties properties;
      properties.set_foreground(forground);
	  system_changed_properties(properties);
};		
	
 void osxGraphicsWindow::SystemPointToLocalPoint(Point &qdGlobalPoint)
 {
    GrafPtr savePort;
    GetPort( &savePort );
    SetPortWindowPort(_osx_window );
    GlobalToLocal( &qdGlobalPoint );
    SetPort( savePort );				
 };
	
	
OSStatus osxGraphicsWindow::handleWindowMouseEvents (EventHandlerCallRef myHandler, EventRef event)
{
    WindowRef			window = NULL;
 //   pRecContext 		pContextInfo = NULL;
	OSStatus			result = eventNotHandledErr;
    UInt32 				kind = GetEventKind (event);
	EventMouseButton	button = 0;
//	HIPoint				location = {0.0f, 0.0f};
	Point qdGlobalPoint = {0, 0};
	UInt32				modifiers = 0;	
	long				wheelDelta = 0;		
	Rect 				rectPort;

	// Mac OS X v10.1 and later
	// should this be front window???
	GetEventParameter(event, kEventParamWindowRef, typeWindowRef, NULL, sizeof(WindowRef), NULL, &window);
//	if (window)
//		pContextInfo = GetCurrentContextInfo (window);
//	if (!pContextInfo)
//		return result; // not an application GLWindow so do not process (there is an exception)
	GetWindowPortBounds (window, &rectPort);
		
	//printf(" Got Mouse Event \n");	
		
	result = CallNextEventHandler(myHandler, event);	
	if (eventNotHandledErr == result) 
	{ // only handle events not already handled (prevents wierd resize interaction)
		switch (kind) {
			// start trackball, pan, or dolly
			case kEventMouseDown:
			{
				GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &button);
//				GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &location);	// Mac OS X v10.1 and later
				GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifiers);
				GetEventParameter(event, kEventParamMouseLocation,typeQDPoint, NULL, sizeof(Point),NULL	, (void*) &qdGlobalPoint);
				SystemPointToLocalPoint(qdGlobalPoint);
    
				ButtonHandle button_h = MouseButton::one();
				if(kEventMouseButtonSecondary == button)
					button_h = MouseButton::two();
				if(kEventMouseButtonTertiary == button)
					button_h = MouseButton::three();
				 
//					cerr << " Mouse Down "	 <<  location.x << " " << location.y << " "<<  button_h << "\n" ;
//					cerr << " Mouse Down "	 <<  qdGlobalPoint.h << " " << qdGlobalPoint.v << " "<<  button_h << "\n" ;		
//					cerr << " Window Port " << 	rectPort.top << " " << rectPort.bottom ;
//				    _input_devices[0].set_pointer_in_window((int)location.x, (int)location.y);
				    _input_devices[0].set_pointer_in_window((int)qdGlobalPoint.h, (int)qdGlobalPoint.v);
 	 		 	   _input_devices[0].button_down(button_h);
			}
				break;
			// stop trackball, pan, or dolly
			case kEventMouseUp:
				{
				GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &button);
//				GetEventParameter(event, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &location);	// Mac OS X v10.1 and later
				GetEventParameter(event, kEventParamMouseLocation,typeQDPoint, NULL, sizeof(Point),NULL	, (void*) &qdGlobalPoint);
				SystemPointToLocalPoint(qdGlobalPoint);
				
				ButtonHandle button_h = MouseButton::one();
				if(kEventMouseButtonSecondary == button)
					button_h = MouseButton::two();
				if(kEventMouseButtonTertiary == button)
					button_h = MouseButton::three();
					
//					cerr << " Mouse Up "	 <<  location.x << " " << location.y << " "<< button_h << "\n";
//					cerr << " Mouse up "	 <<  qdGlobalPoint.h << " " << qdGlobalPoint.v << " "<<  button_h << "\n" ;

				    _input_devices[0].set_pointer_in_window((int)qdGlobalPoint.h, (int)qdGlobalPoint.v);
				   _input_devices[0].button_up(button_h);
				}
				break;
			case kEventMouseMoved:	
			case kEventMouseDragged:
//				GetEventParameter(event, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &location);	// Mac OS X v10.1 and later
//				GetEventParameter(event, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &location);	// Mac OS X v10.1 and later
//					_input_devices[0].set_pointer_in_window((int)location.x, (int)location.y);
//				 _input_devices[0].set_pointer_in_window(event.xmotion.x, event.xmotion.y);
				GetEventParameter(event, kEventParamMouseLocation,typeQDPoint, NULL, sizeof(Point),NULL	, (void*) &qdGlobalPoint);
				SystemPointToLocalPoint(qdGlobalPoint);

			    _input_devices[0].set_pointer_in_window((int)qdGlobalPoint.h, (int)qdGlobalPoint.v);
				 
				break;
			case kEventMouseWheelMoved: 
				GetEventParameter(event, kEventParamMouseWheelDelta, typeLongInteger, NULL, sizeof(long), NULL, &wheelDelta);
				break;
		}
		result = noErr;
	}	
	return result;
}


ButtonHandle OSX_TranslateKey( UInt32 key,   EventRef event)
{


	ButtonHandle nk = ButtonHandle::none();
	switch ( key )
	{
	case 0:    nk = KeyboardButton::ascii_key('a');       break;
	case 11:   nk = KeyboardButton::ascii_key('b');       break;
	case 8:    nk = KeyboardButton::ascii_key('c');       break;
	case 2:    nk = KeyboardButton::ascii_key('d');       break;
	case 14:   nk = KeyboardButton::ascii_key('e');       break;
	case 3:    nk = KeyboardButton::ascii_key('f');       break;
	case 5:    nk = KeyboardButton::ascii_key('g');       break;
	case 4:    nk = KeyboardButton::ascii_key('h');       break;
	case 34:   nk = KeyboardButton::ascii_key('i');       break;
	case 38:   nk = KeyboardButton::ascii_key('j');       break;
	case 40:   nk = KeyboardButton::ascii_key('k');       break;
	case 37:   nk = KeyboardButton::ascii_key('l');       break;
	case 46:   nk = KeyboardButton::ascii_key('m');       break;
	case 45:   nk = KeyboardButton::ascii_key('n');       break;
	case 31:   nk = KeyboardButton::ascii_key('o');       break;
	case 35:   nk = KeyboardButton::ascii_key('p');       break;
	case 12:   nk = KeyboardButton::ascii_key('q');       break;
	case 15:   nk = KeyboardButton::ascii_key('r');       break;
	case 1:    nk = KeyboardButton::ascii_key('s');       break;
	case 17:   nk = KeyboardButton::ascii_key('t');       break;
	case 32:   nk = KeyboardButton::ascii_key('u');       break;
	case 9:    nk = KeyboardButton::ascii_key('v');       break;
	case 13:   nk = KeyboardButton::ascii_key('w');       break;
	case 7:    nk = KeyboardButton::ascii_key('x');       break;
	case 16:   nk = KeyboardButton::ascii_key('y');       break;
	case 6:    nk = KeyboardButton::ascii_key('z');       break;

	// top row numbers
	case 29:   nk = KeyboardButton::ascii_key('0');       break;
	case 18:   nk = KeyboardButton::ascii_key('1');       break;
	case 19:   nk = KeyboardButton::ascii_key('2');       break;
	case 20:   nk = KeyboardButton::ascii_key('3');       break;
	case 21:   nk = KeyboardButton::ascii_key('4');       break;
	case 23:   nk = KeyboardButton::ascii_key('5');       break;
	case 22:   nk = KeyboardButton::ascii_key('6');       break;
	case 26:   nk = KeyboardButton::ascii_key('7');       break;
	case 28:   nk = KeyboardButton::ascii_key('8');       break;
	case 25:   nk = KeyboardButton::ascii_key('9');       break;
	
    // key pad ... do they really map to the top number in panda ?
	case 82:   nk = KeyboardButton::ascii_key('0');       break;
	case 83:   nk = KeyboardButton::ascii_key('1');       break;
	case 84:   nk = KeyboardButton::ascii_key('2');       break;
	case 85:   nk = KeyboardButton::ascii_key('3');       break;
	case 86:   nk = KeyboardButton::ascii_key('4');       break;
	case 87:   nk = KeyboardButton::ascii_key('5');       break;
	case 88:   nk = KeyboardButton::ascii_key('6');       break;
	case 89:   nk = KeyboardButton::ascii_key('7');       break;
	case 91:   nk = KeyboardButton::ascii_key('8');       break;
	case 92:   nk = KeyboardButton::ascii_key('9');       break;


//	case 36:   nk = KeyboardButton::ret();			  break;   // no return  in panda ???
	case 49:   nk = KeyboardButton::space();		 	  break;
	case 51:   nk = KeyboardButton::backspace();		  break;
	case 48:   nk = KeyboardButton::tab();				  break;
	case 53:   nk = KeyboardButton::escape();			  break;
	case 76:   nk = KeyboardButton::enter();			  break;	
	case 36:   nk = KeyboardButton::enter();			  break;	
	
	case 123:  nk = KeyboardButton::left();				  break;
	case 124:  nk = KeyboardButton::right();			  break;
	case 125:  nk = KeyboardButton::down();				  break;
	case 126:  nk = KeyboardButton::up();				  break;
	case 116:  nk = KeyboardButton::page_up();				  break;
	case 121:  nk = KeyboardButton::page_down();				  break;
	case 115:  nk = KeyboardButton::home();				  break;
	case 119:  nk = KeyboardButton::end();				  break;
//	case    :  nk = KeyboardButton::insert();			  break;			
	case 117:  nk = KeyboardButton::del();			  break;			
	
//	case  71:  nk = KeyboardButton::num_lock()        break; 

	case 122:  nk = KeyboardButton::f1();				  break;
	case 120:  nk = KeyboardButton::f2();				  break;
	case  99:  nk = KeyboardButton::f3();				  break;
	case 118:  nk = KeyboardButton::f4();				  break;
	case  96:  nk = KeyboardButton::f5();				  break;
	case  97:  nk = KeyboardButton::f6();				  break;
	case  98:  nk = KeyboardButton::f7();				  break;
	case 100:  nk = KeyboardButton::f8();				  break;
//	case    :  nk = KeyboardButton::f9();				  break;  // seem to be used by the systems..
//	case    :  nk = KeyboardButton::f10();				  break;
//	case    :  nk = KeyboardButton::f11();				  break;
//	case    :  nk = KeyboardButton::f12();				  break;
//	case 105:  nk = KeyboardButton::f13();				  break;  // panda does not have a 13
	
	// shiftable chartablet 
	case  50:  nk = KeyboardButton::ascii_key('`');				  break;
	case  27:  nk = KeyboardButton::ascii_key('-');				  break;
	case  24:  nk = KeyboardButton::ascii_key('=');				  break;
	case  33:  nk = KeyboardButton::ascii_key('[');				  break;
	case  30:  nk = KeyboardButton::ascii_key(']');				  break;
	case  42:  nk = KeyboardButton::ascii_key('\\');			  break;
	case  41:  nk = KeyboardButton::ascii_key(';');				  break;
	case  39:  nk = KeyboardButton::ascii_key('\'');			  break;
	case  43:  nk = KeyboardButton::ascii_key(',');				  break;
	case  47:  nk = KeyboardButton::ascii_key('.');				  break;
	case  44:  nk = KeyboardButton::ascii_key('/');				  break;
	
	default:
//		 printf (" Untranslated KeyCode: %lu (0x%lX)\n", key, key);
			// not sure this is right .. but no mapping for keypad and such
			// this at least does a best gess..
			
        char charCode =  0;	
        if(GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, nil, sizeof( charCode ), nil, &charCode ) == noErr)
			nk = KeyboardButton::ascii_key(charCode);	
	}
	return nk;
}


 void     osxGraphicsWindow::HandleModifireDeleta(UInt32 newModifiers)
 {
	 UInt32 changed = _last_key_modifiers ^ newModifiers;
	 
	
    if ((changed & (shiftKey | rightShiftKey)) != 0)
		SendKeyEvent(KeyboardButton::shift(),(newModifiers & (shiftKey | rightShiftKey)) != 0) ;
		
    if ((changed & (optionKey | rightOptionKey)) != 0) 
		SendKeyEvent(KeyboardButton::alt(),(newModifiers & (optionKey | rightOptionKey)) != 0);

	
	if ((changed & (controlKey | rightControlKey)) != 0) 
		SendKeyEvent(KeyboardButton::control(),(newModifiers & (controlKey | rightControlKey)) != 0);
	
    if ((changed & alphaLock) != 0) 
		SendKeyEvent(KeyboardButton::caps_lock(),(newModifiers & alphaLock) != 0);
	
	// save current state
    _last_key_modifiers = newModifiers;						   
								
 };

