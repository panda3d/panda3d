#----------------------------------------------------------------------
# Name:        wx.lib.wxcairo
# Purpose:     Glue code to allow the pycairo package to be used
#              with a wx.DC as the cairo surface.
#              
# Author:      Robin Dunn
#
# Created:     3-Sept-2008
# RCS-ID:      $Id: wxcairo.py 67480 2011-04-13 18:27:18Z RD $
# Copyright:   (c) 2008 by Total Control Software
# Licence:     wxWindows license
#----------------------------------------------------------------------

"""
This module provides some glue code that allows the pycairo package to
be used for drawing direclty on wx.DCs.  In cairo terms, the DC is the
drawing surface.  The `CairoContextFromDC` function in this module
will return an instance of the pycairo Context class that is ready for
drawing, using the native cairo surface type for the current platform.

This module requires the pycairo pacakge, and makes use of ctypes for 
fetching the pycairo C API and also for digging into the cairo library 
itself.

To use Cairo with wxPython you will need to have a few dependencies
installed.  On Linux and other unix-like systems you may already have
them, or can easily get them with your system's package manager.  Just
check if libcairo and pycairo are installed.

On Mac you can get Cairo from MacPorts or Fink.  If you are also using
MacPorts or Fink for your Python installation then you should be able
to get pycairo the same way.  Otherwise it's real easy to build and
install pycairo for the Python framework installation.  Just get the
source tarball from http://pypi.python.org/pypi/pycairo and do the
normal 'python setup.py install' dance.

On Windows you can get a Cairo DLL from here:

    http://www.gtk.org/download-windows.html

You'll also want to get the zlib and libpng binaries from the same
page.  Once you get those files extract the DLLs from each of the zip
files and copy them to some place on your PATH.  Finally, there is an
installer for the pycairo pacakge here:

    http://wxpython.org/cairo/

"""

# TODO:  Support printer surfaces?


import wx
import cairo
import ctypes
import ctypes.util


#----------------------------------------------------------------------------

# A reference to the cairo shared lib via ctypes.CDLL
cairoLib = None  

# A reference to the pycairo C API structure
pycairoAPI = None


# a convenience funtion, just to save a bit of typing below
def voidp(ptr):
    """Convert a SWIGged void* type to a ctypes c_void_p"""
    return ctypes.c_void_p(int(ptr))

#----------------------------------------------------------------------------

def ContextFromDC(dc):
    """
    Creates and returns a Cairo context object using the wxDC as the
    surface.  (Only window, client, paint and memory DC's are allowed
    at this time.)
    """
    if not isinstance(dc, wx.WindowDC) and not isinstance(dc, wx.MemoryDC):
        raise TypeError, "Only window and memory DC's are supported at this time."
    
    if 'wxMac' in wx.PlatformInfo:
        width, height = dc.GetSize()

        # use the CGContextRef of the DC to make the cairo surface
        cgc = dc.GetCGContext()
        assert cgc is not None, "Unable to get CGContext from DC."
        cgref = voidp( cgc )
        surfaceptr = voidp(
            cairoLib.cairo_quartz_surface_create_for_cg_context(
                cgref, width, height) )

        # create a cairo context for that surface
        ctxptr = cairoLib.cairo_create(surfaceptr)

        # Turn it into a pycairo context object
        ctx = pycairoAPI.Context_FromContext(ctxptr, pycairoAPI.Context_Type, None)

        # The context keeps its own reference to the surface
        cairoLib.cairo_surface_destroy(surfaceptr)

        
    elif 'wxMSW' in wx.PlatformInfo:
        # This one is easy, just fetch the HDC and use PyCairo to make
        # the surface and context.
        hdc = dc.GetHDC()
        surface = cairo.Win32Surface(hdc)
        ctx = cairo.Context(surface)
    

    elif 'wxGTK' in wx.PlatformInfo:
        gdkLib = _findGDKLib()

        # Get the GdkDrawable from the dc
        drawable = voidp( dc.GetGdkDrawable() )

        # Call a GDK API to create a cairo context
        gdkLib.gdk_cairo_create.restype = ctypes.c_void_p
        ctxptr = gdkLib.gdk_cairo_create(drawable)

        # Turn it into a pycairo context object
        ctx = pycairoAPI.Context_FromContext(ctxptr, pycairoAPI.Context_Type, None)
    
    else:
        raise NotImplementedError, "Help  me, I'm lost..."

    return ctx 


