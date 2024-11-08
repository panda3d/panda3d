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
from makepandacore import *
from locations import get_python_lib


MIME_INFO = (
    # ext, mime, desc, app, magic
    ("egg", "model/x-egg", "EGG model file", "pview", None),
    ("bam", "model/x-bam", "Panda3D binary model file", "pview", None),
    ("egg.pz", "model/x-compressed-egg", "Compressed EGG model file", "pview", None),
    ("bam.pz", "model/x-compressed-bam", "Compressed Panda3D binary model file", "pview", None),
    ("pstats", "application/vnd.panda3d.pstats", "PStats session file", "pstats", b"pstat\0\n\r"),
)

APP_INFO = (
  ("pview", "Panda3D Model Viewer", ("egg", "bam", "egg.pz", "bam.pz"), True),
  ("pstats", "Panda3D Profiling Tool", ("pstats",), False),
)

EXCLUDE_BINARIES = ["deploy-stub", "deploy-stubw", "run_tests"]


def WriteApplicationsFile(fname, appinfo, mimeinfo, bindir):
    fhandle = open(fname, "w")
    for app, desc, exts, multiple in appinfo:
        if not os.path.isfile(os.path.join(bindir, app)):
            continue

        fhandle.write("%s\n" % (app))
        fhandle.write("\tcommand=%s\n" % (app))
        fhandle.write("\tname=%s\n" % (desc))
        fhandle.write("\tcan_open_multiple_files=%s\n" % ('true' if multiple else 'false'))
        fhandle.write("\tstartup_notify=true\n")
        fhandle.write("\texpects_uris=false\n")
        fhandle.write("\trequires_terminal=false\n")
        fhandle.write("\tmime_types=")
        first = True
        for ext, mime, desc2, app2, magic in mimeinfo:
            if app == app2 and ext in exts:
                if first:
                    fhandle.write(mime)
                    first = False
                else:
                    fhandle.write("," + mime)
        fhandle.write("\n\n")
    fhandle.close()


def WriteMimeXMLFile(fname, info, bindir):
    fhandle = open(fname, "w")
    fhandle.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
    fhandle.write("<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info\">\n")
    for ext, mime, desc, app, magic in info:
        if not os.path.isfile(os.path.join(bindir, app)):
            continue

        fhandle.write("\t<mime-type type=\"%s\">\n" % (mime))
        fhandle.write("\t\t<comment xml:lang=\"en\">%s</comment>\n" % (desc))
        if app == "pstats":
            fhandle.write("\t\t<generic-icon name=\"x-office-spreadsheet\"/>\n")
        if magic:
            magic = magic.decode('latin-1').encode('unicode-escape').decode('latin-1')
            fhandle.write("\t\t<magic>\n\t\t\t<match type=\"string\" offset=\"0\" value=\"%s\"/>\n\t\t</magic>\n" % (magic))
        fhandle.write("\t\t<glob pattern=\"*.%s\"/>\n" % (ext))
        fhandle.write("\t</mime-type>\n")
    fhandle.write("</mime-info>\n")
    fhandle.close()


def WriteMimeFile(fname, info, bindir):
    fhandle = open(fname, "w")
    for ext, mime, desc, app, magic in info:
        if not os.path.isfile(os.path.join(bindir, app)):
            continue

        fhandle.write("%s\n" % (mime))
        if "." in ext:
            fhandle.write("\tregex,2: \\.%s$\n" % (ext.replace(".", "\\.")))
        fhandle.write("\text: %s\n" % (ext))
        fhandle.write("\n")
    fhandle.close()


def WriteKeysFile(fname, info, bindir):
    fhandle = open(fname, "w")
    for ext, mime, desc, app, magic in info:
        if not os.path.isfile(os.path.join(bindir, app)):
            continue

        fhandle.write("%s\n" % (mime))
        fhandle.write("\tdescription=%s\n" % (desc))
        fhandle.write("\tdefault_action_type=application\n")
        fhandle.write("\tshort_list_application_ids_for_novice_user_level=%s\n" % (app))
        fhandle.write("\tshort_list_application_ids_for_intermediate_user_level=%s\n" % (app))
        fhandle.write("\tshort_list_application_ids_for_advanced_user_level=%s\n" % (app))
        fhandle.write("\topen=%s %%f\n" % (app))
        fhandle.write("\tview=%s %%f\n" % (app))
        fhandle.write("\n")
    fhandle.close()


def GetDebLibDir():
    """ Returns the lib dir according to the debian system. """
    # We're on Debian or Ubuntu, which use multiarch directories.
    # Call dpkg-architecture to get the multiarch libdir.
    handle = os.popen("dpkg-architecture -qDEB_HOST_MULTIARCH")
    multiarch = handle.read().strip()
    if handle.close():
        # It failed.  Old Debian/Ubuntu version?
        pass
    elif len(multiarch) > 0:
        return "lib/" + multiarch

    return "lib"


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


