#!/usr/bin/env python
########################################################################
#
# Caution: there are two separate, independent build systems:
# 'makepanda', and 'ppremake'.  Use one or the other, do not attempt
# to use both.  This file is part of the 'makepanda' system.
#
# To install panda using this script, type 'installpanda.py'.
# To specify an alternate location than the filesystem root /,
# either pass it as only argument or set the DESTDIR environment
# variable. This script only functions on Linux, for now.
#
########################################################################

import os, sys, platform, compileall
from distutils.sysconfig import get_python_lib
from optparse import OptionParser
from makepandacore import *

MIME_INFO = (
  ("egg", "model/x-egg", "EGG model file", "pview"),
  ("bam", "model/x-bam", "Panda3D binary model file", "pview"),
  ("egg.pz", "model/x-compressed-egg", "Compressed EGG model file", "pview"),
  ("bam.pz", "model/x-compressed-bam", "Compressed Panda3D binary model file", "pview"),
)

MIME_INFO_PLUGIN = (
  ("p3d", "application/x-panda3d", "Panda3D game/applet", "panda3d"),
)

APP_INFO = (
  ("pview", "Panda3D Model Viewer", ("egg", "bam", "egg.pz", "bam.pz")),
)

APP_INFO_PLUGIN = (
  ("panda3d", "Panda3D", ("p3d")),
)

def WriteApplicationsFile(fname, appinfo, mimeinfo):
    fhandle = open(fname, "w")
    for app, desc, exts in appinfo:
        fhandle.write("%s\n" % (app))
        fhandle.write("\tcommand=%s\n" % (app))
        fhandle.write("\tname=%s\n" % (desc))
        fhandle.write("\tcan_open_multiple_files=true\n")
        fhandle.write("\texpects_uris=false\n")
        fhandle.write("\trequires_terminal=false\n")
        fhandle.write("\tmime_types=")
        first = True
        for ext, mime, desc2, app2 in mimeinfo:
            if ext in exts:
                if first:
                    fhandle.write(mime)
                    first = False
                else:
                    fhandle.write("," + mime)
        fhandle.write("\n\n")
    fhandle.close()

def WriteMimeXMLFile(fname, info):
    fhandle = open(fname, "w")
    fhandle.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
    fhandle.write("<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info\">\n")
    for ext, mime, desc, app in info:
        fhandle.write("\t<mime-type type=\"%s\">\n" % (mime))
        fhandle.write("\t\t<comment xml:lang=\"en\">%s</comment>\n" % (desc))
        fhandle.write("\t\t<glob pattern=\"*.%s\"/>\n" % (ext))
        fhandle.write("\t</mime-type>\s")
    fhandle.write("</mime-info>\n")
    fhandle.close()

def WriteMimeFile(fname, info):
    fhandle = open(fname, "w")
    for ext, mime, desc, app in info:
        fhandle.write("%s:\n" % (mime))
        if "." in ext:
            fhandle.write("\tregex,2: %s$\n" % (ext.replace(".", "\\.")))
        fhandle.write("\text: %s\n" % (ext))
        fhandle.write("\n")
    fhandle.close()

def WriteKeysFile(fname, info):
    fhandle = open(fname, "w")
    for ext, mime, desc, app in info:
        fhandle.write("%s:\n" % (mime))
        fhandle.write("\tdescription=%s\n" % (desc))
        fhandle.write("\tdefault_action_type=application\n")
        fhandle.write("\tshort_list_application_ids_for_novice_user_level=%s\n" % (app))
        fhandle.write("\topen=%s %%f\n" % (app))
        fhandle.write("\tview=%s %%f\n" % (app))
        fhandle.write("\n")
    fhandle.close()