#----------------------------------------------------------------------------

def FontFaceFromFont(font):
    """
    Creates and returns a cairo.FontFace object from the native
    information in a wx.Font.
    """
    
    if 'wxMac' in wx.PlatformInfo:
        # NOTE: This currently uses the ATSUFontID, but wxMac may
        # someday transition to the CGFont.  If so, this API call will
        # need to be changed.
        fontfaceptr = voidp(
            cairoLib.cairo_quartz_font_face_create_for_atsu_font_id(
                font.MacGetATSUFontID()) )
        fontface = pycairoAPI.FontFace_FromFontFace(fontfaceptr)


    elif 'wxMSW' in wx.PlatformInfo:
        fontfaceptr = voidp( cairoLib.cairo_win32_font_face_create_for_hfont(
            ctypes.c_ulong(font.GetHFONT())) )
        fontface = pycairoAPI.FontFace_FromFontFace(fontfaceptr)


    elif 'wxGTK' in wx.PlatformInfo:
        gdkLib = _findGDKLib()
        pcLib = _findPangoCairoLib()

        # wow, this is a hell of a lot of steps...
        desc = voidp( font.GetPangoFontDescription() )

        pcLib.pango_cairo_font_map_get_default.restype = ctypes.c_void_p
        pcfm = voidp(pcLib.pango_cairo_font_map_get_default())

        gdkLib.gdk_pango_context_get.restype = ctypes.c_void_p
        pctx = voidp(gdkLib.gdk_pango_context_get())

        pcLib.pango_font_map_load_font.restype = ctypes.c_void_p
        pfnt = voidp( pcLib.pango_font_map_load_font(pcfm, pctx, desc) )

        pcLib.pango_cairo_font_get_scaled_font.restype = ctypes.c_void_p
        scaledfontptr = voidp( pcLib.pango_cairo_font_get_scaled_font(pfnt) )

        cairoLib.cairo_scaled_font_get_font_face.restype = ctypes.c_void_p
        fontfaceptr = voidp(cairoLib.cairo_scaled_font_get_font_face(scaledfontptr))
        cairoLib.cairo_font_face_reference(fontfaceptr)

        fontface = pycairoAPI.FontFace_FromFontFace(fontfaceptr)

        gdkLib.g_object_unref(pctx)

    else:
        raise NotImplementedError, "Help  me, I'm lost..."

    return fontface


#----------------------------------------------------------------------------
# wxBitmap <--> ImageSurface

def BitmapFromImageSurface(surface):
    """
    Create a wx.Bitmap from a Cairo ImageSurface.
    """
    format = surface.get_format()
    if format not in [cairo.FORMAT_ARGB32, cairo.FORMAT_RGB24]:
        raise TypeError("Unsupported format")
    
    width  = surface.get_width()
    height = surface.get_height()
    stride = surface.get_stride()
    data   = surface.get_data()
    if format == cairo.FORMAT_ARGB32:
        fmt = wx.BitmapBufferFormat_ARGB32
    else:
        fmt = wx.BitmapBufferFormat_RGB32

    bmp = wx.EmptyBitmap(width, height, 32)
    bmp.CopyFromBuffer(data, fmt, stride)
    return bmp


def ImageSurfaceFromBitmap(bitmap):
    """
    Create an ImageSurface from a wx.Bitmap
    """
    width, height = bitmap.GetSize()
    if bitmap.HasAlpha():
        format = cairo.FORMAT_ARGB32
        fmt = wx.BitmapBufferFormat_ARGB32
    else:
        format = cairo.FORMAT_RGB24
        fmt = wx.BitmapBufferFormat_RGB32

    try:
        stride = cairo.ImageSurface.format_stride_for_width(format, width)
    except AttributeError:
        stride = width * 4
    
    surface = cairo.ImageSurface(format, width, height)
    bitmap.CopyToBuffer(surface.get_data(), fmt, stride)
    return surface


#----------------------------------------------------------------------------
# Only implementation helpers after this point
#----------------------------------------------------------------------------

