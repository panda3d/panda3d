#!/bin/bash

export SDKROOT=/opt/python-wasi-sdk

PYBUILD=3.12

. ${SDKROOT}/wasi-shell.sh


mkdir -p wasmbin
cat > wasmbin/interrogate << END
#!/bin/bash
# cd /
${SDKROOT}/wasi/bin/wasi-run $(pwd)/bin/interrogate \$@
END

cat > wasmbin/interrogate_module << END
#!/bin/bash
# cd /
${SDKROOT}/wasi/bin/wasi-run $(pwd)/bin/interrogate_module \$@
END

chmod +x $(pwd)/wasmbin/*

# HAVE_EGG

${SDKROOT}/devices/x86_64/usr/bin/cmake /data/git/panda3d-pmpp \
 -DCMAKE_SYSTEM_NAME=WASI -DCMAKE_BUILD_TYPE=Release \
 -DHAVE_THREADS=NO \
 -DHAVE_EGL=NO -DHAVE_GL=NO -DHAVE_GLX=NO -DHAVE_X11=NO -DHAVE_GLES1=NO -DHAVE_GLES2=NO \
 -DHAVE_OPENSSL=NO -DHAVE_ZLIB=NO \
 -DHAVE_TIFF=NO -DHAVE_PNG=NO -DHAVE_JPEG=NO\
 -DHAVE_AUDIO=NO -DHAVE_OPUS=NO \
 -DHAVE_HARFBUZZ=NO -DHAVE_FREETYPE=NO \
 -DTHIRDPARTY_DIRECTORY=${SDKROOT} \
\
 -DPHAVE_IOSTREAM=1 -DWANT_NATIVE_NET=NO -DHAVE_TINYDISPLAY=1 \
\
 -DWASI_SDK_PREFIX=${SDKROOT} \
 -DCMAKE_TOOLCHAIN_FILE=${SDKROOT}/wasi/share/cmake/wasi-sdk.cmake \
 -DCMAKE_INSTALL_PREFIX=${SDKROOT} \
 -DHAVE_PYTHON=NO


#\
# -DHAVE_PYTHON=1 \
# -DHOST_PATH_INTERROGATE=$(pwd)/wasmbin/interrogate \
# -DHOST_PATH_INTERROGATE_MODULE=$(pwd)/wasmbin \
#    -DPython3_EXECUTABLE:FILEPATH=${SDKROOT}/python3-wasi \
#    -DPython3_INCLUDE_DIR=${SDKROOT}/devices/wasi/usr/include/python${PYBUILD} \
#    -DPython3_LIBRARY=${SDKROOT}/devices/wasi/usr/lib \
#    -DPython3_FOUND=TRUE \
#    -DPython3_Development_FOUND=TRUE \
#    -DPython3_Development.Module_FOUND=TRUE \
#    -DPython3_Development.Embed_FOUND=TRUE \
#\



make

# -DHOST_PATH_PZIP=${HOST}/bin/pzip
