from direct.showbase.ShowBase import ShowBase
import sys

if sys.version_info >= (3, 0):
    import builtins
else:
    import __builtin__ as builtins


def test_showbase_create_destroy():
    sb = ShowBase(windowType='none')
    try:
        assert builtins.base == sb
    finally:
        sb.destroy()
        sb = None

    assert not hasattr(builtins, 'base')
    assert not hasattr(builtins, 'run')
    assert not hasattr(builtins, 'loader')
    assert not hasattr(builtins, 'taskMgr')
