__all__ = [
    'get_python_inc',
    'get_config_var',
    'get_python_version',
    'PREFIX',
    'get_python_lib',
    'get_config_vars',
]

import sys

if sys.version_info < (3, 12):
    from distutils.sysconfig import *
else:
    from sysconfig import *

    PREFIX = get_config_var('prefix')

    def get_python_inc(plat_specific=False):
        path_name = 'platinclude' if plat_specific else 'include'
        return get_path(path_name)

    def get_python_lib(plat_specific=False, standard_lib=False):
        if standard_lib:
            path_name = 'stdlib'
            if plat_specific:
                path_name = 'plat' + path_name
        elif plat_specific:
            path_name = 'platlib'
        else:
            path_name = 'purelib'
        return get_path(path_name)
