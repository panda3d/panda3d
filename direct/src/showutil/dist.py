import collections
import os
import pip
import sys
import zipfile

import distutils.command.build
import distutils.core
import distutils.dir_util
import distutils.dist
import distutils.file_util

from direct.showutil import FreezeTool
import panda3d.core as p3d


class Application(object):
    def __init__(self, scriptname, runtimename, use_console=False):
        self.scriptname = scriptname
        self.runtimename = runtimename
        self.use_console = use_console


class Distribution(distutils.dist.Distribution):
    def __init__(self, attrs):
        self.applications = []
        self.directories = []
        self.files = []
        self.exclude_paths = []
        self.exclude_modules = []
        self.deploy_platforms = []
        distutils.dist.Distribution.__init__(self, attrs)


# TODO replace with Packager
def find_packages(whlfile):
    if whlfile is None:
        dtool_fn = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name())
        libdir = os.path.dirname(dtool_fn.to_os_specific())
        filelist = [os.path.join(libdir, i) for i in os.listdir(libdir)]
    else:
        filelist = whlfile.namelist()

    return [i for i in filelist if '.so.' in i or i.endswith('.dll') or i.endswith('.dylib') or 'libpandagl' in i]


class build(distutils.command.build.build):
    def run(self):
        distutils.command.build.build.run(self)
        if not self.distribution.deploy_platforms:
            platforms = [p3d.PandaSystem.get_platform()]
            use_wheels = False
        else:
            platforms = self.distribution.deploy_platforms
            use_wheels = True
        print("Building platforms: {}".format(','.join(platforms)))

        saved_path = sys.path[:]

        for platform in platforms:
            builddir = os.path.join(self.build_base, platform)

            if not os.path.exists(builddir):
                distutils.dir_util.mkpath(builddir)

            if use_wheels:
                whldir = os.path.join(self.build_base, '__whl_cache__')

                pip.main(args=[
                    'download',
                    '-d', whldir,
                    '-r', 'requirements.txt',
                    '--only-binary', ':all:',
                    '--platform', platform,
                ])

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


            # Create runtime
            freezer_extras = set()
            freezer_modules = set()
            for app in self.distribution.applications:
                freezer = FreezeTool.Freezer()
                freezer.addModule('__main__', filename=app.scriptname)
                for exmod in self.distribution.exclude_modules:
                    freezer.excludeModule(exmod)
                freezer.done(addStartupModules=True)

                stub_name = 'deploy-stub'
                if platform.startswith('win'):
                    if not app.use_console:
                        stub_name = 'deploy-stubw'
                    stub_name += '.exe'

                if use_wheels:
                    stub_file = p3dwhl.open('panda3d_tools/{}'.format(stub_name))
                else:
                    dtool_path = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name()).to_os_specific()
                    stub_path = os.path.join(os.path.dirname(dtool_path), '..', 'bin', stub_name)
                    stub_file = open(stub_path, 'rb')

                freezer.generateRuntimeFromStub(os.path.join(builddir, app.runtimename), stub_file)
                stub_file.close()

                freezer_extras.update(freezer.extras)
                freezer_modules.update(freezer.getAllModuleNames())

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
            ] + list(freezer_modules) + self.distribution.exclude_paths + [i.scriptname for i  in self.distribution.applications]

            for copydir in self.distribution.directories:
                for item in os.listdir(copydir):
                    src = os.path.join(copydir, item)
                    dst = os.path.join(builddir, item)

                    if item in ignore_copy_list:
                        print("skipping", src)
                        continue

                    if os.path.isdir(src):
                        #print("Copy dir", src, dst)
                        distutils.dir_util.copy_tree(src, dst)
                    else:
                        #print("Copy file", src, dst)
                        distutils.file_util.copy_file(src, dst)

            # Copy extra files
            for extra in self.distribution.files:
                if len(extra) == 2:
                    src, dst = extra
                    dst = os.path.join(builddir, dst)
                else:
                    src = extra
                    dst = builddir
                distutils.file_util.copy_file(src, dst)


class bdist_panda3d(distutils.core.Command):
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        platforms = [p3d.PandaSystem.get_platform()]
        build_base = os.path.join(os.getcwd(), 'build')

        self.run_command("build")
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
    attrs.setdefault("distclass", Distribution)
    commandClasses = attrs.setdefault("cmdclass", {})
    commandClasses['build'] = build
    commandClasses['bdist_panda3d'] = bdist_panda3d
    distutils.core.setup(**attrs)
