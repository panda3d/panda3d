"""
Generates a wheel (.whl) file from the output of makepanda.
"""
import json
import sys
import os
from os.path import join
import zipfile
import hashlib
import tempfile
import subprocess
import time
import struct
from optparse import OptionParser
from base64 import urlsafe_b64encode
from makepandacore import LocateBinary, GetExtensionSuffix, SetVerbose, GetVerbose, GetMetadataValue, CrossCompiling, GetThirdpartyDir, SDK, GetStrip
from locations import get_config_var
from sysconfig import get_platform


def get_abi_tag():
    ver = 'cp%d%d' % sys.version_info[:2]
    if hasattr(sys, 'abiflags'):
        return ver + sys.abiflags

    gil_disabled = get_config_var("Py_GIL_DISABLED")
    if gil_disabled and int(gil_disabled):
        return ver + 't'

    return ver


def is_exe_file(path):
    return os.path.isfile(path) and path.lower().endswith('.exe')


def is_elf_file(path):
    base = os.path.basename(path)
    return os.path.isfile(path) and '.' not in base and \
           open(path, 'rb').read(4) == b'\x7FELF'


def is_macho_or_fat_file(path):
    base = os.path.basename(path)
    return os.path.isfile(path) and '.' not in base and \
           open(path, 'rb').read(4) in (b'\xFE\xED\xFA\xCE', b'\xCE\xFA\xED\xFE',
                                        b'\xFE\xED\xFA\xCF', b'\xCF\xFA\xED\xFE',
                                        b'\xCA\xFE\xBA\xBE', b'\xBE\xBA\xFE\xCA',
                                        b'\xCA\xFE\xBA\xBF', b'\xBF\xBA\xFE\xCA')

def is_fat_file(path):
    return os.path.isfile(path) and \
           open(path, 'rb').read(4) in (b'\xCA\xFE\xBA\xBE', b'\xBE\xBA\xFE\xCA',
                                        b'\xCA\xFE\xBA\xBF', b'\xBF\xBA\xFE\xCA')


def get_python_ext_module_dir():
    if CrossCompiling():
        return os.path.join(GetThirdpartyDir(), "python", "lib", SDK["PYTHONVERSION"], "lib-dynload")
    else:
        import _ctypes
        return os.path.dirname(_ctypes.__file__)


if sys.platform in ('win32', 'cygwin'):
    is_executable = is_exe_file
elif sys.platform == 'darwin':
    is_executable = is_macho_or_fat_file
else:
    is_executable = is_elf_file

# Other global parameters
PY_VERSION = "cp{0}{1}".format(*sys.version_info)
ABI_TAG = get_abi_tag()
EXCLUDE_EXT = [".pyc", ".pyo", ".N", ".prebuilt", ".xcf", ".plist", ".vcproj", ".sln"]

# Plug-ins to install.
PLUGIN_LIBS = ["pandagl", "pandagles", "pandagles2", "pandadx9", "p3tinydisplay", "p3ptloader", "p3assimp", "p3ffmpeg", "p3openal_audio", "p3fmod_audio", "p3headlessgl"]

# Libraries included in manylinux ABI that should be ignored.  See PEP 513/571/599.
MANYLINUX_LIBS = [
    "libgcc_s.so.1", "libstdc++.so.6", "libm.so.6", "libdl.so.2", "librt.so.1",
    "libcrypt.so.1", "libc.so.6", "libnsl.so.1", "libutil.so.1",
    "libpthread.so.0", "libresolv.so.2", "libX11.so.6", "libXext.so.6",
    "libXrender.so.1", "libICE.so.6", "libSM.so.6", "libGL.so.1",
    "libgobject-2.0.so.0", "libgthread-2.0.so.0", "libglib-2.0.so.0",

    # These are not mentioned in manylinux1 spec but should nonetheless always
    # be excluded.
    "linux-vdso.so.1", "linux-gate.so.1", "ld-linux.so.2", "libdrm.so.2",
    "ld-linux-x86-64.so.2", "ld-linux-aarch64.so.1",
    "libEGL.so.1", "libOpenGL.so.0", "libGLX.so.0", "libGLdispatch.so.0",
    "libGLESv2.so.2",
]

# Binaries to never scan for dependencies on non-Windows systems.
IGNORE_UNIX_DEPS_OF = [
    "panda3d_tools/pstats",
]

# Tools to exclude from the wheel.
EXCLUDE_BINARIES = [
    'eggcacher',
    'packpanda',
    'interrogate',
    'interrogate_module',
    'test_interrogate',
    'parse_file',
    'run_tests',
]

