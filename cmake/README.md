Building with CMake
-------------------

The quickest way to build and install panda with CMake is to run:
```sh
cmake .
make
[sudo] make install
```

It is recommended to create a separate directory to build Panda3D:
```sh
mkdir build
cd build/
cmake ..
make
```

To configure CMake, it is recommended to use cmake-gui (`cmake-gui .`),
however it is also possible to configure it entirely through CMake's
command-line interface; see `man cmake` for more details.

In general, the config variable for a particular third party library is:
```
	USE_<LIBRARY>=True/False   # Example: USE_JPEG
```
Optional features typically use the format:
```
	BUILD_<FEATURE>=True/False   # Example: BUILD_THREADS
```
Configuration settings use more direct names:
```
	# Examples
	PANDA_DISTRIBUTOR="MyDistributor"
	DTOOL_INSTALL="/usr/local/panda"

	# ... etc ...

```

For example, `makepanda.py --distributor X` becomes `cmake -DDISTRIBUTOR

All third-party libraries are enabled by default and Panda3D will
be compiled with any 3rd party library that is found.
Third-party libraries can be enabled or disabled through
configuration with the cmake gui or cli.

To quickly enable or disable all third-party libraries, run:
```sh
cmake -DEVERYTHING=True .  # or .. if you are in a separate build/ dir
# OR
cmake -DNOTHING=True .  # or .. if you are in a separate build/ dir
```

To use all available packages, and silence output for missing packages, run:
```sh
cmake- DDISCOVERED=True .  # or .. if you are in a separate build/ dir
```
