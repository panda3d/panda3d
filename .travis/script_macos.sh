#!/bin/bash

if [[ "$PYTHON_INTERP" == "python2.7" ]]; then
  export PY2_CMAKE_ARGS="-DPYTHON_EXECUTABLE=/usr/local/bin/python -DPYTHON_LIBRARY=/usr/local/opt/python@2/Frameworks/Python.framework/Versions/2.7/lib/libpython2.7.dylib"
fi

cmake -DHAVE_CARBON=NO -DOPENSSL_ROOT_DIR=/usr/local/Cellar/openssl/1.0.2o_1 \
-DBUILD_METALIBS=$BUILD_METALIBS -DCOMPOSITE_SOURCE_LIMIT=$COMPOSITE_SOURCE_LIMIT \
$PY2_CMAKE_ARGS ..