WHEEL_DATA = """Wheel-Version: 1.0
Generator: makepanda
Root-Is-Purelib: false
Tag: {0}-{1}-{2}
"""

PROJECT_URLS = dict([line.split('=', 1) for line in GetMetadataValue('project_urls').strip().splitlines()])

METADATA = {
    "license": GetMetadataValue('license'),
    "name": GetMetadataValue('name'),
    "metadata_version": "2.1",
    "generator": "makepanda",
    "summary": GetMetadataValue('description'),
    "extensions": {
        "python.details": {
            "project_urls": dict(PROJECT_URLS, Home=GetMetadataValue('url')),
            "document_names": {
                "license": "LICENSE.txt"
            },
            "contacts": [
                {
                    "role": "author",
                    "name": GetMetadataValue('author'),
                    "email": GetMetadataValue('author_email'),
                }
            ]
        }
    },
    "classifiers": GetMetadataValue('classifiers'),
}

DESCRIPTION = """
The Panda3D free 3D game engine
===============================

Panda3D is a powerful 3D engine written in C++, with a complete set of Python
bindings. Unlike other engines, these bindings are automatically generated,
meaning that they are always up-to-date and complete: all functions of the
engine can be controlled from Python. All major Panda3D applications have been
written in Python, this is the intended way of using the engine.

Panda3D now supports automatic shader generation, which now means you can use
normal maps, gloss maps, glow maps, HDR, cartoon shading, and the like without
having to write any shaders.

Panda3D is a modern engine supporting advanced features such as shaders,
stencil, and render-to-texture. Panda3D is unusual in that it emphasizes a
short learning curve, rapid development, and extreme stability and robustness.
Panda3D is free software that runs under Windows, Linux, or macOS.

The Panda3D team is very concerned with making the engine accessible to new
users. We provide a detailed manual, a complete API reference, and a large
collection of sample programs to help you get started. We have active forums,
with many helpful users, and the developers are regularly online to answer
questions.
"""

PANDA3D_TOOLS_INIT = """import os, sys
import panda3d

dir = os.path.dirname(panda3d.__file__)
del panda3d

if sys.platform in ('win32', 'cygwin'):
    path_var = 'PATH'
    if hasattr(os, 'add_dll_directory'):
        os.add_dll_directory(dir)
elif sys.platform == 'darwin':
    path_var = 'DYLD_LIBRARY_PATH'
else:
    path_var = 'LD_LIBRARY_PATH'

if not os.environ.get(path_var):
    os.environ[path_var] = dir
else:
    os.environ[path_var] = dir + os.pathsep + os.environ[path_var]

del os, sys, path_var, dir


def _exec_tool(tool):
    import os, sys
    from subprocess import Popen
    tools_dir = os.path.dirname(__file__)
    handle = Popen(sys.argv, executable=os.path.join(tools_dir, tool))
    try:
        try:
            return handle.wait()
        except KeyboardInterrupt:
            # Give the program a chance to handle the signal gracefully.
            return handle.wait()
    except:
        handle.kill()
        handle.wait()
        raise

# Register all the executables in this directory as global functions.
{0}
"""


def parse_dependencies_windows(data):
    """ Parses the given output from dumpbin /dependents to determine the list
    of dll's this executable file depends on. """

    lines = data.splitlines()
    li = 0
    while li < len(lines):
        line = lines[li]
        li += 1
        if line.find(' has the following dependencies') != -1:
            break

    if li < len(lines):
        line = lines[li]
        if line.strip() == '':
            # Skip a blank line.
            li += 1

    # Now we're finding filenames, until the next blank line.
    filenames = []
    while li < len(lines):
        line = lines[li]
        li += 1
        line = line.strip()
        if line == '':
            # We're done.
            return filenames
        filenames.append(line)

    # At least we got some data.
    return filenames


def parse_dependencies_unix(data):
    """ Parses the given output from otool -XL or ldd to determine the list of
    libraries this executable file depends on. """

    lines = data.splitlines()
    filenames = []
    for l in lines:
        l = l.strip()
        if l != "statically linked":
            filenames.append(l.split(' ', 1)[0])
    return filenames


