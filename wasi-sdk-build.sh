#!/bin/bash

export WASI_SDK_PREFIX=/opt/wasi-sdk

mkdir -p wasmbin
cat > wasmbin/interrogate << END
#!/bin/bash
cd /
echo "
=========================================
wasm3 $(pwd)/bin/interrogate \$@
=========================================
"
wasm3 $(pwd)/bin/interrogate \$@
END

cat > wasmbin/interrogate_module << END
#!/bin/bash
cd /
echo "
=========================================
wasm3 $(pwd)/bin/interrogate_module \$@
=========================================
"
wasm3 $(pwd)/bin/interrogate_module \$@
END

chmod +x $(pwd)/wasmbin/*


/opt/python-wasm-sdk/devices/x86_64/usr/bin/cmake /data/git/panda3d-pmpp \
 -DCMAKE_SYSTEM_NAME=WASI -DCMAKE_BUILD_TYPE=Release \
 -DHAVE_THREADS=NO \
 -DHAVE_EGL=NO -DHAVE_GL=NO -DHAVE_GLX=NO -DHAVE_X11=NO -DHAVE_GLES1=NO -DHAVE_GLES2=NO \
 -DHAVE_OPENSSL=NO -DHAVE_ZLIB=NO \
 -DHAVE_TIFF=NO -DHAVE_PNG=NO -DHAVE_JPEG=NO\
 -DHAVE_AUDIO=NO -DHAVE_OPUS=NO \
 -DHAVE_HARFBUZZ=NO -DHAVE_FREETYPE=NO \
 -DTHIRDPARTY_DIRECTORY=/opt/wasi-sdk \
\
\
 -DHOST_PATH_INTERROGATE=$(pwd)/wasmbin/interrogate \
 -DHOST_PATH_INTERROGATE_MODULE=$(pwd)/wasmbin \
\
\
 -DWASI_SDK_PREFIX=/opt/wasi-sdk \
 -DCMAKE_TOOLCHAIN_FILE=/opt/wasi-sdk/share/cmake/wasi-sdk.cmake \
 -DCMAKE_INSTALL_PREFIX=/opt/python-wasi-sdk

make

# -DHOST_PATH_PZIP=${HOST}/bin/pzip
