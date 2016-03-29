#! /usr/bin/env python

usageText = """

This command will help you to distribute your Panda application,
consisting of a .p3d package, into a standalone executable, graphical
installer or an HTML webpage. It will attempt to create packages
for every platform, if possible.
Note that pdeploy requires an internet connection to run.

When used with the 'installer' option, it is strongly advisable
to specify most if not all of the descriptive information that can
be passed on the command-line.

Usage:

  %(prog)s [opts] app.p3d standalone|installer|html

Modes:

  standalone
    A standalone executable will be created that embeds the given
    p3d file. The resulting executable will require an
    internet connection in order to run properly.

  installer
    In this mode, installable packages will be created for as many
    platforms as possible. To create Windows installers on
    non-Windows platforms, you need to have the "makensis" utility
    on your system PATH environment variable.

  html
    An HTML webpage will be generated that can be used to view
    the provided p3d file in a browser.

Options:

  -n your_app
     Short, lowercase name of the application or game. May only
     contain alphanumeric characters, underscore or dash. This
     name will also define the output file(s) of the process.
     If omitted, the basename of the p3d file is used.

  -N "Your Application"
     Full name of the application or game. This is the
     name that will be displayed to the end-user.
     The 'display_name' config is used by default.  If it
     is missing, the short name is used.

  -v version_number
     This should define the version number of your application
     or game. In some deploy modes, this argument is required.
     This should only contain alphanumeric characters, dots and
     dashes, as otherwise the result of the deployment may be
     invalid on some platforms.

  -o output_dir
     Indicates the directory where the output will be stored.
     Within this directory, subdirectories will be created
     for every platform, unless -t is provided.
     If omitted, the current working directory is assumed.

  -t token=value
     Defines a web token or parameter to pass to the application.
     Use this to configure how the application will be run.
     You can pass as many -t options as you need. Some examples of
     useful token names are width, height, log_basename, auto_start,
     hidden and console_environment.

  -P platform
     If this option is provided, it should specify a comma-
     separated list of platforms that the p3d package will be
     deployed for. If omitted, it will be built for all platforms.
     This option may be specified multiple times.
     Examples of valid platforms are win_i386, linux_amd64 and osx_ppc.

  -c
     If this option is provided, any -P options are ignored and
     the p3d package is only deployed for the current platform.
     Furthermore, no per-platform subdirectories will be created
     inside the output dirctory.

  -s
     This option only has effect in 'installer' mode. If it is
     provided, the resulting installers will be fully self-contained,
     will not require an internet connection to run, and start up
     much faster. Note that pdeploy will also take a very long time
     to finish when -s is provided.
     If it is omitted, pdeploy will finish much quicker, and the
     resulting installers will be smaller, but they will require
     an internet connection for the first run, and the load time
     will be considerably longer.

  -l "License Name"
     Specifies the name of the software license that the game
     or application is licensed under.
     Only relevant when generating a graphical installer.

  -L licensefile.txt
     This should point to a file that contains the full text
     describing the software license that the game or application
     is licensed under.
     Only relevant when generating a graphical installer.

  -O
     Specify this option when generating a graphical installer to omit
     the default checkboxes for "run this program" and "install a
     desktop shortcut" on completion.

  -a com.your_company
     Short identifier of the author of the application. The default
     is "org.panda3d", but you will most likely want to change
     it to your own name or that of your organization or company.
     Only relevant when generating a graphical installer.

  -A "Your Company"
     Full name of the author of the application.  The default is
     determined from the GECOS information of the current user if
     available; if not, your username is used.
     Only relevant when generating a graphical installer.

  -e "you@your_company.com"
     E-mail address of the maintainer of the application.  The default
     is username@hostname.
     Only relevant when generating a graphical installer.

  -i "path/icon32.png" -i "path/icon48.png" -i "path/icon128.png"
     This option should be repeated several times with different square
     image sizes.  These images will then be combined to form an icon
     file that will be used to represent the installed application.
     To support all platforms, it is recommended to supply images of
     the sizes 16x16, 32x32, 48x48, 128x128, 256x256, and 512x512.
     The larger icon sizes can safely be omitted if you cannot
     provide images in that resolution.
     It is recommended to use .png images for correct transparency.
     If no images are provided, no icon will be generated.
     Only relevant when generating a graphical installer.

  -h
     Display this help

"""

DEPLOY_MODES = ["standalone", "installer", "html"]

import sys
import os
import getopt
from direct.p3d.DeploymentTools import Standalone, Installer, Icon
from panda3d.core import Filename, PandaSystem

def usage(code, msg = ''):
    if not msg:
        sys.stderr.write(usageText % {'prog' : os.path.split(sys.argv[0])[1]})
    sys.stderr.write(msg + '\n')
    sys.exit(code)

shortname = ""
fullname = ""
version = ""
outputDir = Filename("./")
tokens = {}
platforms = []
currentPlatform = False
licensename = ""
licensefile = Filename()
authorid = ""
authorname = ""
authoremail = ""
iconFiles = []
includeRequires = False
omitDefaultCheckboxes = False

try:
    opts, args = getopt.getopt(sys.argv[1:], 'n:N:v:o:t:P:csOl:L:a:A:e:i:h')
except getopt.error as msg:
    usage(1, msg or 'Invalid option')

