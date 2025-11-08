"""The VFS importer allows importing Python modules from Panda3D's virtual
file system, through Python's standard import mechanism.

Calling the :func:`register()` function to register the import hooks should be
sufficient to enable this functionality.
"""

from __future__ import annotations

__all__ = ['register']

from panda3d.core import Filename, VirtualFile, VirtualFileSystem, VirtualFileMountSystem
from panda3d.core import OFileStream, copy_stream
import sys
import marshal
import _imp
import atexit
from importlib.abc import Loader, SourceLoader
from importlib.util import MAGIC_NUMBER, decode_source
from importlib.machinery import ModuleSpec, EXTENSION_SUFFIXES, BYTECODE_SUFFIXES
from types import ModuleType
from typing import Any

vfs = VirtualFileSystem.get_global_ptr()


def _make_spec(fullname: str, loader: VFSLoader, *, is_package: bool) -> ModuleSpec:
    filename = loader._vfile.get_filename()
    spec = ModuleSpec(fullname, loader, origin=filename.to_os_specific(), is_package=is_package)
    if is_package:
        assert spec.submodule_search_locations is not None
        spec.submodule_search_locations.append(Filename(filename.get_dirname()).to_os_specific())
    spec.has_location = True
    return spec


class VFSFinder:
    """ This class serves as a Python importer to support loading
    Python .py and .pyc/.pyo files from Panda's Virtual File System,
    which allows loading Python source files from mounted .mf files
    (among other places). """

    def __init__(self, path: str) -> None:
        self.path = path

    def find_spec(self, fullname: str, target: ModuleType | None = None) -> ModuleSpec | None:
        #print(f"find_spec({fullname}), dir_path = {dir_path}", file=sys.stderr)
        basename = fullname.split('.')[-1]
        filename = Filename(Filename.from_os_specific(self.path), basename)

        loader: VFSLoader

        # First, look for Python files.
        vfile = vfs.get_file(filename + '.py', True)
        if vfile:
            loader = VFSSourceLoader(fullname, vfile)
            return _make_spec(fullname, loader, is_package=False)

        # If there's no .py file, but there's a .pyc file, load that
        # anyway.
        for suffix in BYTECODE_SUFFIXES:
            vfile = vfs.get_file(filename + suffix, True)
            if vfile:
                loader = VFSCompiledLoader(fullname, vfile)
                return _make_spec(fullname, loader, is_package=False)

        # Look for a C/C++ extension module.
        for suffix in EXTENSION_SUFFIXES:
            vfile = vfs.get_file(filename + suffix, True)
            if vfile:
                loader = VFSExtensionLoader(fullname, vfile)
                return _make_spec(fullname, loader, is_package=False)

        # Consider a package, i.e. a directory containing __init__.py.
        init_filename = Filename(filename, '__init__.py')
        vfile = vfs.get_file(init_filename, True)
        if vfile:
            loader = VFSSourceLoader(fullname, vfile)
            return _make_spec(fullname, loader, is_package=True)

        for suffix in BYTECODE_SUFFIXES:
            init_filename = Filename(filename, '__init__' + suffix)
            vfile = vfs.get_file(init_filename, True)
            if vfile:
                loader = VFSCompiledLoader(fullname, vfile)
                return _make_spec(fullname, loader, is_package=True)

        # Consider a namespace package.
        if vfs.is_directory(filename):
            spec = ModuleSpec(fullname, VFSNamespaceLoader(), is_package=True)
            assert spec.submodule_search_locations is not None
            spec.submodule_search_locations.append(filename.to_os_specific())
            return spec

        #print("not found.", file=sys.stderr)
        return None


class VFSLoader(Loader):
    def __init__(self, fullname: str, vfile: VirtualFile) -> None:
        self.name = fullname
        self._vfile = vfile

    def is_package(self, fullname):
        if fullname is not None and self.name != fullname:
            raise ImportError

        filename = self._vfile.get_filename().get_basename()
        filename_base = filename.rsplit('.', 1)[0]
        tail_name = fullname.rpartition('.')[2]
        return filename_base == '__init__' and tail_name != '__init__'

    def create_module(self, spec: ModuleSpec) -> ModuleType | None:
        """Use default semantics for module creation."""

    def exec_module(self, module: ModuleType) -> None:
        """Execute the module."""
        code = self.get_code(module.__name__)  # type: ignore[attr-defined]
        exec(code, module.__dict__)

    def get_filename(self, fullname: str) -> str:
        if fullname is not None and self.name != fullname:
            raise ImportError

        return self._vfile.get_filename().to_os_specific()

    @staticmethod
    def get_data(path: str) -> bytes:
        vfile = vfs.get_file(Filename.from_os_specific(path))
        if vfile:
            return vfile.read_file(True)
        else:
            raise OSError

    @staticmethod
    def path_stats(path: str) -> dict[str, Any]:
        vfile = vfs.get_file(Filename.from_os_specific(path))
        if vfile:
            return {'mtime': vfile.get_timestamp(), 'size': vfile.get_file_size()}
        else:
            raise OSError

    @staticmethod
    def path_mtime(path):
        vfile = vfs.get_file(Filename.from_os_specific(path))
        if vfile:
            return vfile.get_timestamp()
        else:
            raise OSError


