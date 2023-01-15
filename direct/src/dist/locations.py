import sys

if sys.version_info < (3, 10):
    import distutils.sysconfig as sysconf
    PythonIPath = sysconf.get_python_inc()
    PythonVersion = sysconf.get_config_var("LDVERSION") or sysconf.get_python_version()
    Python = sysconf.PREFIX
    lib_dir = sysconf.get_python_lib(plat_specific=1, standard_lib=1)
    config_vars = sysconf.get_config_vars()
else:
    import sysconfig as sysconf
    PythonIPath = sysconf.get_path("include")
    PythonVersion = sysconf.get_config_var("LDVERSION") or sysconf.get_python_version()
    Python = sysconf.get_config_var('prefix')
    lib_dir = sysconf.get_path("stdlib")
    config_vars = sysconf.get_config_vars()