def GetLibDir():
    """Returns the directory to install architecture-dependent
    libraries in, relative to the prefix directory.  This may be
    something like "lib" or "lib64" or in some cases, something
    similar to "lib/x86_64-linux-gnu"."""

    if sys.platform in ("darwin", "win32", "cygwin"):
        return "lib"

    # This one's a bit tricky.  Some systems require us to install
    # 64-bits libraries into /usr/lib64, some into /usr/lib.
    # Debian forbids installing to lib64 nowadays, and the only distros
    # I know of that use lib64 are all RPM-based.  So, the 'solution'
    # seems to be to use the rpm command to give us the libdir for now,
    # unless we know we're on debian, since rpm may be installed on
    # Debian and will give the wrong result.  Ugh.

    if os.environ.get("DEB_HOST_MULTIARCH"):
        # We're building inside the Debian build environment.
        return "lib/" + os.environ["DEB_HOST_MULTIARCH"]

    if os.path.isfile('/etc/debian_version'):
        return GetDebLibDir()
    elif os.path.isfile('/etc/arch-release'):
        # ArchLinux has lib64, but it is a symlink to lib.
        return "lib"
    else:
        # Okay, maybe we're on an RPM-based system?
        return GetRPMLibDir()

    # If Python is installed into /usr/lib64, it's probably safe
    # to assume that we should install there as well.
    python_lib = get_python_lib(1)
    if python_lib.startswith('/usr/lib64/') or \
       python_lib.startswith('/usr/local/lib64/'):
        return "lib64"

    return "lib"

