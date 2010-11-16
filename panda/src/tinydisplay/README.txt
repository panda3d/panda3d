The code in this directory was lifted from TinySDGL, which was itself
modified from TinyGL, a project originally written by Fabrice Bellard.
The README and LICENSE files that accompany that project are included
here.

We have modified the original TinyGL code to interface more directly
with the Panda3D constructs, eliminating the thin OpenGL interface
layer.  We have also added features we deemed necessary, such as
texture modulate mode and alpha blending.


-------------------------------------------------------------------
Original TinySDGL/TinyGL LICENSE

Copyright notice:

 (C) 1997-1998 Fabrice Bellard

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product and its documentation 
     *is* required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

If you redistribute modified sources, I would appreciate that you
include in the files history information documenting your changes.


-------------------------------------------------------------------
Original TinySDGL/TinyGL README

TinyGL 0.4 (c) 1997-2002 Fabrice Bellard.
TinySDGL 0.5, 2005 a port to libSDL by Gerald Franz (gfz@o2online.de).

General Description:
--------------------

TinyGL is intended to be a very small implementation of a subset of
OpenGL* for embedded systems or games. It is a software only
implementation. Only the main OpenGL calls are implemented. All the
calls I considered not important are simply *not implemented*.

The main strength of TinyGL is that it is fast and simple because it
has not to be exactly compatible with OpenGL. In particular, the
texture mapping and the geometrical transformations are very fast.

The main features of TinyGL are:

- Header compatible with OpenGL (the headers are adapted from the very good
Mesa by Brian Paul et al.)

- Zlib-like licence for easy integration in commercial designs (read
the LICENCE file).

- Examples that show the integration into the platform-independent 
  libSDL (http://www.libsdl,org)

- Easy porting to further platforms, since all platform-specific code has 
  been moved from the core library into the examples.

- OpenGL like lighting.

- Complete OpenGL selection mode handling for object picking.

- 16 bit Z buffer. 16/24/32 bit RGB rendering. High speed dithering to
paletted 8 bits if needed. High speed conversion to 24 bit packed
pixel or 32 bit RGBA if needed.

- Fast Gouraud shadding optimized for 16 bit RGB.

- Fast texture mapping capabilities, with perspective correction and
texture objects.

- 32 bit float only arithmetic.

- Very small: compiled (and stripped) code size of about 40 kB on x86. 
  The file include/zfeatures.h can be used to remove some unused 
  features from TinyGL.

- C sources for GCC on 32/64 bit architectures. It has been tested
successfully on x86-Linux and MS Windows.

Examples:
---------

I took three simple examples from the Mesa package to test the main
functions of TinyGL. You can link them to either TinyGL, Mesa or any
other OpenGL implementation.

- texobj illustrates the use of texture objects. Its shows the speed
of TinyGL in this case (example not included in TinySDGL).

- You can download and compile the VReng project to see that TinyGL
has been successfully used in a big project
(http://www-inf.enst.fr/vreng).

Architecture:
-------------

TinyGL is made up four main modules:

- Mathematical routines (zmath).

- OpenGL-like emulation (zgl).

- Z buffer and rasterisation (zbuffer).

To use TinySDGL in a further system, you should look at the examples
and replace the SDL specific code by corresponding commands.

Notes - limitations:
--------------------

- 24 BIT packed pixel format seems not to work correctly under libSDL.

- See the file 'LIMITATIONS' to see the current functions supported by the API.

- Multithreading could be easily implemented since no global state
is maintained. The library gets the current context with a function
which can be modified.

- Lightening is not very fast. I suppose that in most games the
lightening is computed by the 3D engine.

- Some changes are needed for 64 bit pointers for the handling of
arrays of float with the GLParam union.

- No user clipping planes are supported.

- No color index mode (no longer useful !)

- The mipmapping is not implemented.

- The perspecture correction in the mapping code does not use W but
1/Z. In any 'normal scene' it should work.

Why ?
-----

TinyGL was developed as a student project for a Virtual Reality
network system called VReng (see the VReng home page at
http://www-inf.enst.fr/vreng).

At that time (January 1997), my initial project was to write my own 3D
rasterizer based on some old sources I wrote. But I realized that it
would be better to use OpenGL to work on any platform. My problem was
that I wanted to use texture mapping which was (and is still) quite
slower on many software OpenGL implementation. I could have modified
Mesa to suit my needs, but I really wanted to use my old sources for
that project. 

I finally decided to use the same syntax as OpenGL but with my own
libraries, thinking that later it could ease the porting of VReng to
OpenGL.

Now VReng is at last compatible with OpenGL, and I managed to patch
TinyGL so that VReng can still work with it without any modifications.

Since TinyGL may be useful for some people, especially in the world of
embedded designs, I decided to release it 'as is', otherwise, it would
have been lost on my hard disk !

------------------------------------------------------------------------------
* OpenGL(R) is a registered trademark of Silicon Graphics, Inc.
------------------------------------------------------------------------------
Fabrice Bellard. (Minor changes by Gerald Franz)

