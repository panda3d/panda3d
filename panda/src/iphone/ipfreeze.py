#! /usr/bin/env python

"""

This script is used to generate a self-contained IPhone executable
that will run an app multifile.  Still experimental.

Usage:

  ipfreeze.py [opts]

"""

import getopt
import sys
import os
from direct.dist import FreezeTool

def usage(code, msg = ''):
    print >> sys.stderr, __doc__
    print >> sys.stderr, msg
    sys.exit(code)

if __name__ == '__main__':
    freezer = FreezeTool.Freezer()

    basename = 'iphone_runappmf'
    #link_all_static = True
    link_all_static = False

    try:
        opts, args = getopt.getopt(sys.argv[1:], 'h')
    except getopt.error, msg:
        usage(1, msg)

    for opt, arg in opts:
        if opt == '-h':
            usage(0)

    main = open('iphone_runappmf_src.mm', 'r').read()
    freezer.mainInitCode = main

    #target = 'sim'
    target = 'phone'

    if target == 'sim':
        platform = 'IPhoneSimulator'
    else:
        platform = 'IPhoneOS'

    version = '2.0'
    dev = '/Developer/Platforms/%s.platform/Developer' % (platform)
    env = 'env MACOSX_DEPLOYMENT_TARGET=10.5 PATH="%s/usr/bin:/Developer/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin"' % (dev)
    cc = '%s %s/usr/bin/g++-4.0' % (env, dev)
    sysroot = '-isysroot %s/SDKs/%s%s.sdk' % (dev, platform, version)
    cflags = '-D__IPHONE_OS_VERSION_MIN_REQUIRED=20000 %s' % (sysroot)
    if link_all_static:
        cflags += ' -DLINK_ALL_STATIC'
    arch = ''
    if target == 'phone':
        arch = ' -arch armv6 -mcpu=arm1176jzf-s -miphoneos-version-min=2.0'
    lflags = sysroot

    ipath = '-I/Users/drose/thirdparty.%s/Python-2.5.4/Include -I/Users/drose/thirdparty.%s/Python-2.5.4 -I/usr/local/panda/%s/include' % (target, target, target)
    lpath = '-L/Users/drose/thirdparty.%s/Python-2.5.4 -L/usr/local/panda/%s/lib' % (target, target)
    libs = ''
    libs += ' -framework Foundation -framework UIKit'
    if link_all_static:
        libs += ' -lframework -lputil -lcollide -lpgraph -lchan -ltext -lpnmimage -lpnmimagetypes -levent -leffects -lgobj -ldisplay -lmathutil -lexpress -ldgraph -ldevice -ltform -llinmath -lpstatclient -lpanda -lglstuff -lrecorder -lpgui -lchar -lpipeline -lpandabase -llerp -lgsgbase -ldownloader -lparametrics -lpgraphnodes -lcull -lgrutil -lnet -lmovies -lnativenet -laudio -linterrogatedb -ldconfig -ldtoolutil -ldtoolbase -lprc -liphonedisplay -lpandaexpress -lpanda'
        libs += ' -lphysics -lparticlesystem -lpandaphysics'
        libs += ' -ldistort -leffects -lpandafx'
        libs += ' -ldirectbase -ldcparser -ldeadrec -ldistributed -lhttp -lshowbase -linterval -lmotiontrail -ldirect'
        libs += ' -framework QuartzCore -framework OpenGLES'
    libs += ' -lpython2.5'

    lpath += ' -L/Users/drose/thirdparty.%s/lib' % (target)
    #libs += ' -ljpeg -lrfftw -lfftw'

    freezer.sourceExtension = '.mm'
    freezer.compileObj = '%s -c %s %s -o %%(basename)s.o %s %%(filename)s' % (cc, arch, cflags, ipath)
    freezer.linkExe = "%s %s %s -o %%(basename)s %s %s %%(basename)s.o" % (cc, arch, lflags, lpath, libs)

    freezer.addModule('direct.*.*')
    freezer.addModule('direct.p3d.runp3d')
    freezer.done(compileToExe = True)

    freezer.generateCode(basename, compileToExe = True)