def InstallPanda(destdir="", prefix="/usr", outputdir="built", libdir=GetLibDir(), python_versions=[]):
    if not prefix.startswith("/"):
        prefix = "/" + prefix
    libdir = prefix + "/" + libdir

    dest_prefix = destdir + prefix
    dest_libdir = destdir + libdir

    # Create the directory structure that we will be putting our files in.
    # Don't use os.makedirs or mkdir -p; neither properly set permissions for
    # created intermediate directories.
    MakeDirectory(dest_prefix + "/bin", mode=0o755, recursive=True)
    MakeDirectory(dest_prefix + "/include", mode=0o755)
    MakeDirectory(dest_prefix + "/include/panda3d", mode=0o755)
    MakeDirectory(dest_prefix + "/share", mode=0o755)
    MakeDirectory(dest_prefix + "/share/panda3d", mode=0o755)
    MakeDirectory(dest_prefix + "/share/mime-info", mode=0o755)
    MakeDirectory(dest_prefix + "/share/mime", mode=0o755)
    MakeDirectory(dest_prefix + "/share/mime/packages", mode=0o755)
    MakeDirectory(dest_prefix + "/share/application-registry", mode=0o755)
    MakeDirectory(dest_prefix + "/share/applications", mode=0o755)
    MakeDirectory(dest_libdir + "/panda3d", mode=0o755, recursive=True)

    for python_version in python_versions:
        MakeDirectory(destdir + python_version["purelib"], mode=0o755, recursive=True)
        MakeDirectory(destdir + python_version["platlib"] + "/panda3d", mode=0o755, recursive=True)

    if sys.platform.startswith("freebsd"):
        MakeDirectory(dest_prefix + "/etc", mode=0o755)
        MakeDirectory(destdir + "/usr/local/libdata/ldconfig", mode=0o755, recursive=True)
    else:
        MakeDirectory(destdir + "/etc/ld.so.conf.d", mode=0o755, recursive=True)

    # Write the Config.prc file.
    Configrc = ReadFile(outputdir + "/etc/Config.prc")
    Configrc = Configrc.replace("model-path    $THIS_PRC_DIR/..", "model-path    " + prefix + "/share/panda3d")
    if sys.platform.startswith("freebsd"):
        WriteFile(dest_prefix + "/etc/Config.prc", Configrc)
        oscmd(f"cp {outputdir}/etc/Confauto.prc {dest_prefix}/etc/Confauto.prc")
    else:
        WriteFile(destdir+"/etc/Config.prc", Configrc)
        oscmd(f"cp {outputdir}/etc/Confauto.prc {destdir}/etc/Confauto.prc")

    oscmd(f"cp -R {outputdir}/include/* {dest_prefix}/include/panda3d/")
    oscmd(f"cp -R {outputdir}/pandac {dest_prefix}/share/panda3d/")
    oscmd(f"cp -R {outputdir}/models {dest_prefix}/share/panda3d/")
    if os.path.isdir("samples"):
        oscmd(f"cp -R samples {dest_prefix}/share/panda3d/")
    if os.path.isdir(outputdir + "/direct"):
        oscmd(f"cp -R {outputdir}/direct {dest_prefix}/share/panda3d/")
    if os.path.isdir(outputdir + "/Pmw"):
        oscmd(f"cp -R {outputdir}/Pmw {dest_prefix}/share/panda3d/")
    if os.path.isdir(outputdir + "/plugins"):
        oscmd(f"cp -R {outputdir}/plugins {dest_prefix}/share/panda3d/")

    for python_version in python_versions:
        for base in os.listdir(outputdir + "/panda3d"):
            suffix = python_version["ext_suffix"]
            platlib = python_version["platlib"]
            if base.endswith(".py") or (base.endswith(suffix) and '.' not in base[:-len(suffix)]):
                oscmd(f"cp {outputdir}/panda3d/{base} {destdir}{platlib}/panda3d/{base}")

    bindir = outputdir + "/bin"
    WriteMimeFile(dest_prefix + "/share/mime-info/panda3d.mime", MIME_INFO, bindir)
    WriteKeysFile(dest_prefix + "/share/mime-info/panda3d.keys", MIME_INFO, bindir)
    WriteMimeXMLFile(dest_prefix + "/share/mime/packages/panda3d.xml", MIME_INFO, bindir)
    WriteApplicationsFile(dest_prefix + "/share/application-registry/panda3d.applications", APP_INFO, MIME_INFO, bindir)
    if os.path.isfile(outputdir + "/bin/pview"):
        oscmd(f"cp makepanda/pview.desktop {dest_prefix}/share/applications/pview.desktop")
    if os.path.isfile(outputdir + "/bin/pstats"):
        oscmd(f"cp makepanda/pstats.desktop {dest_prefix}/share/applications/pstats.desktop")

    oscmd(f"cp doc/ReleaseNotes {dest_prefix}/share/panda3d/ReleaseNotes")

    for python_version in python_versions:
        oscmd(f"echo '{prefix}/share/panda3d' > {destdir}{python_version['purelib']}/panda3d.pth")

        if os.path.isdir(outputdir + "/panda3d.dist-info"):
            oscmd(f"cp -R {outputdir}/panda3d.dist-info {destdir}{python_version['platlib']}")

    if sys.platform.startswith("freebsd"):
        oscmd(f"echo '{libdir}/panda3d' > {destdir}/usr/local/libdata/ldconfig/panda3d")
    else:
        oscmd(f"echo '{libdir}/panda3d' > {destdir}/etc/ld.so.conf.d/panda3d.conf")

    for base in os.listdir(outputdir + "/lib"):
        if not base.endswith(".a"):
            # We really need to specify -R in order not to follow symlinks on non-GNU
            oscmd(f"cp -R -P {outputdir}/lib/{base} {dest_libdir}/panda3d/{base}")

    for base in os.listdir(outputdir + "/bin"):
        if base not in EXCLUDE_BINARIES:
            oscmd(f"cp -R -P {outputdir}/bin/{base} {dest_prefix}/bin/{base}")

    DeleteVCS(dest_prefix + "/share/panda3d")
    DeleteBuildFiles(dest_prefix + "/share/panda3d")
    DeleteEmptyDirs(dest_prefix + "/share/panda3d")
    DeleteVCS(dest_prefix + "/include/panda3d")
    DeleteBuildFiles(dest_prefix + "/include/panda3d")
    DeleteEmptyDirs(dest_prefix + "/include/panda3d")

    # Change permissions on include directory.
    os.chmod(dest_prefix + "/include/panda3d", 0o755)
    for root, dirs, files in os.walk(dest_prefix + "/include/panda3d"):
        for basename in dirs:
            os.chmod(os.path.join(root, basename), 0o755)
        for basename in files:
            os.chmod(os.path.join(root, basename), 0o644)

    # rpmlint doesn't like this file, for some reason.
    if os.path.isfile(dest_prefix + "/share/panda3d/direct/leveleditor/copyfiles.pl"):
        os.remove(dest_prefix + "/share/panda3d/direct/leveleditor/copyfiles.pl")


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

    SetOutputDir(options.outputdir)

    if options.verbose:
        SetVerbose(True)

    print("Installing Panda3D SDK into " + destdir + options.prefix)
    InstallPanda(
        destdir=destdir,
        prefix=options.prefix,
        outputdir=options.outputdir,
        python_versions=ReadPythonVersionInfoFile(),
    )
    print("Installation finished!")

    if not destdir:
        warn_prefix = "%sNote:%s " % (GetColor("red"), GetColor())
        print(warn_prefix + "You may need to call these commands to update system caches:")
        print("  sudo ldconfig")
        print("  sudo update-desktop-database")
        print("  sudo update-mime-database -n %s/share/mime" % (options.prefix))