def GetLibDir():
    # This one's a bit tricky.  Some systems require us to install
    # 64-bits libraries into /usr/lib64, some into /usr/lib.
    # Debian forbids installing to lib64 nowadays, and the only distros
    # I know of that use lib64 are all RPM-based.  So, the 'solution'
    # seems to be to use the rpm command to give us the libdir for now.

    handle = os.popen("rpm -E '%_lib'")
    result = handle.read().strip()
    handle.close()
    if len(result) > 0:
        assert result == "lib64" or result == "lib"
        return result

    # If Python is installed into /usr/lib64, it's probably safe
    # to assume that we should install there as well.
    python_lib = get_python_lib(1)
    if python_lib.startswith('/usr/lib64/') or \
       python_lib.startswith('/usr/local/lib64/'):
        return "lib64"

    return "lib"

def InstallPanda(destdir="", prefix="/usr", outputdir="built"):
    if (not prefix.startswith("/")): prefix = "/" + prefix
    libdir = prefix + "/" + GetLibDir()
    PPATH = get_python_lib(1)
    if os.path.islink(sys.executable):
        PEXEC = os.path.join(os.path.dirname(sys.executable), os.readlink(sys.executable))
    else:
        PEXEC = sys.executable
    oscmd("mkdir -m 0755 -p "+destdir+prefix+"/bin")
    oscmd("mkdir -m 0755 -p "+destdir+prefix+"/include")
    oscmd("mkdir -m 0755 -p "+destdir+prefix+"/share/panda3d")
    oscmd("mkdir -m 0755 -p "+destdir+prefix+"/share/mime-info")
    oscmd("mkdir -m 0755 -p "+destdir+prefix+"/share/mime/packages")
    oscmd("mkdir -m 0755 -p "+destdir+prefix+"/share/application-registry")
    oscmd("mkdir -m 0755 -p "+destdir+prefix+"/share/applications")
    oscmd("mkdir -m 0755 -p "+destdir+libdir+"/panda3d")
    oscmd("mkdir -m 0755 -p "+destdir+PPATH)
    if (sys.platform.startswith("freebsd")):
        oscmd("mkdir -m 0755 -p "+destdir+prefix+"/etc")
        oscmd("mkdir -m 0755 -p "+destdir+"/usr/local/libdata/ldconfig")
    else:
        oscmd("mkdir -m 0755 -p "+destdir+"/etc/ld.so.conf.d")
    Configrc = ReadFile(outputdir+"/etc/Config.prc")
    Configrc = Configrc.replace("model-path    $THIS_PRC_DIR/..", "model-path    "+prefix+"/share/panda3d")
    if (sys.platform.startswith("freebsd")):
        WriteFile(destdir+prefix+"/etc/Config.prc", Configrc)
        oscmd("cp "+outputdir+"/etc/Confauto.prc    "+destdir+prefix+"/etc/Confauto.prc")
    else:
        WriteFile(destdir+"/etc/Config.prc", Configrc)
        oscmd("cp "+outputdir+"/etc/Confauto.prc    "+destdir+"/etc/Confauto.prc")
    oscmd("cp -R "+outputdir+"/include          "+destdir+prefix+"/include/panda3d")
    oscmd("cp -R "+outputdir+"/direct           "+destdir+prefix+"/share/panda3d/")
    oscmd("cp -R "+outputdir+"/pandac           "+destdir+prefix+"/share/panda3d/")
    oscmd("cp -R "+outputdir+"/panda3d          "+destdir+PPATH+"/")
    oscmd("cp -R "+outputdir+"/models           "+destdir+prefix+"/share/panda3d/")
    if os.path.isdir("samples"):             oscmd("cp -R samples               "+destdir+prefix+"/share/panda3d/")
    if os.path.isdir(outputdir+"/Pmw"):      oscmd("cp -R "+outputdir+"/Pmw     "+destdir+prefix+"/share/panda3d/")
    if os.path.isdir(outputdir+"/plugins"):  oscmd("cp -R "+outputdir+"/plugins "+destdir+prefix+"/share/panda3d/")
    WriteMimeFile(destdir+prefix+"/share/mime-info/panda3d.mime", MIME_INFO)
    WriteKeysFile(destdir+prefix+"/share/mime-info/panda3d.keys", MIME_INFO)
    WriteMimeXMLFile(destdir+prefix+"/share/mime/packages/panda3d.xml", MIME_INFO)
    WriteApplicationsFile(destdir+prefix+"/share/application-registry/panda3d.applications", APP_INFO, MIME_INFO)
    oscmd("cp makepanda/pview.desktop           "+destdir+prefix+"/share/applications/pview.desktop")
    oscmd("cp doc/LICENSE                       "+destdir+prefix+"/share/panda3d/LICENSE")
    oscmd("cp doc/LICENSE                       "+destdir+prefix+"/include/panda3d/LICENSE")
    oscmd("cp doc/ReleaseNotes                  "+destdir+prefix+"/share/panda3d/ReleaseNotes")
    oscmd("echo '"+prefix+"/share/panda3d' >    "+destdir+PPATH+"/panda3d.pth")
    oscmd("echo '"+libdir+"/panda3d'>>   "+destdir+PPATH+"/panda3d.pth")
    if (sys.platform.startswith("freebsd")):
        oscmd("echo '"+libdir+"/panda3d'>    "+destdir+"/usr/local/libdata/ldconfig/panda3d")
    else:
        oscmd("echo '"+libdir+"/panda3d'>    "+destdir+"/etc/ld.so.conf.d/panda3d.conf")
        oscmd("chmod +x "+destdir+"/etc/ld.so.conf.d/panda3d.conf")
    oscmd("ln -f -s "+PEXEC+"                   "+destdir+prefix+"/bin/ppython")
    oscmd("cp "+outputdir+"/bin/*               "+destdir+prefix+"/bin/")
    for base in os.listdir(outputdir+"/lib"):
        if (not base.endswith(".a")) or base == "libp3pystub.a":
            # We really need to specify -R in order not to follow symlinks on non-GNU
            oscmd("cp -R -P "+outputdir+"/lib/"+base+" "+destdir+libdir+"/panda3d/"+base)
    # rpmlint doesn't like it if we compile pyc.
    #for base in os.listdir(destdir+prefix+"/share/panda3d/direct"):
    #    if ((base != "extensions") and (base != "extensions_native")):
    #        compileall.compile_dir(destdir+prefix+"/share/panda3d/direct/"+base)
    #compileall.compile_dir(destdir+prefix+"/share/panda3d/Pmw")

    DeleteCVS(destdir+prefix+"/share/panda3d")
    DeleteBuildFiles(destdir+prefix+"/share/panda3d")
    DeleteEmptyDirs(destdir+prefix+"/share/panda3d")
    DeleteCVS(destdir+prefix+"/include/panda3d")
    DeleteBuildFiles(destdir+prefix+"/include/panda3d")
    DeleteEmptyDirs(destdir+prefix+"/include/panda3d")

    # rpmlint doesn't like this file, for some reason.
    if (os.path.isfile(destdir+prefix+"/share/panda3d/direct/leveleditor/copyfiles.pl")):
        os.remove(destdir+prefix+"/share/panda3d/direct/leveleditor/copyfiles.pl")

