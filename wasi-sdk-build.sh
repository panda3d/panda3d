#!/bin/bash

export PYBUILD=3.11
export SDKROOT=/opt/python-wasm-sdk

export SRCDIR=$(dirname $0)
echo SRCDIR=$SRCDIR

if echo $@|grep -q host
then

    echo "building  ${SRCDIR} for host $(arch)"

    #-DHAVE_EGL=NO -DHAVE_X11=YES -DHAVE_TINYDISPLAY=1 -DHAVE_THREADS=NO -DHAVE_AUDIO=OFF

    ${SDKROOT}/devices/x86_64/usr/bin/cmake -S ${SRCDIR} \
     -DCMAKE_BUILD_TYPE=Release \
     -DHAVE_THREADS=NO \
     -DHAVE_EGL=NO -DHAVE_GL=NO -DHAVE_GLX=NO -DHAVE_X11=NO -DHAVE_GLES1=NO -DHAVE_GLES2=NO \
     -DHAVE_OPENSSL=NO \
     -DHAVE_ZLIB=NO -DHAVE_PNG=NO  \
     -DHAVE_TIFF=NO -DHAVE_JPEG=NO \
     -DHAVE_AUDIO=NO -DHAVE_OPUS=NO \
     -DHAVE_HARFBUZZ=NO -DHAVE_FREETYPE=NO \
    -DHAVE_IOS_TYPEDEFS=1 -DWANT_NATIVE_NET=NO -DHAVE_TINYDISPLAY=1 \
    \
     -DHAVE_PYTHON=NO -DBUILD_SHARED_LIBS=NO \
    \
     -DCMAKE_INSTALL_PREFIX=${SDKROOT}/devices/$(arch)/usr


    exit 0
fi

pushd ${SDKROOT}

if echo $EMSDK|grep emsdk
then
    echo building emsdk
    popd



    emcmake ${SDKROOT}/devices/x86_64/usr/bin/cmake ${SRCDIR} \
     -DCMAKE_BUILD_TYPE=Release \
     -DHAVE_THREADS=NO \
     -DHAVE_EGL=NO -DHAVE_GL=NO -DHAVE_GLX=NO -DHAVE_X11=NO -DHAVE_GLES1=NO -DHAVE_GLES2=NO \
     -DHAVE_OPENSSL=NO \
     -DHAVE_ZLIB=NO -DHAVE_PNG=NO  \
     -DHAVE_TIFF=NO -DHAVE_JPEG=NO \
     -DHAVE_AUDIO=NO -DHAVE_OPUS=NO \
     -DHAVE_HARFBUZZ=NO -DHAVE_FREETYPE=NO \
     -DTHIRDPARTY_DIRECTORY=${PREFIX} \
    -DHAVE_IOS_TYPEDEFS=1 -DWANT_NATIVE_NET=NO -DHAVE_TINYDISPLAY=1 \
    \
     -DHAVE_PYTHON=NO \
    \
     -DCMAKE_INSTALL_PREFIX=${PREFIX}


else

    . scripts/wasisdk-fetch.sh

    popd


    mkdir -p wasmbin
    cat > wasmbin/interrogate << END
    #!/bin/bash
    # cd /
    ${SDKROOT}/wasisdk/bin/wasi-run $(pwd)/bin/interrogate \$@
END

    cat > wasmbin/interrogate_module << END
    #!/bin/bash
    # cd /
    ${SDKROOT}/wasisdk/bin/wasi-run $(pwd)/bin/interrogate_module \$@
END

    chmod +x $(pwd)/wasmbin/*

    # HAVE_EGG

    ${SDKROOT}/devices/x86_64/usr/bin/cmake ${SRCDIR} \
     -DCMAKE_BUILD_TYPE=Release \
     -DHAVE_THREADS=NO \
     -DHAVE_EGL=NO -DHAVE_GL=NO -DHAVE_GLX=NO -DHAVE_X11=NO -DHAVE_GLES1=NO -DHAVE_GLES2=NO \
     -DHAVE_OPENSSL=NO \
     -DHAVE_ZLIB=NO -DHAVE_PNG=NO  \
     -DHAVE_TIFF=NO -DHAVE_JPEG=NO \
     -DHAVE_AUDIO=NO -DHAVE_OPUS=NO \
     -DHAVE_HARFBUZZ=NO -DHAVE_FREETYPE=NO \
     -DTHIRDPARTY_DIRECTORY=${PREFIX} \
     -DPHAVE_IOSTREAM=1 -DWANT_NATIVE_NET=NO -DHAVE_TINYDISPLAY=1 \
    \
     -DHAVE_PYTHON=NO \
    \
     -DCMAKE_SYSTEM_NAME=WASI \
     -DWASI_SDK_PREFIX=${WASI_SDK_PREFIX} \
     -DCMAKE_TOOLCHAIN_FILE=${WASISDK}/share/cmake/wasi-sdk.cmake \
     -DCMAKE_INSTALL_PREFIX=${PREFIX} \






    #\
    # -DHAVE_PYTHON=1 \
    # -DHOST_PATH_INTERROGATE=$(pwd)/wasmbin/interrogate \
    # -DHOST_PATH_INTERROGATE_MODULE=$(pwd)/wasmbin \
    #    -DPython3_EXECUTABLE:FILEPATH=${SDKROOT}/python3-wasi \
    #    -DPython3_INCLUDE_DIR=${SDKROOT}/devices/wasisdk/usr/include/python${PYBUILD} \
    #    -DPython3_LIBRARY=${SDKROOT}/devices/wasisdk/usr/lib \
    #    -DPython3_FOUND=TRUE \
    #    -DPython3_Development_FOUND=TRUE \
    #    -DPython3_Development.Module_FOUND=TRUE \
    #    -DPython3_Development.Embed_FOUND=TRUE \
    #\

# -DHOST_PATH_PZIP=${HOST}/bin/pzip



fi

make


