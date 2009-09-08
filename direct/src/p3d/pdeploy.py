#! /usr/bin/env python

"""

This command will help you to distribute your Panda game, consisting
of a .p3d file, into an installable package or an HTML webpage.

Usage:

  %s [opts] app.p3d installer|web

Modes:

  installer
    In this mode, installable packages will be created for as many
    platforms as possible. To create Windows installers on
    non-Windows platforms, you need to have the "makensis" utility
    on your system PATH environment variable.

  web
    An HTML webpage will be generated that can be used to view
    the provided p3d file in a browser.

Options:

  -v version_number
     This should define the version number of your application
     or game. In some deploy modes, this argument is required.
     This should only contain alphanumeric characters, dots and
     dashes, as the result of the deployment may be in valid
     on some platforms otherwise.

  -n your_app
     Short, lowercase name of the application or game. Can only
     contain alphanumeric characters, underscore or dash. This
     name will also define the output file(s) of the process.
     If omitted, the basename of the p3d file is used.

  -N "Your Application"
     Full name of the application or game. This one will be used
     to display to the end-user.
     If omitted, the short name is used.

  -l "License Name"
     Specifies the name of the software license that the game
     or application is licensed under.

  -L licensefile.txt
     This should point to a file that contains the full text
     describing the software license that the game or application
     is licensed under.

"""

DEPLOY_MODES = ["installer", "web"]

import sys
import os
import getopt
from direct.p3d import InstallerMaker
from pandac.PandaModules import Filename

class ArgumentError(StandardError):
    pass

def deployApp(args):
    opts, args = getopt.getopt(args, 'l:L:n:N:v:h')
    
    version = ""
    shortname = ""
    fullname = ""
    licensename = ""
    licensefile = Filename()
    
    for option, value in opts:
        if option == '-v':
            version = value.strip()
        if option == '-n':
            shortname = value.strip()
        elif option == '-L':
            fullname = value.strip()
        if option == '-l':
            licensename = value
        elif option == '-L':
            licensefile = Filename.fromOsSpecific(value)
        elif option == '-h':
            print __doc__ % (os.path.split(sys.argv[0])[1])
            sys.exit(1)

    if not args or len(args) < 2:
        raise ArgumentError, "No target app and/or deploy type specified.  Use:\n%s app.p3d %s" % (os.path.split(sys.argv[0])[1], '|'.join(DEPLOY_MODES))

    if len(args) > 2:
        raise ArgumentError, "Too many arguments."

    appFilename = Filename.fromOsSpecific(args[0])
    if appFilename.getExtension().lower() != 'p3d':
        raise ArgumentError, 'Application filename must end in ".p3d".'

    deploy_mode = args[1].lower()
    if deploy_mode not in DEPLOY_MODES:
        raise ArgumentError, 'Invalid deploy type, must be one of "%s".' % '", "'.join(DEPLOY_MODES)

    if shortname.lower() != shortname or ' ' in shortname:
        raise ArgumentError, 'Provided short name should be lowercase, and may not contain spaces!'

    if shortname == '':
        shortname = appFilename.getBasenameWoExtension()

    if fullname == '':
        fullname = shortname

    if version == '' and deploy_mode == 'installer':
        raise ArgumentError, 'A version number is required in "installer" mode!'

    try:
        if deploy_mode == 'installer':
            im = InstallerMaker.InstallerMaker(shortname, fullname, appFilename, version)
            im.licensename = licensename
            im.licensefile = licensefile
            im.build()
        elif deploy_mode == 'web':
            print "Creating %s.html..." % shortname
            html = open(shortname + ".html", "w")
            html.write("<html>\n")
            html.write("  <head>\n")
            html.write("    <title>%s</title>\n" % fullname)
            html.write("  </head>\n")
            html.write("  <body>\n")
            html.write("    <object data=\"%s\" type=\"application/x-panda3d\"></object>\n" % appFilename.getBasename())
            html.write("  </body>\n")
            html.write("</html>\n")
            html.close()
        
    except: raise
    #except InstallerMaker.InstallerMakerError:
    #    # Just print the error message and exit gracefully.
    #    inst = sys.exc_info()[1]
    #    print inst.args[0]
    #    sys.exit(1)

if __name__ == '__main__':
    try:
        deployApp(sys.argv[1:])
    except ArgumentError, e:
        print e.args[0]
        sys.exit(1)