def InstallRuntime(destdir="", prefix="/usr", outputdir="built"):
    if (not prefix.startswith("/")): prefix = "/" + prefix
    libdir = prefix + "/" + GetLibDir()
    oscmd("mkdir -m 0755 -p "+destdir+prefix+"/bin")
    oscmd("mkdir -m 0755 -p "+destdir+prefix+"/share/mime-info")
    oscmd("mkdir -m 0755 -p "+destdir+prefix+"/share/mime/packages")
    oscmd("mkdir -m 0755 -p "+destdir+prefix+"/share/application-registry")
    oscmd("mkdir -m 0755 -p "+destdir+prefix+"/share/applications")
    if (os.path.exists(outputdir+"/plugins/nppanda3d.so")):
        oscmd("mkdir -m 0755 -p "+destdir+libdir)
        oscmd("cp "+outputdir+"/plugins/nppanda3d.so "+destdir+libdir+"/nppanda3d.so")
        if sys.platform.startswith("freebsd"):
            oscmd("mkdir -m 0755 -p "+destdir+libdir+"/browser_plugins/symlinks/gecko19")
            oscmd("mkdir -m 0755 -p "+destdir+libdir+"/libxul/plugins")
            oscmd("ln -f -s "+libdir+"/nppanda3d.so  "+destdir+libdir+"/browser_plugins/symlinks/gecko19/nppanda3d.so")
            oscmd("ln -f -s "+libdir+"/nppanda3d.so  "+destdir+libdir+"/libxul/plugins/nppanda3d.so")
        else:
            oscmd("mkdir -m 0755 -p "+destdir+libdir+"/mozilla/plugins")
            oscmd("mkdir -m 0755 -p "+destdir+libdir+"/mozilla-firefox/plugins")
            oscmd("mkdir -m 0755 -p "+destdir+libdir+"/xulrunner-addons/plugins")
            oscmd("ln -f -s "+libdir+"/nppanda3d.so  "+destdir+libdir+"/mozilla/plugins/nppanda3d.so")
            oscmd("ln -f -s "+libdir+"/nppanda3d.so  "+destdir+libdir+"/mozilla-firefox/plugins/nppanda3d.so")
            oscmd("ln -f -s "+libdir+"/nppanda3d.so  "+destdir+libdir+"/xulrunner-addons/plugins/nppanda3d.so")
    WriteMimeFile(destdir+prefix+"/share/mime-info/panda3d-runtime.mime", MIME_INFO_PLUGIN)
    WriteKeysFile(destdir+prefix+"/share/mime-info/panda3d-runtime.keys", MIME_INFO_PLUGIN)
    WriteMimeXMLFile(destdir+prefix+"/share/mime/packages/panda3d-runtime.xml", MIME_INFO_PLUGIN)
    WriteApplicationsFile(destdir+prefix+"/share/application-registry/panda3d-runtime.applications", APP_INFO_PLUGIN, MIME_INFO_PLUGIN)
    oscmd("cp makepanda/panda3d.desktop         "+destdir+prefix+"/share/applications/panda3d.desktop")
    oscmd("cp "+outputdir+"/bin/panda3d         "+destdir+prefix+"/bin/")

