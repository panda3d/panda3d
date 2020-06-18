def _fixup_dlls():
    try:
        path = __path__[0]
    except (NameError, IndexError):
        return # Not a package, or not on filesystem

    import os

    relpath = os.path.relpath(path, __path__[-1])
    dll_path = os.path.abspath(os.path.join(__path__[-1], '../bin', relpath))
    if not os.path.isdir(dll_path):
        return

    if hasattr(os, 'add_dll_directory'):
        os.add_dll_directory(dll_path)
    else:
        os_path = os.environ.get('PATH', '')
        os_path = os_path.split(os.pathsep) if os_path else []
        os_path.insert(0, dll_path)
        os.environ['PATH'] = os.pathsep.join(os_path)

_fixup_dlls()
del _fixup_dlls
