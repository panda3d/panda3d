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

////////////////////////// Global Objects .....
TypeHandle osxGraphicsWindow::_type_handle;
osxGraphicsWindow  * osxGraphicsWindow::FullScreenWindow = NULL;


////////////////////////////////////////////////////////////////////
//     Function: GetCurrentOSxWindow
//       Access: Static,
//  Description: How to find the active window for events  on osx..
//
////////////////////////////////////////////////////////////////////
osxGraphicsWindow * osxGraphicsWindow::GetCurrentOSxWindow (WindowRef window)
{
	if(FullScreenWindow != NULL)
	{
//	 cerr << " Full Screen \n";
	   return FullScreenWindow;
   }
//	 cerr << "Not Full Screen \n";

	if (NULL == window)  // HID use this path
		window = FrontWindow ();
		
	if (window)
		return (osxGraphicsWindow *) GetWRefCon (window);
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: aglReportError
//       Access: public
//  Description: Helper function for AGL error message and Grabing error code if any
//
////////////////////////////////////////////////////////////////////
OSStatus aglReportError ( const std::string  &comment)
{
	GLenum err = aglGetError();
	if (err != AGL_NO_ERROR ) 
		 osxdisplay_cat.error()<<"AGL Error " <<  aglErrorString(err) << "[" <<comment <<"]\n";

	if (err == AGL_NO_ERROR)
		return noErr;
	else
		return (OSStatus) err;
}
////////////////////////////////////////////////////////////////////
//     Function: InvertGLImage
//       Access: file scopre, static
//  Description: Helper function invertiung a gl image
//
////////////////////////////////////////////////////////////////////
static void InvertGLImage( char *imageData, size_t imageSize, size_t rowBytes )
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

////////////////////////////////////////////////////////////////////
//     Function: CompositeGLBufferIntoWindow
//       Access: file scopre, static
//  Description: Drop a Gl overlay onto a carbon window.. 
//
////////////////////////////////////////////////////////////////////
static void CompositeGLBufferIntoWindow (AGLContext ctx, Rect *bufferRect, GrafPtr out_port)
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
////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::SystemCloseWindow
//       Access: private
//  Description: The Windows is closed by a OS resource not by a internal request 
//
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::SystemCloseWindow()
{	
 if (osxdisplay_cat.is_debug())	
	osxdisplay_cat.debug() << "System Closing Window \n";
	ReleaseSystemResources();	
};
////////////////////////////////////////////////////////////////////
//     Function: windowEvtHndlr
//       Access: file scope static
//  Description: The C callback for Window Events ..
//
//   We only hook this up for none fullscreen window... so we only handle system window events..
//        
////////////////////////////////////////////////////////////////////
static pascal OSStatus windowEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
#pragma unused (userData)
    OSStatus			result = eventNotHandledErr;
    UInt32 				the_class = GetEventClass (event);
    UInt32 				kind = GetEventKind (event);
 
//	cerr << " In windowEvtHndlr \n";

				
    WindowRef window = NULL;		
	GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(WindowRef), NULL, &window);

	if(window != NULL)
	{
//	if(osx_win->	
	osxGraphicsWindow * osx_win = osxGraphicsWindow::GetCurrentOSxWindow(window);		
 
	switch (the_class) {
		case kEventClassMouse:
			result  = osx_win->handleWindowMouseEvents (myHandler, event);
			break;
	
		case kEventClassWindow:	
		   // WindowRef window = NULL;		
		///	GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(WindowRef), NULL, &window);
			//osxGraphicsWindow * osx_win = osxGraphicsWindow::GetCurrentOSxWindow(window);		 
			switch (kind) {
				case kEventWindowCollapsing:
					//Rect r;
					//GetWindowPortBounds (window, &r);					
					//CompositeGLBufferIntoWindow( osx_win->get_context(), &r, GetWindowPort (window));
					//result = UpdateCollapsedWindowDockTile (window);
					//osx_win->SystemSetWindowForground(false);
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
	}
    return result;
}
///////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::DoResize
//       Access: 
//  Description: The C callback for Window Events ..
//
//   We only hook this up for none fullscreen window... so we only handle system window events..
//        
////////////////////////////////////////////////////////////////////
void     osxGraphicsWindow::DoResize(void)
{

	osxdisplay_cat.debug() << "In Resize Out....." << _properties << "\n";
    // only in window mode .. not full screen
    if(_osx_window != NULL && _is_fullsreen == false && _properties.has_size())
    {
        Rect				rectPort = {0,0,0,0};
        CGRect 				viewRect = {{0.0f, 0.0f}, {0.0f, 0.0f}};

        GetWindowPortBounds (_osx_window, &rectPort); 
        viewRect.size.width = (float) (rectPort.right - rectPort.left);
        viewRect.size.height = (float) (rectPort.bottom - rectPort.top);
        // tell panda
        WindowProperties properties;
        properties.set_size((int)viewRect.size.width,(int)viewRect.size.height);
        properties.set_origin((int) rectPort.left,(int)rectPort.top);
        system_changed_properties(properties);
		
      if (osxdisplay_cat.is_debug())	
         osxdisplay_cat.debug() << " Resizing Window " << viewRect.size.width << " " << viewRect.size.height << "\n";

	//cerr << " Resizing Window " << viewRect.size.width << " " << viewRect.size.height << "\n";

        // ping gl
        aglUpdateContext (aglGetCurrentContext());
        aglReportError("aglUpdateContext .. This is a Resize..");
    }						
};

///////////////////////////////////////////////////////////////////
//     Function: appEvtHndlr
//       Access: 
//  Description: The C callback for APlication Events..
//
//    Hooked  once for application
//        
////////////////////////////////////////////////////////////////////
static pascal OSStatus appEvtHndlr (EventHandlerCallRef myHandler, EventRef event, void* userData)
{
#pragma unused (myHandler)

//	cerr << " In appEvtHandler \n";

    OSStatus result = eventNotHandledErr;
    osxGraphicsWindow *osx_win = NULL;
    WindowRef window = NULL;	
    UInt32 the_class = GetEventClass (event);
    UInt32 kind = GetEventKind (event);

	GetEventParameter(event, kEventParamWindowRef,
                        typeWindowRef, NULL, sizeof(WindowRef),
                        NULL, (void*) &window);

     osx_win = osxGraphicsWindow::GetCurrentOSxWindow(window);

    if(osx_win == NULL)
	  return eventNotHandledErr;
	  
	
	switch (the_class) {
        case kEventClassTextInput:
			  if(kind == kEventTextInputUnicodeForKeyEvent)
				osx_win->handleTextInput(myHandler, event);
			  //result = noErr; 
				//
				// can not report handled .. the os will not sent the raw key strokes then 
				//	if(osx_win->handleTextInput(myHandler, event) == noErr)
				//	      result = noErr;
			  break;
		case kEventClassKeyboard:
			{
			switch (kind) {
                                case kEventRawKeyRepeat:
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
						{			   
							osx_win->HandleModifireDeleta(newModifiers);
							result = noErr; 	
						  }
					}
					break;
			   }
			}
			break;

		case kEventClassMouse:
			//if(osxGraphicsWindow::FullScreenWindow != NULL)	
				result = osx_win->handleWindowMouseEvents (myHandler, event);
 		    //result = noErr; 
			break;
	}
    return result;
}

