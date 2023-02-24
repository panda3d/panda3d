"""Extends setuptools with the ``build_apps`` and ``bdist_apps`` commands.

See the :ref:`distribution` section of the programming manual for information
on how to use these commands.
"""

import os
import plistlib
import sys
import subprocess
import zipfile
import re
import shutil
import stat
import struct
import imp
import string
import tempfile

import setuptools
import distutils.log

from . import FreezeTool
from . import pefile
from . import installers
from .icon import Icon
import panda3d.core as p3d


def _parse_list(input):
    if isinstance(input, str):
        input = input.strip().replace(',', '\n')
        if input:
            return [item.strip() for item in input.split('\n') if item.strip()]
        else:
            return []
    else:
        return input


def _parse_dict(input):
    if isinstance(input, dict):
        return input
    d = {}
    for item in _parse_list(input):
        key, sep, value = item.partition('=')
        d[key.strip()] = value.strip()
    return d


def _register_python_loaders():
    # We need this method so that we don't depend on direct.showbase.Loader.
    if getattr(_register_python_loaders, 'done', None):
        return

    _register_python_loaders.done = True

    registry = p3d.LoaderFileTypeRegistry.getGlobalPtr()

    import pkg_resources
    for entry_point in pkg_resources.iter_entry_points('panda3d.loaders'):
        registry.register_deferred_type(entry_point)


def _model_to_bam(_build_cmd, srcpath, dstpath):
    if dstpath.endswith('.gz') or dstpath.endswith('.pz'):
        dstpath = dstpath[:-3]
    dstpath = dstpath + '.bam'

    src_fn = p3d.Filename.from_os_specific(srcpath)
    dst_fn = p3d.Filename.from_os_specific(dstpath)

    _register_python_loaders()

    loader = p3d.Loader.get_global_ptr()
    options = p3d.LoaderOptions(p3d.LoaderOptions.LF_report_errors |
                                p3d.LoaderOptions.LF_no_ram_cache)
    node = loader.load_sync(src_fn, options)
    if not node:
        raise IOError('Failed to load model: %s' % (srcpath))

    if not p3d.NodePath(node).write_bam_file(dst_fn):
        raise IOError('Failed to write .bam file: %s' % (dstpath))


macosx_binary_magics = (
    b'\xFE\xED\xFA\xCE', b'\xCE\xFA\xED\xFE',
    b'\xFE\xED\xFA\xCF', b'\xCF\xFA\xED\xFE',
    b'\xCA\xFE\xBA\xBE', b'\xBE\xBA\xFE\xCA',
    b'\xCA\xFE\xBA\xBF', b'\xBF\xBA\xFE\xCA')

# Some dependencies need data directories to be extracted. This dictionary maps
# modules with data to extract. The values are lists of tuples of the form
# (source_pattern, destination_pattern, flags). The flags is a set of strings.

PACKAGE_DATA_DIRS = {
    'matplotlib':  [('matplotlib/mpl-data/*', 'mpl-data', {})],
    'jsonschema':  [('jsonschema/schemas/*', 'schemas', {})],
    'cefpython3': [
        ('cefpython3/*.pak', '', {}),
        ('cefpython3/*.dat', '', {}),
        ('cefpython3/*.bin', '', {}),
        ('cefpython3/*.dll', '', {}),
        ('cefpython3/libcef.so', '', {}),
        ('cefpython3/LICENSE.txt', '', {}),
        ('cefpython3/License', '', {}),
        ('cefpython3/subprocess*', '', {'PKG_DATA_MAKE_EXECUTABLE'}),
        ('cefpython3/locals/*', 'locals', {}),
        ('cefpython3/Chromium Embedded Framework.framework/Resources', 'Chromium Embedded Framework.framework/Resources', {}),
        ('cefpython3/Chromium Embedded Framework.framework/Chromium Embedded Framework', '', {'PKG_DATA_MAKE_EXECUTABLE'}),
    ],
    'pytz': [('pytz/zoneinfo/*', 'zoneinfo', ())],
    'certifi': [('certifi/cacert.pem', '', {})],
    '_tkinter_ext': [('_tkinter_ext/tcl/**', 'tcl', {})],
}

# Some dependencies have extra directories that need to be scanned for DLLs.
# This dictionary maps wheel basenames (ie. the part of the .whl basename
# before the first hyphen) to a list of tuples, the first value being the
# directory inside the wheel, the second being which wheel to look in (or
# None to look in its own wheel).

PACKAGE_LIB_DIRS = {
    'scipy':  [('scipy/extra-dll', None)],
    'PyQt5':  [('PyQt5/Qt5/bin', 'PyQt5_Qt5')],
}

SITE_PY = """
import sys
from _frozen_importlib import _imp, FrozenImporter

sys.frozen = True

if sys.platform == 'win32' and sys.version_info < (3, 10):
    # Make sure the preferred encoding is something we actually support.
    import _bootlocale
    enc = _bootlocale.getpreferredencoding().lower()
    if enc != 'utf-8' and not _imp.is_frozen('encodings.%s' % (enc)):
        def getpreferredencoding(do_setlocale=True):
            return 'mbcs'
        _bootlocale.getpreferredencoding = getpreferredencoding

# Alter FrozenImporter to give a __file__ property to frozen modules.
_find_spec = FrozenImporter.find_spec

def find_spec(fullname, path=None, target=None):
    spec = _find_spec(fullname, path=path, target=target)
    if spec:
        spec.has_location = True
        spec.origin = sys.executable
    return spec

def get_data(path):
    with open(path, 'rb') as fp:
        return fp.read()

FrozenImporter.find_spec = find_spec
FrozenImporter.get_data = get_data
"""

SITE_PY_ANDROID = """
import sys, os
from _frozen_importlib import _imp, FrozenImporter
from importlib import _bootstrap_external
from importlib.abc import Loader, MetaPathFinder
from importlib.machinery import ModuleSpec
from io import RawIOBase, TextIOWrapper

from android_log import write as android_log_write


sys.frozen = True

# Temporary hack for plyer to detect Android, see kivy/plyer#670
os.environ['ANDROID_ARGUMENT'] = ''


# Replace stdout/stderr with something that writes to the Android log.
class AndroidLogStream:
    closed = False
    encoding = 'utf-8'

    def __init__(self, prio, tag):
        self.prio = prio
        self.tag = tag
        self.buffer = ''

    def isatty(self):
        return False

    def write(self, text):
        self.writelines(text.split('\\n'))

    def writelines(self, lines):
        num_lines = len(lines)
        if num_lines == 1:
            self.buffer += lines[0]
        elif num_lines > 1:
            android_log_write(self.prio, self.tag, self.buffer + lines[0])
            for line in lines[1:-1]:
                android_log_write(self.prio, self.tag, line)
            self.buffer = lines[-1]

    def flush(self):
        pass

    def seekable(self):
        return False

    def readable(self):
        return False

    def writable(self):
        return True

sys.stdout = AndroidLogStream(2, 'Python')
sys.stderr = AndroidLogStream(3, 'Python')


# Alter FrozenImporter to give a __file__ property to frozen modules.
_find_spec = FrozenImporter.find_spec

def find_spec(fullname, path=None, target=None):
    spec = _find_spec(fullname, path=path, target=target)
    if spec:
        spec.has_location = True
        spec.origin = sys.executable
    return spec

def get_data(path):
    with open(path, 'rb') as fp:
        return fp.read()

FrozenImporter.find_spec = find_spec
FrozenImporter.get_data = get_data


class AndroidExtensionFinder(MetaPathFinder):
    @classmethod
    def find_spec(cls, fullname, path=None, target=None):
        soname = 'libpy.' + fullname + '.so'
        path = os.path.join(os.path.dirname(sys.executable), soname)

        if os.path.exists(path):
            loader = _bootstrap_external.ExtensionFileLoader(fullname, path)
            return ModuleSpec(fullname, loader, origin=path)


sys.meta_path.append(AndroidExtensionFinder)
"""


