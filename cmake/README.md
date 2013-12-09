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