///////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::handleTextInput
//       Access: 
//  Description:  Trap Unicode  Input.
//
//        
////////////////////////////////////////////////////////////////////
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
			_input_devices[0].keystroke(text[x]);	
		DisposePtr((char *)text);
	}
	
	return ret;
}
///////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::handleTextInput
//       Access: private..
//  Description: Clean up the OS level messes..
////////////////////////////////////////////////////////////////////
 void   osxGraphicsWindow::ReleaseSystemResources()
 {
			
	if(_is_fullsreen)
	{
		_is_fullsreen = false;
		FullScreenWindow = NULL;
		
		
		if(_originalMode != NULL)
			CGDisplaySwitchToMode( kCGDirectMainDisplay, _originalMode );



		CGDisplayRelease( kCGDirectMainDisplay );
		aglSetDrawable (get_ggs_context(), NULL);
		
		_originalMode = NULL;
	}
	
	
	// if the ggs context is assigned to this window
	// clear it..
	if(_osx_window != NULL && GetWindowPort (_osx_window) == (GrafPtr)aglGetDrawable(get_ggs_context()))
		aglSetDrawable (get_ggs_context(),NULL);
	
	// if we are the active gl context clear it..		
	if(aglGetCurrentContext() == get_ggs_context())			
		aglSetCurrentContext (NULL);


 	if(_osx_window != NULL)  
	{
		SetWRefCon (_osx_window, (long int) NULL);
		HideWindow (_osx_window);
		DisposeWindow(_osx_window);
		_osx_window = NULL;
	}
						
	if (_holder_aglcontext)
	 {
		aglDestroyContext (_holder_aglcontext);		
		_holder_aglcontext = NULL;
	}
	

	WindowProperties properties;
    properties.set_foreground(false);
	properties.set_open(false);
	system_changed_properties(properties);
				
	_is_fullsreen = false;	
	_osx_window = NULL;
 }