def _findCairoLib():
    """
    Try to locate the Cairo shared library and make a CDLL for it.
    """
    global cairoLib
    if cairoLib is not None:
        return

    names = ['cairo', 'cairo-2', 'libcairo', 'libcairo-2']

    # first look using just the base name
    for name in names:
        try:
            cairoLib = ctypes.CDLL(name)
            return
        except:
            pass

    # if that didn't work then use the ctypes util to search the paths
    # appropriate for the system
    for name in names:
        location = ctypes.util.find_library(name) 
        try:
            cairoLib = ctypes.CDLL(location)
            return
        except:
            pass
        
    # If the above didn't find it on OS X then we still have a
    # trick up our sleeve...
    if 'wxMac' in wx.PlatformInfo:
        # look at the libs linked to by the pycairo extension module
        import macholib.MachO
        m = macholib.MachO.MachO(cairo._cairo.__file__)
        for h in m.headers:
            for idx, name, path in h.walkRelocatables():
                if 'libcairo' in path:
                    try:
                        cairoLib = ctypes.CDLL(path)
                        return
                    except:
                        pass

    if not cairoLib:
        raise RuntimeError, "Unable to find the Cairo shared library"
    
            

#----------------------------------------------------------------------------

# For other DLLs we'll just use a dictionary to track them, as there
# probably isn't any need to use them outside of this module.
_dlls = dict()

def _findHelper(names, key, msg):
    dll = _dlls.get(key, None)
    if dll is not None:
        return dll
    location = None
    for name in names:
        location = ctypes.util.find_library(name)
        if location:
            break
    if not location:
        raise RuntimeError, msg
    dll = ctypes.CDLL(location)
    _dlls[key] = dll
    return dll


def _findGDKLib():
    return _findHelper(['gdk-x11-2.0'], 'gdk',
                       "Unable to find the GDK shared library")

def _findPangoCairoLib():
    return _findHelper(['pangocairo-1.0'], 'pangocairo',
                       "Unable to find the pangocairo shared library")
def _findAppSvcLib():
    return _findHelper(['ApplicationServices'], 'appsvc',
                       "Unable to find the ApplicationServices Framework")
    

#----------------------------------------------------------------------------

# Pycairo exports a C API in a structure via a PyCObject.  Using
# ctypes will let us use that API from Python too.  We'll use it to
# convert a C pointer value to pycairo objects.  The information about
# this API structure is gleaned from pycairo.h.

