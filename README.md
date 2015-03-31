[![Build Status](https://travis-ci.org/panda3d/panda3d.svg?branch=master)](https://travis-ci.org/panda3d/panda3d)

<img src="https://avatars2.githubusercontent.com/u/590956?v=3&s=200" align="right" />

Panda3D
=======

Panda3D is a game engine, a framework for 3D rendering and game development for
Python and C++ programs.  Panda3D is open-source and free for any purpose,
including commercial ventures, thanks to its
[liberal license](https://www.panda3d.org/license.php).  To learn more about
Panda3D's capabilities, visit the [gallery](https://www.panda3d.org/gallery.php)
and the [feature list](https://www.panda3d.org/features.php).  To learn how to
use Panda3D, check the [documentation](https://www.panda3d.org/documentation.php)
resources. If you get stuck, ask for help from our active
[community](https://www.panda3d.org/community.php).

Panda3D is licensed under the Modified BSD License.  See the LICENSE file for
more details.

Building Panda3D
================

Windows
-------

We currently build using the Microsoft Visual C++ 2010 compiler.  You do not
need Microsoft Visual Studio to build Panda3D, though - the relevant compilers
are included as part of the Windows 7.1 SDK.

You will also need to have the third-party dependency libraries available for
the build scripts to use.  These are available from one of these two URLs,
depending on whether you are on a 32-bit or 64-bit system:
https://www.panda3d.org/download/panda3d-1.9.0/panda3d-1.9.0-tools-win32.zip
https://www.panda3d.org/download/panda3d-1.9.0/panda3d-1.9.0-tools-win64.zip

After acquiring these dependencies, you may simply build Panda3D from the
command prompt using the following command:

```bash
makepanda\makepanda.bat --everything --installer
```

When the build succeeds, it will produce an .exe file that you can use to
install Panda3D on your system.

Linux
-----

Building Panda3D on Linux is easy.  All you need is to invoke the makepanda
script using the version of Python that you want Panda3D to be built against.

Run makepanda.py with the --help option to see which options are available.
Usually, you will want to specify the --everything option (which builds with
support for all features for which it detects the prerequisite dependencies)
and the --installer option (which produces an installable .deb or .rpm file
for you to install, depending on your distribution).

The following command illustrates how to build Panda3D with some common
options:
```bash
python2.7 makepanda/makepanda.py --everything --installer --no-egl --no-gles --no-gles2
```

You will probably see some warnings saying that it's unable to find several
dependency packages.  You should determine which ones you want to include in
your build and install the respective development packages.  You may visit
[this manual page](https://www.panda3d.org/manual/index.php/Dependencies)
for an overview of the various dependencies.

If you are on Ubuntu, this command should cover the most frequently
used third-party packages:

```bash
sudo apt-get install build-essential pkg-config python-dev libpng-dev libjpeg-dev libtiff-dev zlib1g-dev libssl-dev libx11-dev libgl1-mesa-dev libxrandr-dev libxxf86dga-dev libxcursor-dev bison flex libfreetype6-dev libvorbis-dev libeigen3-dev libopenal-dev libode-dev libbullet-dev nvidia-cg-toolkit libgtk2.0-dev
```

Once Panda3D has built, you can either install the .deb or .rpm package that
it produced (if relevant to your platform, and you added --installer).  On
other systems, you will need to use the installpanda script to install it onto
your system.  Careful: it is not easy to uninstall Panda3D in this way!

```bash
python2.7 makepanda/installpanda.py --prefix=/usr/local
```

Mac OS X
--------

On Mac OS X, all you need to compile Panda3D is a set of precompiled
thirdparty packages, which can be acquired from here:
https://www.panda3d.org/download/panda3d-1.9.0/panda3d-1.9.0-tools-mac.tar.gz

After placing the thirdparty directory inside the panda3d source directory,
you may build Panda3D using a command like the following:

```bash
python makepanda/makepanda.py --everything --installer
```

In order to make a universal build, pass the --universal flag.  You may also
target a specific minimum Mac OS X version using the --osxtarget flag followed
by the release number, eg. 10.6 or 10.7.

If the build was successful, makepanda will have generated a .dmg file in
the source directory containing the installer.  Simply open it and run the
package file in order to install the SDK onto your system.
