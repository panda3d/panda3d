#!/usr/bin/env python
########################################################################
#
# To install panda using this script, type 'installpanda.py'.
# To specify an alternate location than the filesystem root /,
# either pass it as only argument or set the DESTDIR environment
# variable. This script only functions on Linux, for now.
#
########################################################################

import os
import sys
from optparse import OptionParser
import pprint
import subprocess
from makepandacore import GetVerbose, SetVerbose


def GetRPMLibDir():
    """ Returns the lib dir according to the rpm system. """
    handle = os.popen("rpm -E '%_lib'")
    result = handle.read().strip()
    handle.close()
    if len(result) > 0:
        assert result == "lib64" or result == "lib"
        return result
    else:
        return "lib"

def InstallPanda(destdir="", prefix="/usr", outputdir="built"):
    if not prefix.startswith("/"):
        prefix = "/" + prefix

    args = []

    if destdir or libdir:
        args += [
            'cmake', '-E', 'env',
        ]

        if destdir:
            args.append(f'DESTDIR={destdir}')

        args.append('--')

    args += [
        'cmake',
        '--install', outputdir,
        '--prefix', prefix,
    ]

    if GetVerbose():
        print('cmake command:')
        pprint.pprint(args)

    try:
        subprocess.check_call(args)
    except subprocess.CalledProcessError as exc:
        sys.exit(exc.returncode)

if __name__ == "__main__":
    if sys.platform.startswith("win") or sys.platform == "darwin":
        exit("This script is not supported on Windows or Mac OS X at the moment!")

    destdir = os.environ.get("DESTDIR", "/")

    parser = OptionParser()
    parser.add_option(
        '',
        '--outputdir',
        dest='outputdir',
        help='Makepanda\'s output directory (default: built)',
        default='built',
    )
    parser.add_option(
        '',
        '--destdir',
        dest='destdir',
        help='Destination directory [default=%s]' % destdir,
        default=destdir,
    )
    parser.add_option(
        '',
        '--prefix',
        dest='prefix',
        help='Prefix [default=/usr/local]',
        default='/usr/local',
    )
    parser.add_option(
        '',
        '--verbose',
        dest='verbose',
        help='Print commands that are executed [default=no]',
        action='store_true',
        default=False,
    )
    (options, args) = parser.parse_args()

    destdir = options.destdir
    if destdir.endswith("/"):
        destdir = destdir[:-1]
    if destdir == "/":
        destdir = ""
    if destdir != "" and not os.path.isdir(destdir):
        exit("Directory '%s' does not exist!" % destdir)

    if options.verbose:
        SetVerbose(True)

    print("Installing Panda3D SDK into " + destdir + options.prefix)
    InstallPanda(
        destdir=destdir,
        prefix=options.prefix,
        outputdir=options.outputdir,
    )
    print("Installation finished!")

    if not destdir:
        warn_prefix = "%sNote:%s " % (GetColor("red"), GetColor())
        print(warn_prefix + "You may need to call these commands to update system caches:")
        print("  sudo ldconfig")
        print("  sudo update-desktop-database")
        print("  sudo update-mime-database -n %s/share/mime" % (options.prefix))
