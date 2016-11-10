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

            freezer = FreezeTool.Freezer()
            freezer.addModule('__main__', filename=startfile)
            freezer.excludeModule('panda3d')
            freezer.done(addStartupModules=True)
            freezer.generateRuntimeFromStub(basename)

            ignore_copy_list = [
                '__pycache__',
                self.distribution.mainfile,
                *freezer.getAllModuleNames(),
            ]

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


def setup(**attrs):
    attrs.setdefault("distclass", Distribution)
    commandClasses = attrs.setdefault("cmdclass", {})
    commandClasses['build'] = build
    distutils.core.setup(**attrs)
