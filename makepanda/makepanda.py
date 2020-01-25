#!/usr/bin/env python3

import argparse
import datetime
import os
import platform
import pprint
import shutil
import subprocess
import sys
import time

# If/when makepanda goes away, we can copy these functions over to this script
from makepandacore import DXVERSIONS, MAYAVERSIONS, MAXVERSIONS


SCRIPTDIR = os.path.join(
    os.path.dirname(os.path.abspath(__file__))
)


PKG_LIST = [
    "PYTHON", "DIRECT",                                  # Python support
    "GL", "GLES", "GLES2"] + DXVERSIONS + ["TINYDISPLAY", "NVIDIACG", # 3D graphics
    "EGL",                                               # OpenGL (ES) integration
    "EIGEN",                                             # Linear algebra acceleration
    "OPENAL", "FMODEX",                                  # Audio playback
    "VORBIS", "OPUS", "FFMPEG", "SWSCALE", "SWRESAMPLE", # Audio decoding
    "ODE", "BULLET", "PANDAPHYSICS",                     # Physics
    "SPEEDTREE",                                         # SpeedTree
    "ZLIB", "PNG", "JPEG", "TIFF", "OPENEXR", "SQUISH",  # 2D Formats support
    ] + MAYAVERSIONS + MAXVERSIONS + [ "FCOLLADA", "ASSIMP", "EGG", # 3D Formats support
    "FREETYPE", "HARFBUZZ",                              # Text rendering
    "VRPN", "OPENSSL",                                   # Transport
    "FFTW",                                              # Algorithm helpers
    "ARTOOLKIT", "OPENCV", "DIRECTCAM", "VISION",        # Augmented Reality
    "GTK2",                                              # GTK2 is used for PStats on Unix
    "MFC", "WX", "FLTK",                                 # Used for web plug-in only
    "COCOA",                                             # Mac OS X toolkits
    "X11",                                               # Unix platform support
    "PANDATOOL", "PVIEW", "DEPLOYTOOLS",                 # Toolchain
    "SKEL",                                              # Example SKEL project
    "PANDAFX",                                           # Some distortion special lenses
    "PANDAPARTICLESYSTEM",                               # Built in particle system
    "CONTRIB",                                           # Experimental
    "SSE2", "NEON",                                      # Compiler features
]

HAVE_PKGS = [
    'ARTOOLKIT',
    'ASSIMP',
    'BULLET',
    'GL',
    'GLES',
    'GLES2',
    'EGL',
    'EIGEN',
    'FCOLLADA',
    'FFMPEG',
    'FFTW',
    'FMODEX',
    'FREETYPE',
    'GTK2',
    'HARFBUZZ',
    'JPEG',
    'NVIDIACG',
    'ODE',
    'OPENAL',
    'OPENCV',
    'OPENEXR',
    'OPENSSL',
    'OPUS',
    'PNG',
    'SPEEDTREE',
    'SQUISH',
    'SSE2',
    'SWRESAMPLE',
    'SWSCALE',
    'TIFF',
    'TINYDISPLAY',
    'VORBIS',
    'VRPN',
    'X11',
    'ZLIB',
]

ARG_TO_HAVE_MAP = {
    'NVIDIACG': 'CG',
    'GLES': 'GLES1',
}


BUILD_PKGS = [
    'DIRECT',
    'PANDATOOL',
    'CONTRIB',
]


