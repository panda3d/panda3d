"""
Generates a wheel (.whl) file from the output of makepanda.

Since the wheel requires special linking, this will only work if compiled with
the `--wheel` parameter.

Please keep this file work with Panda3D 1.9 until that reaches EOL.
"""
from __future__ import print_function, unicode_literals
from distutils.util import get_platform
import json

import sys
import os
from os.path import join
import shutil
import zipfile
import hashlib
import tempfile
import subprocess
from distutils.sysconfig import get_config_var
from optparse import OptionParser
from makepandacore import ColorText, LocateBinary, ParsePandaVersion, GetExtensionSuffix, SetVerbose, GetVerbose, GetMetadataValue
from base64 import urlsafe_b64encode


def get_abi_tag():
    if sys.version_info >= (3, 0):
        soabi = get_config_var('SOABI')
        if soabi and soabi.startswith('cpython-'):
            return 'cp' + soabi.split('-')[1]
        elif soabi:
            return soabi.replace('.', '_').replace('-', '_')

    soabi = 'cp%d%d' % (sys.version_info[:2])

    debug_flag = get_config_var('Py_DEBUG')
    if (debug_flag is None and hasattr(sys, 'gettotalrefcount')) or debug_flag:
        soabi += 'd'

    malloc_flag = get_config_var('WITH_PYMALLOC')
    if malloc_flag is None or malloc_flag:
        soabi += 'm'

    if sys.version_info < (3, 3):
        usize = get_config_var('Py_UNICODE_SIZE')
        if (usize is None and sys.maxunicode == 0x10ffff) or usize == 4:
            soabi += 'u'

    return soabi


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
PLUGIN_LIBS = ["pandagl", "pandagles", "pandagles2", "pandadx9", "p3tinydisplay", "p3ptloader", "p3assimp", "p3ffmpeg", "p3openal_audio", "p3fmod_audio"]

WHEEL_DATA = """Wheel-Version: 1.0
Generator: makepanda
Root-Is-Purelib: false
Tag: {0}-{1}-{2}
"""

