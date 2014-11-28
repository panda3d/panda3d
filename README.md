Panda3D
=======

Panda3D is a game engine that includes graphics, audio, I/O, collision detection, and other abilities relevant to the creation of 3D games

Panda3D is open source and is, as of May 28, 2008, free software under the revised BSD license. Releases prior to that date are not considered Free Software due to certain errors in the design of the old Panda3D license. Despite this, those older releases of Panda3D can also be used for both free and commercial game development at no financial cost.

Panda3D's intended game-development language is Python. The engine itself is written in C++, and utilizes an automatic wrapper-generator to expose the complete functionality of the engine in a Python interface. This approach gives a developer the advantages of Python development, such as rapid development and advanced memory management, but keeps the performance of a compiled language in the engine core. For instance, the engine is integrated with Python's garbage collector, and engine structures are automatically managed.

Building Panda3D
=======

At the present, we are providing two completely unrelated systems for
building Panda.  The original build system, ppremake, is still in
active use by the VR Studio, and is useful if you want advanced build
control. The other build system, makepanda, is designed to build
quickly and painlessly, and is used to generate the official releases.

The ppremake system is a makefile generator, and allows you to
configure your build environment to a high degree of customization.
It is a fairly complex build system, and it requires some comfort with
using the command-line make utilities.

The makepanda system is a Python script that directly invokes the
compiler to build the Panda sources.  Its emphasis is on providing a
hands-off, simple approach to building Panda.

Both systems may require you to first install a number of third-party
tools if you would like to make them available for Panda, such as
FreeType or OpenSSL.  You may also download a zip file that contains
precompiled versions of these third-party libraries from the Panda
website, which is especially useful when used in conjunction with the
makepanda system.

Panda3D Install --- using the 'makepanda' system.

MAKE SURE YOU HAVE ALL OF THE SOURCE CODE
The easiest way to download the source for panda is to download the
"source package" from the panda3d website. If you downloaded a file
labeled "source package", then you have everything you need. Skip to
the next section.

Alternately, it is possible to download the source in pieces. There
are three pieces:
1. Source code from Github.
2. Third-party tools (not strictly necessary for Unix)
3. Sample programs.

You will need all three to use makepanda. You can download all three
pieces from the panda website. Look for the files labeled "Panda3D
source, piecewise, X of 3". You can also obtain the first piece
directly from the github git server over ssh: git clone git@github.com:panda3d/panda3d.git

Make sure you have all three pieces. 
Linux/FreeBSD users may omit the "thirdparty" tree, but this means
they will need to have the thirdparty software installed on the system.

INVOKING MAKEPANDA
Windows:
makepanda\makepanda.bat

To invoke it under Linux or OSX, change directory to the root of
the panda source tree and type this:
makepanda/makepanda.py


The easy way to build panda is to type:
makepanda --everything

For more options and info about building be sure to read documents in ./doc in root of source.

Panda3D licence
=======



What follows is the Modified BSD License. See also http://www.opensource.org/licenses/BSD-3-Clause

Copyright (c) 2008, Carnegie Mellon University. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

    3. Neither the name of Carnegie Mellon University nor the names of other contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

