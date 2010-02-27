#!/usr/bin/python
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

if (platform.architecture()[0] == "64bit"):
  libdir = "/lib64"
else:
  libdir = "/lib"

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
        print >>fhandle, app
        print >>fhandle, "\tcommand=" + app
        print >>fhandle, "\tname=" + desc
        print >>fhandle, "\tcan_open_multiple_files=true"
        print >>fhandle, "\texpects_uris=false"
        print >>fhandle, "\trequires_terminal=false"
        print >>fhandle, "\tmime_types=",
        first = True
        for ext, mime, desc2, app2 in mimeinfo:
            if (ext in exts):
                if (first):
                    fhandle.write(mime)
                    first = False
                else:
                    fhandle.write("," + mime)
        fhandle.write("\n")
        print >>fhandle
    fhandle.close()

def WriteMimeXMLFile(fname, info):
    fhandle = open(fname, "w")
    print >>fhandle, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    print >>fhandle
    print >>fhandle, "<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info\">"
    for ext, mime, desc, app in info:
        print >>fhandle, "\t<mime-type type=\"" + mime + "\">"
        print >>fhandle, "\t\t<comment xml:lang=\"en\">" + desc + "</comment>"
        print >>fhandle, "\t\t<glob pattern=\"*." + ext + "\"/>"
        print >>fhandle, "\t</mime-type>"
    print >>fhandle, "</mime-info>"
    print >>fhandle
    fhandle.close()

def WriteMimeFile(fname, info):
    fhandle = open(fname, "w")
    for ext, mime, desc, app in info:
        print >>fhandle, mime + ":"
        if ("." in ext):
            print >>fhandle, "\tregex,2: " + ext.replace(".", "\\.") + "$"
        print >>fhandle, "\text: " + ext
        print >>fhandle
    fhandle.close()

def WriteKeysFile(fname, info):
    fhandle = open(fname, "w")
    for ext, mime, desc, app in info:
        print >>fhandle, mime + ":"
        print >>fhandle, "\tdescription=" + desc
        print >>fhandle, "\tdefault_action_type=application"
        print >>fhandle, "\tshort_list_application_ids_for_novice_user_level=" + app
        print >>fhandle, "\topen=" + app + " %f"
        print >>fhandle, "\tview=" + app + " %f"
        print >>fhandle
    fhandle.close()

def InstallPanda(destdir="", prefix="/usr", outputdir="built"):
    if (not prefix.startswith("/")): prefix = "/" + prefix
    PPATH=get_python_lib(1)
    PEXEC=os.path.join(os.path.dirname(sys.executable), os.readlink(sys.executable))
    oscmd("mkdir -p "+destdir+prefix+"/bin")
    oscmd("mkdir -p "+destdir+prefix+"/include")
    oscmd("mkdir -p "+destdir+prefix+"/share/panda3d")
    oscmd("mkdir -p "+destdir+prefix+"/share/panda3d/direct")
    oscmd("mkdir -p "+destdir+prefix+"/share/mime-info")
    oscmd("mkdir -p "+destdir+prefix+"/share/mime/packages")
    oscmd("mkdir -p "+destdir+prefix+"/share/application-registry")
    oscmd("mkdir -p "+destdir+prefix+"/share/applications")
    oscmd("mkdir -p "+destdir+prefix+libdir+"/panda3d")
    oscmd("mkdir -p "+destdir+PPATH)
    oscmd("mkdir -p "+destdir+"/etc/ld.so.conf.d")
    WriteFile(destdir+prefix+"/share/panda3d/direct/__init__.py", "")
    Configrc = ReadFile(outputdir+"/etc/Config.prc")
    Configrc = Configrc.replace("model-path    $THIS_PRC_DIR/..", "model-path    "+prefix+"/share/panda3d")
    WriteFile(destdir+"/etc/Config.prc", Configrc)
    oscmd("cp "+outputdir+"/etc/Confauto.prc    "+destdir+"/etc/Confauto.prc")
    oscmd("cp -R "+outputdir+"/include          "+destdir+prefix+"/include/panda3d")
    oscmd("cp -R direct/src/*                   "+destdir+prefix+"/share/panda3d/direct")
    oscmd("cp -R "+outputdir+"/pandac           "+destdir+prefix+"/share/panda3d/pandac")
    oscmd("cp -R "+outputdir+"/models           "+destdir+prefix+"/share/panda3d/models")
    oscmd("cp direct/src/ffi/panda3d.py         "+destdir+prefix+"/share/panda3d/panda3d.py")
    if os.path.isdir("samples"):             oscmd("cp -R samples               "+destdir+prefix+"/share/panda3d/samples")
    if os.path.isdir(outputdir+"/Pmw"):      oscmd("cp -R "+outputdir+"/Pmw     "+destdir+prefix+"/share/panda3d/Pmw")
    if os.path.isdir(outputdir+"/plugins"):  oscmd("cp -R "+outputdir+"/plugins "+destdir+prefix+"/share/panda3d/plugins")
    WriteMimeFile(destdir+prefix+"/share/mime-info/panda3d.mime", MIME_INFO)
    WriteKeysFile(destdir+prefix+"/share/mime-info/panda3d.keys", MIME_INFO)
    WriteMimeXMLFile(destdir+prefix+"/share/mime/packages/panda3d.xml", MIME_INFO)
    WriteApplicationsFile(destdir+prefix+"/share/application-registry/panda3d.applications", APP_INFO, MIME_INFO)
    oscmd("cp makepanda/pview.desktop           "+destdir+prefix+"/share/applications/pview.desktop")
    oscmd("cp doc/LICENSE                       "+destdir+prefix+"/share/panda3d/LICENSE")
    oscmd("cp doc/LICENSE                       "+destdir+prefix+"/include/panda3d/LICENSE")
    oscmd("cp doc/ReleaseNotes                  "+destdir+prefix+"/share/panda3d/ReleaseNotes")
    oscmd("echo '"+prefix+libdir+"/panda3d'>    "+destdir+"/etc/ld.so.conf.d/panda3d.conf")
    oscmd("echo '"+prefix+"/share/panda3d' >    "+destdir+PPATH+"/panda3d.pth")
    oscmd("echo '"+prefix+libdir+"/panda3d'>>   "+destdir+PPATH+"/panda3d.pth")
    oscmd("chmod +x "+destdir+"/etc/ld.so.conf.d/panda3d.conf")
    oscmd("ln -s "+PEXEC+"                      "+destdir+prefix+"/bin/ppython")
    oscmd("cp "+outputdir+"/bin/*               "+destdir+prefix+"/bin/")
    for base in os.listdir(outputdir+"/lib"):
        if (not base.endswith(".a")):
            oscmd("cp -d "+outputdir+"/lib/"+base+" "+destdir+prefix+libdir+"/panda3d/"+base)
    # rpmlint doesn't like it if we compile pyc.
    #for base in os.listdir(destdir+prefix+"/share/panda3d/direct"):
    #    if ((base != "extensions") and (base != "extensions_native")):
    #        compileall.compile_dir(destdir+prefix+"/share/panda3d/direct/"+base)
    #compileall.compile_dir(destdir+prefix+"/share/panda3d/Pmw")
    DeleteCVS(destdir+prefix+"/include/panda3d")
    DeleteCVS(destdir+prefix+"/share/panda3d")
    # rpmlint doesn't like these files, for some reason.
    DeleteBuildFiles(destdir+prefix+"/share/panda3d")
    if (os.path.isfile(destdir+prefix+"/share/panda3d/direct/leveleditor/copyfiles.pl")):
      os.remove(destdir+prefix+"/share/panda3d/direct/leveleditor/copyfiles.pl")