def _scan_dependencies_elf(elf):
    deps = []
    ident = elf.read(12)

    # Make sure we read in the correct endianness and integer size
    byte_order = "<>"[ord(ident[1:2]) - 1]
    elf_class = ord(ident[0:1]) - 1 # 0 = 32-bits, 1 = 64-bits
    header_struct = byte_order + ("HHIIIIIHHHHHH", "HHIQQQIHHHHHH")[elf_class]
    section_struct = byte_order + ("4xI8xIII8xI", "4xI16xQQI12xQ")[elf_class]
    dynamic_struct = byte_order + ("iI", "qQ")[elf_class]

    type, machine, version, entry, phoff, shoff, flags, ehsize, phentsize, phnum, shentsize, shnum, shstrndx \
      = struct.unpack(header_struct, elf.read(struct.calcsize(header_struct)))
    dynamic_sections = []
    string_tables = {}

    # Seek to the section header table and find the .dynamic section.
    elf.seek(shoff)
    for i in range(shnum):
        type, offset, size, link, entsize = struct.unpack_from(section_struct, elf.read(shentsize))
        if type == 6 and link != 0: # DYNAMIC type, links to string table
            dynamic_sections.append((offset, size, link, entsize))
            string_tables[link] = None

    # Read the relevant string tables.
    for idx in string_tables.keys():
        elf.seek(shoff + idx * shentsize)
        type, offset, size, link, entsize = struct.unpack_from(section_struct, elf.read(shentsize))
        if type != 3: continue
        elf.seek(offset)
        string_tables[idx] = elf.read(size)

    # Loop through the dynamic sections to get the NEEDED entries.
    needed = []
    for offset, size, link, entsize in dynamic_sections:
        elf.seek(offset)
        data = elf.read(entsize)
        tag, val = struct.unpack_from(dynamic_struct, data)

        # Read tags until we find a NULL tag.
        while tag != 0:
            if tag == 1: # A NEEDED entry.  Read it from the string table.
                string = string_tables[link][val : string_tables[link].find(b'\0', val)]
                needed.append(string.decode('utf-8'))

            data = elf.read(entsize)
            tag, val = struct.unpack_from(dynamic_struct, data)

    elf.close()
    return needed


def scan_dependencies(pathname):
    """ Checks the named file for DLL dependencies, and adds any appropriate
    dependencies found into pluginDependencies and dependentFiles. """

    with open(pathname, 'rb') as fh:
        if fh.read(4) == b'\x7FELF':
            return _scan_dependencies_elf(fh)

    if sys.platform == "darwin":
        command = ['otool', '-XL', pathname]
    elif sys.platform in ("win32", "cygwin"):
        command = ['dumpbin', '/dependents', pathname]
    else:
        sys.exit("Don't know how to determine dependencies from %s" % (pathname))

    process = subprocess.Popen(command, stdout=subprocess.PIPE, universal_newlines=True)
    output, unused_err = process.communicate()
    retcode = process.poll()
    if retcode:
        raise subprocess.CalledProcessError(retcode, command[0], output=output)
    filenames = None

    if sys.platform in ("win32", "cygwin"):
        filenames = parse_dependencies_windows(output)
    else:
        filenames = parse_dependencies_unix(output)

    if filenames is None:
        sys.exit("Unable to determine dependencies from %s" % (pathname))

    if sys.platform == "darwin" and len(filenames) > 0:
        # Filter out the library ID.
        if os.path.basename(filenames[0]).split('.', 1)[0] == os.path.basename(pathname).split('.', 1)[0]:
            del filenames[0]

    return filenames


