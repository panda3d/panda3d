#!/usr/bin/env python
"""
FMOD Core API Cross-Platform Installation Script for Panda3D
Installs FMOD Core API files from the SDK to the thirdparty directory.

Supports both Windows, MacOS, and Linux platforms with automatic platform detection.
"""

import sys
import shutil
import platform
import argparse
from pathlib import Path


# Platform-specific thirdparty target directories
TARGET_DIRS = {
    'windows': {
        'x64': 'win-libs-vc14-x64',
        'x86': 'win-libs-vc14',
        'arm64': 'win-libs-vc14-arm64',
    },
    'darwin': {
        'arm64': 'darwin-libs-a',
        'x86_64': 'darwin-libs-a',
    },
    'linux': {
        'x64': 'linux-libs-x64',
        'arm64': 'linux-libs-arm64',
        'arm': 'linux-libs-arm',
    },
}

# Windows-specific library architecture paths (macOS uses universal binaries)
WINDOWS_LIB_ARCH_PATHS = {
    'x64': 'x64',
    'x86': 'x86',
    'arm64': 'arm64',
}

# Linux SDK library architecture subdirectory names (under api/core/lib/)
LINUX_LIB_ARCH_PATHS = {
    'x64': 'x86_64',
    'arm64': 'arm64',
    'arm': 'arm',
    'x86': 'x86',
}


def detect_platform():
    """Detect the current platform."""
    if sys.platform == 'win32' or sys.platform == 'cygwin':
        return 'windows'
    elif sys.platform == 'darwin':
        return 'darwin'
    elif sys.platform.startswith('linux'):
        return 'linux'
    else:
        return 'unknown'


def detect_architecture():
    """Detect system architecture."""
    machine = platform.machine().lower()
    # Map to FMOD SDK architecture names
    if machine in ('amd64', 'x86_64'):
        return 'x64'
    elif machine in ('arm64', 'aarch64'):
        return 'arm64'
    elif machine in ('armv7l', 'armv6l', 'arm'):
        return 'arm'
    elif machine in ('x86', 'i386', 'i686'):
        return 'x86'
    return machine


def get_fmod_paths(sdk_path, platform_type, arch):
    """
    Construct FMOD SDK source paths.

    Args:
        sdk_path: Path to FMOD SDK root (contains api/core/)
        platform_type: 'windows' or 'darwin'
        arch: 'x64', 'arm64', etc. (ignored for macOS universal binaries)

    Returns:
        tuple: (fmod_source_path, target_base_path)
    """
    # Strip trailing slashes/backslashes (PowerShell tab completion adds them)
    sdk_path = sdk_path.rstrip('/\\').rstrip('"').rstrip("'")
    sdk_root = Path(sdk_path).resolve()

    # Construct source path - sdk_path should contain api/core/
    fmod_source = sdk_root / "api" / "core"

    # Construct target path
    target_dir_name = TARGET_DIRS.get(platform_type, {}).get(arch)
    if not target_dir_name:
        raise ValueError(f"Unsupported architecture '{arch}' for platform '{platform_type}'")

    repo_root = Path(__file__).parent.absolute()
    target_base = repo_root / "thirdparty" / target_dir_name / "fmod"

    return fmod_source, target_base


def validate_source(fmod_source, platform_type, arch):
    """Validate that source directories and files exist for the platform."""
    errors = []

    # Check main source directory
    if not fmod_source.exists():
        errors.append(f"FMOD source directory not found: {fmod_source}")
        errors.append(f"  Expected: <sdk_path>/api/core/")
        return errors

    # Check inc directory
    inc_dir = fmod_source / "inc"
    if not inc_dir.exists():
        errors.append(f"Headers directory not found: {inc_dir}")
    elif not any(inc_dir.glob('*.h')):
        errors.append(f"No header files found in: {inc_dir}")

    # Check lib directory (platform-specific)
    if platform_type == 'windows':
        lib_arch = WINDOWS_LIB_ARCH_PATHS[arch]
        lib_arch_dir = fmod_source / "lib" / lib_arch

        if not lib_arch_dir.exists():
            errors.append(f"Libraries directory not found: {lib_arch_dir}")
        else:
            if not any(lib_arch_dir.glob('*.lib')):
                errors.append(f"No .lib files found in: {lib_arch_dir}")
            if not any(lib_arch_dir.glob('*.dll')):
                errors.append(f"No .dll files found in: {lib_arch_dir}")

    elif platform_type == 'darwin':
        # macOS uses universal binaries in lib/ directly (no arch subdirectories)
        lib_dir = fmod_source / "lib"

        if not lib_dir.exists():
            errors.append(f"Libraries directory not found: {lib_dir}")
        else:
            dylibs = list(lib_dir.glob('*.dylib'))
            if not dylibs:
                errors.append(f"No .dylib files found in: {lib_dir}")

    elif platform_type == 'linux':
        lib_arch = LINUX_LIB_ARCH_PATHS.get(arch)
        if not lib_arch:
            errors.append(f"Unsupported Linux architecture: {arch}")
        else:
            lib_arch_dir = fmod_source / "lib" / lib_arch
            if not lib_arch_dir.exists():
                errors.append(f"Libraries directory not found: {lib_arch_dir}")
            else:
                solibs = list(lib_arch_dir.glob('libfmod.so*'))
                if not solibs:
                    errors.append(f"No libfmod.so files found in: {lib_arch_dir}")

    return errors