def InstallRuntime(destdir="", prefix="/usr", outputdir="built"):
    if (not prefix.startswith("/")): prefix = "/" + prefix
    oscmd("mkdir -p "+destdir+prefix+"/bin")
    oscmd("mkdir -p "+destdir+prefix+"/share/mime-info")
    oscmd("mkdir -p "+destdir+prefix+"/share/mime/packages")
    oscmd("mkdir -p "+destdir+prefix+"/share/application-registry")
    oscmd("mkdir -p "+destdir+prefix+"/share/applications")
    if (os.path.exists(outputdir+"/plugins/nppanda3d.so")):
        oscmd("mkdir -p "+destdir+prefix+libdir+"/mozilla/plugins")
        oscmd("mkdir -p "+destdir+prefix+libdir+"/mozilla-firefox/plugins")
        oscmd("mkdir -p "+destdir+prefix+libdir+"/xulrunner-addons/plugins")
        oscmd("cp "+outputdir+"/plugins/nppanda3d.so "+destdir+prefix+libdir+"/nppanda3d.so")
        oscmd("ln -s "+prefix+libdir+"/nppanda3d.so  "+destdir+prefix+libdir+"/mozilla/plugins/nppanda3d.so")
        oscmd("ln -s "+prefix+libdir+"/nppanda3d.so  "+destdir+prefix+libdir+"/mozilla-firefox/plugins/nppanda3d.so")
        oscmd("ln -s "+prefix+libdir+"/nppanda3d.so  "+destdir+prefix+libdir+"/xulrunner-addons/plugins/nppanda3d.so")
    WriteMimeFile(destdir+prefix+"/share/mime-info/panda3d-runtime.mime", MIME_INFO_PLUGIN)
    WriteKeysFile(destdir+prefix+"/share/mime-info/panda3d-runtime.keys", MIME_INFO_PLUGIN)
    WriteMimeXMLFile(destdir+prefix+"/share/mime/packages/panda3d-runtime.xml", MIME_INFO_PLUGIN)
    WriteApplicationsFile(destdir+prefix+"/share/application-registry/panda3d-runtime.applications", APP_INFO_PLUGIN, MIME_INFO_PLUGIN)
    oscmd("cp makepanda/panda3d.desktop         "+destdir+prefix+"/share/applications/panda3d.desktop")
    oscmd("cp "+outputdir+"/bin/panda3d         "+destdir+prefix+"/bin/")

if (__name__ == "__main__"):
    if (sys.platform != "linux2"):
        exit("This script only works on linux at the moment!")

    destdir = "/"
    if (os.environ.has_key("DESTDIR")):
        destdir = os.environ["DESTDIR"]

    parser = OptionParser()
    parser.add_option('', '--outputdir', dest = 'outputdir', help = 'Makepanda\'s output directory (default: built)', default = 'built')
    parser.add_option('', '--destdir', dest = 'destdir', help = 'Destination directory [default=%s]' % destdir, default = destdir)
    parser.add_option('', '--prefix', dest = 'prefix', help = 'Prefix [default=/usr]', default = '/usr')
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
        print "Installing Panda3D Runtime into " + destdir + options.prefix
        InstallRuntime(destdir = destdir, prefix = options.prefix, outputdir = options.outputdir)
    else:
        print "Installing Panda3D into " + destdir + options.prefix
        InstallPanda(destdir = destdir, prefix = options.prefix, outputdir = options.outputdir)
    print "Installation finished!"