def gen_usage(problem=None):
    custom_pkg_str = (
        "  --<PKG>-incdir    (custom location for header files of thirdparty package)\n"
        "  --<PKG>-libdir    (custom location for library files of thirdparty package)\n"
    ) if sys.platform != 'win32' else ''
    fail_str = 'makepanda: error: {}' if problem else ''
    return (
        "Makepanda generates a 'built' subdirectory containing a\n"
        "compiled copy of Panda3D.  Command-line arguments are:\n"
        "\n"
        "  --help            (print the help message you're reading now)\n"
        "  --verbose         (print out more information)\n"
        "  --tests           (run the test suite)\n"
        "  --installer       (build an installer)\n"
        "  --wheel           (build a pip-installable .whl)\n"
        "  --optimize X      (optimization level can be 1,2,3,4)\n"
        "  --version X       (set the panda version number)\n"
        "  --lzma            (use lzma compression when building Windows installer)\n"
        "  --distributor X   (short string identifying the distributor of the build)\n"
        "  --outputdir X     (use the specified directory instead of 'built')\n"
        "  --threads N       (use the multithreaded build system. see manual)\n"
        "  --osxtarget N     (the OS X version number to build for (OS X only))\n"
        "  --override \"O=V\"  (override dtool_config/prc option value)\n"
        "  --static          (builds libraries for static linking)\n"
        "  --target X        (experimental cross-compilation (android only))\n"
        "  --arch X          (target architecture for cross-compilation)\n"
        + "".join([
            "  --use-%-9s   --no-%-9s (enable/disable use of %s)\n" % (pkg.lower(), pkg.lower(), pkg)
            for pkg in PKG_LIST
        ]) +
        custom_pkg_str +
        "\n"
        "  --nothing         (disable every third-party lib)\n"
        "  --everything      (enable every third-party lib)\n"
        "  --directx-sdk X   (specify version of DirectX SDK to use: jun2010, aug2009)\n"
        "  --windows-sdk X   (specify Windows SDK version, eg. 7.1, 8.1 or 10.  Default is 8.1)\n"
        "  --msvc-version X  (specify Visual C++ version, eg. 11, 12, 14, 14.1, 14.2.  Default is 14)\n"
        "  --use-icl         (experimental setting to use an intel compiler instead of MSVC on Windows)\n"
        "\n"
        "The simplest way to compile panda is to just type:\n"
        "\n"
        "  makepanda --everything\n"
        "\n" +
        fail_str
    )


def parse_args():
    parser = argparse.ArgumentParser(usage=gen_usage(), prog='makepanda', add_help=False)
    parser.add_argument('--help', '-h', action='store_true')
    parser.add_argument('--verbose', action='store_true')
    parser.add_argument('--tests', action='store_true')
    # parser.add_argument('--installer', action='store_true')
    parser.add_argument('--wheel', action='store_true')
    parser.add_argument('--optimize', type=int, choices=(1, 2, 3, 4), default=3)
    # parser.add_argument('--lzma', action='store_true')
    parser.add_argument('--distributor', default='makepanda')
    parser.add_argument('--outputdir', default='built')
    parser.add_argument('--threads', type=int)
    parser.add_argument('--osxtarget')
    # parser.add_argument('--override', action='append')
    parser.add_argument('--static', action='store_true')
    # parser.add_argument('--target')
    parser.add_argument('--arch')
    parser.add_argument('--nothing', action='store_true')
    parser.add_argument('--everything', action='store_true')
    # parser.add_argument('--directx-sdk')
    parser.add_argument('--windows-sdk')
    # parser.add_argument('--msvc-version')
    # parser.add_argument('--use-icl')
    parser.add_argument('--python-incdir')
    parser.add_argument('--python-libdir')
    parser.add_argument('--git-commit') # unused, cmake handles automatically
    for pkg in HAVE_PKGS + BUILD_PKGS:
        group = parser.add_mutually_exclusive_group()
        group.add_argument(
            f'--use-{pkg.lower()}',
            dest='incpkgs',
            action='append_const',
            const=pkg
        )
        group.add_argument(
            f'--no-{pkg.lower()}',
            dest='expkgs',
            action='append_const',
            const=pkg
        )

    args = parser.parse_args()

    # Generate a package list and store it on args
    pkgs = set(PKG_LIST[:] if args.everything else [])

    if args.incpkgs:
        for pkg in args.incpkgs:
            pkgs.add(pkg)

    if args.expkgs:
        for pkg in args.expkgs:
            if pkg in pkgs:
                pkgs.remove(pkg)

    if not pkgs:
        print(gen_usage(
            'specify a list of packages to use or --everything to enable all packages'
        ), file=sys.stderr)
        sys.exit(1)

    args.pkgs = pkgs

    if args.git_commit:
        print('--git-commit flag is being ignored (this is now handled automatically)')
    if args.windows_sdk:
        print('--windows-sdk flag is being ignored (this is now handled automatically)')

    return args


def opt_to_config(optlevel):
    if optlevel == 4:
        return 'Release'
    if optlevel == 3:
        return 'Standard'
    return 'Debug'


