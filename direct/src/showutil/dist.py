from __future__ import print_function

import collections
import os
import pip
import plistlib
import sys
import subprocess
import zipfile
import shutil
import struct
import io

import distutils.core
import distutils.log

from . import FreezeTool
from . import pefile
import panda3d.core as p3d


if 'basestring' not in globals():
    basestring = str


def egg2bam(_build_cmd, srcpath, dstpath):
    dstpath = dstpath + '.bam'
    subprocess.call([
        'egg2bam',
        '-o',
        dstpath,
        srcpath
    ])
    return dstpath

macosx_binary_magics = (
    b'\xFE\xED\xFA\xCE', b'\xCE\xFA\xED\xFE',
    b'\xFE\xED\xFA\xCF', b'\xCF\xFA\xED\xFE',
    b'\xCA\xFE\xBA\xBE', b'\xBE\xBA\xFE\xCA',
    b'\xCA\xFE\xBA\xBF', b'\xBF\xBA\xFE\xCA')


class build_apps(distutils.core.Command):
    description = 'build Panda3D applications'
    user_options = [
        ('build-base=', None, 'directory to build applications in'),
        ('requirements-path=', None, 'path to requirements.txt file for pip'),
    ]

    def initialize_options(self):
        self.build_base = os.path.join(os.getcwd(), 'build')
        self.gui_apps = {}
        self.console_apps = {}
        self.macos_main_app = None
        self.rename_paths = {}
        self.include_patterns = []
        self.exclude_patterns = []
        self.include_modules = {}
        self.exclude_modules = {}
        self.platforms = []
        self.plugins = []
        self.requirements_path = './requirements.txt'
        self.pypi_extra_indexes = []
        self.file_handlers= {
            '.egg': egg2bam,
        }
        self.exclude_dependencies = [
            # Windows
            'kernel32.dll', 'user32.dll', 'wsock32.dll', 'ws2_32.dll',
            'advapi32.dll', 'opengl32.dll', 'glu32.dll', 'gdi32.dll',
            'shell32.dll', 'ntdll.dll', 'ws2help.dll', 'rpcrt4.dll',
            'imm32.dll', 'ddraw.dll', 'shlwapi.dll', 'secur32.dll',
            'dciman32.dll', 'comdlg32.dll', 'comctl32.dll', 'ole32.dll',
            'oleaut32.dll', 'gdiplus.dll', 'winmm.dll', 'iphlpapi.dll',
            'msvcrt.dll', 'kernelbase.dll', 'msimg32.dll', 'msacm32.dll',

            # manylinux1
            'libdl.so.2', 'libstdc++.so.6', 'libm.so.6', 'libgcc_s.so.1',
            'libpthread.so.0', 'libc.so.6', 'ld-linux-x86-64.so.2',
            'libgl.so.1', 'libx11.so.6', 'libreadline.so.5', 'libncursesw.so.5',
            'libbz2.so.1', 'libz.so.1', 'liblzma.so.0',
        ]

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
        def _parse_list(input):
            if isinstance(input, basestring):
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

        self.gui_apps = _parse_dict(self.gui_apps)
        self.console_apps = _parse_dict(self.console_apps)

        self.rename_paths = _parse_dict(self.rename_paths)
        self.include_patterns = _parse_list(self.include_patterns)
        self.exclude_patterns = _parse_list(self.exclude_patterns)
        self.include_modules = _parse_list(self.include_modules)
        self.exclude_modules = _parse_list(self.exclude_modules)
        self.plugins = _parse_list(self.plugins)

        num_gui_apps = len(self.gui_apps)
        num_console_apps = len(self.console_apps)

        if not self.macos_main_app:
            if num_gui_apps > 1:
                assert False, 'macos_main_app must be defined if more than one gui_app is defined'
            elif num_gui_apps == 1:
                self.macos_main_app = list(self.gui_apps.keys())[0]

        assert os.path.exists(self.requirements_path), 'Requirements.txt path does not exist: {}'.format(self.requirements_path)
        assert num_gui_apps + num_console_apps != 0, 'Must specify at least one app in either gui_apps or console_apps'

    def run(self):
        if not self.platforms:
            platforms = [p3d.PandaSystem.get_platform()]
            use_wheels = False
        elif isinstance(self.platforms, basestring):
            platforms = [self.platforms]
            use_wheels = True
        else:
            platforms = self.platforms
            use_wheels = True
        self.announce('Building platforms: {0}'.format(','.join(platforms)), distutils.log.INFO)

        for platform in platforms:
            self.build_runtimes(platform, use_wheels)

    def download_wheels(self, platform):
        """ Downloads wheels for the given platform using pip. This includes panda3d
        wheels. These are special wheels that are expected to contain a deploy_libs
        directory containing the Python runtime libraries, which will be added
        to sys.path."""

        self.announce('Gathering wheels for platform: {}'.format(platform), distutils.log.INFO)

        whldir = os.path.join(self.build_base, '__whl_cache__')
        abi_tag = pip.pep425tags.get_abi_tag()

        if 'u' in abi_tag and (platform.startswith('win') or platform.startswith('macosx')):
            abi_tag = abi_tag.replace('u', '')

        pip_version = pip.__version__.split('.')
        if int(pip_version[0]) < 9:
            raise RuntimeError("pip 9.0 or greater is required, but found {}".format(pip.__version__))

        pip_args = [
            'download',
            '-d', whldir,
            '-r', self.requirements_path,
            '--only-binary', ':all:',
            '--platform', platform,
            '--abi', abi_tag
        ]

        for index in self.pypi_extra_indexes:
            pip_args += ['--extra-index-url', index]

        pip.main(args=pip_args)

        wheelpaths = [os.path.join(whldir,i) for i in os.listdir(whldir) if platform in i]
        return wheelpaths

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
        with open(os.path.join(contentsdir, 'Info.plist'), 'wb') as f:
            if hasattr(plistlib, 'dump'):
                plistlib.dump(plist, f)
            else:
                plistlib.writePlist(plist, f)


    def build_runtimes(self, platform, use_wheels):
        """ Builds the distributions for the given platform. """

        builddir = os.path.join(self.build_base, platform)

        if os.path.exists(builddir):
            shutil.rmtree(builddir)
        os.makedirs(builddir)

        path = sys.path[:]
        p3dwhl = None

        if use_wheels:
            wheelpaths = self.download_wheels(platform)

            for whl in wheelpaths:
                if 'panda3d-' in whl:
                    p3dwhlfn = whl
                    p3dwhl = self._get_zip_file(p3dwhlfn)
                    break
            else:
                raise RuntimeError("Missing panda3d wheel for platform: {}".format(platform))

            #whlfiles = {whl: self._get_zip_file(whl) for whl in wheelpaths}

            # Add whl files to the path so they are picked up by modulefinder
            for whl in wheelpaths:
                path.insert(0, whl)

            # Add deploy_libs from panda3d whl to the path
            path.insert(0, os.path.join(p3dwhlfn, 'deploy_libs'))


        self.announce('Building runtime for platform: {}'.format(platform), distutils.log.INFO)

        # Create runtimes
        freezer_extras = set()
        freezer_modules = set()
        def create_runtime(appname, mainscript, use_console):
            freezer = FreezeTool.Freezer(platform=platform, path=path)
            freezer.addModule('__main__', filename=mainscript)
            for incmod in self.include_modules.get(appname, []) + self.include_modules.get('*', []):
                freezer.addModule(incmod)
            for exmod in self.exclude_modules.get(appname, []) + self.exclude_modules.get('*', []):
                freezer.excludeModule(exmod)
            freezer.done(addStartupModules=True)

            target_path = os.path.join(builddir, appname)

            stub_name = 'deploy-stub'
            if platform.startswith('win') or 'macosx' in platform:
                if not use_console:
                    stub_name = 'deploy-stubw'

            if platform.startswith('win'):
                stub_name += '.exe'
                target_path += '.exe'

            if use_wheels:
                stub_file = p3dwhl.open('panda3d_tools/{0}'.format(stub_name))
            else:
                dtool_path = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name()).to_os_specific()
                stub_path = os.path.join(os.path.dirname(dtool_path), '..', 'bin', stub_name)
                stub_file = open(stub_path, 'rb')

            freezer.generateRuntimeFromStub(target_path, stub_file, use_console, {
                'prc_data': None,
                'default_prc_dir': None,
                'prc_dir_envvars': None,
                'prc_path_envvars': None,
                'prc_patterns': None,
                'prc_encrypted_patterns': None,
                'prc_encryption_key': None,
                'prc_executable_patterns': None,
                'prc_executable_args_envvar': None,
            })
            stub_file.close()

            # Copy the dependencies.
            search_path = [builddir]
            if use_wheels:
                search_path.append(os.path.join(p3dwhlfn, 'deploy_libs'))
            self.copy_dependencies(open(target_path, 'rb'), builddir, search_path, stub_name)

            freezer_extras.update(freezer.extras)
            freezer_modules.update(freezer.getAllModuleNames())

        for appname, scriptname in self.gui_apps.items():
            create_runtime(appname, scriptname, False)

        for appname, scriptname in self.console_apps.items():
            create_runtime(appname, scriptname, True)

        # Copy extension modules
        whl_modules = []
        whl_modules_ext = ''
        if use_wheels:
            # Get the module libs
            whl_modules = [
                i.replace('deploy_libs/', '') for i in p3dwhl.namelist() if i.startswith('deploy_libs/')
            ]

            # Pull off extension
            if whl_modules:
                whl_modules_ext = '.'.join(whl_modules[0].split('.')[1:])
            whl_modules = [i.split('.')[0] for i in whl_modules]

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
                target_path = os.path.join(builddir, os.path.basename(lib))
                search_path = [os.path.dirname(source_path)]
                self.copy_with_dependencies(source_path, target_path, search_path)

        # Copy any shared objects we need
        for module, source_path in freezer_extras:
            if source_path is not None:
                # Rename panda3d/core.pyd to panda3d.core.pyd
                basename = os.path.basename(source_path)
                if '.' in module:
                    basename = module.rsplit('.', 1)[0] + '.' + basename

                # Remove python version string
                if sys.version_info >= (3, 0):
                    parts = basename.split('.')
                    if len(parts) >= 3 and '-' in parts[-2]:
                        parts = parts[:-2] + parts[-1:]
                        basename = '.'.join(parts)
            else:
                # Builtin module, but might not be builtin in wheel libs, so double check
                if module in whl_modules:
                    source_path = os.path.join(p3dwhlfn, 'deploy_libs/{}.{}'.format(module, whl_modules_ext))#'{0}/deploy_libs/{1}.{2}'.format(p3dwhlfn, module, whl_modules_ext)
                    basename = os.path.basename(source_path)
                    #XXX should we remove python version string here too?
                else:
                    continue

            # If this is a dynamic library, search for dependencies.
            search_path = [os.path.dirname(source_path)]
            if use_wheels:
                search_path.append(os.path.join(p3dwhlfn, 'deploy_libs'))

            target_path = os.path.join(builddir, basename)
            self.copy_with_dependencies(source_path, target_path, search_path)

        # Copy Panda3D files
        etcdir = os.path.join(builddir, 'etc')
        if not use_wheels:
            # etc
            dtool_fn = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name())
            libdir = os.path.dirname(dtool_fn.to_os_specific())
            src = os.path.join(libdir, '..', 'etc')
            shutil.copytree(src, etcdir)
        else:
            os.makedirs(etcdir)

            # Combine prc files with libs and copy the whole list
            panda_files = [i for i in p3dwhl.namelist() if i.endswith('.prc')]
            for pf in panda_files:
                dstdir = etcdir if pf.endswith('.prc') else builddir
                target_path = os.path.join(dstdir, os.path.basename(pf))
                source_path = os.path.join(p3dwhlfn, pf)

                # If this is a dynamic library, search for dependencies.
                search_path = [os.path.dirname(source_path)]
                if use_wheels:
                    search_path.append(os.path.join(p3dwhlfn, 'deploy_libs'))

                self.copy_with_dependencies(source_path, target_path, search_path)

        # Copy Game Files
        self.announce('Copying game files for platform: {}'.format(platform), distutils.log.INFO)
        ignore_copy_list = [
            '__pycache__',
            '*.pyc',
            '*.py',
        ]
        ignore_copy_list += self.exclude_patterns
        ignore_copy_list = [p3d.GlobPattern(i) for i in ignore_copy_list]

        include_copy_list = [p3d.GlobPattern(i) for i in self.include_patterns]

        def check_pattern(src, pattern_list):
            # Normalize file paths across platforms
            path = p3d.Filename.from_os_specific(os.path.normpath(src)).get_fullpath()
            for pattern in pattern_list:
                #print('check ignore: {} {} {}'.format(pattern, src, pattern.matches(path)))
                if pattern.matches(path):
                    return True
            return False

        def check_file(fname):
            return check_pattern(fname, include_copy_list) and \
                not check_pattern(fname, ignore_copy_list)

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
            if not ext:
                ext = os.path.basename(src)

            if ext in self.file_handlers:
                buildscript = self.file_handlers[ext]
                self.announce('running {} on src ({})'.format(buildscript.__name__, src))
                dst = self.file_handlers[ext](self, src, dst)
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
            dirpath = os.path.relpath(dirname, rootdir)
            for fname in filelist:
                src = os.path.join(dirpath, fname)
                dst = os.path.join(builddir, update_path(src))

                copy_file(src, dst)

        # Bundle into an .app on macOS
        if self.macos_main_app and 'macosx' in platform:
            self.bundle_macos_app(builddir)

    def add_dependency(self, name, target_dir, search_path, referenced_by):
        """ Searches for the given DLL on the search path.  If it exists,
        copies it to the target_dir. """

        if os.path.exists(os.path.join(target_dir, name)):
            # We've already added it earlier.
            return

        if name.lower() in self.exclude_dependencies:
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
                wf = wf.replace(os.path.sep, '/')

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
        self.exclude_dependencies.append(name.lower())

    def copy_with_dependencies(self, source_path, target_path, search_path):
        """ Copies source_path to target_path.  It also scans source_path for
        any dependencies, which are located along the given search_path and
        copied to the same directory as target_path.

        source_path may be located inside a .whl file. """

        try:
            self.announce('copying {0} -> {1}'.format(os.path.relpath(source_path, self.build_base), os.path.relpath(target_path, self.build_base)))
        except ValueError:
            # No relative path (e.g., files on different drives in Windows), just print absolute paths instead
            self.announce('copying {0} -> {1}'.format(source_path, target_path))

        # Copy the file, and open it for analysis.
        if '.whl' in source_path:
            # This was found in a wheel, extract it
            whl, wf = source_path.split('.whl' + os.path.sep)
            whl += '.whl'
            whlfile = self._get_zip_file(whl)
            data = whlfile.read(wf.replace(os.path.sep, '/'))
            with open(target_path, 'wb') as f:
                f.write(data)
            # Wrap the data in a BytesIO, since we need to be able to seek in
            # the file; the stream returned by whlfile.open won't let us seek.
            fp = io.BytesIO(data)
        else:
            # Regular file, copy it
            shutil.copyfile(source_path, target_path)
            fp = open(target_path, 'rb')

        target_dir = os.path.dirname(target_path)
        base = os.path.basename(target_path)
        self.copy_dependencies(fp, target_dir, search_path, base)

    def copy_dependencies(self, fp, target_dir, search_path, referenced_by):
        """ Copies the dependencies of the given open file. """

        # What kind of magic does the file contain?
        deps = []
        magic = fp.read(4)
        if magic.startswith(b'MZ'):
            # It's a Windows DLL or EXE file.
            pe = pefile.PEFile()
            pe.read(fp)
            for lib in pe.imports:
                if not lib.lower().startswith('api-ms-win-'):
                    deps.append(lib)

        elif magic == b'\x7FELF':
            # Elf magic.  Used on (among others) Linux and FreeBSD.
            deps = self._read_dependencies_elf(fp, target_dir, search_path)

        elif magic in (b'\xCE\xFA\xED\xFE', b'\xCF\xFA\xED\xFE'):
            # A Mach-O file, as used on macOS.
            deps = self._read_dependencies_macho(fp, '<')

        elif magic in (b'\xFE\xED\xFA\xCE', b'\xFE\xED\xFA\xCF'):
            deps = self._read_dependencies_macho(fp, '>')

        elif magic in (b'\xCA\xFE\xBA\xBE', b'\xBE\xBA\xFE\xCA'):
            # A fat file, containing multiple Mach-O binaries.  In the future,
            # we may want to extract the one containing the architecture we
            # are building for.
            deps = self._read_dependencies_fat(fp, False)

        elif magic in (b'\xCA\xFE\xBA\xBF', b'\xBF\xBA\xFE\xCA'):
            # A 64-bit fat file.
            deps = self._read_dependencies_fat(fp, True)

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
            if type != 3: continue
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

    def _read_dependencies_macho(self, fp, endian):
        """ Having read the first 4 bytes of the Mach-O file, fetches the
        dependent libraries and returns those as a list. """

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
                if dylib.startswith('@loader_path/../Frameworks/'):
                    dylib = dylib.replace('@loader_path/../Frameworks/', '')
                if dylib.startswith('@executable_path/../Frameworks/'):
                    dylib = dylib.replace('@executable_path/../Frameworks/', '')
                if dylib.startswith('@loader_path/'):
                    dylib = dylib.replace('@loader_path/', '')
                load_dylibs.append(dylib)

        return load_dylibs

    def _read_dependencies_fat(self, fp, is_64bit):
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

            for dep in self._read_dependencies_macho(fp, endian):
                if dep not in deps:
                    deps.append(dep)

        return deps