static int id_seed = 100;
////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsWindow::
osxGraphicsWindow(GraphicsPipe *pipe, 
                  const string &name,
                  const FrameBufferProperties &properties,
                  int x_size, int y_size, int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  GraphicsWindow(pipe, name, properties, x_size, y_size, flags, gsg, host),
  _osx_window(NULL),
  _is_fullsreen(false),
#ifdef HACK_SCREEN_HASH_CONTEXT  
  _holder_aglcontext(NULL),
#endif  
	_originalMode(NULL),
	_ID(id_seed++)
{
  GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard("keyboard/mouse");
  _input_devices.push_back(device);
  _input_devices[0].set_pointer_in_window(0, 0);
  _last_key_modifiers = 0;
  
   if (osxdisplay_cat.is_debug())	
	osxdisplay_cat.debug() << "osxGraphicsWindow::osxGraphicsWindow() -" <<_ID << "\n";

  }

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsWindow::~osxGraphicsWindow()
{
 if (osxdisplay_cat.is_debug())	
	osxdisplay_cat.debug() << "osxGraphicsWindow::~osxGraphicsWindow() -" <<_ID << "\n";

	ReleaseSystemResources();	
}
 

///////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::get_context
//       Access: private..
//  Description:  Helper to Decide whitch context to use if any
////////////////////////////////////////////////////////////////////
AGLContext  osxGraphicsWindow::get_context(void)
{
	if(_holder_aglcontext != NULL)
	  return _holder_aglcontext;

    return get_ggs_context();
 }
///////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::get_ggs_context
//       Access: private..
//  Description:  
////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::buildGL
//       Access: private..
//  Description:  Code of the class.. used to control the GL context Allocation .. 
////////////////////////////////////////////////////////////////////
OSStatus osxGraphicsWindow::buildGL (bool full_screen)
{
	// make sure the ggs is up and runnig..
		osxGraphicsStateGuardian *osxgsg = NULL;
		osxgsg = DCAST(osxGraphicsStateGuardian, _gsg);
		OSStatus stat = osxgsg->buildGL(*this);
		if(stat != noErr)
		   return stat;

	OSStatus err = noErr;
	
//	if(!full_screen)
	{
	

	if (osxgsg->getAGlPixelFormat())
	 {
		_holder_aglcontext = aglCreateContext(osxgsg->getAGlPixelFormat(),NULL);
		
		err  = aglReportError ("aglCreateContext");
		if(_holder_aglcontext == NULL)
		{
			osxdisplay_cat.error() << "osxGraphicsWindow::buildG Error aglCreateContext \n";
			if(err ==noErr)
			  err = -1;				
		}
		else
		{			
			aglSetInteger (_holder_aglcontext, AGL_BUFFER_NAME, &osxgsg->SharedBuffer); 	
			err = aglReportError ("aglSetInteger AGL_BUFFER_NAME");
		
		}
		
	}
	else
	{
			osxdisplay_cat.error() << "osxGraphicsWindow::buildG Error Getting PixelFormat \n"; 	
			if(err ==noErr)
			  err = -1;				
	}
	}
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
bool osxGraphicsWindow::begin_frame(FrameMode mode, Thread *current_thread)
 {
  PStatTimer timer(_make_current_pcollector);
 
//  cerr << " begin_frame [" << _ID << "]\n";
 
  begin_frame_spam();
  if (_gsg == (GraphicsStateGuardian *)NULL || (_osx_window == NULL && _is_fullsreen != true)) 
  {
        // not powered up .. just abort..
		return false;
  }
  
  if(_is_fullsreen)
  {
          aglSetFullScreen(get_ggs_context(),0,0,0,0);
			aglReportError ("aglSetFullScreen");	

		if (!aglSetCurrentContext(get_ggs_context()))
				aglReportError ("aglSetCurrentContext");	
  }
  else
  {
  
		if(FullScreenWindow != NULL)
		    return false;
	
  
		if(!aglSetDrawable (get_ggs_context(),GetWindowPort (_osx_window)))
		{
			osxdisplay_cat.error() << "Error  aglSetDrawable \n";		   
			aglReportError("aglSetDrawable");
		}


		if (!aglSetCurrentContext(get_ggs_context()))
		{
					osxdisplay_cat.error() << "Error in aglSetCurrentContext \n";		   
				aglReportError ("aglSetCurrentContext");
		}	

  }
 
 
	
  _gsg->reset_if_new();
  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::end_frame(FrameMode mode, Thread *current_thread) 
{
  end_frame_spam();
  
  if(mode == FM_render )
  {
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

	aglSwapBuffers (get_ggs_context());
  _gsg->end_frame(current_thread);
  }
//  trigger_flip();
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
void osxGraphicsWindow::end_flip()
{
//  cerr << " end_flip [" << _ID << "]\n";
}
void osxGraphicsWindow::begin_flip() 
{
 // this forces a rip to proper context
 //  cerr << " begin_flip [" << _ID << "]\n";
	return;
 
  if(_is_fullsreen)
	{
	
           aglSetFullScreen(get_ggs_context(),0,0,0,0);
			aglReportError ("aglSetFullScreen");	

		if (!aglSetCurrentContext(get_ggs_context()))
				aglReportError ("aglSetCurrentContext");
					
		aglSwapBuffers (get_ggs_context());
	}
	else
	{
		if(!aglSetDrawable (get_ggs_context(),GetWindowPort (_osx_window)))
		{
			osxdisplay_cat.error() << "Error  aglSetDrawable \n";		   
			aglReportError("aglSetDrawable");
		}


		if (!aglSetCurrentContext(get_ggs_context()))
		{
					osxdisplay_cat.error() << "Error in aglSetCurrentContext \n";		   
				aglReportError ("aglSetCurrentContext");
		}	

		aglSwapBuffers (get_ggs_context());
		}
	
/*	
  if(_is_fullsreen)
	aglSwapBuffers (get_ggs_context());
  else	
	aglSwapBuffers (get_context());
	
	
	aglReportError("aglSwapBuffers");
	*/
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::close_window()
{
  ReleaseSystemResources();
  _gsg.clear();
  _active = false;
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
bool osxGraphicsWindow::open_window()
{
	WindowProperties req_properties  = _properties;
	_properties.clear();
		
	return OSOpenWindow(req_properties);
}


bool osxGraphicsWindow::OSOpenWindow(WindowProperties &req_properties ) 
{
    OSErr err = noErr;

    static bool GlobalInits = false;
    if(GlobalInits != true)
    {
	    //
		// one time aplication inits.. to get a window open from a standalone aplication..
	
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
	    { kEventClassKeyboard, kEventRawKeyRepeat },
            { kEventClassKeyboard, kEventRawKeyModifiersChanged }	,
            {kEventClassTextInput,	kEventTextInputUnicodeForKeyEvent},				   
        };
        EventHandlerUPP gEvtHandler;			// main event handler


        gEvtHandler = NewEventHandlerUPP(appEvtHndlr);
        err = InstallApplicationEventHandler (gEvtHandler, GetEventTypeCount (list1) , list1, this, &ref1 );
        GlobalInits = true;

        struct CPSProcessSerNum PSN;
        GetCurrentProcess((ProcessSerialNumber *)&PSN);
        err = CPSGetCurrentProcess(&PSN);

        if(req_properties.has_title())
        {
            err = CPSSetProcessName(&PSN,(char *)req_properties.get_title().c_str());
			//_properties.set_title(req_properties.get_title());
        }
        else		
            err = CPSSetProcessName(&PSN,"");
			
			

        err = CPSEnableForegroundOperation(&PSN);
        err = CPSSetFrontProcess(&PSN);

    }



    Rect r;
    if(req_properties.has_origin())
    {	
        r.top = req_properties.get_y_origin();
        r.left =req_properties.get_x_origin();
    }
    else
    {
        r.top = 50;
        r.left = 10;
    }

    if(req_properties.has_size())
    {
        r.right = r.left + req_properties.get_x_size();
        r.bottom = r.top + req_properties.get_y_size();
    }
    else
    {			
        r.right = r.left + 512;
        r.bottom = r.top + 512;
    }
			

    if(req_properties.has_fullscreen() && req_properties.get_fullscreen() == true)
    {
//		if(FullScreenWindow != NULL)
//			return false;
	
        // capture the main display
        CGDisplayCapture( kCGDirectMainDisplay );
        // if sized try and switch it..
        if(req_properties.has_size())
        {
            _originalMode = CGDisplayCurrentMode( kCGDirectMainDisplay );	
            CGDisplaySwitchToMode( kCGDirectMainDisplay,
                CGDisplayBestModeForParameters( kCGDirectMainDisplay, 32,  req_properties.get_x_size(), req_properties.get_y_size(), 0 ) );
				
        }

	   
        if(buildGL(true) != noErr)
		{
			if(_originalMode != NULL)
				CGDisplaySwitchToMode( kCGDirectMainDisplay, _originalMode );
			_originalMode = NULL;
		
			CGDisplayRelease( kCGDirectMainDisplay );
			return false;		     
		 }	
//        if (!aglSetCurrentContext(get_context()))
  //          err = aglReportError ();
//        aglSetFullScreen(get_context(),0,0,0,0);
  //      aglReportError ();

        // VBL SYNC
      //  GLint swap = 1;	
      //  if (!aglSetInteger (get_context(), AGL_SWAP_INTERVAL, &swap))
      //      aglReportError ();

		_properties.set_fullscreen(true);

        _is_fullsreen	=true;	
        FullScreenWindow = this;
		req_properties.clear_fullscreen();

    }
    else
    {

		
		// lets use this as a crome based window..
		
	
		if(	!req_properties.has_undecorated() || req_properties.get_undecorated() == false)
		{ // create a window with crome and sizing and sucj
			CreateNewWindow(//
				kDocumentWindowClass,   
				kWindowStandardDocumentAttributes |  kWindowStandardHandlerAttribute, 
				&r, 
				&_osx_window);
		}
		else
		{ // create a unmovable .. no edge window..

			CreateNewWindow(//
				kDocumentWindowClass,   
				kWindowStandardDocumentAttributes |  kWindowNoTitleBarAttribute,
				&r, 
				&_osx_window);
		}

        if (_osx_window)
        {
            EventHandlerUPP gWinEvtHandler;			// window event handler
            //EventHandlerRef		ref;
            EventTypeSpec list[] = { 
			{ kEventClassWindow, kEventWindowCollapsing },
            { kEventClassWindow, kEventWindowShown },
            { kEventClassWindow, kEventWindowActivated },
            { kEventClassWindow, kEventWindowClose },
			{ kEventClassWindow,kEventWindowBoundsChanged },
			
        //    { kEventClassMouse, kEventMouseDown },// handle trackball functionality globaly because there is only a single user
        //    { kEventClassMouse, kEventMouseUp }, 
        //    { kEventClassMouse, kEventMouseMoved },
        //    { kEventClassMouse, kEventMouseDragged },
        //    { kEventClassMouse, kEventMouseWheelMoved } ,
			
			
//            { kEventClassKeyboard, kEventRawKeyDown },
  //          { kEventClassKeyboard, kEventRawKeyUp } ,
    //        { kEventClassKeyboard, kEventRawKeyModifiersChanged }	,
      //      {kEventClassTextInput,	kEventTextInputUnicodeForKeyEvent},				   
			
			
            };

            SetWRefCon (_osx_window, (long) this); // point to the window record in the ref con of the window
            gWinEvtHandler = NewEventHandlerUPP(windowEvtHndlr); 
            InstallWindowEventHandler (_osx_window, gWinEvtHandler, GetEventTypeCount (list), list, (void*)this, NULL); // add event handler
            ShowWindow (_osx_window);

			if(buildGL(false) != noErr)
			{
				osxdisplay_cat.error() << " Errror In Generate GL \n";
				
				HideWindow (_osx_window);
				SetWRefCon (_osx_window, (long int) NULL);
				DisposeWindow(_osx_window);
				_osx_window = NULL;
				return false;
			}

//            GrafPtr portSave = NULL;
  //          GetPort (&portSave);
    //        SetPort ((GrafPtr) GetWindowPort (_osx_window));

			//
			// atach the holder context to the window..
			//
            if(!aglSetDrawable(_holder_aglcontext, GetWindowPort (_osx_window)))
                err = aglReportError ("aglSetDrawable");

//            if (!aglSetCurrentContext(get_context()))
  //              err = aglReportError ("aglSetCurrentContext");

           // VBL SYNC
           // GLint swap = 1;	
           // if (!aglSetInteger (get_context(), AGL_SWAP_INTERVAL, &swap))
		   //   aglReportError ();

      //      SetPort (portSave);		
			if(req_properties.has_fullscreen())
			{
					_properties.set_fullscreen(false);		
				   req_properties.clear_fullscreen();			
			}		
			if(req_properties.has_undecorated())
			{
					_properties.set_undecorated(req_properties.get_undecorated());		
					req_properties.clear_undecorated();
			}
					
        }
    }


    //
    // pull the size from the real window .. do not trust the requested values?f	
  //  WindowProperties properties;

    _properties.set_foreground(true);
    _properties.set_minimized(false);
    _properties.set_open(true);
    Rect				rectPort = {0,0,0,0};
    if(_is_fullsreen)
    {
        CGDirectDisplayID display =   CGMainDisplayID ();

//		if (osxdisplay_cat.is_debug())	
			osxdisplay_cat.debug() << "Full Screen Size ["<< 	CGDisplayPixelsWide (display) <<","<< CGDisplayPixelsHigh (display) << "]\n";
        //	  _properties.set_size((int)800,(int) 600);
        _properties.set_size((int)CGDisplayPixelsWide (display),(int) CGDisplayPixelsHigh (display));
        _properties.set_origin((int) 0,(int)0);
		req_properties.clear_size();
		req_properties.clear_origin();		
    }
    else
    {
        GetWindowPortBounds (_osx_window, &rectPort); 	
        _properties.set_size((int)(rectPort.right - rectPort.left),(int) (rectPort.bottom - rectPort.top));
        _properties.set_origin((int) rectPort.left,(int)rectPort.top);
		req_properties.clear_size();
		req_properties.clear_origin();
    }


//	cerr << " Generate Output Properties "<< _properties <<"\n";
	
    return (err == noErr);
}
////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::process_events()
//       Access: virtual, protected
//  Description: Required Event upcall . Used to dispatch Window and Aplication Events 
//               back into panda
//               
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::process_events()
{
    GraphicsWindow::process_events();
    EventRef theEvent;
    EventTargetRef theTarget;
    theTarget = GetEventDispatcherTarget();

    //    while  (ReceiveNextEvent(0, NULL,kEventDurationForever,true, &theEvent)== noErr)
    while  (ReceiveNextEvent(0, NULL,kEventDurationNoWait,true, &theEvent)== noErr)
    {
//	   static int pass = 0;
//	    cerr << "--------------------------------------------Dispatch Event " << pass++ << "\n";
        SendEventToEventTarget (theEvent, theTarget);
        ReleaseEvent(theEvent);
//		cerr << "------------------------------------Done Dispatch \n";
    }

};

// handle display config changes meaing we need to update the GL context via the resize function and check for windwo dimension changes
// also note we redraw the content here as it could be lost in a display config change
void handleWindowDMEvent (void *userData, short theMessage, void *notifyData)
{

	if (kDMNotifyEvent == theMessage) { // post change notifications only
		osxGraphicsWindow * osxwin = NULL;
		WindowRef window = (WindowRef) userData;
		if (window)
			osxwin = osxGraphicsWindow::GetCurrentOSxWindow (window);
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
////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::process_events()
//       Access: virtual, protected
//  Description: Required Event upcall . Used to dispatch Window and Aplication Events 
//               back into panda
//               
////////////////////////////////////////////////////////////////////
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
			  result = noErr; 
			}
			else
			{
			  SendKeyEvent(OSX_TranslateKey( keyCode , event ),false);
			  result = noErr; 
			}
	}	
			
	return result;
}
 ////////////////////////////////////////////////////////////////////
 //     Function: 
 //       Access: 
 //  Description: 
 ////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::SystemSetWindowForground(bool forground)
{
	  WindowProperties properties;
      properties.set_foreground(forground);
	  system_changed_properties(properties);
};		
 ////////////////////////////////////////////////////////////////////
 //     Function: 
 //       Access: 
 //  Description: 
 ////////////////////////////////////////////////////////////////////
	
 void osxGraphicsWindow::SystemPointToLocalPoint(Point &qdGlobalPoint)
 {
	if(_osx_window != NULL)
	{
		GrafPtr savePort;	
		GetPort( &savePort );
		SetPortWindowPort(_osx_window );
		GlobalToLocal( &qdGlobalPoint );
		SetPort( savePort );
	}
					
 };
	
 ////////////////////////////////////////////////////////////////////
 //     Function: 
 //       Access: 
 //  Description: 
 ////////////////////////////////////////////////////////////////////

 OSStatus osxGraphicsWindow::handleWindowMouseEvents (EventHandlerCallRef myHandler, EventRef event)
 {
     WindowRef			window = NULL;
     OSStatus			result = eventNotHandledErr;
     UInt32 				kind = GetEventKind (event);
     EventMouseButton	button = 0;
     Point qdGlobalPoint = {0, 0};
     UInt32				modifiers = 0;	
     Rect 				rectPort;

//	 cerr <<" Start Mouse Event " << _ID << "\n";

     // Mac OS X v10.1 and later
     // should this be front window???
     GetEventParameter(event, kEventParamWindowRef, typeWindowRef, NULL, sizeof(WindowRef), NULL, &window);
	 if(!_is_fullsreen && (window == NULL || window != _osx_window ))
	     return eventNotHandledErr;
	 
	 
     GetWindowPortBounds (window, &rectPort);

     result = CallNextEventHandler(myHandler, event);	
     if (eventNotHandledErr == result) 
     { // only handle events not already handled (prevents wierd resize interaction)
         switch (kind) {
             // start trackball, pan, or dolly
            case kEventMouseDown:
                {
                    GetEventParameter(event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &button);
                    GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifiers);
                    GetEventParameter(event, kEventParamMouseLocation,typeQDPoint, NULL, sizeof(Point),NULL	, (void*) &qdGlobalPoint);
                    SystemPointToLocalPoint(qdGlobalPoint);

                    ButtonHandle button_h = MouseButton::one();
                    if(kEventMouseButtonSecondary == button)
                        button_h = MouseButton::two();
                    if(kEventMouseButtonTertiary == button)
                        button_h = MouseButton::three();
                    _input_devices[0].set_pointer_in_window((int)qdGlobalPoint.h, (int)qdGlobalPoint.v);
                    _input_devices[0].button_down(button_h);
					result = noErr;
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
                    _input_devices[0].set_pointer_in_window((int)qdGlobalPoint.h, (int)qdGlobalPoint.v);
                    _input_devices[0].button_up(button_h);
					result = noErr;					
                }
                break;
            case kEventMouseMoved:	
            case kEventMouseDragged:
                GetEventParameter(event, kEventParamMouseLocation,typeQDPoint, NULL, sizeof(Point),NULL	, (void*) &qdGlobalPoint);
                SystemPointToLocalPoint(qdGlobalPoint);

                _input_devices[0].set_pointer_in_window((int)qdGlobalPoint.h, (int)qdGlobalPoint.v);
				result = noErr;

                break;
			//long				wheelDelta = 0;		
				
//            case kEventMouseWheelMoved: 
//					result = NoErr;
  //              GetEventParameter(event, kEventParamMouseWheelDelta, typeLongInteger, NULL, sizeof(long), NULL, &wheelDelta);
                break;
         }
//         result = noErr;
     }	
	 
//	 	 cerr <<" End Mouse Event \n";

     return result;
 }

 ////////////////////////////////////////////////////////////////////
 //     Function: osxGraphicsWindow::OSX_TranslateKey
 //       Access: Private
 //  Description: MAC Key Codes to Panda Key Codes
 ////////////////////////////////////////////////////////////////////
 ButtonHandle osxGraphicsWindow::OSX_TranslateKey( UInt32 key,   EventRef event)
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
 ////////////////////////////////////////////////////////////////////
 //     Function: osxGraphicsWindow::HandleModifireDeleta
 //       Access: Private
 //  Description: Used to emulate key events for the MAC key Modifiers..
 ////////////////////////////////////////////////////////////////////
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
 ////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::move_pointer
//       Access: Published, Virtual
//  Description: Forces the pointer to the indicated position within
//               the window, if possible.  
//
//               Returns true if successful, false on failure.  This
//               may fail if the mouse is not currently within the
//               window, or if the API doesn't support this operation.
////////////////////////////////////////////////////////////////////
bool osxGraphicsWindow::move_pointer(int device, int x, int y)
{
	return true;
};

bool osxGraphicsWindow::do_reshape_request(int x_origin, int y_origin, bool has_origin,int x_size, int y_size)
{
	if(_osx_window == NULL)
	  return false;

 if (osxdisplay_cat.is_debug())	
	osxdisplay_cat.debug() << "do_reshape_request " << x_origin <<" "<<  y_origin <<" "<< has_origin<< " " << x_size <<" "<<  y_size <<"\n";


	// location is optional 
	Rect  rect;
	if(has_origin)
	{
		rect.left =  x_origin;
		rect.top = y_origin;
	}
	else
	{
		GetWindowPortBounds (_osx_window, &rect);					
	}
	
	// ok set size .. not oprional
	rect.left = rect.left + x_size;
	rect.bottom = rect.top + y_size;
	
	SetWindowUserState(_osx_window, &rect);
	SetWindowStandardState (_osx_window, &rect );

	return true;
}								



////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsWindow::set_properties_now
//       Access: Public, Virtual
//  Description: Applies the requested set of properties to the
//               window, if possible, for instance to request a change
//               in size or minimization status.
//
//               The window properties are applied immediately, rather
//               than waiting until the next frame.  This implies that
//               this method may *only* be called from within the
//               window thread.
//
//               The properties that have been applied are cleared
//               from the structure by this function; so on return,
//               whatever remains in the properties structure are
//               those that were unchanged for some reason (probably
//               because the underlying interface does not support
//               changing that property on an open window).
////////////////////////////////////////////////////////////////////
void osxGraphicsWindow::set_properties_now(WindowProperties &properties) 
{
	// if (osxdisplay_cat.is_debug())	
//	cerr<< "-------------------------------------set_properties_now in Request=[" <<properties <<"]\n";

	// ok dork with open flag
	//
	
 if (osxdisplay_cat.is_debug())	
 {
	osxdisplay_cat.debug() << "------------------------------------------------------\n";
	osxdisplay_cat.debug() << "set_properties_now " << properties << "\n";
	}
	
	  GraphicsWindow::set_properties_now(properties);
	  
 if (osxdisplay_cat.is_debug())	
	osxdisplay_cat.debug() << "set_properties_now After Base Class" << properties << "\n";
	  
	  
	  
//	  if(_osx_window == NULL)
//	  {
//		cerr<< "-------------------------------------set_properties_now out(not Window)  Request=[" <<properties <<"]\n";
//		return;
//	  }
	
	// open is weird..
	// looks like it is a bad thing to muck
	//
	/*
	if(properties.has_open())
	{
	  printf(" properties Has Open Flag \n");
//	    _properties.set_open(properties.get_open());
		properties.clear_open();		
	}
	*/	
	

 // this will set the main title on the crome of the window
 //
 
 // for some changes .. a full rebuild is required for the OS layer Window.
 //    I think it is the crome atribute and full screen behaviour.
 bool need_full_rebuild = false;
 
 // if we are not full and transitioning o full
  if (properties.has_fullscreen() &&   properties.get_fullscreen() != _properties.get_fullscreen()) 
  {
		need_full_rebuild = true;
  }
  
  
  
  
  
  if(need_full_rebuild)
  {
	// Login here is ..  tage a union of the properties .. with the new  allowed to overwrite the old states.
	// and start a bootstrap of a new window ..
    // get a copy of my properties..
	WindowProperties req_properties(_properties); 
	cerr<< "-------------------------------------Lets Go Full Rebuild   Request=[" <<properties <<"]\n";
	ReleaseSystemResources();
	_properties.clear();	
	req_properties.add_properties(properties);	
	// put back in the request bucket..
	properties = req_properties;
	
	OSOpenWindow(properties); 
  }

 
 
 
  if(properties.has_title())
  {
		_properties.set_title(properties.get_title());
		if(_osx_window)
			SetWindowTitleWithCFString(_osx_window,CFStringCreateWithCString(NULL,properties.get_title().c_str(),kCFStringEncodingMacRoman));
		properties.clear_title();
  }
  // decorated .. if this changes it reqires a new window
  if(properties.has_undecorated())
  {
		_properties.set_undecorated(properties.get_undecorated());
		properties.clear_undecorated();
  }
  
  // cursor managment..
  if(properties.has_cursor_hidden())
  {
		_properties.set_cursor_hidden(properties.get_cursor_hidden());
		properties.clear_cursor_hidden();
  }
	//
	// icons
	if(properties.has_icon_filename())
	{
		_properties.set_icon_filename(properties.get_icon_filename());
		properties.clear_icon_filename();
	}
	
	if(properties.has_cursor_filename())
	{
		_properties.set_cursor_filename(properties.get_cursor_filename());
		properties.clear_cursor_filename();
	}	


	if(properties.has_minimized())
	{
		_properties.set_minimized(properties.get_minimized());
		properties.clear_minimized();	
	}



	if(properties.has_foreground())
	{
		_properties.set_foreground(properties.get_foreground());
		properties.clear_foreground();	
	
	}

/*
	if(properties.has_size())
	{
		_properties.set_size(properties.get_size());
		properties.clear_size();
		
		do_reshape_request(_properties.
	};
*/


	if(properties.has_open())
	{
	   
//	  cerr << " properties Has Open Flag \n";
		properties.clear_open();		
	}


    
 // cerr<< "-------------------------------------- out request=[" <<properties <<"]\n";
   if (osxdisplay_cat.is_debug())	
	osxdisplay_cat.debug() << "set_properties_now  Out....." << _properties << "\n";

	return;
}


