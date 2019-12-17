from __future__ import print_function

import platform
import pip
import sys
import os

patch_environment = False

target_platform = None
sys_platform = None
platform_machine = None
platform_system = None
os_name = None

for (index, arg) in enumerate(sys.argv):
    if arg == '--platform':
        target_platform = sys.argv[index + 1]

if target_platform is not None:

    # Only patch markers environment if we have a target platform
    patch_environment = True

    if target_platform.startswith("win"):
        sys_platform = 'win32'
        platform_system = 'Windows'
        os_name = 'nt'
    elif 'linux' in target_platform:
        sys_platform = 'linux'
        platform_system = 'Linux'
        os_name = 'posix'
    elif target_platform.startswith('macosx'):
        sys_platform = 'darwin'
        platform_system = 'Darwin'
        os_name = 'posix'
    elif target_platform.startswith('freebsd'):
        sys_platform = 'freebsd'
        platform_system = 'FreeBSD'
        os_name = 'posix'
    elif target_platform.startswith('openbsd'):
        sys_platform = 'openbsd'
        platform_system = 'OpenBSD'
        os_name = 'posix'
    else:
        print("ERROR: pip_wrapper: Unsupported target platform '{}'".format(target_platform))
        patch_environment = False

    for machine in ("x86_64", "amd64", "i386", "i586", "i686",
                    "ppc", "ppc64", "ppc64le",
                    'arm64', 'aarch64', 'arm7l',
                    'mipsel', 'mips',
                    'mips64'):
        if target_platform.endswith(machine):
            platform_machine = machine
            break
    else:
        print("ERROR: pip_wrapper: Unsupported target platform machine '{}'".format(target_platform))
        patch_environment = False

def format_full_version(info):
    version = '{0.major}.{0.minor}.{0.micro}'.format(info)
    kind = info.releaselevel
    if kind != 'final':
        version += kind[0] + str(info.serial)
    return version

def patched_default_environment():
    if hasattr(sys, "implementation"):
        iver = format_full_version(sys.implementation.version)
        implementation_name = sys.implementation.name
    else:
        iver = "0"
        implementation_name = ""

    return {
        "implementation_name": implementation_name,
        "implementation_version": iver,
        "os_name": os_name,
        "platform_machine": platform_machine,
        # Can't extract platform.release() from target_platform
        "platform_release": '',
        "platform_system": platform_system,
         # Can't extract platform.version() from target_platform, so we're
         # using platform system as fallback
        "platform_version": platform_system,
        "python_full_version": platform.python_version(),
        "platform_python_implementation": platform.python_implementation(),
        "python_version": ".".join(platform.python_version_tuple()[:2]),
        "sys_platform": sys_platform,
    }

# pip main has been moved to _internal in version 10
if hasattr(pip, 'main'):
    from pip import main
else:
    from pip._internal.main import main

if patch_environment:
    try:
        from pip._vendor.packaging import markers

        if hasattr(markers, 'default_environment'):
            markers.default_environment = patched_default_environment
        else:
            print("ERROR: pip_wrapper: Could not patch markers.default_environment, environment markers will have host values")
    except ImportError:
        print("ERROR: pip_wrapper: Your version of pip is too old")

# If no parameters are given to pip main() it will use sys.argv
main()