if (__name__ == "__main__"):
    if (sys.platform.startswith("win") or sys.platform == "darwin"):
        exit("This script is not supported on Windows or Mac OS X at the moment!")

    destdir = os.environ.get("DESTDIR", "/")

    parser = OptionParser()
    parser.add_option('', '--outputdir', dest = 'outputdir', help = 'Makepanda\'s output directory (default: built)', default = 'built')
    parser.add_option('', '--destdir', dest = 'destdir', help = 'Destination directory [default=%s]' % destdir, default = destdir)
    parser.add_option('', '--prefix', dest = 'prefix', help = 'Prefix [default=/usr/local]', default = '/usr/local')
    parser.add_option('', '--runtime', dest = 'runtime', help = 'Specify if runtime build [default=no]', action = 'store_true', default = False)
    (options, args) = parser.parse_args()

    destdir = options.destdir
    if (destdir.endswith("/")):
        destdir = destdir[:-1]
    if (destdir == "/"):
        destdir = ""
    if (destdir != "" and not os.path.isdir(destdir)):
        exit("Directory '%s' does not exist!" % destdir)

    if (options.runtime):
        print("Installing Panda3D Runtime into " + destdir + options.prefix)
        InstallRuntime(destdir = destdir, prefix = options.prefix, outputdir = options.outputdir)
    else:
        print("Installing Panda3D into " + destdir + options.prefix)
        InstallPanda(destdir = destdir, prefix = options.prefix, outputdir = options.outputdir)
    print("Installation finished!")

