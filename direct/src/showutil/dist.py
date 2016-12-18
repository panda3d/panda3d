import collections
import os
import sys

import distutils.command.build
import distutils.core
import distutils.dir_util
import distutils.dist
import distutils.file_util

from direct.showutil import FreezeTool
import panda3d.core as p3d


Application = collections.namedtuple('Application', 'scriptname runtimename')


class Distribution(distutils.dist.Distribution):
    def __init__(self, attrs):
        self.applications = []
        self.directories = []
        self.files = []
        self.exclude_paths = []
        self.exclude_modules = []
        distutils.dist.Distribution.__init__(self, attrs)


class build(distutils.command.build.build):
    def run(self):
        distutils.command.build.build.run(self)
        platforms = [p3d.PandaSystem.get_platform()]

        for platform in platforms:
            builddir = os.path.join(self.build_base, platform)

            if os.path.exists(builddir):
                distutils.dir_util.remove_tree(builddir)
            distutils.dir_util.mkpath(builddir)

            # Create runtime
            for app in self.distribution.applications:
                freezer = FreezeTool.Freezer()
                freezer.addModule('__main__', filename=app.scriptname)
                for exmod in self.distribution.exclude_modules:
                    freezer.excludeModule(exmod)
                freezer.done(addStartupModules=True)
                freezer.generateRuntimeFromStub(app.runtimename)

            # Copy extension modules
            for module, source_path in freezer.extras:
                if source_path is None:
                    # Built-in module.
                    continue

                # Rename panda3d/core.pyd to panda3d.core.pyd
                basename = os.path.basename(source_path)
                if '.' in module:
                    basename = module.rsplit('.', 1)[0] + '.' + basename

                target_path = os.path.join(builddir, basename)
                distutils.file_util.copy_file(source_path, target_path)

            # Copy Panda3D libs
            dtool_fn = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name())
            libdir = os.path.dirname(dtool_fn.to_os_specific())

            for item in os.listdir(libdir):
                if '.so.' in item or item.endswith('.dll') or item.endswith('.dylib') or 'libpandagl' in item:
                    source_path = os.path.join(libdir, item)
                    target_path = os.path.join(builddir, item)
                    if not os.path.islink(source_path):
                        distutils.file_util.copy_file(source_path, target_path)

            # Copy etc
            src = os.path.join(libdir, '..', 'etc')
            dst = os.path.join(builddir, 'etc')
            distutils.dir_util.copy_tree(src, dst)

            # Copy Game Files
            ignore_copy_list = [
                '__pycache__',
            ] + freezer.getAllModuleNames() + self.distribution.exclude_paths + [i.scriptname for i  in self.distribution.applications]

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


def setup(**attrs):
    attrs.setdefault("distclass", Distribution)
    commandClasses = attrs.setdefault("cmdclass", {})
    commandClasses['build'] = build
    distutils.core.setup(**attrs)
