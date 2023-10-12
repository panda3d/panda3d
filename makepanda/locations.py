import sys
__all__ = [
    'get_python_inc',
    'get_config_var',
    'get_python_version',
    'PREFIX',
    'get_python_lib',
    'get_config_vars',
]

if sys.version_info <= (3, 12):
    from distutils.sysconfig import *
else:
    from sysconfig import *

    def get_python_inc():
        return get_path('include')

    PREFIX = get_config_var('prefix')

    def get_python_lib(*args, **kwargs):
        return get_path('stdlib')

