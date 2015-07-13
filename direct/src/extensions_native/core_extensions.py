import sys

main_dir = Filename()

if sys.argv and sys.argv[0]:
    main_dir = Filename.from_os_specific(sys.argv[0])

if main_dir.empty():
    # We must be running in the Python interpreter directly, so return the CWD.
    main_dir = ExecutionEnvironment.get_cwd()
else:
    main_dir.make_absolute()
    main_dir = Filename(main_dir.get_dirname())
ExecutionEnvironment.shadow_environment_variable('MAIN_DIR', main_dir.to_os_specific())
del sys, main_dir


def Dtool_funcToMethod(func, cls, method_name=None):
    """Adds func to class so it is an accessible method; use method_name to specify the name to be used for calling the method.
    The new method is accessible to any instance immediately."""
    #if sys.version_info < (3, 0):
    #    func.im_class = cls
    func.im_func = func
    func.im_self = None
    if not method_name:
        method_name = func.__name__
    cls.DtoolClassDict[method_name] = func;

