#!/bin/bash

brew install python@3 eigen libtar fftw libsquish zlib ffmpeg freetype bullet \
wxmac opencv assimp libvorbis openssl@1.0 || true
brew install ode --with-shared

# We can't trust brew to make the right symlinks, so execute commands as modules
/usr/local/bin/${PYTHON_INTERP:-python3} -m pip install virtualenv
/usr/local/bin/${PYTHON_INTERP:-python3} -m virtualenv --python=${PYTHON_INTERP:-python3} venv
