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

If this command fails, please make sure your version of pip is up-to-date.

If you prefer to install the full SDK with all tools, the latest development
builds can be obtained from this page:

https://www.panda3d.org/download.php?sdk&version=devel

These are automatically kept up-to-date with the latest GitHub version of Panda.

Building Panda3D
================

Windows
-------

You can build Panda3D with the Microsoft Visual C++ 2015 or 2017 compiler,
which can be downloaded for free from the [Visual Studio site](https://visualstudio.microsoft.com/downloads/).
You will also need to install the [Windows 10 SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk),
and if you intend to target Windows XP, you will also need the
[Windows 7.1 SDK](https://www.microsoft.com/en-us/download/details.aspx?id=8279).

You will also need to have the third-party dependency libraries available for
the build scripts to use.  These are available from one of these two URLs,
depending on whether you are on a 32-bit or 64-bit system, or you can
[click here](https://github.com/rdb/panda3d-thirdparty) for instructions on
building them from source.

http://rdb.name/thirdparty-vc14-x64.7z
http://rdb.name/thirdparty-vc14.7z

After acquiring these dependencies, you may simply build Panda3D from the
command prompt using the following command.  (Change `14.1` to `14` if you are
using Visual C++ 2015 instead of 2017.  Add the `--windows-sdk=10` option if
you don't need to support Windows XP and did not install the Windows 7.1 SDK.)

```bash
makepanda\makepanda.bat --everything --installer --msvc-version=14.1 --no-eigen --threads=2
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
sudo apt-get install build-essential pkg-config python-dev libpng-dev libjpeg-dev libtiff-dev zlib1g-dev libssl-dev libx11-dev libgl1-mesa-dev libxrandr-dev libxxf86dga-dev libxcursor-dev bison flex libfreetype6-dev libvorbis-dev libeigen3-dev libopenal-dev libode-dev libbullet-dev nvidia-cg-toolkit libgtk2.0-dev libassimp-dev libopenexr-dev
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

FreeBSD
-------

Building on FreeBSD is very similar to building on Linux.  You will need to
install the requisite packages using the system package manager.  To install
the recommended set of dependencies, you can use this command:

```bash
pkg install pkgconf png jpeg-turbo tiff freetype2 eigen squish openal opusfile libvorbis libX11 libGL ode bullet assimp openexr
```

You will also need to choose which version of Python you want to use.
Install the appropriate package for it (such as `python2` or `python36`) and
run the makepanda script with your chosen Python version:

```bash
python3.6 makepanda/makepanda.py --everything --installer --no-egl --no-gles --no-gles2
```

If successful, this will produce a .pkg file in the root of the source
directory which you can install using `pkg install`.

Android
-------

Note: building on Android is very experimental and not guaranteed to work.

You can experimentally build the Android Python runner via the [termux](https://termux.com/)
shell.  You will need to install [Termux](https://play.google.com/store/apps/details?id=com.termux)
and [Termux API](https://play.google.com/store/apps/details?id=com.termux.api)
from the Play Store.  Many of the dependencies can be installed by running the
following command in the Termux shell:

```bash
pkg install python-dev termux-tools ndk-stl ndk-sysroot clang libvorbis-dev libopus-dev opusfile-dev openal-soft-dev freetype-dev harfbuzz-dev libpng-dev ecj4.6 dx patchelf aapt apksigner libcrypt-dev
```

Then, you can build and install the .apk right away using these commands:

```bash
python makepanda/makepanda.py --everything --target android-21 --installer
xdg-open panda3d.apk
```

To launch a Python program from Termux, you can use the `run_python.sh` script
inside the `panda/src/android` directory.  It will launch Python in a separate
activity, load it with the Python script you passed as argument, and use a
socket for returning the command-line output to the Termux shell.  Do note
that this requires the Python application to reside on the SD card and that
Termux needs to be set up with access to the SD card (using the
`termux-setup-storage` command).

Running Tests
=============

Install [PyTest](https://docs.pytest.org/en/latest/getting-started.html#installation)
and run the `pytest` command.  If you have not installed Panda3D, you will
need to configure your enviroment by pointing the `PYTHONPATH` variable at
the `built` directory.  On Linux, you will also need to point the
`LD_LIBRARY_PATH` variable at the `built/lib` directory.

As a convenience, you can alternatively pass the `--tests` option to makepanda.

Reporting Issues
================

If you encounter any bugs when using Panda3D, please report them in the bug
tracker.  This is hosted at:

  https://github.com/panda3d/panda3d/issues

Make sure to first use the search function to see if the bug has already been
reported.  When filling out a bug report, make sure that you include as much
information as possible to help the developers track down the issue, such as
your version of Panda3D, operating system, architecture, and any code and
models that are necessary for the developers to reproduce the issue.

If you're not sure whether you've encountered a bug, feel free to ask about
it in the forums or the IRC channel first.
