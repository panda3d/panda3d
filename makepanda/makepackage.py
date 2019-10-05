#!/usr/bin/env python

from makepandacore import *
from installpanda import *
import sys
import os
import shutil
import glob
import re
import subprocess


INSTALLER_DEB_FILE = """
Package: panda3dMAJOR
Version: VERSION
Section: libdevel
Priority: optional
Architecture: ARCH
Essential: no
Depends: DEPENDS
Recommends: RECOMMENDS
Provides: PROVIDES
Conflicts: PROVIDES
Replaces: PROVIDES
Maintainer: rdb <me@rdb.name>
Installed-Size: INSTSIZE
Description: Panda3D free 3D engine SDK
 Panda3D is a game engine which includes graphics, audio, I/O, collision detection, and other abilities relevant to the creation of 3D games. Panda3D is open source and free software under the revised BSD license, and can be used for both free and commercial game development at no financial cost.
 Panda3D's intended game-development language is Python. The engine itself is written in C++, and utilizes an automatic wrapper-generator to expose the complete functionality of the engine in a Python interface.
 .
 This package contains the SDK for development with Panda3D.

"""

# We're not putting "python" in the "Requires" field,
# since the rpm-based distros don't have a common
# naming for the Python package.
INSTALLER_SPEC_FILE = """
Summary: The Panda3D free 3D engine SDK
Name: panda3d
Version: VERSION
Release: RPMRELEASE
License: BSD License
Group: Development/Libraries
BuildRoot: PANDASOURCE/targetroot
%description
Panda3D is a game engine which includes graphics, audio, I/O, collision detection, and other abilities relevant to the creation of 3D games. Panda3D is open source and free software under the revised BSD license, and can be used for both free and commercial game development at no financial cost.
Panda3D's intended game-development language is Python. The engine itself is written in C++, and utilizes an automatic wrapper-generator to expose the complete functionality of the engine in a Python interface.

This package contains the SDK for development with Panda3D.
%post
/sbin/ldconfig
%postun
/sbin/ldconfig
%files
%defattr(-,root,root)
/etc/Confauto.prc
/etc/Config.prc
/usr/share/panda3d
/etc/ld.so.conf.d/panda3d.conf
/usr/%_lib/panda3d
/usr/include/panda3d
"""
INSTALLER_SPEC_FILE_PVIEW = \
"""/usr/share/applications/pview.desktop
/usr/share/mime-info/panda3d.mime
/usr/share/mime-info/panda3d.keys
/usr/share/mime/packages/panda3d.xml
/usr/share/application-registry/panda3d.applications
"""

# plist file for Mac OSX
Info_plist = """<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleIdentifier</key>
  <string>{package_id}</string>
  <key>CFBundleShortVersionString</key>
  <string>{version}</string>
  <key>IFPkgFlagRelocatable</key>
  <false/>
  <key>IFPkgFlagAuthorizationAction</key>
  <string>RootAuthorization</string>
  <key>IFPkgFlagAllowBackRev</key>
  <true/>
</dict>
</plist>
"""

# FreeBSD pkg-descr
INSTALLER_PKG_DESCR_FILE = """
Panda3D is a game engine which includes graphics, audio, I/O, collision detection, and other abilities relevant to the creation of 3D games. Panda3D is open source and free software under the revised BSD license, and can be used for both free and commercial game development at no financial cost.
Panda3D's intended game-development language is Python. The engine itself is written in C++, and utilizes an automatic wrapper-generator to expose the complete functionality of the engine in a Python interface.

This package contains the SDK for development with Panda3D.

WWW: https://www.panda3d.org/
"""

# FreeBSD PKG Manifest template file
INSTALLER_PKG_MANIFEST_FILE = """
name: NAME
version: VERSION
arch: ARCH
origin: ORIGIN
comment: "Panda3D free 3D engine SDK"
www: https://www.panda3d.org
maintainer: rdb <me@rdb.name>
prefix: /usr/local
flatsize: INSTSIZEMB
deps: {DEPENDS}
"""


def MakeInstallerNSIS(version, file, title, installdir, compressor="lzma", **kwargs):
    outputdir = GetOutputDir()

    if os.path.isfile(file):
        os.remove(file)
    elif os.path.isdir(file):
        shutil.rmtree(file)

    if GetTargetArch() == 'x64':
        regview = '64'
    else:
        regview = '32'

    print("Building "+title+" installer at %s" % (file))
    if compressor != "lzma":
        print("Note: you are using zlib, which is faster, but lzma gives better compression.")
    if os.path.exists("nsis-output.exe"):
        os.remove("nsis-output.exe")
    WriteFile(outputdir+"/tmp/__init__.py", "")

    nsis_defs = {
        'COMPRESSOR': compressor,
        'TITLE'     : title,
        'INSTALLDIR': installdir,
        'OUTFILE'   : '..\\' + file,
        'BUILT'     : '..\\' + outputdir,
        'SOURCE'    : '..',
        'REGVIEW'   : regview,
    }

    # Are we shipping a version of Python?
    if os.path.isfile(os.path.join(outputdir, "python", "python.exe")):
        py_dlls = glob.glob(os.path.join(outputdir, "python", "python[0-9][0-9].dll")) \
                + glob.glob(os.path.join(outputdir, "python", "python[0-9][0-9]_d.dll"))
        assert py_dlls
        py_dll = os.path.basename(py_dlls[0])
        pyver = py_dll[6] + "." + py_dll[7]

        if GetTargetArch() != 'x64':
            pyver += '-32'

        nsis_defs['INCLUDE_PYVER'] = pyver

    if GetHost() == 'windows':
        cmd = os.path.join(GetThirdpartyBase(), 'win-nsis', 'makensis') + ' /V2'
        for item in nsis_defs.items():
            cmd += ' /D%s="%s"' % item
    else:
        cmd = 'makensis -V2'
        for item in nsis_defs.items():
            cmd += ' -D%s="%s"' % item

    cmd += ' "makepanda\\installer.nsi"'
    oscmd(cmd)


