import os
import sys

import distutils.command.build
import distutils.core
import distutils.dir_util
import distutils.dist
import distutils.file_util

from direct.showutil import FreezeTool
import panda3d.core as p3d

class Distribution(distutils.dist.Distribution):
    def __init__(self, attrs):
        self.mainfile = 'main.py'
        self.game_dir = 'game'
        self.exclude_modules = []
        self.extras = []
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

            basename = os.path.abspath(os.path.join(builddir, self.distribution.get_fullname()))
            gamedir = self.distribution.game_dir
            startfile = os.path.join(gamedir, self.distribution.mainfile)

            # Create runtime
            freezer = FreezeTool.Freezer()
            freezer.addModule('__main__', filename=startfile)
            freezer.excludeModule('panda3d')
            for exmod in self.distribution.exclude_modules:
                freezer.excludeModule(exmod)
            freezer.done(addStartupModules=True)
            freezer.generateRuntimeFromStub(basename)

            # Copy Panda3D libs
            dtool_fn = p3d.Filename(p3d.ExecutionEnvironment.get_dtool_name())
            libdir = os.path.dirname(dtool_fn.to_os_specific())
            src = os.path.normpath(os.path.join(libdir, '..', 'panda3d'))
            dst = os.path.join(builddir, 'panda3d')
            distutils.dir_util.copy_tree(src, dst)

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
                self.distribution.mainfile,
            ] + freezer.getAllModuleNames()

            for item in os.listdir(gamedir):
                src = os.path.join(gamedir, item)
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
            for extra in self.distribution.extras:
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