class WheelFile(object):
    def __init__(self, name, version, platform):
        self.name = name
        self.version = version
        self.platform = platform

        wheel_name = "{0}-{1}-{2}-{3}-{4}.whl".format(
            name, version, PY_VERSION, ABI_TAG, platform)

        print("Writing %s" % (wheel_name))
        self.zip_file = zipfile.ZipFile(wheel_name, 'w', zipfile.ZIP_DEFLATED)
        self.records = []

        # Used to locate dependency libraries.
        self.lib_path = []
        self.dep_paths = {}
        self.ignore_deps = set()

        # This can be set if a reproducible (deterministic) build is desired, in
        # which case we have to clamp all dates to the given SOURCE_DATE_EPOCH.
        epoch = os.environ.get('SOURCE_DATE_EPOCH')
        self.max_date_time = time.localtime(int(epoch) if epoch else time.time())[:6]
        if self.max_date_time < (1980, 1, 1, 0, 0, 0):
            # Earliest representable time in zip archives.
            self.max_date_time = (1980, 1, 1, 0, 0, 0)

    def consider_add_dependency(self, target_path, dep, search_path=None):
        """Considers adding a dependency library.
        Returns the target_path if it was added, which may be different from
        target_path if it was already added earlier, or None if it wasn't."""

        if dep in self.dep_paths:
            # Already considered this.
            return self.dep_paths[dep]

        self.dep_paths[dep] = None

        if dep in self.ignore_deps:
            if GetVerbose():
                print("Ignoring {0} (explicitly ignored)".format(dep))
            return

        if not self.platform.startswith("android"):
            if dep.lower().startswith("python") or os.path.basename(dep).startswith("libpython"):
                if GetVerbose():
                    print("Ignoring {0} (explicitly ignored)".format(dep))
                return

        if self.platform.startswith("macosx"):
            if dep.endswith(".so"):
                # Temporary hack for 1.9, which had link deps on modules.
                return

            if dep.startswith("/System/"):
                return

        if dep.startswith('/'):
            source_path = dep
        else:
            source_path = None

            if search_path is None:
                search_path = self.lib_path

            for lib_dir in search_path:
                # Ignore static stuff.
                path = os.path.join(lib_dir, dep)
                if os.path.isfile(path):
                    source_path = os.path.normpath(path)
                    break

        if not source_path:
            # Couldn't find library in the panda3d lib dir.
            if GetVerbose():
                print("Ignoring {0} (not in search path)".format(dep))
            return

        self.dep_paths[dep] = target_path
        self.write_file(target_path, source_path)
        return target_path

    def write_file(self, target_path, source_path):
        """Adds the given file to the .whl file."""

        orig_source_path = source_path

        # If this is a .so file, we should set the rpath appropriately.
        temp = None
        basename, ext = os.path.splitext(source_path)
        if ext in ('.so', '.dylib') or '.so.' in os.path.basename(source_path) or \
            (not ext and is_executable(source_path)):

            # Scan Unix dependencies.
            if target_path not in IGNORE_UNIX_DEPS_OF:
                deps = scan_dependencies(source_path)
            else:
                deps = []

            suffix = ''
            if '.so' in os.path.basename(source_path):
                suffix = '.so'
            elif ext == '.dylib':
                suffix = '.dylib'

            temp = tempfile.NamedTemporaryFile(suffix=suffix, prefix='whl', delete=False)

            # On macOS, if no fat wheel was requested, extract the right architecture.
            if self.platform.startswith("macosx") and is_fat_file(source_path) \
                and not self.platform.endswith("_intel") \
                and "_fat" not in self.platform \
                and "_universal" not in self.platform:

                if self.platform.endswith("_x86_64"):
                    arch = 'x86_64'
                else:
                    arch = self.platform.split('_')[-1]
                subprocess.call(['lipo', source_path, '-extract', arch, '-output', temp.name])
            else:
                # Otherwise, just copy it over.
                temp.write(open(source_path, 'rb').read())

            temp.close()
            os.chmod(temp.name, os.stat(temp.name).st_mode | 0o711)

            # Now add dependencies.  On macOS, fix @loader_path references.
            if self.platform.startswith("macosx"):
                if source_path.endswith('deploy-stubw'):
                    deps_path = '@executable_path/../Frameworks'
                else:
                    deps_path = '@loader_path'
                loader_path = [os.path.dirname(source_path)]
                for dep in deps:
                    if dep.endswith('/Python'):
                        # If this references the Python framework, change it
                        # to reference libpython instead.
                        new_dep = deps_path + '/libpython{0}.{1}.dylib'.format(*sys.version_info)

                    elif '@loader_path' in dep:
                        dep_path = dep.replace('@loader_path', '.')
                        target_dep = os.path.dirname(target_path) + '/' + os.path.basename(dep)
                        target_dep = self.consider_add_dependency(target_dep, dep_path, loader_path)
                        if not target_dep:
                            # It won't be included, so no use adjusting the path.
                            continue
                        new_dep = os.path.join(deps_path, os.path.relpath(target_dep, os.path.dirname(target_path)))

                    elif '@rpath' in dep:
                        # Unlike makepanda, CMake uses @rpath instead of
                        # @loader_path. This means we can just search for the
                        # dependencies like normal.
                        dep_path = dep.replace('@rpath', '.')
                        target_dep = os.path.dirname(target_path) + '/' + os.path.basename(dep)
                        self.consider_add_dependency(target_dep, dep_path)
                        continue

                    elif dep.startswith('/Library/Frameworks/Python.framework/') or \
                         dep.startswith('/Library/Frameworks/PythonT.framework/'):
                        # Add this dependency if it's in the Python directory.
                        target_dep = os.path.dirname(target_path) + '/' + os.path.basename(dep)
                        target_dep = self.consider_add_dependency(target_dep, dep, loader_path)
                        if not target_dep:
                            # It won't be included, so no use adjusting the path.
                            continue
                        new_dep = os.path.join(deps_path, os.path.relpath(target_dep, os.path.dirname(target_path)))

                    else:
                        if '/' in dep:
                            if GetVerbose():
                                print("Ignoring dependency %s" % (dep))
                        continue

                    subprocess.call(["install_name_tool", "-change", dep, new_dep, temp.name])

                # Make sure it has an ad-hoc code signature.
                subprocess.call(["codesign", "-f", "-s", "-", temp.name])
            else:
                # On other unixes, we just add dependencies normally.
                for dep in deps:
                    # Only include dependencies with relative path, for now.
                    if '/' in dep:
                        continue

                    if self.platform.startswith('android') and '.so.' in dep:
                        # Change .so.1.2 suffix to .so, to allow loading in .apk
                        new_dep = dep.rpartition('.so.')[0] + '.so'
                        subprocess.call(["patchelf", "--replace-needed", dep, new_dep, temp.name])
                        target_dep = os.path.dirname(target_path) + '/' + new_dep
                    else:
                        target_dep = os.path.dirname(target_path) + '/' + dep

                    self.consider_add_dependency(target_dep, dep)

                subprocess.call([GetStrip(), "-s", temp.name])

                if self.platform.startswith('android'):
                    # We must link explicitly with Python, because the usual
                    # -rdynamic trick doesn't work from a shared library loaded
                    # through ANativeActivity.
                    if suffix == '.so' and not os.path.basename(source_path).startswith('lib'):
                        pylib_name = "libpython" + get_config_var('LDVERSION') + ".so"
                        subprocess.call(["patchelf", "--add-needed", pylib_name, temp.name])
                else:
                    # On other systems, we use the rpath to force it to locate
                    # dependencies in the same directory.
                    subprocess.call(["patchelf", "--force-rpath", "--set-rpath", "$ORIGIN", temp.name])

            source_path = temp.name

        ext = ext.lower()
        if ext in ('.dll', '.pyd', '.exe'):
            # Scan and add Win32 dependencies.
            for dep in scan_dependencies(source_path):
                target_dep = os.path.dirname(target_path) + '/' + dep
                self.consider_add_dependency(target_dep, dep)

        if GetVerbose():
            print("Adding {0} from {1}".format(target_path, orig_source_path))

        zinfo = zipfile.ZipInfo.from_file(source_path, target_path)
        zinfo.compress_type = self.zip_file.compression
        if zinfo.date_time > self.max_date_time:
            zinfo.date_time = self.max_date_time

        # Copy the data to the zip file, while also calculating the SHA-256.
        size = 0
        sha = hashlib.sha256()
        with open(source_path, 'rb') as source_fp, self.zip_file.open(zinfo, 'w') as target_fp:
            data = source_fp.read(1024 * 1024)
            while data:
                size += len(data)
                target_fp.write(data)
                sha.update(data)
                data = source_fp.read(1024 * 1024)

        # Save it in PEP-0376 format for writing out later.
        digest = urlsafe_b64encode(sha.digest()).decode('ascii')
        digest = digest.rstrip('=')
        self.records.append("{0},sha256={1},{2}\n".format(target_path, digest, size))

        #if temp:
        #    os.unlink(temp.name)

    def write_file_data(self, target_path, source_data):
        """Adds the given file from a string."""

        sha = hashlib.sha256()
        sha.update(source_data.encode())
        digest = urlsafe_b64encode(sha.digest()).decode('ascii')
        digest = digest.rstrip('=')
        self.records.append("{0},sha256={1},{2}\n".format(target_path, digest, len(source_data)))

        if GetVerbose():
            print("Adding %s from data" % target_path)

        zinfo = zipfile.ZipInfo(filename=target_path,
                                date_time=self.max_date_time)
        zinfo.compress_type = self.zip_file.compression
        zinfo.external_attr = 0o600 << 16
        self.zip_file.writestr(zinfo, source_data)

    def write_directory(self, target_dir, source_dir):
        """Adds the given directory recursively to the .whl file."""

        for root, dirs, files in os.walk(source_dir):
            dirs.sort()
            for file in sorted(files):
                if os.path.splitext(file)[1] in EXCLUDE_EXT:
                    continue

                source_path = os.path.join(root, file)
                target_path = os.path.join(target_dir, os.path.relpath(source_path, source_dir))
                target_path = target_path.replace('\\', '/')
                self.write_file(target_path, source_path)

    def close(self):
        # Write the RECORD file.
        record_file = "{0}-{1}.dist-info/RECORD".format(self.name, self.version)
        self.records.append(record_file + ",,\n")

        zinfo = zipfile.ZipInfo(filename=record_file,
                                date_time=self.max_date_time)
        zinfo.compress_type = self.zip_file.compression
        zinfo.external_attr = 0o600 << 16
        self.zip_file.writestr(zinfo, "".join(self.records))
        self.zip_file.close()


