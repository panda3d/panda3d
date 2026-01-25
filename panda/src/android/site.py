import sys
import os

from importlib.abc import Loader, MetaPathFinder
from importlib.machinery import ModuleSpec

from importlib import _bootstrap_external

# This should not be needed once we bump the minimum python version requirement to 3.13
sys.platform = "android"

class AndroidExtensionFinder(MetaPathFinder):
    @classmethod
    def find_spec(cls, fullname, path=None, target=None):
        soname = 'libpy.' + fullname + '.so'
        path = os.path.join(sys._native_library_dir, soname)

        if os.path.exists(path):
            loader = _bootstrap_external.ExtensionFileLoader(fullname, path)
            return ModuleSpec(fullname, loader, origin=path)


def main():
    """Adds the site-packages directory to the sys.path.
    Also, registers the import hook for extension modules."""

    sys.path.append('{0}/lib/python{1}.{2}/site-packages'.format(sys.prefix, *sys.version_info))
    sys.meta_path.append(AndroidExtensionFinder)


if not sys.flags.no_site:
    main()