class Pycairo_CAPI(ctypes.Structure):
    if cairo.version_info < (1,8):  # This structure is known good with pycairo 1.6.4
        _fields_ = [
            ('Context_Type', ctypes.py_object),
            ('Context_FromContext', ctypes.PYFUNCTYPE(ctypes.py_object,
                                                      ctypes.c_void_p,
                                                      ctypes.py_object,
                                                      ctypes.py_object)),
            ('FontFace_Type', ctypes.py_object),
            ('FontFace_FromFontFace', ctypes.PYFUNCTYPE(ctypes.py_object, ctypes.c_void_p)),
            ('FontOptions_Type', ctypes.py_object),
            ('FontOptions_FromFontOptions', ctypes.PYFUNCTYPE(ctypes.py_object, ctypes.c_void_p)),
            ('Matrix_Type', ctypes.py_object),
            ('Matrix_FromMatrix', ctypes.PYFUNCTYPE(ctypes.py_object, ctypes.c_void_p)),
            ('Path_Type', ctypes.py_object),
            ('Path_FromPath', ctypes.PYFUNCTYPE(ctypes.py_object, ctypes.c_void_p)),
            ('Pattern_Type', ctypes.py_object),
            ('SolidPattern_Type', ctypes.py_object),
            ('SurfacePattern_Type', ctypes.py_object),
            ('Gradient_Type', ctypes.py_object),
            ('LinearGradient_Type', ctypes.py_object),
            ('RadialGradient_Type', ctypes.py_object),
            ('Pattern_FromPattern', ctypes.PYFUNCTYPE(ctypes.py_object, ctypes.c_void_p)),
            ('ScaledFont_Type', ctypes.py_object),
            ('ScaledFont_FromScaledFont', ctypes.PYFUNCTYPE(ctypes.py_object, ctypes.c_void_p)),
            ('Surface_Type', ctypes.py_object),
            ('ImageSurface_Type', ctypes.py_object),
            ('PDFSurface_Type', ctypes.py_object),
            ('PSSurface_Type', ctypes.py_object),
            ('SVGSurface_Type', ctypes.py_object),
            ('Win32Surface_Type', ctypes.py_object),
            ('XlibSurface_Type', ctypes.py_object),
            ('Surface_FromSurface', ctypes.PYFUNCTYPE(ctypes.py_object,
                                                      ctypes.c_void_p,
                                                      ctypes.py_object)),
            ('Check_Status', ctypes.PYFUNCTYPE(ctypes.c_int, ctypes.c_int))]

    # This structure is known good with pycairo 1.8.4.
    # We have to also test for (1,10,8) because pycairo 1.8.10 has an
    # incorrect version_info value
    elif cairo.version_info < (1,9) or cairo.version_info == (1,10,8):  
        _fields_ = [
            ('Context_Type', ctypes.py_object),
            ('Context_FromContext', ctypes.PYFUNCTYPE(ctypes.py_object,
                                                      ctypes.c_void_p,
                                                      ctypes.py_object,
                                                      ctypes.py_object)),
            ('FontFace_Type', ctypes.py_object),
            ('ToyFontFace_Type', ctypes.py_object),  #** new in 1.8.4
            ('FontFace_FromFontFace', ctypes.PYFUNCTYPE(ctypes.py_object, ctypes.c_void_p)),
            ('FontOptions_Type', ctypes.py_object),
            ('FontOptions_FromFontOptions', ctypes.PYFUNCTYPE(ctypes.py_object, ctypes.c_void_p)),
            ('Matrix_Type', ctypes.py_object),
            ('Matrix_FromMatrix', ctypes.PYFUNCTYPE(ctypes.py_object, ctypes.c_void_p)),
            ('Path_Type', ctypes.py_object),
            ('Path_FromPath', ctypes.PYFUNCTYPE(ctypes.py_object, ctypes.c_void_p)),
            ('Pattern_Type', ctypes.py_object),
            ('SolidPattern_Type', ctypes.py_object),
            ('SurfacePattern_Type', ctypes.py_object),
            ('Gradient_Type', ctypes.py_object),
            ('LinearGradient_Type', ctypes.py_object),
            ('RadialGradient_Type', ctypes.py_object),
            ('Pattern_FromPattern', ctypes.PYFUNCTYPE(ctypes.py_object, ctypes.c_void_p,
                                                      ctypes.py_object)), #** changed in 1.8.4
            ('ScaledFont_Type', ctypes.py_object),
            ('ScaledFont_FromScaledFont', ctypes.PYFUNCTYPE(ctypes.py_object, ctypes.c_void_p)),
            ('Surface_Type', ctypes.py_object),
            ('ImageSurface_Type', ctypes.py_object),
            ('PDFSurface_Type', ctypes.py_object),
            ('PSSurface_Type', ctypes.py_object),
            ('SVGSurface_Type', ctypes.py_object),
            ('Win32Surface_Type', ctypes.py_object),
            ('XlibSurface_Type', ctypes.py_object),
            ('Surface_FromSurface', ctypes.PYFUNCTYPE(ctypes.py_object,
                                                      ctypes.c_void_p,
                                                      ctypes.py_object)),
            ('Check_Status', ctypes.PYFUNCTYPE(ctypes.c_int, ctypes.c_int))]


def _loadPycairoAPI():
    global pycairoAPI
    if pycairoAPI is not None:
        return
    
    PyCObject_AsVoidPtr = ctypes.pythonapi.PyCObject_AsVoidPtr
    PyCObject_AsVoidPtr.argtypes = [ctypes.py_object]
    PyCObject_AsVoidPtr.restype = ctypes.c_void_p
    ptr = PyCObject_AsVoidPtr(cairo.CAPI)
    pycairoAPI = ctypes.cast(ptr, ctypes.POINTER(Pycairo_CAPI)).contents

#----------------------------------------------------------------------------

# Load these at import time.  That seems a bit better than doing it at
# first use...
_findCairoLib()
_loadPycairoAPI()

#----------------------------------------------------------------------------