class build_apps(setuptools.Command):
    description = 'build Panda3D applications'
    user_options = [
        ('build-base=', None, 'directory to build applications in'),
        ('requirements-path=', None, 'path to requirements.txt file for pip'),
        ('platforms=', 'p', 'a list of platforms to build for'),
    ]
    default_file_handlers = {
    }

    def initialize_options(self):
        self.build_base = os.path.join(os.getcwd(), 'build')
        self.application_id = None
        self.android_abis = None
        self.android_debuggable = False
        self.android_version_code = 1
        self.android_min_sdk_version = 21
        self.android_max_sdk_version = None
        self.android_target_sdk_version = 30
        self.gui_apps = {}
        self.console_apps = {}
        self.macos_main_app = None
        self.rename_paths = {}
        self.include_patterns = []
        self.exclude_patterns = []
        self.include_modules = {}
        self.exclude_modules = {}
        self.icons = {}
        self.platforms = [
            'manylinux2014_x86_64',
            'macosx_10_9_x86_64',
            'win_amd64',
        ]
        self.plugins = []
        self.embed_prc_data = True
        self.extra_prc_files = []
        self.extra_prc_data = ''
        self.default_prc_dir = None
        self.log_filename = None
        self.log_filename_strftime = True
        self.log_append = False
        self.prefer_discrete_gpu = False
        self.requirements_path = os.path.join(os.getcwd(), 'requirements.txt')
        self.strip_docstrings = True
        self.use_optimized_wheels = True
        self.optimized_wheel_index = ''
        self.pypi_extra_indexes = [
            'https://archive.panda3d.org/thirdparty',
        ]
        self.file_handlers = {}
        self.bam_model_extensions = ['.egg', '.gltf', '.glb']
        self.exclude_dependencies = [
            # Windows
            'kernel32.dll', 'user32.dll', 'wsock32.dll', 'ws2_32.dll',
            'advapi32.dll', 'opengl32.dll', 'glu32.dll', 'gdi32.dll',
            'shell32.dll', 'ntdll.dll', 'ws2help.dll', 'rpcrt4.dll',
            'imm32.dll', 'ddraw.dll', 'shlwapi.dll', 'secur32.dll',
            'dciman32.dll', 'comdlg32.dll', 'comctl32.dll', 'ole32.dll',
            'oleaut32.dll', 'gdiplus.dll', 'winmm.dll', 'iphlpapi.dll',
            'msvcrt.dll', 'kernelbase.dll', 'msimg32.dll', 'msacm32.dll',
            'setupapi.dll', 'version.dll', 'userenv.dll', 'netapi32.dll',
            'crypt32.dll', 'bcrypt.dll',

            # manylinux1/linux
            'libdl.so.*', 'libstdc++.so.*', 'libm.so.*', 'libgcc_s.so.*',
            'libpthread.so.*', 'libc.so.*', 'ld-linux-x86-64.so.*',
            'libgl.so.*', 'libx11.so.*', 'libncursesw.so.*', 'libz.so.*',
            'librt.so.*', 'libutil.so.*', 'libnsl.so.1', 'libXext.so.6',
            'libXrender.so.1', 'libICE.so.6', 'libSM.so.6', 'libEGL.so.1',
            'libOpenGL.so.0', 'libGLdispatch.so.0', 'libGLX.so.0',
            'libgobject-2.0.so.0', 'libgthread-2.0.so.0', 'libglib-2.0.so.0',

            # macOS
            '/usr/lib/libc++.1.dylib',
            '/usr/lib/libstdc++.*.dylib',
            '/usr/lib/libz.*.dylib',
            '/usr/lib/libobjc.*.dylib',
            '/usr/lib/libSystem.*.dylib',
            '/usr/lib/libbz2.*.dylib',
            '/usr/lib/libedit.*.dylib',
            '/usr/lib/libffi.dylib',
            '/usr/lib/libauditd.0.dylib',
            '/usr/lib/libgermantok.dylib',
            '/usr/lib/liblangid.dylib',
            '/usr/lib/libarchive.2.dylib',
            '/usr/lib/libipsec.A.dylib',
            '/usr/lib/libpanel.5.4.dylib',
            '/usr/lib/libiodbc.2.1.18.dylib',
            '/usr/lib/libhunspell-1.2.0.0.0.dylib',
            '/usr/lib/libsqlite3.dylib',
            '/usr/lib/libpam.1.dylib',
            '/usr/lib/libtidy.A.dylib',
            '/usr/lib/libDHCPServer.A.dylib',
            '/usr/lib/libpam.2.dylib',
            '/usr/lib/libXplugin.1.dylib',
            '/usr/lib/libxslt.1.dylib',
            '/usr/lib/libiodbcinst.2.1.18.dylib',
            '/usr/lib/libBSDPClient.A.dylib',
            '/usr/lib/libsandbox.1.dylib',
            '/usr/lib/libform.5.4.dylib',
            '/usr/lib/libbsm.0.dylib',
            '/usr/lib/libMatch.1.dylib',
            '/usr/lib/libresolv.9.dylib',
            '/usr/lib/libcharset.1.dylib',
            '/usr/lib/libxml2.2.dylib',
            '/usr/lib/libiconv.2.dylib',
            '/usr/lib/libScreenReader.dylib',
            '/usr/lib/libdtrace.dylib',
            '/usr/lib/libicucore.A.dylib',
            '/usr/lib/libsasl2.2.dylib',
            '/usr/lib/libpcap.A.dylib',
            '/usr/lib/libexslt.0.dylib',
            '/usr/lib/libcurl.4.dylib',
            '/usr/lib/libncurses.5.4.dylib',
            '/usr/lib/libxar.1.dylib',
            '/usr/lib/libmenu.5.4.dylib',
            '/System/Library/**',

            # Android
            'libc.so', 'libm.so', 'liblog.so', 'libdl.so', 'libandroid.so',
            'libGLESv1_CM.so', 'libGLESv2.so', 'libjnigraphics.so', 'libEGL.so',
            'libOpenSLES.so', 'libandroid.so', 'libOpenMAXAL.so', 'libz.so',
        ]

        self.package_data_dirs = {}
        self.hidden_imports = {}

        # We keep track of the zip files we've opened.
        self._zip_files = {}

    def _get_zip_file(self, path):
        if path in self._zip_files:
            return self._zip_files[path]

        zip = zipfile.ZipFile(path)
        self._zip_files[path] = zip
        return zip

    def finalize_options(self):
        # We need to massage the inputs a bit in case they came from a
        # setup.cfg file.
        self.gui_apps = _parse_dict(self.gui_apps)
        self.console_apps = _parse_dict(self.console_apps)

        self.rename_paths = _parse_dict(self.rename_paths)
        self.include_patterns = _parse_list(self.include_patterns)
        self.exclude_patterns = _parse_list(self.exclude_patterns)
        self.include_modules = {
            key: _parse_list(value)
            for key, value in _parse_dict(self.include_modules).items()
        }
        self.exclude_modules = {
            key: _parse_list(value)
            for key, value in _parse_dict(self.exclude_modules).items()
        }
        self.icons = _parse_dict(self.icons)
        self.platforms = _parse_list(self.platforms)
        self.plugins = _parse_list(self.plugins)
        self.extra_prc_files = _parse_list(self.extra_prc_files)
        self.hidden_imports = {
            key: _parse_list(value)
            for key, value in _parse_dict(self.hidden_imports).items()
        }

        if self.default_prc_dir is None:
            self.default_prc_dir = '<auto>etc' if not self.embed_prc_data else ''

        num_gui_apps = len(self.gui_apps)
        num_console_apps = len(self.console_apps)

        if not self.macos_main_app:
            if num_gui_apps > 1:
                assert False, 'macos_main_app must be defined if more than one gui_app is defined'
            elif num_gui_apps == 1:
                self.macos_main_app = list(self.gui_apps.keys())[0]

        use_pipenv = (
            'Pipfile' in os.path.basename(self.requirements_path) or
            not os.path.exists(self.requirements_path) and os.path.exists('Pipfile')
        )
        if use_pipenv:
            reqspath = os.path.join(self.build_base, 'requirements.txt')
            with open(reqspath, 'w') as reqsfile:
                subprocess.check_call(['pipenv', 'lock', '--requirements'], stdout=reqsfile)
            self.requirements_path = reqspath

        if self.use_optimized_wheels:
            if not self.optimized_wheel_index:
                # Try to find an appropriate wheel index

                # Start with the release index
                self.optimized_wheel_index = 'https://archive.panda3d.org/simple/opt'

                # See if a buildbot build is being used
                with open(self.requirements_path) as reqsfile:
                    reqsdata = reqsfile.read()
                matches = re.search(r'--extra-index-url (https*://archive.panda3d.org/.*\b)', reqsdata)
                if matches and matches.group(1):
                    self.optimized_wheel_index = matches.group(1)
                    if not matches.group(1).endswith('opt'):
                        self.optimized_wheel_index += '/opt'

            assert self.optimized_wheel_index, 'An index for optimized wheels must be defined if use_optimized_wheels is set'

        assert os.path.exists(self.requirements_path), 'Requirements.txt path does not exist: {}'.format(self.requirements_path)
        assert num_gui_apps + num_console_apps != 0, 'Must specify at least one app in either gui_apps or console_apps'

        self.exclude_dependencies = [p3d.GlobPattern(i) for i in self.exclude_dependencies]
        for glob in self.exclude_dependencies:
            glob.case_sensitive = False

        # bam_model_extensions registers a 2bam handler for each given extension.
        # They can override a default handler, but not a custom handler.
        if self.bam_model_extensions:
            for ext in self.bam_model_extensions:
                ext = '.' + ext.lstrip('.')
                assert ext not in self.file_handlers, \
                    'Extension {} occurs in both file_handlers and bam_model_extensions!'.format(ext)
                self.file_handlers[ext] = _model_to_bam

        tmp = self.default_file_handlers.copy()
        tmp.update(self.file_handlers)
        self.file_handlers = tmp

        tmp = PACKAGE_DATA_DIRS.copy()
        tmp.update(self.package_data_dirs)
        self.package_data_dirs = tmp

        # Default to all supported ABIs (for the given Android version).
        if self.android_max_sdk_version and self.android_max_sdk_version < 21:
            assert self.android_max_sdk_version >= 19, \
                'Panda3D requires at least Android API level 19!'

            if self.android_abis:
                for abi in self.android_abis:
                    assert abi not in ('mips64', 'x86_64', 'arm64-v8a'), \
                        f'{abi} was not a valid Android ABI before Android 21!'
            else:
                self.android_abis = ['armeabi-v7a', 'x86']

        elif not self.android_abis:
            self.android_abis = ['arm64-v8a', 'armeabi-v7a', 'x86_64', 'x86']

        supported_abis = 'armeabi', 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64', 'mips', 'mips64'
        unsupported_abis = set(self.android_abis) - set(supported_abis)
        if unsupported_abis:
            raise ValueError(f'Unrecognized value(s) for android_abis: {", ".join(unsupported_abis)}\n'
                             f'Valid ABIs are: {", ".join(supported_abis)}')

        self.icon_objects = {}
        for app, iconpaths in self.icons.items():
            if not isinstance(iconpaths, list) and not isinstance(iconpaths, tuple):
                iconpaths = (iconpaths,)

            iconobj = Icon()
            for iconpath in iconpaths:
                iconobj.addImage(iconpath)

            iconobj.generateMissingImages()
            self.icon_objects[app] = iconobj

    def run(self):
        self.announce('Building platforms: {0}'.format(','.join(self.platforms)), distutils.log.INFO)

        for platform in self.platforms:
            # Create the build directory, or ensure it is empty.
            build_dir = os.path.join(self.build_base, platform)

            if os.path.exists(build_dir):
                for entry in os.listdir(build_dir):
                    path = os.path.join(build_dir, entry)
                    if os.path.islink(path) or os.path.isfile(path):
                        os.unlink(path)
                    else:
                        shutil.rmtree(path)
            else:
                os.makedirs(build_dir)

            if platform == 'android':
                # Make a multi-arch build for Android.
                data_dir = os.path.join(build_dir, 'assets')
                os.makedirs(data_dir, exist_ok=True)

                for abi in self.android_abis:
                    lib_dir = os.path.join(build_dir, 'lib', abi)
                    os.makedirs(lib_dir, exist_ok=True)

                    suffix = None
                    if abi == 'arm64-v8a':
                        suffix = '_arm64'
                    elif abi == 'armeabi-v7a':
                        suffix = '_armv7a'
                    elif abi == 'armeabi':
                        suffix = '_arm'
                    else: # e.g. x86, x86_64, mips, mips64
                        suffix = '_' + abi.replace('-', '_')

                    # We end up copying the data multiple times to the same
                    # directory, but that's probably fine for now.
                    self.build_binaries(platform + suffix, lib_dir, data_dir)

                # Write out the icons to the res directory.
                for appname, icon in self.icon_objects.items():
                    if appname == '*' or (appname == self.macos_main_app and '*' not in self.icon_objects):
                        # Conventional name for icon on Android.
                        basename = 'ic_launcher.png'
                    else:
                        basename = f'ic_{appname}.png'

                    res_dir = os.path.join(build_dir, 'res')
                    icon.writeSize(48, os.path.join(res_dir, 'mipmap-mdpi-v4', basename))
                    icon.writeSize(72, os.path.join(res_dir, 'mipmap-hdpi-v4', basename))
                    icon.writeSize(96, os.path.join(res_dir, 'mipmap-xhdpi-v4', basename))
                    icon.writeSize(144, os.path.join(res_dir, 'mipmap-xxhdpi-v4', basename))

                    if icon.getLargestSize() >= 192:
                        icon.writeSize(192, os.path.join(res_dir, 'mipmap-xxxhdpi-v4', basename))

                self.build_assets(platform, data_dir)

                # Generate an AndroidManifest.xml
                self.generate_android_manifest(os.path.join(build_dir, 'AndroidManifest.xml'))
            else:
                self.build_binaries(platform, build_dir, build_dir)
                self.build_assets(platform, build_dir)

            # Bundle into an .app on macOS
            if self.macos_main_app and 'macosx' in platform:
                self.bundle_macos_app(build_dir)

    def download_wheels(self, platform):
        """ Downloads wheels for the given platform using pip. This includes panda3d
        wheels. These are special wheels that are expected to contain a deploy_libs
        directory containing the Python runtime libraries, which will be added
        to sys.path."""

        import pip

        self.announce('Gathering wheels for platform: {}'.format(platform), distutils.log.INFO)

        whlcache = os.path.join(self.build_base, '__whl_cache__')

        pip_version = int(pip.__version__.split('.', 1)[0])
        if pip_version < 9:
            raise RuntimeError("pip 9.0 or greater is required, but found {}".format(pip.__version__))

        abi_tag = 'cp%d%d' % (sys.version_info[:2])
        if sys.version_info < (3, 8):
            abi_tag += 'm'

        whldir = os.path.join(whlcache, '_'.join((platform, abi_tag)))
        if not os.path.isdir(whldir):
            os.makedirs(whldir)

        # Remove any .zip files. These are built from a VCS and block for an
        # interactive prompt on subsequent downloads.
        if os.path.exists(whldir):
            for whl in os.listdir(whldir):
                if whl.endswith('.zip'):
                    os.remove(os.path.join(whldir, whl))

        pip_args = [
            '--disable-pip-version-check',
            'download',
            '-d', whldir,
            '-r', self.requirements_path,
            '--only-binary', ':all:',
            '--abi', abi_tag,
            '--platform', platform,
        ]

        if platform.startswith('linux_'):
            # Also accept manylinux.
            arch = platform[6:]
            pip_args += ['--platform', 'manylinux2014_' + arch]

        if self.use_optimized_wheels:
            pip_args += [
                '--extra-index-url', self.optimized_wheel_index
            ]

        for index in self.pypi_extra_indexes:
            pip_args += ['--extra-index-url', index]

        subprocess.check_call([sys.executable, '-m', 'pip'] + pip_args)

        # Return a list of paths to the downloaded whls
        return [
            os.path.join(whldir, filename)
            for filename in os.listdir(whldir)
            if filename.endswith('.whl')
        ]

    def update_pe_resources(self, appname, runtime):
        """Update resources (e.g., icons) in windows PE file"""

        icon = self.icon_objects.get(
            appname,
            self.icon_objects.get('*', None),
        )

        if icon is not None or self.prefer_discrete_gpu:
            pef = pefile.PEFile()
            pef.open(runtime, 'r+')
            if icon is not None:
                pef.add_icon(icon)
                pef.add_resource_section()
            if self.prefer_discrete_gpu:
                if not pef.rename_export("SymbolPlaceholder___________________", "AmdPowerXpressRequestHighPerformance") or \
                   not pef.rename_export("SymbolPlaceholder__", "NvOptimusEnablement"):
                    self.warn("Failed to apply prefer_discrete_gpu, newer target Panda3D version may be required")
            pef.write_changes()
            pef.close()

    def bundle_macos_app(self, builddir):
        """Bundle built runtime into a .app for macOS"""

        appname = '{}.app'.format(self.macos_main_app)
        appdir = os.path.join(builddir, appname)
        contentsdir = os.path.join(appdir, 'Contents')
        macosdir = os.path.join(contentsdir, 'MacOS')
        fwdir = os.path.join(contentsdir, 'Frameworks')
        resdir = os.path.join(contentsdir, 'Resources')

        self.announce('Bundling macOS app into {}'.format(appdir), distutils.log.INFO)

        # Create initial directory structure
        os.makedirs(macosdir)
        os.makedirs(fwdir)
        os.makedirs(resdir)

        # Move files over
        for fname in os.listdir(builddir):
            src = os.path.join(builddir, fname)
            if appdir in src:
                continue

            if fname in self.gui_apps or self.console_apps:
                dst = macosdir
            elif os.path.isfile(src) and open(src, 'rb').read(4) in macosx_binary_magics:
                dst = fwdir
            else:
                dst = resdir
            shutil.move(src, dst)

        # Write out Info.plist
        plist = {
            'CFBundleName': appname,
            'CFBundleDisplayName': appname, #TODO use name from setup.py/cfg
            'CFBundleIdentifier': '', #TODO
            'CFBundleVersion': '0.0.0', #TODO get from setup.py
            'CFBundlePackageType': 'APPL',
            'CFBundleSignature': '', #TODO
            'CFBundleExecutable': self.macos_main_app,
        }

        icon = self.icon_objects.get(
            self.macos_main_app,
            self.icon_objects.get('*', None)
        )
        if icon is not None:
            plist['CFBundleIconFile'] = 'iconfile'
            icon.makeICNS(os.path.join(resdir, 'iconfile.icns'))

        with open(os.path.join(contentsdir, 'Info.plist'), 'wb') as f:
            plistlib.dump(plist, f)

    def generate_android_manifest(self, path):
        import xml.etree.ElementTree as ET

        name = self.distribution.get_name()
        version = self.distribution.get_version()
        classifiers = self.distribution.get_classifiers()

        is_game = False
        for classifier in classifiers:
            if classifier == 'Topic :: Games/Entertainment' or classifier.startswith('Topic :: Games/Entertainment ::'):
                is_game = True

        manifest = ET.Element('manifest')
        manifest.set('xmlns:android', 'http://schemas.android.com/apk/res/android')
        manifest.set('package', self.application_id)
        manifest.set('android:versionCode', str(int(self.android_version_code)))
        manifest.set('android:versionName', version)
        manifest.set('android:installLocation', 'auto')

        uses_sdk = ET.SubElement(manifest, 'uses-sdk')
        uses_sdk.set('android:minSdkVersion', str(int(self.android_min_sdk_version)))
        uses_sdk.set('android:targetSdkVersion', str(int(self.android_target_sdk_version)))
        if self.android_max_sdk_version:
            uses_sdk.set('android:maxSdkVersion', str(int(self.android_max_sdk_version)))

        if 'pandagles2' in self.plugins:
            uses_feature = ET.SubElement(manifest, 'uses-feature')
            uses_feature.set('android:glEsVersion', '0x00020000')
            uses_feature.set('android:required', 'false' if 'pandagles' in self.plugins else 'true')

        if 'p3openal_audio' in self.plugins:
            uses_feature = ET.SubElement(manifest, 'uses-feature')
            uses_feature.set('android:name', 'android.hardware.audio.output')
            uses_feature.set('android:required', 'false')

        uses_feature = ET.SubElement(manifest, 'uses-feature')
        uses_feature.set('android:name', 'android.hardware.gamepad')
        uses_feature.set('android:required', 'false')

        application = ET.SubElement(manifest, 'application')
        application.set('android:label', name)
        application.set('android:isGame', ('false', 'true')[is_game])
        application.set('android:debuggable', ('false', 'true')[self.android_debuggable])
        application.set('android:extractNativeLibs', 'true')

        app_icon = self.icon_objects.get('*', self.icon_objects.get(self.macos_main_app))
        if app_icon:
            application.set('android:icon', '@mipmap/ic_launcher')

        for appname in self.gui_apps:
            activity = ET.SubElement(application, 'activity')
            activity.set('android:name', 'org.panda3d.android.PythonActivity')
            activity.set('android:label', appname)
            activity.set('android:theme', '@android:style/Theme.NoTitleBar')
            activity.set('android:configChanges', 'orientation|keyboardHidden')
            activity.set('android:launchMode', 'singleInstance')

            act_icon = self.icon_objects.get(appname)
            if act_icon and act_icon is not app_icon:
                activity.set('android:icon', '@mipmap/ic_' + appname)

            meta_data = ET.SubElement(activity, 'meta-data')
            meta_data.set('android:name', 'android.app.lib_name')
            meta_data.set('android:value', appname)

            intent_filter = ET.SubElement(activity, 'intent-filter')
            ET.SubElement(intent_filter, 'action').set('android:name', 'android.intent.action.MAIN')
            ET.SubElement(intent_filter, 'category').set('android:name', 'android.intent.category.LAUNCHER')
            ET.SubElement(intent_filter, 'category').set('android:name', 'android.intent.category.LEANBACK_LAUNCHER')

        tree = ET.ElementTree(manifest)
        with open(path, 'wb') as fh:
            tree.write(fh, encoding='utf-8', xml_declaration=True)

    def build_binaries(self, platform, binary_dir, data_dir=None):
        """ Builds the binary data for the given platform. """

        use_wheels = True
        path = sys.path[:]
        p3dwhl = None
        wheelpaths = []
        has_tkinter_wheel = False

        if use_wheels:
            wheelpaths = self.download_wheels(platform)

            for whl in wheelpaths:
                if os.path.basename(whl).startswith('panda3d-'):
                    p3dwhlfn = whl
                    p3dwhl = self._get_zip_file(p3dwhlfn)
                    break
                elif os.path.basename(whl).startswith('tkinter-'):
                    has_tkinter_wheel = True
            else:
                raise RuntimeError("Missing panda3d wheel for platform: {}".format(platform))

            if self.use_optimized_wheels:
                # Check to see if we have an optimized wheel
                localtag = p3dwhlfn.split('+')[1].split('-')[0] if '+' in p3dwhlfn else ''
                if not localtag.endswith('opt'):
                    self.announce(
                        'Could not find an optimized wheel (using index {}) for platform: {}'.format(self.optimized_wheel_index, platform),
                        distutils.log.WARN
                    )

            for whl in wheelpaths:
                if os.path.basename(whl).startswith('tkinter-'):
                    has_tkinter_wheel = True
                    break

            #whlfiles = {whl: self._get_zip_file(whl) for whl in wheelpaths}

            # Add whl files to the path so they are picked up by modulefinder
            for whl in wheelpaths:
                path.insert(0, whl)

            # Add deploy_libs from panda3d whl to the path
            path.insert(0, os.path.join(p3dwhlfn, 'deploy_libs'))


        self.announce('Building runtime for platform: {}'.format(platform), distutils.log.INFO)

        # Gather PRC data
        prcstring = ''
        if not use_wheels:
            dtool_fn = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name())
            libdir = os.path.dirname(dtool_fn.to_os_specific())
            etcdir = os.path.join(libdir, '..', 'etc')

            etcfiles = os.listdir(etcdir)
            etcfiles.sort(reverse=True)
            for fn in etcfiles:
                if fn.lower().endswith('.prc'):
                    with open(os.path.join(etcdir, fn)) as f:
                        prcstring += f.read()
        else:
            etcfiles = [i for i in p3dwhl.namelist() if i.endswith('.prc')]
            etcfiles.sort(reverse=True)
            for fn in etcfiles:
                with p3dwhl.open(fn) as f:
                    prcstring += f.read().decode('utf8')

        user_prcstring = self.extra_prc_data
        for fn in self.extra_prc_files:
            with open(fn) as f:
                user_prcstring += f.read()

        # Clenup PRC data
        check_plugins = [
            #TODO find a better way to get this list
            'pandaegg',
            'p3ffmpeg',
            'p3ptloader',
            'p3assimp',
        ]
        def parse_prc(prcstr, warn_on_missing_plugin):
            out = []
            for ln in prcstr.split('\n'):
                ln = ln.strip()
                useline = True

                if ln.startswith('#') or not ln:
                    continue

                words = ln.split(None, 1)
                if not words:
                    continue
                var = words[0]
                value = words[1] if len(words) > 1 else ''

                # Strip comment after value.
                c = value.find(' #')
                if c > 0:
                    value = value[:c].rstrip()

                if var == 'model-cache-dir' and value:
                    if platform.startswith('android'):
                        # Ignore on Android, where the cache dir is fixed.
                        continue
                    value = value.replace('/panda3d', '/{}'.format(self.distribution.get_name()))

                if var == 'audio-library-name':
                    # We have the default set to p3fmod_audio on macOS in 1.10,
                    # but this can be unexpected as other platforms use OpenAL
                    # by default.  Switch it up if FMOD is not included.
                    if value not in self.plugins and value == 'p3fmod_audio' and 'p3openal_audio' in self.plugins:
                        self.warn("Missing audio plugin p3fmod_audio referenced in PRC data, replacing with p3openal_audio")
                        value = 'p3openal_audio'

                if var == 'aux-display':
                    # Silently remove aux-display lines for missing plugins.
                    if value not in self.plugins:
                        continue

                for plugin in check_plugins:
                    if plugin in value and plugin not in self.plugins:
                        useline = False
                        if warn_on_missing_plugin:
                            self.warn(
                                "Missing plugin ({0}) referenced in user PRC data".format(plugin)
                            )
                        break
                if useline:
                    if value:
                        out.append(var + ' ' + value)
                    else:
                        out.append(var)
            return out
        prcexport = parse_prc(prcstring, 0) + parse_prc(user_prcstring, 1)

        # Export PRC data
        prcexport = '\n'.join(prcexport)
        if not self.embed_prc_data:
            prcdir = self.default_prc_dir.replace('<auto>', '')
            prcdir = os.path.join(binary_dir, prcdir)
            os.makedirs(prcdir)
            with open(os.path.join(prcdir, '00-panda3d.prc'), 'w') as f:
                f.write(prcexport)

        # Create runtimes
        freezer_extras = set()
        freezer_modules = set()
        ext_suffixes = set()

        def get_search_path_for(source_path):
            search_path = [os.path.dirname(source_path)]
            if use_wheels:
                search_path.append(os.path.join(p3dwhlfn, 'deploy_libs'))

                # If the .whl containing this file has a .libs directory, add
                # it to the path.  This is an auditwheel/numpy convention.
                if '.whl' + os.sep in source_path:
                    whl, wf = source_path.split('.whl' + os.path.sep)
                    whl += '.whl'
                    rootdir = wf.split(os.path.sep, 1)[0]
                    search_path.append(os.path.join(whl, rootdir, '.libs'))

                    # Also look for eg. numpy.libs or Pillow.libs in the root
                    whl_name = os.path.basename(whl).split('-', 1)[0]
                    search_path.append(os.path.join(whl, whl_name + '.libs'))

                    # Also look for more specific per-package cases, defined in
                    # PACKAGE_LIB_DIRS at the top of this file.
                    extra_dirs = PACKAGE_LIB_DIRS.get(whl_name, [])
                    for extra_dir, search_in in extra_dirs:
                        if not search_in:
                            search_path.append(os.path.join(whl, extra_dir.replace('/', os.path.sep)))
                        else:
                            for whl2 in wheelpaths:
                                if os.path.basename(whl2).startswith(search_in + '-'):
                                    search_path.append(os.path.join(whl2, extra_dir.replace('/', os.path.sep)))

            return search_path

        def create_runtime(platform, appname, mainscript, use_console):
            freezer = FreezeTool.Freezer(
                platform=platform,
                path=path,
                hiddenImports=self.hidden_imports,
                optimize=2 if self.strip_docstrings else 1
            )
            freezer.addModule('__main__', filename=mainscript)
            if platform.startswith('android'):
                freezer.addModule('site', filename='site.py', text=SITE_PY_ANDROID)
            else:
                freezer.addModule('site', filename='site.py', text=SITE_PY)
            for incmod in self.include_modules.get(appname, []) + self.include_modules.get('*', []):
                freezer.addModule(incmod)
            for exmod in self.exclude_modules.get(appname, []) + self.exclude_modules.get('*', []):
                freezer.excludeModule(exmod)
            freezer.done(addStartupModules=True)

            stub_name = 'deploy-stub'
            target_name = appname
            if platform.startswith('win') or 'macosx' in platform:
                if not use_console:
                    stub_name = 'deploy-stubw'
            elif platform.startswith('android'):
                if not use_console:
                    stub_name = 'libdeploy-stubw.so'
                    target_name = 'lib' + target_name + '.so'

            if platform.startswith('win'):
                stub_name += '.exe'
                target_name += '.exe'

            if use_wheels:
                if stub_name.endswith('.so'):
                    stub_file = p3dwhl.open('deploy_libs/{0}'.format(stub_name))
                else:
                    stub_file = p3dwhl.open('panda3d_tools/{0}'.format(stub_name))
            else:
                dtool_path = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name()).to_os_specific()
                stub_path = os.path.join(os.path.dirname(dtool_path), '..', 'bin', stub_name)
                stub_file = open(stub_path, 'rb')

            # Do we need an icon?  On Windows, we need to add this to the stub
            # before we add the blob.
            if 'win' in platform:
                temp_file = tempfile.NamedTemporaryFile(suffix='-icon.exe', delete=False)
                temp_file.write(stub_file.read())
                stub_file.close()
                temp_file.close()
                self.update_pe_resources(appname, temp_file.name)
                stub_file = open(temp_file.name, 'rb')
            else:
                temp_file = None

            use_strftime = self.log_filename_strftime
            if not self.log_filename or '%' not in self.log_filename:
                use_strftime = False

            target_path = os.path.join(binary_dir, target_name)
            freezer.generateRuntimeFromStub(target_path, stub_file, use_console, {
                'prc_data': prcexport if self.embed_prc_data else None,
                'default_prc_dir': self.default_prc_dir,
                'prc_dir_envvars': None,
                'prc_path_envvars': None,
                'prc_patterns': None,
                'prc_encrypted_patterns': None,
                'prc_encryption_key': None,
                'prc_executable_patterns': None,
                'prc_executable_args_envvar': None,
                'main_dir': None,
                'log_filename': self.expand_path(self.log_filename, platform),
            }, self.log_append, use_strftime)
            stub_file.close()

            if temp_file:
                os.unlink(temp_file.name)

            # Copy the dependencies.
            search_path = [binary_dir]
            if use_wheels:
                search_path.append(os.path.join(p3dwhlfn, 'panda3d'))
                search_path.append(os.path.join(p3dwhlfn, 'deploy_libs'))
            self.copy_dependencies(target_path, binary_dir, search_path, stub_name)

            freezer_extras.update(freezer.extras)
            freezer_modules.update(freezer.getAllModuleNames())
            for suffix in freezer.mf.suffixes:
                if suffix[2] == imp.C_EXTENSION:
                    ext_suffixes.add(suffix[0])

        for appname, scriptname in self.gui_apps.items():
            create_runtime(platform, appname, scriptname, False)

        for appname, scriptname in self.console_apps.items():
            create_runtime(platform, appname, scriptname, True)

        # Warn if tkinter is used but hasn't been added to requirements.txt
        if not has_tkinter_wheel and '_tkinter' in freezer_modules:
            self.warn("Detected use of tkinter, but tkinter is not specified in requirements.txt!")

        # Copy extension modules
        whl_modules = {}
        if use_wheels:
            # Get the module libs
            for i in p3dwhl.namelist():
                if not i.startswith('deploy_libs/'):
                    continue

                if not any(i.endswith(suffix) for suffix in ext_suffixes):
                    continue

                if has_tkinter_wheel and i.startswith('deploy_libs/_tkinter.'):
                    # Ignore this one, we have a separate tkinter package
                    # nowadays that contains all the dependencies.
                    continue

                base = os.path.basename(i)
                module, _, ext = base.partition('.')
                whl_modules[module] = i

        # Make sure to copy any builtins that have shared objects in the
        # deploy libs, assuming they are not already in freezer_extras.
        for mod, source_path in freezer_extras:
            freezer_modules.discard(mod)

        for mod in freezer_modules:
            if mod in whl_modules:
                freezer_extras.add((mod, None))

        # Copy over necessary plugins
        plugin_list = ['panda3d/lib{}'.format(i) for i in self.plugins]
        for lib in p3dwhl.namelist():
            plugname = lib.split('.', 1)[0]
            if plugname in plugin_list:
                source_path = os.path.join(p3dwhlfn, lib)
                target_path = os.path.join(binary_dir, os.path.basename(lib))
                search_path = [os.path.dirname(source_path)]
                self.copy_with_dependencies(source_path, target_path, search_path)

        # Copy any shared objects we need
        for module, source_path in freezer_extras:
            if source_path is not None:
                # Rename panda3d/core.pyd to panda3d.core.pyd
                source_path = os.path.normpath(source_path)
                basename = os.path.basename(source_path)
                if '.' in module:
                    basename = module.rsplit('.', 1)[0] + '.' + basename

                # Remove python version string
                parts = basename.split('.')
                if len(parts) >= 3 and ('-' in parts[-2] or parts[-2] == 'abi' + str(sys.version_info[0])):
                    parts = parts[:-2] + parts[-1:]
                    basename = '.'.join(parts)

                # Was this not found in a wheel?  Then we may have a problem,
                # since it may be for the current platform instead of the target
                # platform.
                if use_wheels:
                    found_in_wheel = False
                    for whl in wheelpaths:
                        whl = os.path.normpath(whl)
                        if source_path.lower().startswith(os.path.join(whl, '').lower()):
                            found_in_wheel = True
                            break

                    if not found_in_wheel:
                        self.warn('{} was not found in any downloaded wheel, is a dependency missing from requirements.txt?'.format(basename))
            else:
                # Builtin module, but might not be builtin in wheel libs, so double check
                if module in whl_modules:
                    source_path = os.path.join(p3dwhlfn, whl_modules[module])
                    basename = os.path.basename(source_path)
                    #XXX should we remove python version string here too?
                else:
                    continue

            if platform.startswith('android'):
                # Python modules on Android need a special prefix to be loadable
                # as a library.
                basename = 'libpy.' + basename

            # If this is a dynamic library, search for dependencies.
            target_path = os.path.join(binary_dir, basename)
            search_path = get_search_path_for(source_path)
            self.copy_with_dependencies(source_path, target_path, search_path)

        # Copy classes.dex on Android
        if use_wheels and platform.startswith('android'):
            self.copy(os.path.join(p3dwhlfn, 'deploy_libs', 'classes.dex'),
                      os.path.join(binary_dir, '..', '..', 'classes.dex'))

        # Extract any other data files from dependency packages.
        if data_dir is None:
            return

        for module, datadesc in self.package_data_dirs.items():
            if module not in freezer_modules:
                continue

            self.announce('Copying data files for module: {}'.format(module), distutils.log.INFO)

            # OK, find out in which .whl this occurs.
            for whl in wheelpaths:
                whlfile = self._get_zip_file(whl)
                filenames = whlfile.namelist()
                for source_pattern, target_dir, flags in datadesc:
                    srcglob = p3d.GlobPattern(source_pattern.lower())
                    source_dir = os.path.dirname(source_pattern)
                    # Relocate the target dir to the build directory.
                    target_dir = target_dir.replace('/', os.sep)
                    target_dir = os.path.join(data_dir, target_dir)

                    for wf in filenames:
                        if wf.endswith('/'):
                            # Skip directories.
                            continue

                        if wf.lower().startswith(source_dir.lower() + '/'):
                            if not srcglob.matches(wf.lower()):
                                continue

                            wf = wf.replace('/', os.sep)
                            relpath = wf[len(source_dir) + 1:]
                            source_path = os.path.join(whl, wf)
                            target_path = os.path.join(target_dir, relpath)

                            if 'PKG_DATA_MAKE_EXECUTABLE' in flags:
                                search_path = get_search_path_for(source_path)
                                self.copy_with_dependencies(source_path, target_path, search_path)
                                mode = os.stat(target_path).st_mode
                                mode |= stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH
                                os.chmod(target_path, mode)
                            else:
                                self.copy(source_path, target_path)

    def build_assets(self, platform, data_dir):
        """ Builds the data files for the given platform. """

        # Copy Game Files
        self.announce('Copying assets for platform: {}'.format(platform), distutils.log.INFO)
        ignore_copy_list = [
            '**/__pycache__/**',
            '**/*.pyc',
            '**/*.py',
            '{}/**'.format(self.build_base),
        ]
        ignore_copy_list += self.exclude_patterns
        ignore_copy_list += self.extra_prc_files
        ignore_copy_list = [p3d.GlobPattern(p3d.Filename.from_os_specific(i).get_fullpath()) for i in ignore_copy_list]

        include_copy_list = [p3d.GlobPattern(i) for i in self.include_patterns]

        def check_pattern(src, pattern_list):
            # Normalize file paths across platforms
            fn = p3d.Filename.from_os_specific(os.path.normpath(src))
            path = fn.get_fullpath()
            fn.make_absolute()
            abspath = fn.get_fullpath()

            for pattern in pattern_list:
                # If the pattern is absolute, match against the absolute filename.
                if pattern.pattern[0] == '/':
                    #print('check ignore: {} {} {}'.format(pattern, src, pattern.matches_file(abspath)))
                    if pattern.matches_file(abspath):
                        return True
                else:
                    #print('check ignore: {} {} {}'.format(pattern, src, pattern.matches_file(path)))
                    if pattern.matches_file(path):
                        return True
            return False

        def check_file(fname):
            return check_pattern(fname, include_copy_list) and \
                not check_pattern(fname, ignore_copy_list)

        def skip_directory(src):
            # Provides a quick-out for directory checks.  NOT recursive.
            fn = p3d.Filename.from_os_specific(os.path.normpath(src))
            path = fn.get_fullpath()
            fn.make_absolute()
            abspath = fn.get_fullpath()

            for pattern in ignore_copy_list:
                if not pattern.pattern.endswith('/*') and \
                   not pattern.pattern.endswith('/**'):
                    continue

                pattern_dir = p3d.Filename(pattern.pattern).get_dirname()
                if abspath.startswith(pattern_dir + '/'):
                    return True

                if path.startswith(pattern_dir + '/'):
                    return True

            return False

        def copy_file(src, dst):
            src = os.path.normpath(src)
            dst = os.path.normpath(dst)

            if not check_file(src):
                self.announce('skipping file {}'.format(src))
                return

            dst_dir = os.path.dirname(dst)
            if not os.path.exists(dst_dir):
                os.makedirs(dst_dir)

            ext = os.path.splitext(src)[1]
            # If the file ends with .gz/.pz, we strip this off.
            if ext in ('.gz', '.pz'):
                ext = os.path.splitext(src[:-3])[1]
            if not ext:
                ext = os.path.basename(src)

            if ext in self.file_handlers:
                buildscript = self.file_handlers[ext]
                self.announce('running {} on src ({})'.format(buildscript.__name__, src))
                try:
                    dst = self.file_handlers[ext](self, src, dst)
                except Exception as err:
                    self.announce('{}'.format(err), distutils.log.ERROR)
            else:
                self.announce('copying {0} -> {1}'.format(src, dst))
                shutil.copyfile(src, dst)

        def update_path(path):
            normpath = p3d.Filename.from_os_specific(os.path.normpath(src)).c_str()
            for inputpath, outputpath in self.rename_paths.items():
                if normpath.startswith(inputpath):
                    normpath = normpath.replace(inputpath, outputpath, 1)
            return p3d.Filename(normpath).to_os_specific()

        rootdir = os.getcwd()
        for dirname, subdirlist, filelist in os.walk(rootdir):
            subdirlist.sort()
            dirpath = os.path.relpath(dirname, rootdir)
            if skip_directory(dirpath):
                self.announce('skipping directory {}'.format(dirpath))
                continue

            for fname in filelist:
                src = os.path.join(dirpath, fname)
                dst = os.path.join(data_dir, update_path(src))

                copy_file(src, dst)

    def add_dependency(self, name, target_dir, search_path, referenced_by):
        """ Searches for the given DLL on the search path.  If it exists,
        copies it to the target_dir. """

        if os.path.exists(os.path.join(target_dir, name)):
            # We've already added it earlier.
            return

        for dep in self.exclude_dependencies:
            if dep.matches_file(name):
                return

        for dir in search_path:
            source_path = os.path.join(dir, name)

            if os.path.isfile(source_path):
                target_path = os.path.join(target_dir, name)
                self.copy_with_dependencies(source_path, target_path, search_path)
                return

            elif '.whl' in source_path:
                # Check whether the file exists inside the wheel.
                whl, wf = source_path.split('.whl' + os.path.sep)
                whl += '.whl'
                whlfile = self._get_zip_file(whl)

                # Normalize the path separator
                wf = os.path.normpath(wf).replace(os.path.sep, '/')

                # Look case-insensitively.
                namelist = whlfile.namelist()
                namelist_lower = [file.lower() for file in namelist]

                if wf.lower() in namelist_lower:
                    # We have a match.  Change it to the correct case.
                    wf = namelist[namelist_lower.index(wf.lower())]
                    source_path = os.path.join(whl, wf)
                    target_path = os.path.join(target_dir, os.path.basename(wf))
                    self.copy_with_dependencies(source_path, target_path, search_path)
                    return

        # If we didn't find it, look again, but case-insensitively.
        name_lower = name.lower()

        for dir in search_path:
            if os.path.isdir(dir):
                files = os.listdir(dir)
                files_lower = [file.lower() for file in files]

                if name_lower in files_lower:
                    name = files[files_lower.index(name_lower)]
                    source_path = os.path.join(dir, name)
                    target_path = os.path.join(target_dir, name)
                    self.copy_with_dependencies(source_path, target_path, search_path)

        # Warn if we can't find it, but only once.
        self.warn("could not find dependency {0} (referenced by {1})".format(name, referenced_by))
        self.exclude_dependencies.append(p3d.GlobPattern(name.lower()))

    def copy(self, source_path, target_path):
        """ Copies source_path to target_path.

        source_path may be located inside a .whl file. """

        try:
            self.announce('copying {0} -> {1}'.format(os.path.relpath(source_path, self.build_base), os.path.relpath(target_path, self.build_base)))
        except ValueError:
            # No relative path (e.g., files on different drives in Windows), just print absolute paths instead
            self.announce('copying {0} -> {1}'.format(source_path, target_path))

        # Make the directory if it does not yet exist.
        target_dir = os.path.dirname(target_path)
        if not os.path.isdir(target_dir):
            os.makedirs(target_dir)

        # Copy the file, and open it for analysis.
        if '.whl' in source_path:
            # This was found in a wheel, extract it
            whl, wf = source_path.split('.whl' + os.path.sep)
            whl += '.whl'
            whlfile = self._get_zip_file(whl)
            data = whlfile.read(wf.replace(os.path.sep, '/'))
            with open(target_path, 'wb') as f:
                f.write(data)
        else:
            # Regular file, copy it
            shutil.copyfile(source_path, target_path)

    def copy_with_dependencies(self, source_path, target_path, search_path):
        """ Copies source_path to target_path.  It also scans source_path for
        any dependencies, which are located along the given search_path and
        copied to the same directory as target_path.

        source_path may be located inside a .whl file. """

        self.copy(source_path, target_path)

        source_dir = os.path.dirname(source_path)
        target_dir = os.path.dirname(target_path)
        base = os.path.basename(target_path)

        if source_dir not in search_path:
            search_path = search_path + [source_dir]
        self.copy_dependencies(target_path, target_dir, search_path, base)

    def copy_dependencies(self, target_path, target_dir, search_path, referenced_by):
        """ Copies the dependencies of target_path into target_dir. """

        fp = open(target_path, 'rb+')

        # What kind of magic does the file contain?
        deps = []
        magic = fp.read(4)
        if magic.startswith(b'MZ'):
            # It's a Windows DLL or EXE file.
            pe = pefile.PEFile()
            pe.read(fp)
            for lib in pe.imports:
                deps.append(lib)

        elif magic == b'\x7FELF':
            # Elf magic.  Used on (among others) Linux and FreeBSD.
            deps = self._read_dependencies_elf(fp, target_dir, search_path)

        elif magic in (b'\xCE\xFA\xED\xFE', b'\xCF\xFA\xED\xFE'):
            # A Mach-O file, as used on macOS.
            deps = self._read_dependencies_macho(fp, '<', flatten=True)

        elif magic in (b'\xFE\xED\xFA\xCE', b'\xFE\xED\xFA\xCF'):
            rel_dir = os.path.relpath(target_dir, os.path.dirname(target_path))
            deps = self._read_dependencies_macho(fp, '>', flatten=True)

        elif magic in (b'\xCA\xFE\xBA\xBE', b'\xBE\xBA\xFE\xCA'):
            # A fat file, containing multiple Mach-O binaries.  In the future,
            # we may want to extract the one containing the architecture we
            # are building for.
            deps = self._read_dependencies_fat(fp, False, flatten=True)

        elif magic in (b'\xCA\xFE\xBA\xBF', b'\xBF\xBA\xFE\xCA'):
            # A 64-bit fat file.
            deps = self._read_dependencies_fat(fp, True, flatten=True)

        # If we discovered any dependencies, recursively add those.
        for dep in deps:
            self.add_dependency(dep, target_dir, search_path, referenced_by)

    def _read_dependencies_elf(self, elf, origin, search_path):
        """ Having read the first 4 bytes of the ELF file, fetches the
        dependent libraries and returns those as a list. """

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
            if type != 3:
                continue
            elf.seek(offset)
            string_tables[idx] = elf.read(size)

        # Loop through the dynamic sections and rewrite it if it has an rpath/runpath.
        needed = []
        rpath = []
        for offset, size, link, entsize in dynamic_sections:
            elf.seek(offset)
            data = elf.read(entsize)
            tag, val = struct.unpack_from(dynamic_struct, data)

            # Read tags until we find a NULL tag.
            while tag != 0:
                if tag == 1: # A NEEDED entry.  Read it from the string table.
                    string = string_tables[link][val : string_tables[link].find(b'\0', val)]
                    needed.append(string.decode('utf-8'))

                elif tag == 15 or tag == 29:
                    # An RPATH or RUNPATH entry.
                    string = string_tables[link][val : string_tables[link].find(b'\0', val)]
                    rpath += [
                        os.path.normpath(i.decode('utf-8').replace('$ORIGIN', origin))
                        for i in string.split(b':')
                    ]

                data = elf.read(entsize)
                tag, val = struct.unpack_from(dynamic_struct, data)
        elf.close()

        search_path += rpath
        return needed

    def _read_dependencies_macho(self, fp, endian, flatten=False):
        """ Having read the first 4 bytes of the Mach-O file, fetches the
        dependent libraries and returns those as a list.

        If flatten is True, if the dependencies contain paths like
        @loader_path/../.dylibs/libsomething.dylib, it will rewrite them to
        instead contain @loader_path/libsomething.dylib if possible.
        This requires the file pointer to be opened in rb+ mode. """

        cputype, cpusubtype, filetype, ncmds, sizeofcmds, flags = \
            struct.unpack(endian + 'IIIIII', fp.read(24))

        is_64bit = (cputype & 0x1000000) != 0
        if is_64bit:
            fp.read(4)

        # After the header, we get a series of linker commands.  We just
        # iterate through them and gather up the LC_LOAD_DYLIB commands.
        load_dylibs = []
        for i in range(ncmds):
            cmd, cmd_size = struct.unpack(endian + 'II', fp.read(8))
            cmd_data = fp.read(cmd_size - 8)
            cmd &= ~0x80000000

            if cmd == 0x0c: # LC_LOAD_DYLIB
                dylib = cmd_data[16:].decode('ascii').split('\x00', 1)[0]
                orig = dylib

                if dylib.startswith('@loader_path/../Frameworks/'):
                    dylib = dylib.replace('@loader_path/../Frameworks/', '')
                elif dylib.startswith('@executable_path/../Frameworks/'):
                    dylib = dylib.replace('@executable_path/../Frameworks/', '')
                else:
                    for prefix in ('@loader_path/', '@rpath/'):
                        if dylib.startswith(prefix):
                            dylib = dylib.replace(prefix, '')

                            # Do we need to flatten the relative reference?
                            if '/' in dylib and flatten:
                                new_dylib = prefix + os.path.basename(dylib)
                                str_size = len(cmd_data) - 16
                                if len(new_dylib) < str_size:
                                    fp.seek(-str_size, os.SEEK_CUR)
                                    fp.write(new_dylib.encode('ascii').ljust(str_size, b'\0'))
                                else:
                                    self.warn('Unable to rewrite dependency {}'.format(orig))

                load_dylibs.append(dylib)

        return load_dylibs

    def _read_dependencies_fat(self, fp, is_64bit, flatten=False):
        num_fat, = struct.unpack('>I', fp.read(4))

        # After the header we get a table of executables in this fat file,
        # each one with a corresponding offset into the file.
        offsets = []
        for i in range(num_fat):
            if is_64bit:
                cputype, cpusubtype, offset, size, align = \
                    struct.unpack('>QQQQQ', fp.read(40))
            else:
                cputype, cpusubtype, offset, size, align = \
                    struct.unpack('>IIIII', fp.read(20))
            offsets.append(offset)

        # Go through each of the binaries in the fat file.
        deps = []
        for offset in offsets:
            # Add 4, since it expects we've already read the magic.
            fp.seek(offset)
            magic = fp.read(4)

            if magic in (b'\xCE\xFA\xED\xFE', b'\xCF\xFA\xED\xFE'):
                endian = '<'
            elif magic in (b'\xFE\xED\xFA\xCE', b'\xFE\xED\xFA\xCF'):
                endian = '>'
            else:
                # Not a Mach-O file we can read.
                continue

            for dep in self._read_dependencies_macho(fp, endian, flatten=flatten):
                if dep not in deps:
                    deps.append(dep)

        return deps

    def expand_path(self, path, platform):
        "Substitutes variables in the given path string."

        if path is None:
            return None

        t = string.Template(path)
        if platform.startswith('win'):
            return t.substitute(HOME='~', USER_APPDATA='~/AppData/Local')
        elif platform.startswith('macosx'):
            return t.substitute(HOME='~', USER_APPDATA='~/Documents')
        else:
            return t.substitute(HOME='~', USER_APPDATA='~/.local/share')


