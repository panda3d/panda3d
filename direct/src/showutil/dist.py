import os
import shutil
import sys

import distutils.dist
import distutils.command.build
import distutils.core

from direct.showutil import FreezeTool

class Distribution(distutils.dist.Distribution):
    def __init__(self, attrs):
        self.mainfile = 'main.py'
        self.game_dir = 'game'
        distutils.dist.Distribution.__init__(self, attrs)


class build(distutils.command.build.build):
    def run(self):
        distutils.command.build.build.run(self)
        builddir = self.build_base
        basename = os.path.abspath(os.path.join(builddir, self.distribution.get_fullname()))
        gamedir = self.distribution.game_dir
        startfile = os.path.join(gamedir, self.distribution.mainfile)

        if not os.path.exists(builddir):
            os.makedirs(builddir)

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
                print("Skip", src)
                continue

            if os.path.isdir(src):
                print("Copy dir", src, dst)
                shutil.copytree(src, dst)
            else:
                print("Copy file", src, dst)
                shutil.copy(src, dst)


def setup(**attrs):
    attrs.setdefault("distclass", Distribution)
    commandClasses = attrs.setdefault("cmdclass", {})
    commandClasses['build'] = build
    distutils.core.setup(**attrs)