class bdist_apps(distutils.core.Command):
    description = 'bundle built Panda3D applications into distributable forms'
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        build_cmd = self.get_finalized_command('build_apps')
        if not build_cmd.platforms:
            platforms = [p3d.PandaSystem.get_platform()]
        else:
            platforms = build_cmd.platforms
        build_base = build_cmd.build_base

        self.run_command('build_apps')
        os.chdir(build_base)

        for platform in platforms:
            build_dir = os.path.join(build_base, platform)
            base_dir = self.distribution.get_name()
            temp_dir = os.path.join(build_base, base_dir)
            archive_format = 'gztar' if 'linux' in platform else 'zip'
            basename = '{}_{}'.format(self.distribution.get_fullname(), platform)

            if (os.path.exists(temp_dir)):
                shutil.rmtree(temp_dir)
            shutil.copytree(build_dir, temp_dir)

            self.announce('Building {} for platform: {}'.format(archive_format, platform), distutils.log.INFO)

            distutils.archive_util.make_archive(basename, archive_format, root_dir=build_base, base_dir=base_dir)

            shutil.rmtree(temp_dir)

def setup(**attrs):
    commandClasses = attrs.setdefault("cmdclass", {})
    commandClasses['build_apps'] = build_apps
    commandClasses['bdist_apps'] = bdist_apps
    distutils.core.setup(**attrs)
