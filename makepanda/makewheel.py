"""
Generates a wheel (.whl) file from the output of makepanda.

Since the wheel requires special linking, this will only work if compiled with
the `--wheel` parameter.
"""
from __future__ import print_function, unicode_literals
from distutils.util import get_platform as get_dist
import json

import sys
import os
from os.path import join
import shutil
import zipfile
from makepandacore import ColorText


def get_platform():
    p = get_dist().replace('-', '_').replace('.', '_')
    if "linux" in p:
        print(ColorText("red", "WARNING:") +
              " Linux-specific wheel files are not supported."
              " We will generate this wheel as a generic package instead.")
        return "any"
    return p


# Global filepaths
LIB_SRC = join("built", "lib")  # TODO: A special wheel lib dir
PANDA3D_SRC = join("built", "panda3d")  # TODO: Empty?
PANDAC_SRC = join("built", "pandac")
DIRECT_SRC = join("built", "direct")
MODELS_SRC = join("built", "models")
PLUGINS_SRC = join("built", "plugins")
ETC_SRC = join("built", "etc")
BIN_SRC = join("built", "bin")
LICENSE_SRC = "LICENSE"
README_SRC = "README.md"
WHEEL_TARGET = join("built", "wheel")
PANDA3D_TARGET = join(WHEEL_TARGET, "panda3d")
PANDAC_TARGET = join(WHEEL_TARGET, "pandac")
DIRECT_TARGET = join(WHEEL_TARGET, "direct")

# Other global parameters
PY_VERSION = "py3" if sys.version_info >= (3, 0) else "py2"
PLATFORM_VERSION = get_platform()

WHEEL_DATA = """Wheel-Version: 1.0
Generator: bdist_wheel 1.0
Root-Is-Purelib: false
Tag: py2-none-any
Tag: py3-none-any
Build: 1
"""

METADATA = {
    "license": "BSD",
    "name": "Panda3D",
    "metadata_version": "2.0",
    "generator": "makepanda",
    "summary": "Panda3D is a game engine, a framework for 3D rendering and "
               "game development for Python and C++ programs.",
    "extensions": {
        "python.details": {
            "project_urls": {
                "Home": "http://www.panda3d.org/"
            },
            "document_names": {
                "license": "LICENSE.txt"
            },
            "contacts": [
                {
                    "role": "author",
                    "email": "etc-panda3d@lists.andrew.cmu.edu",
                    "name": "Panda3D Team"
                }
            ]
        }
    },
    "classifiers": [
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Intended Audience :: End Users/Desktop",
        "License :: OSI Approved :: BSD License",
        "Operating System :: OS Independent",
        "Programming Language :: C++",
        "Programming Language :: Python",
        "Topic :: Games/Entertainment",
        "Topic :: Multimedia",
        "Topic :: Multimedia :: Graphics",
        "Topic :: Multimedia :: Graphics :: 3D Rendering"
    ]
}


def write_metadata(info_target):
    details = METADATA["extensions"]["python.details"]
    homepage = details["project_urls"]["Home"]
    author = details["contacts"][0]["name"]
    email = details["contacts"][0]["email"]
    with open(join(info_target, 'METADATA'), 'w') as outfile:
        outfile.writelines([
            "Metadata-Version: {metadata_version}",
            "Name: {name}",
            "Version: {version}",
            "Summary: {summary}",
            "License: {license}".format(**METADATA),
            "Home-page: {}".format(homepage),
            "Author: {}".format(author),
            "Author-email: {}".format(email),
            "Platform: {}".format(PLATFORM_VERSION),
        ] + ["Classifier: {}".format(c) for c in METADATA['classifiers']])


def zip_directory(path, zip_handler):
    for root, dirs, files in os.walk(path):
        for file in files:
            full_path = os.path.join(root, file)
            short_path = full_path.replace(path, '')
            zip_handler.write(full_path, short_path)


def copy_files(src, target):
    """Copy all files from a directory to another. Differs from copytree
    because it copies the files contained, not the directory itself.
    """
    for f in os.listdir(src):
        shutil.copy2(join(src, f), join(target, f))


def makewheel(version):
    # Global parameters that depend on version
    data_target = join(WHEEL_TARGET, "panda3d-{}.data".format(version))
    info_target = join(WHEEL_TARGET, "panda3d-{}.dist-info".format(version))
    wheel_name = "panda3d-{}-{}-none-{}.whl".format(
        version, PY_VERSION, PLATFORM_VERSION)

    # Update relevant METADATA entries
    METADATA['version'] = version
    version_classifiers = [
        "Programming Language :: Python :: {}".format(*sys.version_info),
        "Programming Language :: Python :: {}.{}".format(*sys.version_info),
    ]
    METADATA['classifiers'].extend(version_classifiers)

    # Clean
    print("Clearing wheel cache...")
    try:
        shutil.rmtree(WHEEL_TARGET)
    except Exception:
        pass

    # Create the wheel directory structure
    print("Copying files...")
    os.makedirs(WHEEL_TARGET)
    os.makedirs(data_target)
    os.makedirs(info_target)

    # Copy the built libraries
    shutil.copytree(PANDA3D_SRC, PANDA3D_TARGET)
    copy_files(LIB_SRC, PANDA3D_TARGET)
    shutil.copytree(DIRECT_SRC, DIRECT_TARGET)
    shutil.copytree(PANDAC_SRC, PANDAC_TARGET)

    # For linux only, we need to set the rpath on each .so file
    if get_platform() == "any":
        for filename in os.listdir(PANDA3D_TARGET):
            if ".so" in filename:
                path = os.path.join(PANDA3D_TARGET, filename)
                os.system("patchelf --set-rpath '$ORIGIN' {}".format(path))

    # Copy the data files
    shutil.copytree(MODELS_SRC, join(data_target, "data", "models"))
    shutil.copytree(PLUGINS_SRC, join(data_target, "data", "plugins"))
    shutil.copytree(ETC_SRC, join(data_target, "data", "etc"))
    shutil.copytree(BIN_SRC, join(data_target, "scripts"))

    # Build out the metadata
    shutil.copy2(LICENSE_SRC, join(info_target, "LICENSE.txt"))
    shutil.copy2(README_SRC, info_target)
    with open(join(info_target, 'metadata.json'), 'w') as outfile:
        json.dump(METADATA, outfile)
    with open(join(info_target, 'RECORD'), 'w') as outfile:
        # TODO: Populate the RECORD file with all the proper hashes
        pass
    with open(join(info_target, 'WHEEL'), 'w') as outfile:
        outfile.write(WHEEL_DATA)
    write_metadata(info_target)

    # Zip it up and name it the right thing
    print("Zipping files into a wheel...")
    zip_file = zipfile.ZipFile(wheel_name, 'w')
    zip_directory(WHEEL_TARGET, zip_file)
    zip_file.close()
