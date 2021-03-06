import distutils.log
import os
import subprocess
import sys
import tarfile
import zipfile
import struct

import panda3d.core as p3d

def create_zip(command, basename, build_dir):
    base_dir = command.get_archive_basedir()

    with zipfile.ZipFile(basename+'.zip', 'w', compression=zipfile.ZIP_DEFLATED) as zf:
        zf.write(build_dir, base_dir)

        for dirpath, dirnames, filenames in os.walk(build_dir):
            dirnames.sort()
            for name in dirnames:
                path = os.path.normpath(os.path.join(dirpath, name))
                zf.write(path, path.replace(build_dir, base_dir, 1))
            for name in filenames:
                path = os.path.normpath(os.path.join(dirpath, name))
                if os.path.isfile(path):
                    zf.write(path, path.replace(build_dir, base_dir, 1))


def create_tarball(command, basename, build_dir, tar_compression):
    base_dir = command.get_archive_basedir()
    build_cmd = command.get_finalized_command('build_apps')
    binary_names = list(build_cmd.console_apps.keys()) + list(build_cmd.gui_apps.keys())

    source_date = os.environ.get('SOURCE_DATE_EPOCH', '').strip()
    if source_date:
        max_mtime = int(source_date)
    else:
        max_mtime = None

    def tarfilter(tarinfo):
        if tarinfo.isdir() or os.path.basename(tarinfo.name) in binary_names:
            tarinfo.mode = 0o755
        else:
            tarinfo.mode = 0o644

        # This isn't interesting information to retain for distribution.
        tarinfo.uid = 0
        tarinfo.gid = 0
        tarinfo.uname = ""
        tarinfo.gname = ""

        if max_mtime is not None and tarinfo.mtime >= max_mtime:
            tarinfo.mtime = max_mtime

        return tarinfo

    filename = '{}.tar.{}'.format(basename, tar_compression)
    with tarfile.open(filename, 'w|{}'.format(tar_compression)) as tf:
        tf.add(build_dir, base_dir, filter=tarfilter)

    if tar_compression == 'gz' and max_mtime is not None:
        # Python provides no elegant way to overwrite the gzip timestamp.
        with open(filename, 'r+b') as fp:
            fp.seek(4)
            fp.write(struct.pack("<L", max_mtime))


def create_gztar(command, basename, build_dir):
    return create_tarball(command, basename, build_dir, 'gz')


def create_bztar(command, basename, build_dir):
    return create_tarball(command, basename, build_dir, 'bz2')


def create_xztar(command, basename, build_dir):
    return create_tarball(command, basename, build_dir, 'xz')


