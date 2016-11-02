#!/usr/bin/env python
from direct.showutil import FreezeTool

if __name__ == '__main__':
    start_file = 'app.py'
    basename = 'built_app_ng'
    # Setup Freezer
    freezer = FreezeTool.Freezer()
    freezer.keepTemporaryFiles = True
    freezer.addModule('__main__', filename=start_file)
    freezer.done(addStartupModules=True)

    freezer.generateRuntimeFromStub(basename)