def create_target_directories(target_base, platform_type):
    """Create target directory structure based on platform."""
    if platform_type == 'windows':
        directories = [
            target_base,
            target_base / "include",
            target_base / "lib",
            target_base / "bin"
        ]
    elif platform_type == 'darwin':
        directories = [
            target_base,
            target_base / "include",
            target_base / "lib"
        ]
    elif platform_type == 'linux':
        directories = [
            target_base,
            target_base / "include",
            target_base / "lib"
        ]
    else:
        raise ValueError(f"Unsupported platform: {platform_type}")

    for directory in directories:
        directory.mkdir(parents=True, exist_ok=True)


def copy_files_windows(fmod_source, target_base, arch):
    """Copy files for Windows platform."""
    stats = {'headers': 0, 'libraries': 0, 'binaries': 0, 'errors': []}

    # Copy header files
    inc_source = fmod_source / "inc"
    inc_target = target_base / "include"

    for pattern in ['*.h', '*.hpp', '*.cs']:
        for src_file in inc_source.glob(pattern):
            try:
                dst_file = inc_target / src_file.name
                shutil.copy2(src_file, dst_file)
                stats['headers'] += 1
            except Exception as e:
                stats['errors'].append(f"Error copying {src_file.name}: {e}")

    # Copy library files (.lib)
    lib_arch = WINDOWS_LIB_ARCH_PATHS[arch]
    lib_source = fmod_source / "lib" / lib_arch
    lib_target = target_base / "lib"

    for src_file in lib_source.glob('*.lib'):
        try:
            dst_file = lib_target / src_file.name
            shutil.copy2(src_file, dst_file)
            stats['libraries'] += 1
        except Exception as e:
            stats['errors'].append(f"Error copying {src_file.name}: {e}")

    # Copy DLL files
    bin_target = target_base / "bin"

    for src_file in lib_source.glob('*.dll'):
        try:
            dst_file = bin_target / src_file.name
            shutil.copy2(src_file, dst_file)
            stats['binaries'] += 1
        except Exception as e:
            stats['errors'].append(f"Error copying {src_file.name}: {e}")

    return stats


def copy_files_darwin(fmod_source, target_base):
    """Copy files for macOS platform."""
    stats = {'headers': 0, 'libraries': 0, 'binaries': 0, 'errors': []}

    # Copy header files
    inc_source = fmod_source / "inc"
    inc_target = target_base / "include"

    for pattern in ['*.h', '*.hpp', '*.cs']:
        for src_file in inc_source.glob(pattern):
            try:
                dst_file = inc_target / src_file.name
                shutil.copy2(src_file, dst_file)
                stats['headers'] += 1
            except Exception as e:
                stats['errors'].append(f"Error copying {src_file.name}: {e}")

    # Copy universal dylib files (from lib/ directly, no arch subdirectory)
    lib_source = fmod_source / "lib"
    lib_target = target_base / "lib"

    for src_file in lib_source.glob('*.dylib'):
        try:
            dst_file = lib_target / src_file.name
            shutil.copy2(src_file, dst_file)
            stats['libraries'] += 1
        except Exception as e:
            stats['errors'].append(f"Error copying {src_file.name}: {e}")

    return stats


def copy_files_linux(fmod_source, target_base, arch):
    """Copy files for Linux platform."""
    stats = {'headers': 0, 'libraries': 0, 'binaries': 0, 'errors': []}

    # Copy header files flat into include/ (Panda3D uses #include <fmod.hpp>)
    inc_source = fmod_source / "inc"
    inc_target = target_base / "include"

    for pattern in ['*.h', '*.hpp']:
        for src_file in inc_source.glob(pattern):
            try:
                shutil.copy2(src_file, inc_target / src_file.name)
                stats['headers'] += 1
            except Exception as e:
                stats['errors'].append(f"Error copying {src_file.name}: {e}")

    # Copy .so files from the arch-specific lib subdirectory
    lib_arch = LINUX_LIB_ARCH_PATHS[arch]
    lib_source = fmod_source / "lib" / lib_arch
    lib_target = target_base / "lib"

    for src_file in lib_source.glob('libfmod.so*'):
        try:
            dst_file = lib_target / src_file.name
            if src_file.is_symlink():
                # Recreate the symlink rather than copying the target
                link_target = src_file.readlink()
                if dst_file.exists() or dst_file.is_symlink():
                    dst_file.unlink()
                dst_file.symlink_to(link_target)
            else:
                shutil.copy2(src_file, dst_file)
            stats['libraries'] += 1
        except Exception as e:
            stats['errors'].append(f"Error copying {src_file.name}: {e}")

    return stats