METADATA = {
    "license": GetMetadataValue('license'),
    "name": GetMetadataValue('name'),
    "metadata_version": "2.0",
    "generator": "makepanda",
    "summary": GetMetadataValue('description'),
    "extensions": {
        "python.details": {
            "project_urls": {
                "Home": GetMetadataValue('url'),
            },
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

PANDA3D_TOOLS_INIT = """import os, sys
import panda3d

if sys.platform in ('win32', 'cygwin'):
    path_var = 'PATH'
elif sys.platform == 'darwin':
    path_var = 'DYLD_LIBRARY_PATH'
else:
    path_var = 'LD_LIBRARY_PATH'

dir = os.path.dirname(panda3d.__file__)
del panda3d
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


def scan_dependencies(pathname):
    """ Checks the named file for DLL dependencies, and adds any appropriate
    dependencies found into pluginDependencies and dependentFiles. """

    if sys.platform == "darwin":
        command = ['otool', '-XL', pathname]
    elif sys.platform in ("win32", "cygwin"):
        command = ['dumpbin', '/dependents', pathname]
    else:
        command = ['ldd', pathname]

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

    def consider_add_dependency(self, target_path, dep, search_path=None):
        """Considers adding a dependency library.
        Returns the target_path if it was added, which may be different from
        target_path if it was already added earlier, or None if it wasn't."""

        if dep in self.dep_paths:
            # Already considered this.
            return self.dep_paths[dep]

        self.dep_paths[dep] = None

        if dep.lower().startswith("python") or os.path.basename(dep).startswith("libpython"):
            # Don't include the Python library.
            return

        if sys.platform == "darwin" and dep.endswith(".so"):
            # Temporary hack for 1.9, which had link deps on modules.
            return

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
            #print("Ignoring %s" % (dep))
            return

        self.dep_paths[dep] = target_path
        self.write_file(target_path, source_path)
        return target_path

    def write_file(self, target_path, source_path):
        """Adds the given file to the .whl file."""

        # If this is a .so file, we should set the rpath appropriately.
        temp = None
        ext = os.path.splitext(source_path)[1]
        if ext in ('.so', '.dylib') or '.so.' in os.path.basename(source_path) or \
            (not ext and is_executable(source_path)):
            # Scan and add Unix dependencies.
            deps = scan_dependencies(source_path)
            for dep in deps:
                # Only include dependencies with relative path.  Otherwise we
                # end up overwriting system files like /lib/ld-linux.so.2!
                # Yes, it happened to me.
                if '/' not in dep:
                    target_dep = os.path.dirname(target_path) + '/' + dep
                    self.consider_add_dependency(target_dep, dep)

            suffix = ''
            if '.so' in os.path.basename(source_path):
                suffix = '.so'
            elif ext == '.dylib':
                suffix = '.dylib'

            temp = tempfile.NamedTemporaryFile(suffix=suffix, prefix='whl', delete=False)

            # On macOS, if no fat wheel was requested, extract the right architecture.
            if sys.platform == "darwin" and is_fat_file(source_path) \
                and not self.platform.endswith("_intel") \
                and "_fat" not in self.platform:

                if self.platform.endswith("_x86_64"):
                    arch = 'x86_64'
                else:
                    arch = self.platform.split('_')[-1]
                subprocess.call(['lipo', source_path, '-extract', arch, '-output', temp.name])
            else:
                # Otherwise, just copy it over.
                temp.write(open(source_path, 'rb').read())

            temp.write(open(source_path, 'rb').read())
            os.fchmod(temp.fileno(), os.fstat(temp.fileno()).st_mode | 0o111)
            temp.close()

            # Fix things like @loader_path/../lib references
            if sys.platform == "darwin":
                loader_path = [os.path.dirname(source_path)]
                for dep in deps:
                    if '@loader_path' not in dep:
                        continue

                    dep_path = dep.replace('@loader_path', '.')
                    target_dep = os.path.dirname(target_path) + '/' + os.path.basename(dep)
                    target_dep = self.consider_add_dependency(target_dep, dep_path, loader_path)
                    if not target_dep:
                        # It won't be included, so no use adjusting the path.
                        continue

                    new_dep = os.path.join('@loader_path', os.path.relpath(target_dep, os.path.dirname(target_path)))
                    subprocess.call(["install_name_tool", "-change", dep, new_dep, temp.name])
            else:
                subprocess.call(["strip", "-s", temp.name])
                subprocess.call(["patchelf", "--set-rpath", "$ORIGIN", temp.name])

            source_path = temp.name

        ext = ext.lower()
        if ext in ('.dll', '.pyd', '.exe'):
            # Scan and add Win32 dependencies.
            for dep in scan_dependencies(source_path):
                target_dep = os.path.dirname(target_path) + '/' + dep
                self.consider_add_dependency(target_dep, dep)

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
        digest = urlsafe_b64encode(sha.digest()).decode('ascii')
        digest = digest.rstrip('=')
        self.records.append("{0},sha256={1},{2}\n".format(target_path, digest, size))

        if GetVerbose():
            print("Adding %s from %s" % (target_path, source_path))
        self.zip_file.write(source_path, target_path)

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
        self.zip_file.writestr(target_path, source_data)

    def write_directory(self, target_dir, source_dir):
        """Adds the given directory recursively to the .whl file."""

        for root, dirs, files in os.walk(source_dir):
            for file in files:
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

        self.zip_file.writestr(record_file, "".join(self.records))
        self.zip_file.close()


def makewheel(version, output_dir, platform=None):
    if sys.platform not in ("win32", "darwin") and not sys.platform.startswith("cygwin"):
        if not LocateBinary("patchelf"):
            raise Exception("patchelf is required when building a Linux wheel.")

    if platform is None:
        # Determine the platform from the build.
        platform_dat = os.path.join(output_dir, 'tmp', 'platform.dat')
        if os.path.isfile(platform_dat):
            platform = open(platform_dat, 'r').read().strip()
        else:
            print("Could not find platform.dat in build directory")
            platform = get_platform()
            if platform.startswith("linux-"):
                # Is this manylinux1?
                if os.path.isfile("/lib/libc-2.5.so") and os.path.isdir("/opt/python"):
                    platform = platform.replace("linux", "manylinux1")

    platform = platform.replace('-', '_').replace('.', '_')

    # Global filepaths
    panda3d_dir = join(output_dir, "panda3d")
    pandac_dir = join(output_dir, "pandac")
    direct_dir = join(output_dir, "direct")
    models_dir = join(output_dir, "models")
    etc_dir = join(output_dir, "etc")
    bin_dir = join(output_dir, "bin")
    if sys.platform == "win32":
        libs_dir = join(output_dir, "bin")
    else:
        libs_dir = join(output_dir, "lib")
    license_src = "LICENSE"
    readme_src = "README.md"

    # Update relevant METADATA entries
    METADATA['version'] = version
    version_classifiers = [
        "Programming Language :: Python :: {0}".format(*sys.version_info),
        "Programming Language :: Python :: {0}.{1}".format(*sys.version_info),
    ]
    METADATA['classifiers'].extend(version_classifiers)

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
        "Author: {0}\n".format(author),
        "Author-email: {0}\n".format(email),
        "Platform: {0}\n".format(platform),
    ] + ["Classifier: {0}\n".format(c) for c in METADATA['classifiers']])

    # Zip it up and name it the right thing
    whl = WheelFile('panda3d', version, platform)
    whl.lib_path = [libs_dir]

    # Add the trees with Python modules.
    whl.write_directory('direct', direct_dir)

    # Write the panda3d tree.  We use a custom empty __init__ since the
    # default one adds the bin directory to the PATH, which we don't have.
    whl.write_file_data('panda3d/__init__.py', '')

    ext_suffix = '.pyd' if sys.platform in ('win32', 'cygwin') else '.so'

    ext_mod_dirs = [
        (panda3d_dir, 'panda3d/'),
        (get_python_ext_module_dir(), 'deploy_libs/'),
    ]
    for srcdir, targetdir in ext_mod_dirs:
        for file in os.listdir(srcdir):
            if file == '__init__.py':
                pass
            elif file.endswith(ext_suffix) or file.endswith('.py'):
                source_path = os.path.join(srcdir, file)

                if file.endswith('.pyd') and platform.startswith('cygwin'):
                    # Rename it to .dll for cygwin Python to be able to load it.
                    target_path = targetdir + os.path.splitext(file)[0] + '.dll'
                else:
                    target_path = targetdir + file

                whl.write_file(target_path, source_path)

    # Add plug-ins.
    for lib in PLUGIN_LIBS:
        plugin_name = 'lib' + lib
        if sys.platform in ('win32', 'cygwin'):
            plugin_name += '.dll'
        elif sys.platform == 'darwin':
            plugin_name += '.dylib'
        else:
            plugin_name += '.so'
        plugin_path = os.path.join(libs_dir, plugin_name)
        if os.path.isfile(plugin_path):
            whl.write_file('panda3d/' + plugin_name, plugin_path)

    # Add the .data directory, containing additional files.
    data_dir = 'panda3d-{0}.data'.format(version)
    #whl.write_directory(data_dir + '/data/etc', etc_dir)
    #whl.write_directory(data_dir + '/data/models', models_dir)

    # Actually, let's not.  That seems to install the files to the strangest
    # places in the user's filesystem.  Let's instead put them in panda3d.
    whl.write_directory('panda3d/etc', etc_dir)
    whl.write_directory('panda3d/models', models_dir)

    # Add the pandac tree for backward compatibility.
    for file in os.listdir(pandac_dir):
        if file.endswith('.py'):
            whl.write_file('pandac/' + file, os.path.join(pandac_dir, file))

    # Add a panda3d-tools directory containing the executables.
    entry_points = '[console_scripts]\n'
    entry_points += 'eggcacher = direct.directscripts.eggcacher:main\n'
    entry_points += 'pfreeze = direct.showutil.pfreeze:main\n'
    tools_init = ''
    for file in os.listdir(bin_dir):
        basename = os.path.splitext(file)[0]
        if basename in ('eggcacher', 'packpanda'):
            continue

        source_path = os.path.join(bin_dir, file)

        if is_executable(source_path):
            # Put the .exe files inside the panda3d-tools directory.
            whl.write_file('panda3d_tools/' + file, source_path)

            # Tell pip to create a wrapper script.
            funcname = basename.replace('-', '_')
            entry_points += '{0} = panda3d_tools:{1}\n'.format(basename, funcname)
            tools_init += '{0} = lambda: _exec_tool({1!r})\n'.format(funcname, file)
    entry_points += '[distutils.commands]\n'
    entry_points += 'build_apps = direct.showutil.dist:build_apps\n'
    entry_points += 'bdist_apps = direct.showutil.dist:bdist_apps\n'

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

    # Add libpython for deployment
    if sys.platform in ('win32', 'cygwin'):
        pylib_name = 'python{0}{1}.dll'.format(*sys.version_info)
        pylib_path = os.path.join(get_config_var('BINDIR'), pylib_name)
    elif sys.platform == 'darwin':
        pylib_name = 'libpython{0}.{1}.dylib'.format(*sys.version_info)
        pylib_path = os.path.join(get_config_var('LIBDIR'), pylib_name)
    else:
        pylib_name = get_config_var('LDLIBRARY')
        pylib_path = os.path.join(get_config_var('LIBDIR'), pylib_name)
    whl.write_file('/deploy_libs/' + pylib_name, pylib_path)

    whl.close()


if __name__ == "__main__":
    version = ParsePandaVersion("dtool/PandaVersion.pp")

    parser = OptionParser()
    parser.add_option('', '--version', dest = 'version', help = 'Panda3D version number (default: %s)' % (version), default = version)
    parser.add_option('', '--outputdir', dest = 'outputdir', help = 'Makepanda\'s output directory (default: built)', default = 'built')
    parser.add_option('', '--verbose', dest = 'verbose', help = 'Enable verbose output', action = 'store_true', default = False)
    parser.add_option('', '--platform', dest = 'platform', help = 'Override platform tag', default = None)
    (options, args) = parser.parse_args()

    SetVerbose(options.verbose)
    makewheel(options.version, options.outputdir, options.platform)