def create_nsis(command, basename, build_dir):
    platform = command.get_current_platform()
    if not platform.startswith('win'):
        command.announce(
            '\tNSIS installer not supported for platform: {}'.format(platform),
            distutils.log.ERROR
        )
        return
    try:
        subprocess.call(['makensis', '--version'])
    except OSError:
        command.announce(
            '\tCould not find makensis tool that is required to build NSIS installers',
            distutils.log.ERROR
        )
        return

    is_64bit = platform == 'win_amd64'
    # Get a list of build applications
    build_cmd = command.get_finalized_command('build_apps')
    apps = build_cmd.gui_apps.copy()
    apps.update(build_cmd.console_apps)
    apps = [
        '{}.exe'.format(i)
        for i in apps
    ]

    shortname = command.distribution.get_name()

    # Create the .nsi installer script
    nsifile = p3d.Filename(build_cmd.build_base, shortname + ".nsi")
    nsifile.unlink()
    nsi = open(nsifile.to_os_specific(), "w")

    # Some global info
    nsi.write('Name "%s"\n' % shortname)
    nsi.write('OutFile "%s"\n' % os.path.join(command.dist_dir, basename+'.exe'))
    if is_64bit:
        nsi.write('InstallDir "$PROGRAMFILES64\\%s"\n' % shortname)
    else:
        nsi.write('InstallDir "$PROGRAMFILES\\%s"\n' % shortname)
    nsi.write('SetCompress auto\n')
    nsi.write('SetCompressor lzma\n')
    nsi.write('ShowInstDetails nevershow\n')
    nsi.write('ShowUninstDetails nevershow\n')
    nsi.write('InstType "Typical"\n')

    # Tell Vista that we require admin rights
    nsi.write('RequestExecutionLevel admin\n')
    nsi.write('\n')

    # TODO offer run and desktop shortcut after we figure out how to deal
    # with multiple apps

    nsi.write('!include "MUI2.nsh"\n')
    nsi.write('!define MUI_ABORTWARNING\n')
    nsi.write('\n')
    nsi.write('Var StartMenuFolder\n')
    nsi.write('!insertmacro MUI_PAGE_WELCOME\n')
    # TODO license file
    nsi.write('!insertmacro MUI_PAGE_DIRECTORY\n')
    nsi.write('!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder\n')
    nsi.write('!insertmacro MUI_PAGE_INSTFILES\n')
    nsi.write('!insertmacro MUI_PAGE_FINISH\n')
    nsi.write('!insertmacro MUI_UNPAGE_WELCOME\n')
    nsi.write('!insertmacro MUI_UNPAGE_CONFIRM\n')
    nsi.write('!insertmacro MUI_UNPAGE_INSTFILES\n')
    nsi.write('!insertmacro MUI_UNPAGE_FINISH\n')
    nsi.write('!insertmacro MUI_LANGUAGE "English"\n')

    # This section defines the installer.
    nsi.write('Section "" SecCore\n')
    nsi.write('  SetOutPath "$INSTDIR"\n')
    curdir = ""
    nsi_dir = p3d.Filename.fromOsSpecific(build_cmd.build_base)
    build_root_dir = p3d.Filename.fromOsSpecific(build_dir)
    for root, dirs, files in os.walk(build_dir):
        dirs.sort()
        for name in files:
            basefile = p3d.Filename.fromOsSpecific(os.path.join(root, name))
            file = p3d.Filename(basefile)
            file.makeAbsolute()
            file.makeRelativeTo(nsi_dir)
            outdir = p3d.Filename(basefile)
            outdir.makeAbsolute()
            outdir.makeRelativeTo(build_root_dir)
            outdir = outdir.getDirname().replace('/', '\\')
            if curdir != outdir:
                nsi.write('  SetOutPath "$INSTDIR\\%s"\n' % outdir)
                curdir = outdir
            nsi.write('  File "%s"\n' % (file.toOsSpecific()))
    nsi.write('  SetOutPath "$INSTDIR"\n')
    nsi.write('  WriteUninstaller "$INSTDIR\\Uninstall.exe"\n')
    nsi.write('  ; Start menu items\n')
    nsi.write('  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application\n')
    nsi.write('    CreateDirectory "$SMPROGRAMS\\$StartMenuFolder"\n')
    for app in apps:
        nsi.write('    CreateShortCut "$SMPROGRAMS\\$StartMenuFolder\\%s.lnk" "$INSTDIR\\%s"\n' % (shortname, app))
    nsi.write('    CreateShortCut "$SMPROGRAMS\\$StartMenuFolder\\Uninstall.lnk" "$INSTDIR\\Uninstall.exe"\n')
    nsi.write('  !insertmacro MUI_STARTMENU_WRITE_END\n')
    nsi.write('SectionEnd\n')

    # This section defines the uninstaller.
    nsi.write('Section Uninstall\n')
    nsi.write('  RMDir /r "$INSTDIR"\n')
    nsi.write('  ; Desktop icon\n')
    nsi.write('  Delete "$DESKTOP\\%s.lnk"\n' % shortname)
    nsi.write('  ; Start menu items\n')
    nsi.write('  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder\n')
    nsi.write('  RMDir /r "$SMPROGRAMS\\$StartMenuFolder"\n')
    nsi.write('SectionEnd\n')
    nsi.close()

    cmd = ['makensis']
    for flag in ["V2"]:
        cmd.append(
            '{}{}'.format('/' if sys.platform.startswith('win') else '-', flag)
        )
    cmd.append(nsifile.to_os_specific())
    subprocess.check_call(cmd)
