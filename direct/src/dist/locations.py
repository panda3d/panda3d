import sys

if sys.version_info >= (3, 12):
    from distutils.sysconfig import *
    __all__ = ['get_python_inc', 'get_config_var', "get_python_version", "PREFIX", "get_python_lib", "get_config_vars"]
else:
    from sysconfig import *

    __all__ = ['get_python_inc', 'get_config_var', "get_python_version", "PREFIX", "get_python_lib", "get_config_vars"]

    def get_python_inc():
        return get_path("include")

    PREFIX = get_config_var('prefix')

    def get_python_lib(*args, **kwargs):
        return get_path("stdlib")

