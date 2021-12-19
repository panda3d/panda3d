Building with CMake
-------------------

On Windows and macOS, please ensure that you have the very latest version of
CMake installed; older versions may work, but if not, please upgrade to the
latest available version of CMake before requesting help.

On systems that package CMake themselves (e.g. Linux distributions), we most
likely support the provided version of CMake as long as the system itself is
supported.

CMake will also require that you already have your system's developer tools
installed.

The quickest way to build and install Panda with CMake is to install any
third-party dependencies and then run:
```sh
mkdir build && cd build
cmake ..
cmake --build . --config Standard --parallel 4
[sudo] cmake --build . --config Standard --target install
```

Note that, if you are targeting 64-bit on Windows, it is necessary to supply
the `-A x64` option when first invoking `cmake` (as `cmake -A x64 ..`).

CMake itself does not build Panda; rather, it generates project files for an
existing IDE or build tool. To select a build tool, pass the `-G` option when
first invoking CMake, (`cmake -G Ninja ..` is highly recommended on Linux).
Some of these (Xcode, Visual Studio) support targeting multiple configurations
(the `--config Standard`, above, selects the `Standard` configuration in those
cases). Other build tools (Ninja, Makefiles, ...) do not support multiple
configurations, and the `--config` option is ignored. To change the
configuration in these cases (from `Standard`, the default), it is necessary to
change the `CMAKE_BUILD_TYPE` variable as explained below.

The configurations are:

| Configuration  | Explanation                                            |
| -------------- | ------------------------------------------------------ |
| Standard       | Default; build provided to users of SDK                |
| Release        | Distribution for end-users                             |
| MinSizeRel     | Like Release, but optimized for size                   |
| RelWithDebInfo | Like Release, but include debug symbols                |
| Debug          | Do not optimize, enable optional debugging features    |
| Coverage       | Like Debug, but profile code coverage; developers only |

To configure CMake, it is recommended to use cmake-gui (`cmake-gui .`),
or ccmake (`ccmake .`), however it is also possible to configure it entirely
through CMake's command-line interface; see `man cmake` for more details.

In general, the config variable for a particular third party library is:
```
	HAVE_<LIBRARY>=YES/NO   # Example: USE_JPEG
```
Panda subpackage building is handled by:
```
	BUILD_<SUBPACKAGE>=YES/NO   # Example: BUILD_DTOOL, BUILD_PANDA
```
Other configuration settings use their historical names (same names as in-source):
```
	# Examples
	PANDA_DISTRIBUTOR="MyDistributor"
	LINMATH_ALIGN=YES

	# ... etc ...

```

For example, `makepanda.py --distributor X` becomes `cmake -DPANDA_DISTRIBUTOR=X`

All found third-party libraries are enabled by default, and makepanda-style
tools packages are searched in the same path as makepanda (however this may be
overridden with the `THIRDPARTY_DIRECTORY` option).

Most config settings are set to a sensible default for a typical PC/desktop
Panda3D distribution. Third-party libraries and other settings can be enabled
or disabled through configuration with the cmake GUI or CLI.