for opt, arg in opts:
    if opt == '-n':
        shortname = arg.strip()
    elif opt == '-N':
        fullname = arg.strip()
    elif opt == '-v':
        version = arg.strip()
    elif opt == '-o':
        outputDir = Filename.fromOsSpecific(arg)
    elif opt == '-t':
        token = arg.strip().split("=", 1)
        tokens[token[0]] = token[1]
    elif opt == '-P':
        platforms.append(arg)
    elif opt == '-c':
        currentPlatform = True
    elif opt == '-s':
        includeRequires = True
    elif opt == '-O':
        omitDefaultCheckboxes = True
    elif opt == '-l':
        licensename = arg.strip()
    elif opt == '-L':
        licensefile = Filename.fromOsSpecific(arg)
    elif opt == '-a':
        authorid = arg.strip()
    elif opt == '-A':
        authorname = arg.strip()
    elif opt == '-e':
        authoremail = arg.strip()
    elif opt == '-i':
        iconFiles.append(Filename.fromOsSpecific(arg))

    elif opt == '-h':
        usage(0)
    else:
        msg = 'illegal option: ' + flag
        print(msg)
        sys.exit(1, msg)

if not args or len(args) != 2:
    msg = 'Wrong number of arguments, received %s, expected 2' % (
        len(args or []))
    usage(1, msg)

appFilename = Filename.fromOsSpecific(args[0])
if appFilename.getExtension().lower() != 'p3d':
    print('Application filename must end in ".p3d".')
    sys.exit(1)
deploy_mode = args[1].lower()

if not appFilename.exists():
    print('Application filename does not exist!')
    sys.exit(1)

if shortname == '':
    shortname = appFilename.getBasenameWoExtension()

if shortname.lower() != shortname or ' ' in shortname:
    print('\nProvided short name should be lowercase, and may not contain spaces!\n')

if version == '' and deploy_mode == 'installer':
    print('\nA version number is required in "installer" mode.\n')
    sys.exit(1)

if not outputDir:
    print('\nYou must name the output directory with the -o parameter.\n')
    sys.exit(1)
if not outputDir.exists():
    print('\nThe specified output directory does not exist!\n')
    sys.exit(1)
elif not outputDir.isDirectory():
    print('\nThe specified output directory is a file!\n')
    sys.exit(1)

if deploy_mode == 'standalone':
    s = Standalone(appFilename, tokens)
    s.basename = shortname

    if currentPlatform:
        platform = PandaSystem.getPlatform()
        if platform.startswith("win"):
            s.build(Filename(outputDir, shortname + ".exe"), platform)
        else:
            s.build(Filename(outputDir, shortname), platform)
    elif len(platforms) == 0:
        s.buildAll(outputDir)
    else:
        for platform in platforms:
            if platform.startswith("win"):
                s.build(Filename(outputDir, platform + "/" + shortname + ".exe"), platform)
            else:
                s.build(Filename(outputDir, platform + "/" + shortname), platform)

elif deploy_mode == 'installer':
    if includeRequires:
        tokens["verify_contents"] = "never"
    i = Installer(appFilename, shortname, fullname, version, tokens = tokens)
    i.includeRequires = includeRequires
    if omitDefaultCheckboxes:
        i.offerRun = False
        i.offerDesktopShortcut = False
    i.licensename = licensename
    i.licensefile = licensefile
    if authorid:
        i.authorid = authorid
    if authorname:
        i.authorname = authorname
    if authoremail:
        i.authoremail = authoremail
    if not authorname or not authoremail or not authorid:
        print("Using author \"%s\" <%s> with ID %s" % \
            (i.authorname, i.authoremail, i.authorid))

    # Add the supplied icon images
    if len(iconFiles) > 0:
        failed = False
        i.icon = Icon()
        for iconFile in iconFiles:
            if not i.icon.addImage(iconFile):
                print('\nFailed to add icon image "%s"!\n' % iconFile)
                failed = True
        if failed:
            sys.exit(1)

    # Now build for the requested platforms.
    if currentPlatform:
        platform = PandaSystem.getPlatform()
        if platform.startswith("win"):
            i.build(outputDir, platform)
        else:
            i.build(outputDir, platform)
    elif len(platforms) == 0:
        i.buildAll(outputDir)
    else:
        for platform in platforms:
            output = Filename(outputDir, platform + "/")
            output.makeDir()
            i.build(output, platform)

    del i

elif deploy_mode == 'html':
    w, h = tokens.get("width", 800), tokens.get("height", 600)
    if "data" not in tokens:
        tokens["data"] = appFilename.getBasename()

    print("Creating %s.html..." % shortname)
    html = open(Filename(outputDir, shortname + ".html").toOsSpecific(), "w")
    html.write("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n")
    html.write("<html xmlns=\"http://www.w3.org/1999/xhtml\">\n")
    html.write("  <head>\n")
    html.write("    <title>%s</title>\n" % fullname)
    html.write("    <meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\" />\n")
    if authorname:
        html.write("    <meta name=\"Author\" content=\"%s\" />\n" % authorname.replace('"', '\\"'))
    html.write("  </head>\n")
    html.write("  <body>\n")
    html.write("    <object")
    for key, value in tokens.items():
        html.write(" %s=\"%s\"" % (key, value.replace('"', '\\"')))
    if "width" not in tokens:
        html.write(" width=\"%s\"" % w)
    if "height" not in tokens:
        html.write(" height=\"%s\"" % h)
    html.write(" type=\"application/x-panda3d\">")
    html.write("      <object width=\"%s\" height=\"%s\" classid=\"CLSID:924B4927-D3BA-41EA-9F7E-8A89194AB3AC\">\n" % (w, h))
    for key, value in tokens.items():
        html.write("        <param name=\"%s\" value=\"%s\" />\n" % (key, value.replace('"', '\\"')))
    html.write("      </object>\n")
    html.write("    </object>\n")
    html.write("  </body>\n")
    html.write("</html>\n")
    html.close()
else:
    usage(1, 'Invalid deployment mode!')

# An explicit call to exit() is required to exit the program, when
# this module is packaged in a p3d file.
sys.exit(0)

