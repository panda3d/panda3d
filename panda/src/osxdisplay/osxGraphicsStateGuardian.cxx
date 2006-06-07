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

#include "osxGraphicsStateGuardian.h"
#include "osxGraphicsBuffer.h"
#include "string_utils.h"
#include "config_osxdisplay.h"

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#import <mach-o/dyld.h>

TypeHandle osxGraphicsStateGuardian::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::get_extension_func
//       Access: Public, Virtual
//  Description: Returns the pointer to the GL extension function with
//               the indicated name.  It is the responsibility of the
//               caller to ensure that the required extension is
//               defined in the OpenGL runtime prior to calling this;
//               it is an error to call this for a function that is
//               not defined.
////////////////////////////////////////////////////////////////////
void *osxGraphicsStateGuardian::get_extension_func(const char *prefix, const char *name) 
{	
	string fullname = "_" + string(prefix) + string(name);
    NSSymbol symbol = NULL;
    
    if (NSIsSymbolNameDefined (fullname.c_str()))
        symbol = NSLookupAndBindSymbol (fullname.c_str());

    if (osxdisplay_cat.is_debug())	
	{		
		osxdisplay_cat.debug() << "  Looking Up Symbol " << fullname <<" \n" ;
	}
		
    return symbol ? NSAddressOfSymbol (symbol) : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsStateGuardian::
osxGraphicsStateGuardian(GraphicsPipe *pipe,
                         osxGraphicsStateGuardian *share_with) :
  GLGraphicsStateGuardian(properties),
  _share_with(share_with),
  _aglPixFmt(NULL),
  _aglcontext(NULL)
{
  SharedBuffer = 1011;
  cerr << "osxGraphicsStateGuardian::osxGraphicsStateGuardian()\n";
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsStateGuardian::~osxGraphicsStateGuardian() 
{
	cerr << "osxGraphicsStateGuardian::~osxGraphicsStateGuardian()\n";

  if(_aglcontext != (AGLContext)NULL)
  {
     aglDestroyContext(_aglcontext);
	 aglReportError("osxGraphicsStateGuardian::~osxGraphicsStateGuardian()  aglDestroyContext");
	 _aglcontext = (AGLContext)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.
////////////////////////////////////////////////////////////////////
void osxGraphicsStateGuardian::reset() 
{
/*
  if(_aglcontext != (AGLContext)NULL)
  {
     aglDestroyContext(_aglcontext);
	 aglReportError();
	 _aglcontext = (AGLContext)NULL;
  }
  */

  GLGraphicsStateGuardian::reset();
 }

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsStateGuardian::buildGL
//       Access: Public, Virtual
//  Description: This function will build up a context for a gsg..  
//   rhh..  This does not respect the flags passed into it for context type ?? hmmm things to do things to do..
//
////////////////////////////////////////////////////////////////////
OSStatus osxGraphicsStateGuardian::buildGL (osxGraphicsWindow  &window)
{
	OSStatus err = noErr;
//	GLint attrib[] = { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_DEPTH_SIZE, 16, AGL_NONE };
//	GLint attrib[] = { AGL_RGBA, AGL_NO_RECOVERY, AGL_FULLSCREEN, AGL_DOUBLEBUFFER, AGL_DEPTH_SIZE, 32, AGL_SAMPLE_BUFFERS_ARB, 1, AGL_SAMPLES_ARB, 0, 0 };
// 	GLint attrib[] = { AGL_RGBA, AGL_NO_RECOVERY,  AGL_DOUBLEBUFFER, AGL_NONE, 0, 0 };   
//	GLint attrib[] = { AGL_RGBA, AGL_DOUBLEBUFFER,AGL_FULLSCREEN,AGL_ACCELERATED,AGL_DEPTH_SIZE, 32, AGL_NONE };	
	GLint attrib[] = { AGL_RGBA, AGL_DOUBLEBUFFER,AGL_NO_RECOVERY,AGL_FULLSCREEN,AGL_DEPTH_SIZE, 32, AGL_NONE };	
	if (_aglcontext)
		return noErr; // already built
	
	GDHandle display = GetMainDevice ();		
	// build context
	_aglcontext = NULL;
	_aglPixFmt = aglChoosePixelFormat(&display, 1, attrib);
	err = aglReportError ("aglChoosePixelFormat");
	if (_aglPixFmt)
	 {
	  if(_share_with == NULL)
		_aglcontext = aglCreateContext(_aglPixFmt, NULL);
	  else
		_aglcontext = aglCreateContext(_aglPixFmt, ((osxGraphicsStateGuardian *)_share_with)->_aglcontext);
	  err = aglReportError ("aglCreateContext");

	if (_aglcontext == NULL)
	{
		osxdisplay_cat.error() << "osxGraphicsStateGuardian::buildG Error Getting Gl Context \n" ;
		if(err == noErr)
		   err = -1;
	}
	else
	{
			aglSetInteger (_aglcontext, AGL_BUFFER_NAME, &SharedBuffer); 	
			err = aglReportError ("aglSetInteger AGL_BUFFER_NAME");			
	}
	
	}
	else
	{
		osxdisplay_cat.error() << "osxGraphicsStateGuardian::buildG Error Getting Pixel Format  \n" ;
		if(err == noErr)
		   err = -1;
	
	}
	
	osxdisplay_cat.debug() << "osxGraphicsStateGuardian::buildGL Returning :" << err << "\n"; 
	
    return err;
}

