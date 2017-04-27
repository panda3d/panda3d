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

Installing Panda3D
==================

By far, the easiest way to install the latest development build of Panda3D
into an existing Python installation is using the following command:

```bash
pip install --pre --extra-index-url https://archive.panda3d.org/ panda3d
```

If you prefer to install the full SDK with all tools, the latest development
builds can be obtained from this page:

https://www.panda3d.org/download.php?sdk&version=devel

These are automatically kept up-to-date with the latest GitHub version of Panda.

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
https://www.panda3d.org/download/panda3d-1.9.4/panda3d-1.9.4-tools-win32.zip
https://www.panda3d.org/download/panda3d-1.9.4/panda3d-1.9.4-tools-win64.zip

After acquiring these dependencies, you may simply build Panda3D from the
command prompt using the following command:

```bash
makepanda\makepanda.bat --everything --installer --no-eigen
```

When the build succeeds, it will produce an .exe file that you can use to
install Panda3D on your system.

Note: you may choose to remove --no-eigen and build with Eigen support in
order to improve runtime performance.  However, this will cause the build to
take hours to complete, as Eigen is a heavily template-based library, and the
the MSVC compiler does not perform well under these circumstances.

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
python makepanda/makepanda.py --everything --installer --no-egl --no-gles --no-gles2 --no-opencv
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
it produced, depending on which Linux distribution you are using.  For example,
to install the package on Debian or Ubuntu, use this:

```bash
sudo dpkg -i panda3d*.deb
```

If you are not using a Linux distribution that supports .deb or .rpm packages, you
may have to use the installpanda.py script instead, which will directly copy the
files into the appropriate locations on your computer.  You may have to run the
`ldconfig` tool in order to update your library cache after installing Panda3D.

Alternatively, you can add the `--wheel` option, which will produce a .whl
file that can be installed into a Python installation using `pip`.

macOS
-----

On macOS, you will need to download a set of precompiled thirdparty packages in order to
compile Panda3D, which can be acquired from [here](https://www.panda3d.org/download/panda3d-1.9.4/panda3d-1.9.4-tools-mac.tar.gz).

After placing the thirdparty directory inside the panda3d source directory,
you may build Panda3D using a command like the following:

```bash
python makepanda/makepanda.py --everything --installer
```

In order to make a universal build, pass the --universal flag.  You may also
target a specific minimum macOS version using the --osxtarget flag followed
by the release number, eg. 10.6 or 10.7.

If the build was successful, makepanda will have generated a .dmg file in
the source directory containing the installer.  Simply open it and run the
package file in order to install the SDK onto your system.

Reporting Issues
================

If you encounter any bugs when using Panda3D, please report them in the bug
tracker.  This is hosted at:

  https://bugs.launchpad.net/panda3d

Make sure to first use the search function to see if the bug has already been
reported.  When filling out a bug report, make sure that you include as much
information as possible to help the developers track down the issue, such as
your version of Panda3D, operating system, architecture, and any code and
models that are necessary for the developers to reproduce the issue.

If you're not sure whether you've encountered a bug, feel free to ask about
it in the forums or the IRC channel first.