def gen_config(cmakedir, cli_args):
    cachepath = os.path.join(cmakedir, 'CMakeCache.txt')

    args = [
        'cmake',
        '-S', '.',
        '-B', cmakedir,
        f'-DWANT_PYTHON_VERSION={".".join([str(i) for i in sys.version_info[0:2]])}',
    ]

    if cli_args.arch:
        args += [
            '-A', 'x64'
        ]
    elif sys.platform == 'win32' and platform.architecture()[0] == '64bit':
        args += [
            '-A', 'x64'
        ]

    # Prefer Ninja if available
    if shutil.which('ninja'):
        args += [
            '-G', 'Ninja'
        ]

    # Handle HAVE_* flags
    try:
        with open(cachepath) as cachefile:
            found_pkgs = {
                line.split('_')[0]
                for line in cachefile.readlines()
                if 'INCLUDE_DIR:PATH' in line and 'NOTFOUND' not in line
            }
    except FileNotFoundError:
        found_pkgs = set()

    def have_value(pkg):
        if pkg not in cli_args.pkgs:
            # This is easy, just disable
            return 'OFF'
        # Otherwise we only want to turn this ON if we've found the package
        # before
        return 'ON' if ARG_TO_HAVE_MAP.get(pkg, pkg) in found_pkgs else None


    args += [
        f'-DHAVE_{ARG_TO_HAVE_MAP.get(pkg, pkg)}={have_value(pkg)}'
        for pkg in HAVE_PKGS
        if have_value(pkg)
    ]

    # Handle BUILD_* flags
    args += [
        f'-DBUILD_{comp}={"ON" if comp in cli_args.pkgs else "OFF"}'
        for comp in BUILD_PKGS
    ]

    # Other CLI args
    if cli_args.distributor:
        args.append(f'-DPANDA_DISTRIBUTOR={cli_args.distributor}')
    if cli_args.osxtarget:
        args.append(f'-DCMAKE_OSX_DEPLOYMENT_TARGET={cli_args.osxtarget}')
    args.append(f'-DBUILD_SHARED_LIBS={"OFF" if cli_args.static else "ON"}')

    pyloc = None
    if cli_args.python_incdir:
        pyloc = os.path.dirname(cli_args.python_incdir)
    elif cli_args.python_libdir:
        pyloc = os.path.dirname(cli_args.python_libdir)
    if pyloc:
        args += [
            '-DHAVE_PYTHON=YES',
            '-DPython_FIND_REGISTRY=NEVER',
            f'-DPython_ROOT={pyloc}',
        ]


    # Env vars
    thirdparty = os.environ.get('MAKEPANDA_THIRDPARTY', None)
    if thirdparty:
        args.append(f'-DTHIRDPARTY_DIRECTORY={thirdparty}')

    if cli_args.verbose:
        print('cmake config command:')
        pprint.pprint(args)

    try:
        subprocess.check_call(args)
    except subprocess.CalledProcessError as exc:
        sys.exit(exc.returncode)


def build(cmakedir, cli_args):
    args = [
        'cmake',
        '--build', cmakedir,
        '--config', opt_to_config(cli_args.optimize)
    ]
    if cli_args.threads:
        args += [
            '--parallel', str(cli_args.threads)
        ]

    if cli_args.verbose:
        # args.append('--verbose')
        print('cmake build command:')
        pprint.pprint(args)

    try:
        subprocess.check_call(args)
    except subprocess.CalledProcessError as exc:
        sys.exit(exc.returncode)


def run_tests(cmakedir, cli_args):
    args = [
        'cmake',
        '--build', cmakedir,
        '--target', 'test'
    ]

    if cli_args.verbose:
        print('cmake test command:')
        pprint.pprint(args)
    try:
        subprocess.check_call(args)
    except subprocess.CalledProcessError as exc:
        sys.exit(exc.returncode)


def make_wheel(cmakedir, cli_args):
    print('Building wheel...')
    os.chdir(os.path.join(SCRIPTDIR, '..'))
    args = [
        sys.executable,
        os.path.join(
            SCRIPTDIR,
            'makewheel.py'
        ),
        '--outputdir', cmakedir
    ]

    if cli_args.verbose:
        print('make_wheel command:')
        pprint.pprint(args)

    try:
        subprocess.check_call(args)
    except subprocess.CalledProcessError as exc:
        sys.exit(exc.returncode)


def main():
    args = parse_args()

    if args.help:
        print(gen_usage())
        sys.exit(1)

    starttime = time.perf_counter()

    cmakedir = os.path.abspath(args.outputdir)
    os.makedirs(cmakedir, exist_ok=True)
    gen_config(cmakedir, args)
    build(cmakedir, args)

    if args.tests:
        run_tests(cmakedir, args)

    if args.wheel:
        make_wheel(cmakedir, args)

    deltatime = datetime.timedelta(seconds=round(time.perf_counter() - starttime))
    print(f'Build completed in {deltatime}')


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