def copy_files(platform_type, fmod_source, target_base, arch):
    """Dispatch to platform-specific copy function."""
    if platform_type == 'windows':
        return copy_files_windows(fmod_source, target_base, arch)
    elif platform_type == 'darwin':
        return copy_files_darwin(fmod_source, target_base)
    elif platform_type == 'linux':
        return copy_files_linux(fmod_source, target_base, arch)
    else:
        raise ValueError(f"Unsupported platform: {platform_type}")


def print_summary(stats, platform_type, arch, target_base):
    """Print installation summary."""
    total_files = stats['headers'] + stats['libraries'] + stats['binaries']

    print("\n" + "=" * 60)
    if stats['errors']:
        print("Installation completed with errors!")
    else:
        print("Installation completed successfully!")
    print(f"Platform: {platform_type}")
    print(f"Architecture: {arch}")
    print(f"Files copied: {total_files}")
    print(f"Target: {target_base}")
    print("=" * 60)

    return len(stats['errors']) == 0


def main():
    """Main entry point with argument parsing."""
    parser = argparse.ArgumentParser(
        description="Install FMOD Core API to Panda3D thirdparty directory",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Point to FMOD SDK root directory (contains api/core/)
  python install_fmod.py --sdk-path "FMOD Programmers API"

  # Windows with custom SDK location
  python install_fmod.py --sdk-path "C:/SDKs/FMOD Studio API Windows"

  # macOS from project root
  python install_fmod.py --sdk-path "FMOD Programmers API"

  # Linux (auto-detected)
  python install_fmod.py --sdk-path fmodstudioapi20312linux
        """
    )

    parser.add_argument(
        '--sdk-path',
        type=str,
        required=True,
        help='Path to FMOD SDK root directory (must contain api/core/)'
    )

    parser.add_argument(
        '--platform',
        type=str,
        choices=['windows', 'darwin', 'linux', 'auto'],
        default='auto',
        help='Target platform (default: auto-detect)'
    )

    parser.add_argument(
        '--arch',
        type=str,
        choices=['x64', 'x86', 'arm64', 'auto'],
        default='auto',
        help='Target architecture (default: auto-detect)'
    )

    args = parser.parse_args()

    print("FMOD Core API Cross-Platform Installation Script")
    print("-" * 60)

    # Detect platform and architecture
    platform_type = detect_platform() if args.platform == 'auto' else args.platform
    arch = detect_architecture() if args.arch == 'auto' else args.arch

    print(f"Detected platform: {platform_type}")
    print(f"Detected architecture: {arch}")
    print()

    # Validate platform
    if platform_type == 'unknown':
        print("ERROR: Unsupported platform", file=sys.stderr)
        return 1

    # Get paths
    try:
        fmod_source, target_base = get_fmod_paths(args.sdk_path, platform_type, arch)
    except ValueError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1

    # Validate source
    print("Validating source files...")
    errors = validate_source(fmod_source, platform_type, arch)
    if errors:
        print("ERROR: Source validation failed:", file=sys.stderr)
        for error in errors:
            print(f"  {error}", file=sys.stderr)
        return 1
    print("[OK] Source validation passed")

    # Create target directories
    print("\nCreating target directories...")
    try:
        create_target_directories(target_base, platform_type)
        print("[OK] Directories created")
    except Exception as e:
        print(f"ERROR: Failed to create directories: {e}", file=sys.stderr)
        return 1

    # Copy files
    print("\nCopying files...")
    stats = copy_files(platform_type, fmod_source, target_base, arch)

    # Print results
    if stats['headers'] > 0:
        print(f"[OK] Copied {stats['headers']} header files")
    if stats['libraries'] > 0:
        print(f"[OK] Copied {stats['libraries']} library files")
    if stats['binaries'] > 0:
        print(f"[OK] Copied {stats['binaries']} DLL files")

    # Print any errors
    if stats['errors']:
        print("\nErrors encountered:", file=sys.stderr)
        for error in stats['errors']:
            print(f"  {error}", file=sys.stderr)

    # Print summary
    success = print_summary(stats, platform_type, arch, target_base)

    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
