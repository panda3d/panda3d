Building with CMake
-------------------

The quickest way to build and install panda with CMake is to run:
```sh
mkdir build && cd build
cmake ..
make
[sudo] make install
```

To configure CMake, it is recommended to use cmake-gui (`cmake-gui .`),
however it is also possible to configure it entirely through CMake's
command-line interface; see `man cmake` for more details.

In general, the config variable for a particular third party library is:
```
	HAVE_<LIBRARY>=True/False   # Example: USE_JPEG
```
Panda subpackage building is handled by:
```
	BUILD_<SUBPACKAGE>=True/False   # Example: BUILD_DTOOL, BUILD_PANDA
```
Other configuration settings use their historical names (same names as in-source):
```
	# Examples
	PANDA_DISTRIBUTOR="MyDistributor"
	LINMATH_ALIGN=On

	# ... etc ...

```

For example, `makepanda.py --distributor X` becomes `cmake -DPANDA_DISTRIBUTOR=X`

All found third-party libraries are enabled by default.
Most config settings are set to a sensible default for typical
a PC/desktop Panda3D distribution.
Third-party libraries and other settings can be enabled or disabled
through configuration with the cmake gui or cli.

Running Panda3D with `-DCMAKE_BUILD_TYPE=` and one of `Release`, `Debug`,
`MinSizeRel`, or `RelWithDebInfo` will cause some configuration settings
to change their defaults to more appropriate values.

If cmake has already been generated, changing the build type will not cause
some of these values to change to their expected values, because the values
are cached so that they don't overwrite custom settings.

To reset CMake's config to defaults, delete the CMakeCache.txt file, and rerun
CMake with the preferred build mode specified
(example: `cmake .. -DCMAKE_BUILD_TYPE=Debug`).
