import collections
import os
import pip
import sys
import subprocess
import zipfile

import distutils.core
import distutils.dir_util
import distutils.file_util

from direct.showutil import FreezeTool
import panda3d.core as p3d



# TODO replace with Packager
def find_packages(whlfile):
    if whlfile is None:
        dtool_fn = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name())
        libdir = os.path.dirname(dtool_fn.to_os_specific())
        filelist = [os.path.join(libdir, i) for i in os.listdir(libdir)]
    else:
        filelist = whlfile.namelist()

    return [
         i for i in filelist
         if '.so.' in i or
         #TODO: find a better way to exclude deploy_libs from this.
         (i.endswith('.so') and not i.startswith('deploy_libs/')) or
         i.endswith('.dll') or
         i.endswith('.dylib') or
         'libpandagl' in i or
         'libpython' in i
    ]


class build_apps(distutils.core.Command):
    user_options = [] # TODO

    def initialize_options(self):
        self.build_base = os.path.join(os.getcwd(), 'build')
        self.gui_apps = {}
        self.console_apps = {}
        self.directories = []
        self.files = []
        self.exclude_paths = []
        self.exclude_modules = []
        self.deploy_platforms = []
        self.requirements_path = './requirements.txt'
        self.pypi_extra_indexes = []
        self.build_scripts= {
            '.egg': ('.bam', 'egg2bam -o {1} {0}'),
        }

    def finalize_options(self):
        # TODO
        pass

    def run(self):
        if not self.deploy_platforms:
            platforms = [p3d.PandaSystem.get_platform()]
            use_wheels = False
        else:
            platforms = self.deploy_platforms
            use_wheels = True
        print("Building platforms: {}".format(','.join(platforms)))

        saved_path = sys.path[:]

        for platform in platforms:
            builddir = os.path.join(self.build_base, platform)

            if os.path.exists(builddir):
                distutils.dir_util.remove_tree(builddir)
            distutils.dir_util.mkpath(builddir)

            if use_wheels:
                whldir = os.path.join(self.build_base, '__whl_cache__')

                pip_args = [
                    'download',
                    '-d', whldir,
                    '-r', self.requirements_path,
                    '--only-binary', ':all:',
                    '--platform', platform,
                ]

                for index in self.pypi_extra_indexes:
                    pip_args += ['--extra-index-url', index]

                pip.main(args=pip_args)

                wheelpaths = [os.path.join(whldir,i) for i in os.listdir(whldir) if platform in i]

                p3dwhl = None
                for whl in wheelpaths:
                    if 'panda3d-' in whl:
                        p3dwhlfn = whl
                        p3dwhl = zipfile.ZipFile(p3dwhlfn)
                        break
                else:
                    raise RuntimeError("Missing panda3d wheel")

                whlfiles = {whl: zipfile.ZipFile(whl) for whl in wheelpaths}

                # Add whl files to the path so they are picked up by modulefinder
                sys.path = saved_path[:]
                for whl in wheelpaths:
                    sys.path.insert(0, whl)

                # Add deploy_libs from panda3d whl to the path
                sys.path.insert(0, os.path.join(p3dwhlfn, 'deploy_libs'))


            # Create runtimes
            freezer_extras = set()
            freezer_modules = set()
            def create_runtime(appname, mainscript, use_console):
                freezer = FreezeTool.Freezer(platform=platform)
                freezer.addModule('__main__', filename=mainscript)
                for exmod in self.exclude_modules:
                    freezer.excludeModule(exmod)
                freezer.done(addStartupModules=True)

                stub_name = 'deploy-stub'
                if platform.startswith('win'):
                    if use_console:
                        stub_name = 'deploy-stubw'
                    stub_name += '.exe'

                if use_wheels:
                    stub_file = p3dwhl.open('panda3d_tools/{}'.format(stub_name))
                else:
                    dtool_path = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name()).to_os_specific()
                    stub_path = os.path.join(os.path.dirname(dtool_path), '..', 'bin', stub_name)
                    stub_file = open(stub_path, 'rb')

                freezer.generateRuntimeFromStub(os.path.join(builddir, appname), stub_file)
                stub_file.close()

                freezer_extras.update(freezer.extras)
                freezer_modules.update(freezer.getAllModuleNames())

            for appname, scriptname in self.gui_apps.items():
                create_runtime(appname, scriptname, False)

            for appname, scriptname in self.console_apps.items():
                create_runtime(appname, scriptname, True)

            # Copy extension modules
            for module, source_path in freezer_extras:
                if source_path is None:
                    # Built-in module.
                    continue

                # Rename panda3d/core.pyd to panda3d.core.pyd
                basename = os.path.basename(source_path)
                if '.' in module:
                    basename = module.rsplit('.', 1)[0] + '.' + basename

                # Remove python version string
                if sys.version_info >= (3, 0):
                    parts = basename.split('.')
                    parts = parts[:-2] + parts[-1:]
                    basename = '.'.join(parts)

                target_path = os.path.join(builddir, basename)
                if '.whl/' in source_path:
                    # This was found in a wheel, extract it
                    whl, wf = source_path.split('.whl/')
                    whl += '.whl'
                    whlfile = whlfiles[whl]
                    print("copying {} -> {}".format(os.path.join(whl, wf), target_path))
                    with open(target_path, 'wb') as f:
                        f.write(whlfile.read(wf))
                else:
                    # Regular file, copy it
                    distutils.file_util.copy_file(source_path, target_path)

            # Find Panda3D libs
            libs = find_packages(p3dwhl if use_wheels else None)

            # Copy Panda3D files
            etcdir = os.path.join(builddir, 'etc')
            if not use_wheels:
                # Libs
                for lib in libs:
                    target_path = os.path.join(builddir, os.path.basename(lib))
                    if not os.path.islink(source_path):
                        distutils.file_util.copy_file(lib, target_path)

                # etc
                dtool_fn = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name())
                libdir = os.path.dirname(dtool_fn.to_os_specific())
                src = os.path.join(libdir, '..', 'etc')
                distutils.dir_util.copy_tree(src, etcdir)
            else:
                distutils.dir_util.mkpath(etcdir)

                # Combine prc files with libs and copy the whole list
                panda_files = libs + [i for i in p3dwhl.namelist() if i.endswith('.prc')]
                for pf in panda_files:
                    dstdir = etcdir if pf.endswith('.prc') else builddir
                    target_path = os.path.join(dstdir, os.path.basename(pf))
                    print("copying {} -> {}".format(os.path.join(p3dwhlfn, pf), target_path))
                    with open(target_path, 'wb') as f:
                        f.write(p3dwhl.read(pf))

            # Copy Game Files
            ignore_copy_list = [
                '__pycache__',
            ] + list(freezer_modules) + self.exclude_paths + list(self.gui_apps.values()) + list(self.console_apps.values())

            for copydir in self.directories:
                for root, dirs, files in os.walk(copydir):
                    for item in files:
                        src = os.path.join(root, item)
                        dst = os.path.normpath(os.path.join(builddir, root, item))

                        if item in ignore_copy_list:
                            print("skipping", src)
                            continue

                        ext = os.path.splitext(src)[1]
                        dst_root = os.path.splitext(dst)[0]

                        if ext in self.build_scripts:
                            dst_ext, script = self.build_scripts[ext]
                            dst = dst_root + dst_ext
                            script = script.format(src, dst)
                            print("using script:", script)
                            subprocess.call(script.split())
                        else:
                            #print("Copy file", src, dst)
                            distutils.file_util.copy_file(src, dst)

                    for item in dirs[:]:
                        path = os.path.normpath(os.path.join(builddir, root, item))
                        if item in ignore_copy_list:
                            print("skipping", path)
                            dirs.remove(item)
                        else:
                            print("making directory", path)
                            distutils.dir_util.mkpath(path)

            # Copy extra files
            for extra in self.files:
                if len(extra) == 2:
                    src, dst = extra
                    dst = os.path.join(builddir, dst)
                else:
                    src = extra
                    dst = builddir
                distutils.file_util.copy_file(src, dst)


class bdist_apps(distutils.core.Command):
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        build_cmd = self.get_finalized_command('build_apps')
        if not build_cmd.deploy_platforms:
            platforms = [p3d.PandaSystem.get_platform()]
        else:
            platforms = build_cmd.deploy_platforms
        build_base = build_cmd.build_base

        self.run_command('build_apps')
        os.chdir(build_base)

        for platform in platforms:
            build_dir = os.path.join(build_base, platform)
            base_dir = self.distribution.get_name()
            temp_dir = os.path.join(build_base, base_dir)
            archive_format = 'gztar' if platform.startswith('linux') else 'zip'
            basename = '{}_{}'.format(self.distribution.get_fullname(), platform)

            if (os.path.exists(temp_dir)):
                distutils.dir_util.remove_tree(temp_dir)
            distutils.dir_util.copy_tree(build_dir, temp_dir)

            distutils.archive_util.make_archive(basename, archive_format, root_dir=build_base, base_dir=base_dir)

            distutils.dir_util.remove_tree(temp_dir)

def setup(**attrs):
    commandClasses = attrs.setdefault("cmdclass", {})
    commandClasses['build_apps'] = build_apps
    commandClasses['bdist_apps'] = bdist_apps
    distutils.core.setup(**attrs)