class bdist_apps(setuptools.Command):
    DEFAULT_INSTALLERS = {
        'manylinux1_x86_64': ['gztar'],
        'manylinux1_i686': ['gztar'],
        'manylinux2010_x86_64': ['gztar'],
        'manylinux2010_i686': ['gztar'],
        'manylinux2014_x86_64': ['gztar'],
        'manylinux2014_i686': ['gztar'],
        'manylinux2014_aarch64': ['gztar'],
        'manylinux2014_armv7l': ['gztar'],
        'manylinux2014_ppc64': ['gztar'],
        'manylinux2014_ppc64le': ['gztar'],
        'manylinux2014_s390x': ['gztar'],
        'manylinux_2_24_x86_64': ['gztar'],
        'manylinux_2_24_i686': ['gztar'],
        'manylinux_2_24_aarch64': ['gztar'],
        'manylinux_2_24_armv7l': ['gztar'],
        'manylinux_2_24_ppc64': ['gztar'],
        'manylinux_2_24_ppc64le': ['gztar'],
        'manylinux_2_24_s390x': ['gztar'],
        'manylinux_2_28_x86_64': ['gztar'],
        'manylinux_2_28_aarch64': ['gztar'],
        'manylinux_2_28_ppc64le': ['gztar'],
        'manylinux_2_28_s390x': ['gztar'],
        'android': ['aab'],
        # Everything else defaults to ['zip']
    }

    DEFAULT_INSTALLER_FUNCS = {
        'zip': installers.create_zip,
        'gztar': installers.create_gztar,
        'bztar': installers.create_bztar,
        'xztar': installers.create_xztar,
        'nsis': installers.create_nsis,
        'aab': installers.create_aab,
    }

    description = 'bundle built Panda3D applications into distributable forms'
    user_options = build_apps.user_options + [
        ('dist-dir=', 'd', 'directory to put final built distributions in'),
        ('skip-build', None, 'skip rebuilding everything (for testing/debugging)'),
    ]

    def _build_apps_options(self):
        return [opt[0].replace('-', '_').replace('=', '') for opt in build_apps.user_options]

    def initialize_options(self):
        self.installers = {}
        self.dist_dir = os.path.join(os.getcwd(), 'dist')
        self.skip_build = False
        self.signing_certificate = None
        self.signing_private_key = None
        self.signing_passphrase = None
        self.installer_functions = {}
        self._current_platform = None
        for opt in self._build_apps_options():
            setattr(self, opt, None)

    def finalize_options(self):
        # We need to massage the inputs a bit in case they came from a
        # setup.cfg file.
        self.installers = {
            key: _parse_list(value)
            for key, value in _parse_dict(self.installers).items()
        }

        if self.signing_certificate:
            assert self.signing_private_key, 'Missing signing_private_key'
            self.signing_certificate = os.path.abspath(self.signing_certificate)
            self.signing_private_key = os.path.abspath(self.signing_private_key)

        tmp = self.DEFAULT_INSTALLER_FUNCS.copy()
        tmp.update(self.installer_functions)
        tmp.update({
            entrypoint.name: entrypoint.load()
            for entrypoint in pkg_resources.iter_entry_points('panda3d.bdist_apps.installers')
        })
        self.installer_functions = tmp

    def get_archive_basedir(self):
        return self.distribution.get_name()

    def get_current_platform(self):
        return self._current_platform

    def run(self):
        build_cmd = self.distribution.get_command_obj('build_apps')
        for opt in self._build_apps_options():
            optval = getattr(self, opt)
            if optval is not None:
                setattr(build_cmd, opt, optval)
        if not self.skip_build:
            self.run_command('build_apps')
        else:
            build_cmd.finalize_options()

        platforms = build_cmd.platforms
        build_base = os.path.abspath(build_cmd.build_base)
        if not os.path.exists(self.dist_dir):
            os.makedirs(self.dist_dir)
        os.chdir(self.dist_dir)

        for platform in platforms:
            build_dir = os.path.join(build_base, platform)
            basename = '{}_{}'.format(self.distribution.get_fullname(), platform)
            installers = self.installers.get(platform, self.DEFAULT_INSTALLERS.get(platform, ['zip']))
            self._current_platform = platform

            for installer in installers:
                self.announce('\nBuilding {} for platform: {}'.format(installer, platform), distutils.log.INFO)
                if installer not in self.installer_functions:
                    self.announce(
                        '\tUnknown installer: {}'.format(installer),
                        distutils.log.ERROR
                    )
                    continue

                self.installer_functions[installer](self, basename, build_dir)


def finalize_distribution_options(dist):
    """Entry point for compatibility with setuptools>=61, see #1394."""

    options = dist.get_option_dict('build_apps')
    if options.get('gui_apps') or options.get('console_apps'):
        # Make sure this is set to avoid auto-discovery taking place.
        if getattr(dist.metadata, 'py_modules', None) is None and \
           getattr(dist.metadata, 'packages', None) is None:
            dist.py_modules = []
