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
import hashlib
from makepandacore import ColorText
from base64 import urlsafe_b64encode


def get_platform():
    p = get_dist().replace('-', '_').replace('.', '_')
    if "linux" in p:
        print(ColorText("red", "WARNING:") +
              " Linux-specific wheel files are not supported."
              " We will generate this wheel as a generic package instead.")
        return "any"
    return p

# Other global parameters
PY_VERSION = "cp{}{}".format(sys.version_info.major, sys.version_info.minor)
ABI_TAG = "none"
PLATFORM_TAG = get_platform()
EXCLUDE_EXT = [".pyc", ".pyo", ".N", ".prebuilt", ".xcf", ".plist", ".vcproj", ".sln"]

WHEEL_DATA = """Wheel-Version: 1.0
Generator: makepanda
Root-Is-Purelib: false
Tag: {}-{}-{}
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
                "Home": "https://www.panda3d.org/"
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
            "Metadata-Version: {metadata_version}\n",
            "Name: {name}\n",
            "Version: {version}\n",
            "Summary: {summary}\n",
            "License: {license}\n".format(**METADATA),
            "Home-page: {}\n".format(homepage),
            "Author: {}\n".format(author),
            "Author-email: {}\n".format(email),
            "Platform: {}\n".format(PLATFORM_TAG),
        ] + ["Classifier: {}\n".format(c) for c in METADATA['classifiers']])


class WheelFile(object):
    def __init__(self, name, version):
        self.name = name
        self.version = version

        wheel_name = "{}-{}-{}-{}-{}.whl".format(
            name, version, PY_VERSION, ABI_TAG, PLATFORM_TAG)

        self.zip_file = zipfile.ZipFile(wheel_name, 'w')
        self.records = []

    def write_file(self, target_path, source_path):
        """Adds the given file to the .whl file."""

        # Calculate the SHA-256 hash and size.
        sha = hashlib.sha256()
        fp = open(source_path, 'rb')
        size = 0
        data = fp.read(1024 * 1024)
        while data:
            size += len(data)
            sha.update(data)
            data = fp.read(1024 * 1024)
        fp.close()

        # Save it in PEP-0376 format for writing out later.
        digest = str(urlsafe_b64encode(sha.digest()))
        digest = digest.rstrip('=')
        self.records.append("{},sha256={},{}\n".format(target_path, digest, size))

        print("Adding %s from %s" % (target_path, source_path))
        self.zip_file.write(source_path, target_path)

    def write_file_data(self, target_path, source_data):
        """Adds the given file from a string."""

        sha = hashlib.sha256()
        sha.update(source_data.encode())
        digest = str(urlsafe_b64encode(sha.digest()))
        digest = digest.rstrip('=')
        self.records.append("{},sha256={},{}\n".format(target_path, digest, len(source_data)))

        print("Adding %s from data" % target_path)
        self.zip_file.writestr(target_path, source_data)

    def write_directory(self, target_dir, source_dir):
        """Adds the given directory recursively to the .whl file."""

        for root, dirs, files in os.walk(source_dir):
            for file in files:
                if os.path.splitext(file)[1] in EXCLUDE_EXT:
                    continue

                source_path = os.path.join(root, file)
                target_path = os.path.join(target_dir, os.path.relpath(source_path, source_dir))
                self.write_file(target_path, source_path)

    def close(self):
        # Write the RECORD file.
        record_file = "{}-{}.dist-info/RECORD".format(self.name, self.version)
        self.records.append(record_file + ",,\n")

        self.zip_file.writestr(record_file, "".join(self.records))
        self.zip_file.close()


def copy_files(src, target):
    """Copy all files from a directory to another. Differs from copytree
    because it copies the files contained, not the directory itself.
    """
    for f in os.listdir(src):
        shutil.copy2(join(src, f), join(target, f))


def makewheel(version, output_dir):
    # Global filepaths
    lib_src = join(output_dir, "lib")
    panda3d_src = join(output_dir, "panda3d")
    pandac_src = join(output_dir, "pandac")
    direct_src = join(output_dir, "direct")
    models_src = join(output_dir, "models")
    etc_src = join(output_dir, "etc")
    bin_src = join(output_dir, "bin")
    license_src = "LICENSE"
    readme_src = "README.md"

    # TODO: Do this without a wheel directory.
    wheel_target = join(output_dir, "wheel")
    panda3d_target = join(wheel_target, "panda3d")
    data_target = join(wheel_target, "panda3d-{}.data".format(version))
    info_target = join(wheel_target, "panda3d-{}.dist-info".format(version))

    # Update relevant METADATA entries
    METADATA['version'] = version
    version_classifiers = [
        "Programming Language :: Python :: {}".format(*sys.version_info),
        "Programming Language :: Python :: {}.{}".format(*sys.version_info),
    ]
    METADATA['classifiers'].extend(version_classifiers)

    # Clean TODO: Should go away
    print("Clearing wheel cache...")
    try:
        shutil.rmtree(wheel_target)
    except Exception:
        pass

    # Create the wheel directory structure
    print("Copying files...")
    os.makedirs(wheel_target)
    os.makedirs(data_target)
    os.makedirs(info_target)

    # For linux only, we need to set the rpath on each .so file
    if get_platform() == "any":
        shutil.copytree(panda3d_src, panda3d_target)
        copy_files(lib_src, panda3d_target)
        for filename in os.listdir(panda3d_target):
            if ".so" in filename:
                path = os.path.join(panda3d_target, filename)
                os.system("patchelf --set-rpath '$ORIGIN' {}".format(path))

    # Build out the metadata
    details = METADATA["extensions"]["python.details"]
    homepage = details["project_urls"]["Home"]
    author = details["contacts"][0]["name"]
    email = details["contacts"][0]["email"]
    metadata = ''.join([
        "Metadata-Version: {metadata_version}\n" \
        "Name: {name}\n" \
        "Version: {version}\n" \
        "Summary: {summary}\n" \
        "License: {license}\n".format(**METADATA),
        "Home-page: {}\n".format(homepage),
        "Author: {}\n".format(author),
        "Author-email: {}\n".format(email),
        "Platform: {}\n".format(PLATFORM_TAG),
    ] + ["Classifier: {}\n".format(c) for c in METADATA['classifiers']])

    shutil.copy2(readme_src, info_target)
    with open(join(info_target, 'metadata.json'), 'w') as outfile:
        json.dump(METADATA, outfile)

    with open(join(info_target, 'WHEEL'), 'w') as outfile:
        outfile.write(WHEEL_DATA.format(PY_VERSION, ABI_TAG, PLATFORM_TAG))
    write_metadata(info_target)

    # Zip it up and name it the right thing
    print("Zipping files into a wheel...")
    whl = WheelFile('panda3d', version)

    # Add the trees with Python modules.
    whl.write_directory('direct', direct_src)

    for file in os.listdir(panda3d_target):  # TODO: Should be panda3d_src (except rpath)
        file_ext = os.path.splitext(file)[1]
        print(file_ext)
        if file_ext in ('.pyd', '.so', '.py', '.10'):  # TODO: The .10 is a hack
            source_path = os.path.join(panda3d_target, file)

            if file_ext == '.pyd' and PLATFORM_TAG.startswith('cygwin'):
                # Rename it to .dll for cygwin Python to be able to load it.
                target_path = 'panda3d/' + os.path.splitext(file)[0] + '.dll'
            else:
                target_path = 'panda3d/' + file
            whl.write_file(target_path, source_path)

    # Add dependent DLLs from the bin directory.
    for file in os.listdir(bin_src):
        file_ext = os.path.splitext(file)[1]
        if file_ext == '.dll':
            source_path = os.path.join(bin_src, file)
            target_path = 'panda3d/' + file
            whl.write_file(target_path, source_path)

    # Add the pandac tree for backward compatibility.
    whl.write_file('pandac/__init__.py', os.path.join(pandac_src, '__init__.py'))
    whl.write_file('pandac/PandaModules.py', os.path.join(pandac_src, 'PandaModules.py'))

    # Add the .data directory, containing additional files.
    data_dir = 'panda3d-{}.data'.format(version)
    whl.write_directory(data_dir + '/data/etc', etc_src)
    whl.write_directory(data_dir + '/data/models', models_src)

    # Add the dist-info directory last.
    info_dir = 'panda3d-{}.dist-info'.format(version)
    whl.write_file_data(info_dir + '/metadata.json', json.dumps(METADATA, indent=4, separators=(',', ': ')))
    whl.write_file_data(info_dir + '/METADATA', metadata)
    whl.write_file_data(info_dir + '/WHEEL', WHEEL_DATA.format(PY_VERSION, ABI_TAG, PLATFORM_TAG))
    whl.write_file(info_dir + '/LICENSE.txt', license_src)
    whl.write_file(info_dir + '/README.md', readme_src)
    whl.write_file_data(info_dir + '/top_level.txt', 'direct\npanda3d\npandac\n')

    whl.close()

# makewheel("1.10.0", "built_x64")