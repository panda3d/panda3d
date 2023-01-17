import sys

if sys.version_info < (3, 10):
    import distutils.sysconfig as sysconf
    pythonIncludePath = sysconf.get_python_inc()
    pythonVersion = sysconf.get_config_var("LDVERSION") or sysconf.get_python_version()
    pythonPrefix = sysconf.PREFIX
    pythonLibDir = sysconf.get_python_lib(plat_specific=1, standard_lib=1)
    pythonConfigVars = sysconf.get_config_vars()
else:
    import sysconfig as sysconf
    pythonIncludePath = sysconf.get_path("include")
    pythonVersion = sysconf.get_config_var("LDVERSION") or sysconf.get_python_version()
    pythonPrefix = sysconf.get_config_var('prefix')
    pythonLibDir = sysconf.get_path("stdlib")
    pythonConfigVars = sysconf.get_config_vars()