def makewheel(version, output_dir, platform=None):
    if sys.platform not in ("win32", "darwin") and not sys.platform.startswith("cygwin"):
        if not LocateBinary("patchelf"):
            raise Exception("patchelf is required when building a Linux wheel.")

    if sys.version_info < (3, 8):
        raise Exception("Python 3.8 or higher is required to produce a wheel.")

    if platform is None:
        # Determine the platform from the build.
        platform_dat = os.path.join(output_dir, 'tmp', 'platform.dat')
        cmake_cache = os.path.join(output_dir, 'CMakeCache.txt')
        if os.path.isfile(platform_dat):
            # This is written by makepanda.
            platform = open(platform_dat, 'r').read().strip()
        elif os.path.isfile(cmake_cache):
            # This variable is written to the CMake cache by Package.cmake.
            for line in open(cmake_cache, 'r').readlines():
                if line.startswith('PYTHON_PLATFORM_TAG:STRING='):
                    platform = line[27:].strip()
                    break
            if not platform:
                raise Exception("Could not find PYTHON_PLATFORM_TAG in CMakeCache.txt, specify --platform manually.")
        else:
            print("Could not find platform.dat or CMakeCache.txt in build directory")
            platform = get_platform()
            if platform.startswith("linux-") and os.path.isdir("/opt/python"):
                # Is this manylinux?
                if os.path.isfile("/lib/libc-2.5.so") or os.path.isfile("/lib64/libc-2.5.so"):
                    platform = platform.replace("linux", "manylinux1")
                elif os.path.isfile("/lib/libc-2.12.so") or os.path.isfile("/lib64/libc-2.12.so"):
                    platform = platform.replace("linux", "manylinux2010")
                elif os.path.isfile("/lib/libc-2.17.so") or os.path.isfile("/lib64/libc-2.17.so"):
                    platform = platform.replace("linux", "manylinux2014")
                elif os.path.isfile("/lib/i386-linux-gnu/libc-2.24.so") or os.path.isfile("/lib/x86_64-linux-gnu/libc-2.24.so"):
                    platform = platform.replace("linux", "manylinux_2_24")
                elif os.path.isfile("/lib64/libc-2.28.so") and os.path.isfile('/etc/almalinux-release'):
                    platform = platform.replace("linux", "manylinux_2_28")

    platform = platform.replace('-', '_').replace('.', '_')

    is_windows = platform == 'win32' \
        or platform.startswith('win_') \
        or platform.startswith('cygwin_')
    is_macosx = platform.startswith('macosx_')
    is_android = platform.startswith('android_')

    # Global filepaths
    panda3d_dir = join(output_dir, "panda3d")
    pandac_dir = join(output_dir, "pandac")
    direct_dir = join(output_dir, "direct")
    models_dir = join(output_dir, "models")
    etc_dir = join(output_dir, "etc")
    bin_dir = join(output_dir, "bin")
    if is_windows:
        libs_dir = join(output_dir, "bin")
    else:
        libs_dir = join(output_dir, "lib")
    ext_mod_dir = get_python_ext_module_dir()
    license_src = "LICENSE"
    readme_src = "README.md"

    # Update relevant METADATA entries
    METADATA['version'] = version

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
        "Home-page: {0}\n".format(homepage),
    ] + ["Project-URL: {0}, {1}\n".format(*url) for url in PROJECT_URLS.items()] + [
        "Author: {0}\n".format(author),
        "Author-email: {0}\n".format(email),
        "Platform: {0}\n".format(platform),
    ] + ["Classifier: {0}\n".format(c) for c in METADATA['classifiers']])

    metadata += '\n' + DESCRIPTION.strip() + '\n'

    # Zip it up and name it the right thing
    whl = WheelFile('panda3d', version, platform)
    whl.lib_path = [libs_dir]

    if is_windows:
        whl.lib_path.append(ext_mod_dir)

    if platform.startswith("manylinux"):
        # On manylinux1, we pick up all libraries except for the ones specified
        # by the manylinux1 ABI.
        whl.lib_path.append("/usr/local/lib")

        if platform.endswith("_x86_64"):
            whl.lib_path += ["/lib64", "/usr/lib64"]
        else:
            whl.lib_path += ["/lib", "/usr/lib"]

        whl.ignore_deps.update(MANYLINUX_LIBS)

    # Add libpython for deployment.
    suffix = ''
    gil_disabled = get_config_var("Py_GIL_DISABLED")
    if gil_disabled and int(gil_disabled):
        suffix = 't'

    if is_windows:
        pylib_name = 'python{0}{1}{2}.dll'.format(sys.version_info[0], sys.version_info[1], suffix)
        pylib_path = os.path.join(get_config_var('BINDIR'), pylib_name)
    elif is_macosx:
        pylib_name = 'libpython{0}.{1}{2}.dylib'.format(sys.version_info[0], sys.version_info[1], suffix)
        pylib_path = os.path.join(get_config_var('LIBDIR'), pylib_name)
    elif is_android and CrossCompiling():
        pylib_name = 'libpython{0}.{1}{2}.so'.format(sys.version_info[0], sys.version_info[1], suffix)
        pylib_path = os.path.join(GetThirdpartyDir(), 'python', 'lib', pylib_name)
    else:
        pylib_name = get_config_var('LDLIBRARY')
        pylib_arch = get_config_var('MULTIARCH')
        libdir = get_config_var('LIBDIR')
        if pylib_arch and os.path.exists(os.path.join(libdir, pylib_arch, pylib_name)):
            pylib_path = os.path.join(libdir, pylib_arch, pylib_name)
        else:
            pylib_path = os.path.join(libdir, pylib_name)

    # If Python was linked statically, we don't need to include this.
    if not pylib_name.endswith('.a'):
        whl.write_file('deploy_libs/' + pylib_name, pylib_path)

    # Add the trees with Python modules.
    whl.write_directory('direct', direct_dir)

    # Write the panda3d tree.  We use a custom empty __init__ since the
    # default one adds the bin directory to the PATH, which we don't have.
    p3d_init = """"Python bindings for the Panda3D libraries"

__version__ = '{0}'
""".format(version)

    if '27' in ABI_TAG:
        p3d_init += """
if __debug__:
    if 1 / 2 == 0:
        raise ImportError(\"Python 2 is not supported.\")
"""

    whl.write_file_data('panda3d/__init__.py', p3d_init)

    # Copy the extension modules from the panda3d directory.
    ext_suffix = GetExtensionSuffix()

    for file in sorted(os.listdir(panda3d_dir)):
        if file == '__init__.py':
            pass
        elif file.endswith('.py') or (file.endswith(ext_suffix) and '.' not in file[:-len(ext_suffix)]):
            source_path = os.path.join(panda3d_dir, file)

            if file.endswith('.pyd') and platform.startswith('cygwin'):
                # Rename it to .dll for cygwin Python to be able to load it.
                target_path = 'panda3d/' + os.path.splitext(file)[0] + '.dll'
            elif file.endswith(ext_suffix) and platform.startswith('android'):
                # Strip the extension suffix on Android.
                target_path = 'panda3d/' + file[:-len(ext_suffix)] + '.so'
            else:
                target_path = 'panda3d/' + file

            whl.write_file(target_path, source_path)

    # And copy the extension modules from the Python installation into the
    # deploy_libs directory, for use by deploy-ng.
    ext_suffix = '.pyd' if is_windows else '.so'

    for file in sorted(os.listdir(ext_mod_dir)):
        if file.endswith(ext_suffix):
            if file.startswith('_tkinter.'):
                # Tkinter is supplied in a separate wheel.
                continue

            source_path = os.path.join(ext_mod_dir, file)

            if file.endswith('.pyd') and platform.startswith('cygwin'):
                # Rename it to .dll for cygwin Python to be able to load it.
                target_path = 'deploy_libs/' + os.path.splitext(file)[0] + '.dll'
            else:
                target_path = 'deploy_libs/' + file

            whl.write_file(target_path, source_path)

    # Include the special sysconfigdata module.
    if os.name == 'posix':
        import sysconfig

        if hasattr(sysconfig, '_get_sysconfigdata_name'):
            modname = sysconfig._get_sysconfigdata_name() + '.py'
        else:
            modname = '_sysconfigdata.py'

        for entry in sys.path:
            source_path = os.path.join(entry, modname)
            if os.path.isfile(source_path):
                whl.write_file('deploy_libs/' + modname, source_path)
                break

    # Add plug-ins.
    for lib in PLUGIN_LIBS:
        plugin_name = 'lib' + lib
        if is_windows:
            plugin_name += '.dll'
        elif is_macosx:
            plugin_name += '.dylib'
        else:
            plugin_name += '.so'
        plugin_path = os.path.join(libs_dir, plugin_name)
        if os.path.isfile(plugin_path):
            whl.write_file('panda3d/' + plugin_name, plugin_path)

    if platform.startswith('android'):
        deploy_stub_path = os.path.join(libs_dir, 'libdeploy-stubw.so')
        if os.path.isfile(deploy_stub_path):
            whl.write_file('deploy_libs/libdeploy-stubw.so', deploy_stub_path)

        classes_dex_path = os.path.join(output_dir, 'classes.dex')
        if os.path.isfile(classes_dex_path):
            whl.write_file('deploy_libs/classes.dex', classes_dex_path)

    # Add the .data directory, containing additional files.
    data_dir = 'panda3d-{0}.data'.format(version)
    #whl.write_directory(data_dir + '/data/etc', etc_dir)
    #whl.write_directory(data_dir + '/data/models', models_dir)

    # Actually, let's not.  That seems to install the files to the strangest
    # places in the user's filesystem.  Let's instead put them in panda3d.
    whl.write_directory('panda3d/etc', etc_dir)
    whl.write_directory('panda3d/models', models_dir)

    # Add the pandac tree for backward compatibility.
    for file in sorted(os.listdir(pandac_dir)):
        if file.endswith('.py'):
            whl.write_file('pandac/' + file, os.path.join(pandac_dir, file))

    # Let's also add the interrogate databases.
    input_dir = os.path.join(pandac_dir, 'input')
    if os.path.isdir(input_dir):
        for file in sorted(os.listdir(input_dir)):
            if file.endswith('.in'):
                whl.write_file('pandac/input/' + file, os.path.join(input_dir, file))

    # Add a panda3d-tools directory containing the executables.
    entry_points = '[console_scripts]\n'
    entry_points += 'eggcacher = direct.directscripts.eggcacher:main\n'
    entry_points += 'pfreeze = direct.dist.pfreeze:main\n'
    tools_init = ''
    for file in sorted(os.listdir(bin_dir)):
        basename = os.path.splitext(file)[0]
        if basename in EXCLUDE_BINARIES:
            continue

        source_path = os.path.join(bin_dir, file)

        if is_executable(source_path):
            # Put the .exe files inside the panda3d-tools directory.
            whl.write_file('panda3d_tools/' + file, source_path)

            if basename.endswith('_bin'):
                # These tools won't be invoked by the user directly.
                continue

            # Tell pip to create a wrapper script.
            funcname = basename.replace('-', '_')
            entry_points += '{0} = panda3d_tools:{1}\n'.format(basename, funcname)
            tools_init += '{0} = lambda: _exec_tool({1!r})\n'.format(funcname, file)

    entry_points += '[distutils.commands]\n'
    entry_points += 'build_apps = direct.dist.commands:build_apps\n'
    entry_points += 'bdist_apps = direct.dist.commands:bdist_apps\n'
    entry_points += '[setuptools.finalize_distribution_options]\n'
    entry_points += 'build_apps = direct.dist._dist_hooks:finalize_distribution_options\n'

    whl.write_file_data('panda3d_tools/__init__.py', PANDA3D_TOOLS_INIT.format(tools_init))

    # Add the dist-info directory last.
    info_dir = 'panda3d-{0}.dist-info'.format(version)
    whl.write_file_data(info_dir + '/entry_points.txt', entry_points)
    whl.write_file_data(info_dir + '/metadata.json', json.dumps(METADATA, indent=4, separators=(',', ': ')))
    whl.write_file_data(info_dir + '/METADATA', metadata)
    whl.write_file_data(info_dir + '/WHEEL', WHEEL_DATA.format(PY_VERSION, ABI_TAG, platform))
    whl.write_file(info_dir + '/LICENSE.txt', license_src)
    whl.write_file(info_dir + '/README.md', readme_src)
    whl.write_file_data(info_dir + '/top_level.txt', 'direct\npanda3d\npandac\npanda3d_tools\n')

    whl.close()


if __name__ == "__main__":
    version = GetMetadataValue('version')

    parser = OptionParser()
    parser.add_option('', '--version', dest = 'version', help = 'Panda3D version number (default: %s)' % (version), default = version)
    parser.add_option('', '--outputdir', dest = 'outputdir', help = 'Makepanda\'s output directory (default: built)', default = 'built')
    parser.add_option('', '--verbose', dest = 'verbose', help = 'Enable verbose output', action = 'store_true', default = False)
    parser.add_option('', '--platform', dest = 'platform', help = 'Override platform tag', default = None)
    (options, args) = parser.parse_args()

    SetVerbose(options.verbose)
    makewheel(options.version, options.outputdir, options.platform)