class VFSSourceLoader(VFSLoader, SourceLoader): # type: ignore[misc]
    def get_source(self, fullname):
        if fullname is not None and self.name != fullname:
            raise ImportError

        return decode_source(self._vfile.read_file(True))


class VFSCompiledLoader(VFSLoader):
    def get_code(self, fullname):
        if fullname is not None and self.name != fullname:
            raise ImportError

        vfile = self._vfile
        data = vfile.read_file(True)
        if data[:4] != MAGIC_NUMBER:
            raise ImportError("Bad magic number in %s" % (vfile))

        return marshal.loads(data[16:])

    def get_source(self, fullname):
        return None


class VFSExtensionLoader(VFSLoader):
    def create_module(self, spec):
        vfile = self._vfile
        filename = vfile.get_filename()

        # We can only import an extension module if it already exists on
        # disk.  This means if it's a truly virtual file that has no
        # on-disk equivalent, we have to write it to a temporary file
        # first.
        if isinstance(vfile.get_mount(), VirtualFileMountSystem):
            # It's a real file.
            pass
        elif filename.exists():
            # It's a virtual file, but it's shadowing a real file in
            # the same directory.  Assume they're the same, and load
            # the real one.
            pass
        else:
            # It's a virtual file with no real-world existence.  Dump
            # it to disk.
            ext = filename.get_extension()
            tmp_filename = Filename.temporary('', filename.get_basename_wo_extension(),
                                              '.' + ext,
                                              type = Filename.T_dso)
            tmp_filename.set_extension(ext)
            tmp_filename.set_binary()
            sin = vfile.open_read_file(True)
            try:
                sout = OFileStream()
                if not tmp_filename.open_write(sout):
                    raise IOError
                if not copy_stream(sin, sout):
                    raise IOError
            finally:
                vfile.close_read_file(sin)
            del sout

            # Delete when the process ends.
            atexit.register(tmp_filename.unlink)

            # Make a dummy spec to pass to create_dynamic with the path to
            # our temporary file.
            spec = ModuleSpec(spec.name, spec.loader,
                              origin=tmp_filename.to_os_specific(),
                              is_package=False)

        module = _imp.create_dynamic(spec)
        module.__file__ = filename.to_os_specific()
        return module

    def exec_module(self, module):
        _imp.exec_dynamic(module)

    def is_package(self, fullname):
        return False

    def get_code(self, fullname):
        return None

    def get_source(self, fullname):
        return None


class VFSNamespaceLoader(Loader):
    def create_module(self, spec: ModuleSpec) -> ModuleType | None:
        """Use default semantics for module creation."""

    def exec_module(self, module: ModuleType) -> None:
        pass

    def is_package(self, fullname):
        return True

    def get_source(self, fullname):
        return ''

    def get_code(self, fullname):
        return compile('', '<string>', 'exec', dont_inherit=True)


def _path_hook(entry: str) -> VFSFinder:
    # If this is a directory in the VFS, create a VFSFinder for this entry.
    vfile = vfs.get_file(Filename.from_os_specific(entry), False)
    if vfile and vfile.is_directory() and not isinstance(vfile.get_mount(), VirtualFileMountSystem):
        return VFSFinder(entry)
    else:
        raise ImportError


_registered = False

def register() -> None:
    """ Register the VFSFinder on the path_hooks, if it has not
    already been registered, so that future Python import statements
    will vector through here (and therefore will take advantage of
    Panda's virtual file system). """

    global _registered
    if not _registered:
        _registered = True
        sys.path_hooks.insert(0, _path_hook)

        # Blow away the importer cache, so we'll come back through the
        # VFSFinder for every folder in the future, even those
        # folders that previously were loaded directly.
        sys.path_importer_cache = {}