def MakeDebugSymbolArchive(zipname, dirname):
    outputdir = GetOutputDir()

    import zipfile
    zip = zipfile.ZipFile(zipname, 'w', zipfile.ZIP_DEFLATED)

    for fn in glob.glob(os.path.join(outputdir, 'bin', '*.pdb')):
        zip.write(fn, dirname + '/bin/' + os.path.basename(fn))

    for fn in glob.glob(os.path.join(outputdir, 'panda3d', '*.pdb')):
        zip.write(fn, dirname + '/panda3d/' + os.path.basename(fn))

    for fn in glob.glob(os.path.join(outputdir, 'plugins', '*.pdb')):
        zip.write(fn, dirname + '/plugins/' + os.path.basename(fn))

    for fn in glob.glob(os.path.join(outputdir, 'python', '*.pdb')):
        zip.write(fn, dirname + '/python/' + os.path.basename(fn))

    for fn in glob.glob(os.path.join(outputdir, 'python', 'DLLs', '*.pdb')):
        zip.write(fn, dirname + '/python/DLLs/' + os.path.basename(fn))

    zip.close()


def MakeInstallerLinux(version, debversion=None, rpmrelease=1,
                       python_versions=[], **kwargs):
    outputdir = GetOutputDir()

    # We pack Python 2 and Python 3, if we built with support for it.
    python2_ver = None
    python3_ver = None
    install_python_versions = []

    # What's the system version of Python 3?
    oscmd('python3 -V > "%s/tmp/python3_version.txt"' % (outputdir))
    sys_python3_ver = '.'.join(ReadFile(outputdir + "/tmp/python3_version.txt").strip().split(' ')[1].split('.')[:2])

    # Check that we built with support for these.
    for version_info in python_versions:
        if version_info["version"] == "2.7":
            python2_ver = "2.7"
            install_python_versions.append(version_info)
        elif version_info["version"] == sys_python3_ver:
            python3_ver = sys_python3_ver
            install_python_versions.append(version_info)

    major_version = '.'.join(version.split('.')[:2])
    if not debversion:
        debversion = version

    # Clean and set up a directory to install Panda3D into
    oscmd("rm -rf targetroot data.tar.gz control.tar.gz panda3d.spec")
    oscmd("mkdir -m 0755 targetroot")

    dpkg_present = False
    if os.path.exists("/usr/bin/dpkg-architecture") and os.path.exists("/usr/bin/dpkg-deb"):
        dpkg_present = True
    rpmbuild_present = False
    if os.path.exists("/usr/bin/rpmbuild"):
        rpmbuild_present = True

    if dpkg_present and rpmbuild_present:
        Warn("both dpkg and rpmbuild present.")

    if dpkg_present:
        # Invoke installpanda.py to install it into a temporary dir
        lib_dir = GetDebLibDir()
        InstallPanda(destdir="targetroot", prefix="/usr",
                        outputdir=outputdir, libdir=lib_dir,
                        python_versions=install_python_versions)
        oscmd("chmod -R 755 targetroot/usr/share/panda3d")
        oscmd("mkdir -m 0755 -p targetroot/usr/share/man/man1")
        oscmd("install -m 0644 doc/man/*.1 targetroot/usr/share/man/man1/")

        oscmd("dpkg --print-architecture > "+outputdir+"/tmp/architecture.txt")
        pkg_arch = ReadFile(outputdir+"/tmp/architecture.txt").strip()
        txt = INSTALLER_DEB_FILE[1:]
        txt = txt.replace("VERSION", debversion).replace("ARCH", pkg_arch).replace("MAJOR", major_version)
        txt = txt.replace("INSTSIZE", str(GetDirectorySize("targetroot") // 1024))

        oscmd("mkdir -m 0755 -p targetroot/DEBIAN")
        oscmd("cd targetroot && (find usr -type f -exec md5sum {} ;) > DEBIAN/md5sums")
        oscmd("cd targetroot && (find etc -type f -exec md5sum {} ;) >> DEBIAN/md5sums")
        WriteFile("targetroot/DEBIAN/conffiles","/etc/Config.prc\n")
        WriteFile("targetroot/DEBIAN/postinst","#!/bin/sh\necho running ldconfig\nldconfig\n")
        oscmd("cp targetroot/DEBIAN/postinst targetroot/DEBIAN/postrm")

        # Determine the package name and the locations that
        # dpkg-shlibdeps should look in for executables.
        pkg_version = debversion
        pkg_name = "panda3d" + major_version
        lib_pattern = "debian/%s/usr/%s/panda3d/*.so*" % (pkg_name, lib_dir)
        bin_pattern = "debian/%s/usr/bin/*" % (pkg_name)

        # dpkg-shlibdeps looks in the debian/{pkg_name}/DEBIAN/shlibs directory
        # and also expects a debian/control file, so we create this dummy set-up.
        oscmd("mkdir targetroot/debian")
        oscmd("ln -s .. targetroot/debian/" + pkg_name)
        WriteFile("targetroot/debian/control", "")

        dpkg_shlibdeps = "dpkg-shlibdeps"
        if GetVerbose():
            dpkg_shlibdeps += " -v"

        pkg_name = "panda3d" + major_version
        pkg_dir = "debian/panda3d" + major_version

        # Generate a symbols file so that other packages can know which symbols we export.
        oscmd("cd targetroot && dpkg-gensymbols -q -ODEBIAN/symbols -v%(pkg_version)s -p%(pkg_name)s -e%(lib_pattern)s" % locals())

        # Library dependencies are required, binary dependencies are recommended.
        # We explicitly exclude libphysx-extras since we don't want to depend on PhysX.
        oscmd("cd targetroot && LD_LIBRARY_PATH=usr/%(lib_dir)s/panda3d %(dpkg_shlibdeps)s -Tdebian/substvars_dep --ignore-missing-info -x%(pkg_name)s -xlibphysx-extras %(lib_pattern)s" % locals())
        oscmd("cd targetroot && LD_LIBRARY_PATH=usr/%(lib_dir)s/panda3d %(dpkg_shlibdeps)s -Tdebian/substvars_rec --ignore-missing-info -x%(pkg_name)s %(bin_pattern)s" % locals())

        # Parse the substvars files generated by dpkg-shlibdeps.
        depends = ReadFile("targetroot/debian/substvars_dep").replace("shlibs:Depends=", "").strip()
        recommends = ReadFile("targetroot/debian/substvars_rec").replace("shlibs:Depends=", "").strip()
        provides = "panda3d"

        if python2_ver or python3_ver:
            recommends += ", python-pmw"

        if python2_ver:
            depends += ", python%s" % (python2_ver)
            recommends += ", python-wxversion"
            recommends += ", python-tk (>= %s)" % (python2_ver)
            provides += ", python2-panda3d"

        if python3_ver:
            depends += ", python%s" % (python3_ver)
            recommends += ", python3-tk (>= %s)" % (python3_ver)
            provides += ", python3-panda3d"

        if not PkgSkip("NVIDIACG"):
            depends += ", nvidia-cg-toolkit"

        # Write back the dependencies, and delete the dummy set-up.
        txt = txt.replace("DEPENDS", depends.strip(', '))
        txt = txt.replace("RECOMMENDS", recommends.strip(', '))
        txt = txt.replace("PROVIDES", provides.strip(', '))
        WriteFile("targetroot/DEBIAN/control", txt)
        oscmd("rm -rf targetroot/debian")

        # Package it all up into a .deb file.
        oscmd("chmod -R 755 targetroot/DEBIAN")
        oscmd("chmod 644 targetroot/DEBIAN/control targetroot/DEBIAN/md5sums")
        oscmd("chmod 644 targetroot/DEBIAN/conffiles targetroot/DEBIAN/symbols")
        oscmd("fakeroot dpkg-deb -b targetroot %s_%s_%s.deb" % (pkg_name, pkg_version, pkg_arch))

    elif rpmbuild_present:
        # Invoke installpanda.py to install it into a temporary dir
        InstallPanda(destdir="targetroot", prefix="/usr",
                        outputdir=outputdir, libdir=GetRPMLibDir(),
                        python_versions=install_python_versions)
        oscmd("chmod -R 755 targetroot/usr/share/panda3d")

        oscmd("rpm -E '%_target_cpu' > "+outputdir+"/tmp/architecture.txt")
        arch = ReadFile(outputdir+"/tmp/architecture.txt").strip()
        pandasource = os.path.abspath(os.getcwd())

        txt = INSTALLER_SPEC_FILE[1:]

        # Add the MIME associations if we have pview
        if not PkgSkip("PVIEW"):
            txt += INSTALLER_SPEC_FILE_PVIEW

        # Add the platform-specific Python directories.
        dirs = set()
        for version_info in install_python_versions:
            dirs.add(version_info["platlib"])
            dirs.add(version_info["purelib"])

        for dir in dirs:
            txt += dir + "\n"

        # Add the binaries in /usr/bin explicitly to the spec file
        for base in os.listdir(outputdir + "/bin"):
            if not base.startswith("deploy-stub"):
                txt += "/usr/bin/%s\n" % (base)

        # Write out the spec file.
        txt = txt.replace("VERSION", version)
        txt = txt.replace("RPMRELEASE", str(rpmrelease))
        txt = txt.replace("PANDASOURCE", pandasource)
        WriteFile("panda3d.spec", txt)

        oscmd("fakeroot rpmbuild --define '_rpmdir "+pandasource+"' --buildroot '"+os.path.abspath("targetroot")+"' -bb panda3d.spec")
        oscmd("mv "+arch+"/panda3d-"+version+"-"+rpmrelease+"."+arch+".rpm .")
        oscmd("rm -rf "+arch, True)

    else:
        exit("To build an installer, either rpmbuild or dpkg-deb must be present on your system!")


def MakeInstallerOSX(version, python_versions=[], **kwargs):
    outputdir = GetOutputDir()

    dmg_name = "Panda3D-" + version
    if len(python_versions) == 1 and not python_versions[0]["version"].startswith("2."):
        dmg_name += "-py" + python_versions[0]["version"]
    dmg_name += ".dmg"

    if (os.path.isfile(dmg_name)): oscmd("rm -f %s" % dmg_name)
    if (os.path.exists("dstroot")): oscmd("rm -rf dstroot")
    if (os.path.exists("Panda3D-rw.dmg")): oscmd('rm -f Panda3D-rw.dmg')

    oscmd("mkdir -p dstroot/base/Developer/Panda3D/lib")
    oscmd("mkdir -p dstroot/base/Developer/Panda3D/etc")
    oscmd("cp %s/etc/Config.prc           dstroot/base/Developer/Panda3D/etc/Config.prc" % outputdir)
    oscmd("cp %s/etc/Confauto.prc         dstroot/base/Developer/Panda3D/etc/Confauto.prc" % outputdir)
    oscmd("cp -R %s/models                dstroot/base/Developer/Panda3D/models" % outputdir)
    oscmd("cp -R doc/LICENSE              dstroot/base/Developer/Panda3D/LICENSE")
    oscmd("cp -R doc/ReleaseNotes         dstroot/base/Developer/Panda3D/ReleaseNotes")
    oscmd("cp -R %s/Frameworks            dstroot/base/Developer/Panda3D/Frameworks" % outputdir)
    if os.path.isdir(outputdir+"/plugins"):
        oscmd("cp -R %s/plugins           dstroot/base/Developer/Panda3D/plugins" % outputdir)

    # Libraries that shouldn't be in base, but are instead in other modules.
    no_base_libs = ['libp3ffmpeg', 'libp3fmod_audio', 'libfmodex', 'libfmodexL']

    for base in os.listdir(outputdir+"/lib"):
        if not base.endswith(".a") and base.split('.')[0] not in no_base_libs:
            libname = "dstroot/base/Developer/Panda3D/lib/" + base
            # We really need to specify -R in order not to follow symlinks
            # On OSX, just specifying -P is not enough to do that.
            oscmd("cp -R -P " + outputdir + "/lib/" + base + " " + libname)

    oscmd("mkdir -p dstroot/tools/Developer/Panda3D/bin")
    oscmd("mkdir -p dstroot/tools/Developer/Tools")
    oscmd("ln -s ../Panda3D/bin dstroot/tools/Developer/Tools/Panda3D")
    oscmd("mkdir -p dstroot/tools/etc/paths.d")
    # Trailing newline is important, works around a bug in OSX
    WriteFile("dstroot/tools/etc/paths.d/Panda3D", "/Developer/Panda3D/bin\n")

    oscmd("mkdir -m 0755 -p dstroot/tools/usr/local/share/man/man1")
    oscmd("install -m 0644 doc/man/*.1 dstroot/tools/usr/local/share/man/man1/")

    for base in os.listdir(outputdir+"/bin"):
        if not base.startswith("deploy-stub"):
            binname = "dstroot/tools/Developer/Panda3D/bin/" + base
            # OSX needs the -R argument to copy symbolic links correctly, it doesn't have -d. How weird.
            oscmd("cp -R " + outputdir + "/bin/" + base + " " + binname)

    if python_versions:
        # Let's only write a ppython link if there is only one Python version.
        if len(python_versions) == 1:
            oscmd("mkdir -p dstroot/pythoncode/usr/local/bin")
            oscmd("ln -s %s dstroot/pythoncode/usr/local/bin/ppython" % (python_versions[0]["executable"]))

        oscmd("mkdir -p dstroot/pythoncode/Developer/Panda3D/panda3d")
        oscmd("cp -R %s/pandac                dstroot/pythoncode/Developer/Panda3D/pandac" % outputdir)
        oscmd("cp -R %s/direct                dstroot/pythoncode/Developer/Panda3D/direct" % outputdir)
        oscmd("cp -R %s/*.so                  dstroot/pythoncode/Developer/Panda3D/" % outputdir, True)
        oscmd("cp -R %s/*.py                  dstroot/pythoncode/Developer/Panda3D/" % outputdir, True)
        if os.path.isdir(outputdir+"/Pmw"):
            oscmd("cp -R %s/Pmw               dstroot/pythoncode/Developer/Panda3D/Pmw" % outputdir)

        # Copy over panda3d.dist-info directory.
        if os.path.isdir(outputdir + "/panda3d.dist-info"):
            oscmd("cp -R %s/panda3d.dist-info dstroot/pythoncode/Developer/Panda3D/panda3d.dist-info" % (outputdir))

        for base in os.listdir(outputdir+"/panda3d"):
            if base.endswith('.py'):
                libname = "dstroot/pythoncode/Developer/Panda3D/panda3d/" + base
                oscmd("cp -R " + outputdir + "/panda3d/" + base + " " + libname)

    for version_info in python_versions:
        pyver = version_info["version"]
        oscmd("mkdir -p dstroot/pybindings%s/Library/Python/%s/site-packages" % (pyver, pyver))
        oscmd("mkdir -p dstroot/pybindings%s/Developer/Panda3D/panda3d" % (pyver))

        # Copy over extension modules.
        suffix = version_info["ext_suffix"]
        for base in os.listdir(outputdir+"/panda3d"):
            if base.endswith(suffix) and '.' not in base[:-len(suffix)]:
                libname = "dstroot/pybindings%s/Developer/Panda3D/panda3d/%s" % (pyver, base)
                # We really need to specify -R in order not to follow symlinks
                # On OSX, just specifying -P is not enough to do that.
                oscmd("cp -R -P " + outputdir + "/panda3d/" + base + " " + libname)

        # Write a .pth file.
        oscmd("mkdir -p dstroot/pybindings%s/Library/Python/%s/site-packages" % (pyver, pyver))
        WriteFile("dstroot/pybindings%s/Library/Python/%s/site-packages/Panda3D.pth" % (pyver, pyver), "/Developer/Panda3D")

        # Somewhere in Python 2.7.13 and 3.7, the above path was removed from
        # sys.path of the python.org distribution.  See bpo-28440 and GH #502.
        if pyver not in ("3.0", "3.1", "3.2", "3.3", "3.4", "3.5", "3.6"):
            dir = "dstroot/pybindings%s/Library/Frameworks/Python.framework/Versions/%s/lib/python%s/site-packages" % (pyver, pyver, pyver)
            oscmd("mkdir -p %s" % (dir))
            WriteFile("%s/Panda3D.pth" % (dir), "/Developer/Panda3D")

        # Also place it somewhere the Homebrew version of Python can find it.
        dir = "dstroot/pybindings%s/usr/local/lib/python%s/site-packages" % (pyver, pyver)
        oscmd("mkdir -p %s" % (dir))
        WriteFile("%s/Panda3D.pth" % (dir), "/Developer/Panda3D")

    if not PkgSkip("FFMPEG"):
        oscmd("mkdir -p dstroot/ffmpeg/Developer/Panda3D/lib")
        oscmd("cp -R %s/lib/libp3ffmpeg.* dstroot/ffmpeg/Developer/Panda3D/lib/" % outputdir)

    #if not PkgSkip("OPENAL"):
    #    oscmd("mkdir -p dstroot/openal/Developer/Panda3D/lib")
    #    oscmd("cp -R %s/lib/libp3openal_audio.* dstroot/openal/Developer/Panda3D/lib/" % outputdir)

    if not PkgSkip("FMODEX"):
        oscmd("mkdir -p dstroot/fmodex/Developer/Panda3D/lib")
        oscmd("cp -R %s/lib/libp3fmod_audio.* dstroot/fmodex/Developer/Panda3D/lib/" % outputdir)
        oscmd("cp -R %s/lib/libfmodex* dstroot/fmodex/Developer/Panda3D/lib/" % outputdir)

    oscmd("mkdir -p dstroot/headers/Developer/Panda3D/lib")
    oscmd("cp -R %s/include               dstroot/headers/Developer/Panda3D/include" % outputdir)

    if os.path.isdir("samples"):
        oscmd("mkdir -p dstroot/samples/Developer/Examples/Panda3D")
        oscmd("cp -R samples/* dstroot/samples/Developer/Examples/Panda3D/")

    DeleteVCS("dstroot")
    DeleteBuildFiles("dstroot")

    # Compile Python files.  Do this *after* the DeleteVCS step, above, which
    # deletes __pycache__ directories.
    for version_info in python_versions:
        if os.path.isdir("dstroot/pythoncode/Developer/Panda3D/Pmw"):
            oscmd("%s -m compileall -q -f -d /Developer/Panda3D/Pmw dstroot/pythoncode/Developer/Panda3D/Pmw" % (version_info["executable"]), True)
        oscmd("%s -m compileall -q -f -d /Developer/Panda3D/direct dstroot/pythoncode/Developer/Panda3D/direct" % (version_info["executable"]))
        oscmd("%s -m compileall -q -f -d /Developer/Panda3D/pandac dstroot/pythoncode/Developer/Panda3D/pandac" % (version_info["executable"]))
        oscmd("%s -m compileall -q -f -d /Developer/Panda3D/panda3d dstroot/pythoncode/Developer/Panda3D/panda3d" % (version_info["executable"]))

    oscmd("chmod -R 0775 dstroot/*")
    # We need to be root to perform a chown. Bleh.
    # Fortunately PackageMaker does it for us, on 10.5 and above.
    #oscmd("chown -R root:admin dstroot/*", True)

    oscmd("mkdir -p dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/")
    oscmd("mkdir -p dstroot/Panda3D/Panda3D.mpkg/Contents/Resources/en.lproj/")

    pkgs = ["base", "tools", "headers"]
    if python_versions:
        pkgs.append("pythoncode")
    for version_info in python_versions:
        pkgs.append("pybindings" + version_info["version"])
    if not PkgSkip("FFMPEG"):    pkgs.append("ffmpeg")
    #if not PkgSkip("OPENAL"):    pkgs.append("openal")
    if not PkgSkip("FMODEX"):    pkgs.append("fmodex")
    if os.path.isdir("samples"): pkgs.append("samples")
    for pkg in pkgs:
        identifier = "org.panda3d.panda3d.%s.pkg" % pkg
        plist = open("/tmp/Info_plist", "w")
        plist.write(Info_plist.format(package_id=identifier, version=version))
        plist.close()
        if not os.path.isdir("dstroot/" + pkg):
            os.makedirs("dstroot/" + pkg)

        if os.path.exists("/usr/bin/pkgbuild"):
            # This new package builder is used in Lion and above.
            cmd = '/usr/bin/pkgbuild --identifier ' + identifier + ' --version ' + version + ' --root dstroot/' + pkg + '/ dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/' + pkg + '.pkg'

        # In older versions, we use PackageMaker.  Apple keeps changing its location.
        elif os.path.exists("/Developer/usr/bin/packagemaker"):
            cmd = '/Developer/usr/bin/packagemaker --info /tmp/Info_plist --version ' + version + ' --out dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/' + pkg + '.pkg ' + target + ' --domain system --root dstroot/' + pkg + '/ --no-relocate'
        elif os.path.exists("/Applications/Xcode.app/Contents/Applications/PackageMaker.app/Contents/MacOS/PackageMaker"):
            cmd = '/Applications/Xcode.app/Contents/Applications/PackageMaker.app/Contents/MacOS/PackageMaker --info /tmp/Info_plist --version ' + version + ' --out dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/' + pkg + '.pkg ' + target + ' --domain system --root dstroot/' + pkg + '/ --no-relocate'
        elif os.path.exists("/Developer/Tools/PackageMaker.app/Contents/MacOS/PackageMaker"):
            cmd = '/Developer/Tools/PackageMaker.app/Contents/MacOS/PackageMaker --info /tmp/Info_plist --version ' + version + ' --out dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/' + pkg + '.pkg ' + target + ' --domain system --root dstroot/' + pkg + '/ --no-relocate'
        elif os.path.exists("/Developer/Tools/packagemaker"):
            cmd = '/Developer/Tools/packagemaker -build -f dstroot/' + pkg + '/ -p dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/' + pkg + '.pkg -i /tmp/Info_plist'
        elif os.path.exists("/Applications/PackageMaker.app/Contents/MacOS/PackageMaker"):
            cmd = '/Applications/PackageMaker.app/Contents/MacOS/PackageMaker --info /tmp/Info_plist --version ' + version + ' --out dstroot/Panda3D/Panda3D.mpkg/Contents/Packages/' + pkg + '.pkg ' + target + ' --domain system --root dstroot/' + pkg + '/ --no-relocate'
        else:
            exit("Neither pkgbuild nor PackageMaker could be found!")
        oscmd(cmd)

    if os.path.isfile("/tmp/Info_plist"):
        oscmd("rm -f /tmp/Info_plist")

    # Now that we've built all of the individual packages, build the metapackage.
    dist = open("dstroot/Panda3D/Panda3D.mpkg/Contents/distribution.dist", "w")
    dist.write('<?xml version="1.0" encoding="utf-8"?>\n')
    dist.write('<installer-script minSpecVersion="1.000000" authoringTool="com.apple.PackageMaker" authoringToolVersion="3.0.3" authoringToolBuild="174">\n')
    dist.write('    <title>Panda3D SDK %s</title>\n' % (version))
    dist.write('    <options customize="always" allow-external-scripts="no" rootVolumeOnly="false"/>\n')
    dist.write('    <license language="en" mime-type="text/plain">%s</license>\n' % ReadFile("doc/LICENSE"))
    dist.write('    <script>\n')
    dist.write('    function isPythonVersionInstalled(version) {\n')
    dist.write('        return system.files.fileExistsAtPath("/usr/bin/python" + version)\n')
    dist.write('            || system.files.fileExistsAtPath("/usr/local/bin/python" + version)\n')
    dist.write('            || system.files.fileExistsAtPath("/opt/local/bin/python" + version)\n')
    dist.write('            || system.files.fileExistsAtPath("/sw/bin/python" + version)\n')
    dist.write('            || system.files.fileExistsAtPath("/System/Library/Frameworks/Python.framework/Versions/" + version + "/bin/python")\n')
    dist.write('            || system.files.fileExistsAtPath("/Library/Frameworks/Python.framework/Versions/" + version + "/bin/python");\n')
    dist.write('    }\n')
    dist.write('    </script>\n')
    dist.write('    <choices-outline>\n')
    dist.write('        <line choice="base"/>\n')
    if python_versions:
        dist.write('        <line choice="pythoncode">\n')
        for version_info in sorted(python_versions, key=lambda info:info["version"], reverse=True):
            dist.write('            <line choice="pybindings%s"/>\n' % (version_info["version"]))
        dist.write('        </line>\n')
    dist.write('        <line choice="tools"/>\n')
    if os.path.isdir("samples"):
        dist.write('        <line choice="samples"/>\n')
    if not PkgSkip("FFMPEG"):
        dist.write('        <line choice="ffmpeg"/>\n')
    if not PkgSkip("FMODEX"):
        dist.write('        <line choice="fmodex"/>\n')
    dist.write('        <line choice="headers"/>\n')
    dist.write('    </choices-outline>\n')
    dist.write('    <choice id="base" title="Panda3D Base Installation" description="This package contains the Panda3D libraries, configuration files and models/textures that are needed to use Panda3D.&#10;&#10;Location: /Developer/Panda3D/" start_enabled="false">\n')
    dist.write('        <pkg-ref id="org.panda3d.panda3d.base.pkg"/>\n')
    dist.write('    </choice>\n')
    dist.write('    <choice id="tools" title="Tools" tooltip="Useful tools and model converters to help with Panda3D development" description="This package contains the various utilities that ship with Panda3D, including packaging tools, model converters, and many more.&#10;&#10;Location: /Developer/Panda3D/bin/">\n')
    dist.write('        <pkg-ref id="org.panda3d.panda3d.tools.pkg"/>\n')
    dist.write('    </choice>\n')

    if python_versions:
        dist.write('    <choice id="pythoncode" title="Python Support" tooltip="Python bindings for the Panda3D libraries" description="This package contains the \'direct\', \'pandac\' and \'panda3d\' python packages that are needed to do Python development with Panda3D.&#10;&#10;Location: /Developer/Panda3D/">\n')
        dist.write('        <pkg-ref id="org.panda3d.panda3d.pythoncode.pkg"/>\n')
        dist.write('    </choice>\n')

    for version_info in python_versions:
        pyver = version_info["version"]
        if pyver == "2.7":
            # Always install Python 2.7 by default; it's included on macOS.
            cond = "true"
        else:
            cond = "isPythonVersionInstalled('%s')" % (pyver)
        dist.write('    <choice id="pybindings%s" start_selected="%s" title="Python %s Bindings" tooltip="Python bindings for the Panda3D libraries" description="Support for Python %s.">\n' % (pyver, cond, pyver, pyver))
        dist.write('        <pkg-ref id="org.panda3d.panda3d.pybindings%s.pkg"/>\n' % (pyver))
        dist.write('    </choice>\n')

    if not PkgSkip("FFMPEG"):
        dist.write('    <choice id="ffmpeg" title="FFMpeg Plug-In" tooltip="FFMpeg video and audio decoding plug-in" description="This package contains the FFMpeg plug-in, which is used for decoding video and audio files with OpenAL.')
        if PkgSkip("VORBIS") and PkgSkip("OPUS"):
            dist.write('  It is not required for loading .wav files, which Panda3D can read out of the box.">\n')
        elif PkgSkip("VORBIS"):
            dist.write('  It is not required for loading .wav or .opus files, which Panda3D can read out of the box.">\n')
        elif PkgSkip("OPUS"):
            dist.write('  It is not required for loading .wav or .ogg files, which Panda3D can read out of the box.">\n')
        else:
            dist.write('  It is not required for loading .wav, .ogg or .opus files, which Panda3D can read out of the box.">\n')
        dist.write('        <pkg-ref id="org.panda3d.panda3d.ffmpeg.pkg"/>\n')
        dist.write('    </choice>\n')

    #if not PkgSkip("OPENAL"):
    #    dist.write('    <choice id="openal" title="OpenAL Audio Plug-In" tooltip="OpenAL audio output plug-in" description="This package contains the OpenAL audio plug-in, which is an open-source library for playing sounds.">\n')
    #    dist.write('        <pkg-ref id="org.panda3d.panda3d.openal.pkg"/>\n')
    #    dist.write('    </choice>\n')

    if not PkgSkip("FMODEX"):
        dist.write('    <choice id="fmodex" title="FMOD Ex Plug-In" tooltip="FMOD Ex audio output plug-in" description="This package contains the FMOD Ex audio plug-in, which is a commercial library for playing sounds.  It is an optional component as Panda3D can use the open-source alternative OpenAL instead.">\n')
        dist.write('        <pkg-ref id="org.panda3d.panda3d.fmodex.pkg"/>\n')
        dist.write('    </choice>\n')

    if os.path.isdir("samples"):
        dist.write('    <choice id="samples" title="Sample Programs" tooltip="Python sample programs that use Panda3D" description="This package contains the Python sample programs that can help you with learning how to use Panda3D.&#10;&#10;Location: /Developer/Examples/Panda3D/">\n')
        dist.write('        <pkg-ref id="org.panda3d.panda3d.samples.pkg"/>\n')
        dist.write('    </choice>\n')

    dist.write('    <choice id="headers" title="C++ Header Files" tooltip="Header files for C++ development with Panda3D" description="This package contains the C++ header files that are needed in order to do C++ development with Panda3D. You don\'t need this if you want to develop in Python.&#10;&#10;Location: /Developer/Panda3D/include/" start_selected="false">\n')
    dist.write('        <pkg-ref id="org.panda3d.panda3d.headers.pkg"/>\n')
    dist.write('    </choice>\n')
    for pkg in pkgs:
        size = GetDirectorySize("dstroot/" + pkg) // 1024
        dist.write('    <pkg-ref id="org.panda3d.panda3d.%s.pkg" installKBytes="%d" version="1" auth="Root">file:./Contents/Packages/%s.pkg</pkg-ref>\n' % (pkg, size, pkg))
    dist.write('</installer-script>\n')
    dist.close()

    oscmd('hdiutil create Panda3D-rw.dmg -volname "Panda3D SDK %s" -srcfolder dstroot/Panda3D' % (version))
    oscmd('hdiutil convert Panda3D-rw.dmg -format UDBZ -o %s' % (dmg_name))
    oscmd('rm -f Panda3D-rw.dmg')


def MakeInstallerFreeBSD(version, python_versions=[], **kwargs):
    outputdir = GetOutputDir()

    oscmd("rm -rf targetroot +DESC pkg-plist +MANIFEST")
    oscmd("mkdir targetroot")

    # Invoke installpanda.py to install it into a temporary dir
    InstallPanda(destdir="targetroot", prefix="/usr/local", outputdir=outputdir, python_versions=python_versions)

    if not os.path.exists("/usr/sbin/pkg"):
        exit("Cannot create an installer without pkg")

    plist_txt = ''
    for root, dirs, files in os.walk("targetroot/usr/local/", True):
        for f in files:
            plist_txt += os.path.join(root, f)[21:] + "\n"

    plist_txt += "@postexec /sbin/ldconfig -m /usr/local/lib/panda3d\n"
    plist_txt += "@postunexec /sbin/ldconfig -R\n"

    for remdir in ("lib/panda3d", "share/panda3d", "include/panda3d"):
        for root, dirs, files in os.walk("targetroot/usr/local/" + remdir, False):
            for d in dirs:
                plist_txt += "@dir %s\n" % os.path.join(root, d)[21:]
        plist_txt += "@dir %s\n" % remdir

    oscmd("echo \"`pkg config abi | tr '[:upper:]' '[:lower:]' | cut -d: -f1,2`:*\" > " + outputdir + "/tmp/architecture.txt")
    pkg_arch = ReadFile(outputdir+"/tmp/architecture.txt").strip()

    dependencies = ''
    if not PkgSkip("PYTHON"):
        # If this version of Python was installed from a package or ports, let's mark it as dependency.
        oscmd("rm -f %s/tmp/python_dep" % outputdir)

        if "PYTHONVERSION" in SDK:
            pyver_nodot = SDK["PYTHONVERSION"][6:9:2]
        else:
            pyver_nodot = "%d%d" % (sys.version_info[:2])

        oscmd("pkg query \"\n\t%%n : {\n\t\torigin : %%o,\n\t\tversion : %%v\n\t},\n\" python%s > %s/tmp/python_dep" % (pyver_nodot, outputdir), True)
        if os.path.isfile(outputdir + "/tmp/python_dep"):
            python_pkg = ReadFile(outputdir + "/tmp/python_dep")
            if python_pkg:
                dependencies += python_pkg

    manifest_txt = INSTALLER_PKG_MANIFEST_FILE[1:].replace("NAME", 'panda3d')
    manifest_txt = manifest_txt.replace("VERSION", version)
    manifest_txt = manifest_txt.replace("ARCH", pkg_arch)
    manifest_txt = manifest_txt.replace("ORIGIN", 'devel/panda3d')
    manifest_txt = manifest_txt.replace("DEPENDS", dependencies)
    manifest_txt = manifest_txt.replace("INSTSIZE", str(GetDirectorySize("targetroot") // 1024 // 1024))

    WriteFile("pkg-plist", plist_txt)
    WriteFile("+DESC", INSTALLER_PKG_DESCR_FILE[1:])
    WriteFile("+MANIFEST", manifest_txt)
    oscmd("pkg create -p pkg-plist -r %s  -m . -o . %s" % (os.path.abspath("targetroot"), "--verbose" if GetVerbose() else "--quiet"))


def MakeInstallerAndroid(version, **kwargs):
    outputdir = GetOutputDir()
    oscmd("rm -rf apkroot")
    oscmd("mkdir apkroot")

    # Also remove the temporary apks.
    apk_unaligned = os.path.join(outputdir, "tmp", "panda3d-unaligned.apk")
    apk_unsigned = os.path.join(outputdir, "tmp", "panda3d-unsigned.apk")
    if os.path.exists(apk_unaligned):
        os.unlink(apk_unaligned)
    if os.path.exists(apk_unsigned):
        os.unlink(apk_unsigned)

    # Compile the Java classes into a Dalvik executable.
    dx_cmd = "dx --dex --output=apkroot/classes.dex "
    if GetOptimize() <= 2:
        dx_cmd += "--debug "
    if GetVerbose():
        dx_cmd += "--verbose "
    if "ANDROID_API" in SDK:
        dx_cmd += "--min-sdk-version=%d " % (SDK["ANDROID_API"])
    dx_cmd += os.path.join(outputdir, "classes")
    oscmd(dx_cmd)

    # Copy the libraries one by one.  In case of library dependencies, strip
    # off any suffix (eg. libfile.so.1.0), as Android does not support them.
    source_dir = os.path.join(outputdir, "lib")
    target_dir = os.path.join("apkroot", "lib", SDK["ANDROID_ABI"])
    if not os.path.exists(target_dir):
        os.makedirs(target_dir, mode=0o755)

    # Determine the library directories we should look in.
    libpath = [source_dir]
    for dir in os.environ.get("LD_LIBRARY_PATH", "").split(':'):
        dir = os.path.expandvars(dir)
        dir = os.path.expanduser(dir)
        if os.path.isdir(dir):
            dir = os.path.realpath(dir)
            if not dir.startswith("/system") and not dir.startswith("/vendor"):
                libpath.append(dir)

    def copy_library(source, base):
        # Copy file to destination, stripping version suffix.
        target = os.path.join(target_dir, base)
        if not target.endswith('.so'):
            target = target.rpartition('.so.')[0] + '.so'

        if os.path.isfile(target):
            # Already processed.
            return

        shutil.copy(source, target)

        # Walk through the library dependencies.
        handle = subprocess.Popen(['readelf', '--dynamic', target], stdout=subprocess.PIPE)
        for line in handle.communicate()[0].splitlines():
            # The line will look something like:
            # 0x0000000000000001 (NEEDED)             Shared library: [libpanda.so]
            line = line.decode('utf-8', 'replace').strip()
            if not line or '(NEEDED)' not in line or '[' not in line or ']' not in line:
                continue

            # Extract the part between square brackets.
            idx = line.index('[')
            dep = line[idx + 1 : line.index(']', idx)]

            # Change .so.1.2 suffix to .so, as needed for loading in .apk
            if '.so.' in dep:
                orig_dep = dep
                dep = dep.rpartition('.so.')[0] + '.so'
                oscmd("patchelf --replace-needed %s %s %s" % (orig_dep, dep, target), True)

            # Find it on the LD_LIBRARY_PATH.
            for dir in libpath:
                fulldep = os.path.join(dir, dep)
                if os.path.isfile(fulldep):
                    copy_library(os.path.realpath(fulldep), dep)
                    break

    # Now copy every lib in the lib dir, and its dependencies.
    for base in os.listdir(source_dir):
        if not base.startswith('lib'):
            continue
        if not base.endswith('.so') and '.so.' not in base:
            continue

        source = os.path.join(source_dir, base)
        if os.path.islink(source):
            continue
        copy_library(source, base)

    # Same for Python extension modules.  However, Android is strict about
    # library naming, so we have a special naming scheme for these, in
    # conjunction with a custom import hook to find these modules.
    if not PkgSkip("PYTHON"):
        suffix = GetExtensionSuffix()
        source_dir = os.path.join(outputdir, "panda3d")
        for base in os.listdir(source_dir):
            if not base.endswith(suffix):
                continue
            modname = base[:-len(suffix)]
            if '.' not in modname:
                source = os.path.join(source_dir, base)
                copy_library(source, "libpy.panda3d.{}.so".format(modname))

        # Same for standard Python modules.
        import _ctypes
        source_dir = os.path.dirname(_ctypes.__file__)
        for base in os.listdir(source_dir):
            if not base.endswith('.so'):
                continue
            modname = base.partition('.')[0]
            source = os.path.join(source_dir, base)
            copy_library(source, "libpy.{}.so".format(modname))

    def copy_python_tree(source_root, target_root):
        for source_dir, dirs, files in os.walk(source_root):
            if 'site-packages' in dirs:
                dirs.remove('site-packages')

            if not any(base.endswith('.py') for base in files):
                continue

            target_dir = os.path.join(target_root, os.path.relpath(source_dir, source_root))
            target_dir = os.path.normpath(target_dir)
            os.makedirs(target_dir, 0o755)

            for base in files:
                if base.endswith('.py'):
                    target = os.path.join(target_dir, base)
                    shutil.copy(os.path.join(source_dir, base), target)

    # Copy the Python standard library to the .apk as well.
    from distutils.sysconfig import get_python_lib
    stdlib_source = get_python_lib(False, True)
    stdlib_target = os.path.join("apkroot", "lib", "python{0}.{1}".format(*sys.version_info))
    copy_python_tree(stdlib_source, stdlib_target)

    # But also copy over our custom site.py.
    shutil.copy("panda/src/android/site.py", os.path.join(stdlib_target, "site.py"))

    # And now make a site-packages directory containing our direct/panda3d/pandac modules.
    for tree in "panda3d", "direct", "pandac":
        copy_python_tree(os.path.join(outputdir, tree), os.path.join(stdlib_target, "site-packages", tree))

    # Copy the models and config files to the virtual assets filesystem.
    oscmd("mkdir apkroot/assets")
    oscmd("cp -R %s apkroot/assets/models" % (os.path.join(outputdir, "models")))
    oscmd("cp -R %s apkroot/assets/etc" % (os.path.join(outputdir, "etc")))

    # Make an empty res folder.  It's needed for the apk to be installable, apparently.
    oscmd("mkdir apkroot/res")

    # Now package up the application
    oscmd("cp panda/src/android/pview_manifest.xml apkroot/AndroidManifest.xml")
    aapt_cmd = "aapt package"
    aapt_cmd += " -F %s" % (apk_unaligned)
    aapt_cmd += " -M apkroot/AndroidManifest.xml"
    aapt_cmd += " -A apkroot/assets -S apkroot/res"
    aapt_cmd += " -I %s" % (SDK["ANDROID_JAR"])
    oscmd(aapt_cmd)

    # And add all the libraries to it.
    oscmd("aapt add %s classes.dex" % (os.path.join('..', apk_unaligned)), cwd="apkroot")
    for path, dirs, files in os.walk('apkroot/lib'):
        if files:
            rel = os.path.relpath(path, 'apkroot')
            rel_files = [os.path.join(rel, file).replace('\\', '/') for file in files]
            oscmd("aapt add %s %s" % (os.path.join('..', apk_unaligned), ' '.join(rel_files)), cwd="apkroot")

    # Now align the .apk, which is necessary for Android to load it.
    oscmd("zipalign -v -p 4 %s %s" % (apk_unaligned, apk_unsigned))

    # Finally, sign it using a debug key.  This is generated if it doesn't exist.
    if GetHost() == 'android':
        # Termux version of apksigner automatically generates a debug key.
        oscmd("apksigner debug.ks %s panda3d.apk" % (apk_unsigned))
    else:
        if not os.path.isfile('debug.ks'):
            oscmd("keytool -genkey -noprompt -dname CN=Panda3D,O=Panda3D,C=US -keystore debug.ks -storepass android -alias androiddebugkey -keypass android -keyalg RSA -keysize 2048 -validity 1000")
        oscmd("apksigner sign --ks debug.ks --ks-pass pass:android --min-sdk-version %s --out panda3d.apk %s" % (SDK["ANDROID_API"], apk_unsigned))

    # Clean up.
    oscmd("rm -rf apkroot")
    os.unlink(apk_unaligned)
    os.unlink(apk_unsigned)


def MakeInstaller(version, **kwargs):
    target = GetTarget()
    if target == 'windows':
        fn = "Panda3D-"
        dir = "Panda3D-" + version

        title = "Panda3D SDK " + version

        fn += version

        python_versions = kwargs.get('python_versions', [])
        if len(python_versions) == 1 and python_versions[0]["version"] != "2.7":
            fn += '-py' + python_versions[0]["version"]

        if GetOptimize() <= 2:
            fn += "-dbg"
        if GetTargetArch() == 'x64':
            fn += '-x64'
            dir += '-x64'

        MakeInstallerNSIS(version, fn + '.exe', title, 'C:\\' + dir, **kwargs)
        MakeDebugSymbolArchive(fn + '-pdb.zip', dir)
    elif target == 'linux':
        MakeInstallerLinux(version, **kwargs)
    elif target == 'darwin':
        MakeInstallerOSX(version, **kwargs)
    elif target == 'freebsd':
        MakeInstallerFreeBSD(version, **kwargs)
    elif target == 'android':
        MakeInstallerAndroid(version, **kwargs)
    else:
        exit("Do not know how to make an installer for this platform")


if __name__ == "__main__":
    version = GetMetadataValue('version')

    parser = OptionParser()
    parser.add_option('', '--version', dest='version', help='Panda3D version number (default: %s)' % (version), default=version)
    parser.add_option('', '--debversion', dest='debversion', help='Version number for .deb file', default=None)
    parser.add_option('', '--rpmrelease', dest='rpmrelease', help='Release number for .rpm file', default='1')
    parser.add_option('', '--outputdir', dest='outputdir', help='Makepanda\'s output directory (default: built)', default='built')
    parser.add_option('', '--verbose', dest='verbose', help='Enable verbose output', action='store_true', default=False)
    parser.add_option('', '--lzma', dest='compressor', help='Use LZMA compression', action='store_const', const='lzma', default='zlib')
    (options, args) = parser.parse_args()

    SetVerbose(options.verbose)
    SetOutputDir(options.outputdir)

    # Read out the optimize option.
    opt = ReadFile(os.path.join(options.outputdir, "tmp", "optimize.dat"))
    SetOptimize(int(opt.strip()))

    # Read out whether we should set PkgSkip("PYTHON") and some others.
    # Temporary hack; needs better solution.
    pkg_list = "PYTHON", "NVIDIACG", "FFMPEG", "OPENAL", "FMODEX", "PVIEW", "NVIDIACG", "VORBIS", "OPUS"
    PkgListSet(pkg_list)
    for pkg in pkg_list:
        dat_path = "dtool_have_%s.dat" % (pkg.lower())
        content = ReadFile(os.path.join(options.outputdir, "tmp", dat_path))
        if int(content.strip()):
            PkgEnable(pkg)
        else:
            PkgDisable(pkg)

    # Parse the version.
    match = re.match(r'^\d+\.\d+(\.\d+)+', options.version)
    if not match:
        exit("version requires three digits")

    MakeInstaller(version=match.group(),
                  outputdir=options.outputdir,
                  optimize=GetOptimize(),
                  compressor=options.compressor,
                  debversion=options.debversion,
                  rpmrelease=options.rpmrelease,
                  python_versions=ReadPythonVersionInfoFile())
