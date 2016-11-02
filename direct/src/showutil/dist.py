import os

import distutils.dist
import distutils.command.build
import distutils.core

from direct.showutil import FreezeTool

class Distribution(distutils.dist.Distribution):
    def __init__(self, attrs):
        self.mainfile = ''
        distutils.dist.Distribution.__init__(self, attrs)


class build(distutils.command.build.build):
    def run(self):
        distutils.command.build.build.run(self)
        basename = os.path.join(self.build_base, self.distribution.get_fullname())
        startfile = self.distribution.mainfile

        if not os.path.exists(self.build_base):
            os.makedirs(self.build_base)

        freezer = FreezeTool.Freezer()
        freezer.addModule('__main__', filename=startfile)
        freezer.done(addStartupModules=True)
        freezer.generateRuntimeFromStub(basename)


def setup(**attrs):
    attrs.setdefault("distclass", Distribution)
    commandClasses = attrs.setdefault("cmdclass", {})
    commandClasses['build'] = build
    distutils.core.setup(**attrs)
