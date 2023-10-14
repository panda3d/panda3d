from direct.dist.FreezeTool import Freezer, PandaModuleFinder
import pytest
import os
import sys
import subprocess
import platform


def test_Freezer_moduleSuffixes():
    freezer = Freezer()

    for suffix, mode, type in freezer.moduleSuffixes:
        if type == 2: # imp.PY_SOURCE
            assert mode == 'rb'


def test_Freezer_getModulePath_getModuleStar(tmp_path):
    # Package 1 can be imported
    package1 = tmp_path / "package1"
    package1.mkdir()
    (package1 / "submodule1.py").write_text(u"")
    (package1 / "__init__.py").write_text(u"")

    # Package 2 can not be imported
    package2 = tmp_path / "package2"
    package2.mkdir()
    (package2 / "submodule2.py").write_text(u"")
    (package2 / "__init__.py").write_text(u"raise ImportError\n")

    # Module 1 can be imported
    (tmp_path / "module1.py").write_text(u"")

    # Module 2 can not be imported
    (tmp_path / "module2.py").write_text(u"raise ImportError\n")

    # Module 3 has a custom __path__ and __all__
    (tmp_path / "module3.py").write_text(u"__path__ = ['foobar']\n__all__ = ['test']\n")

    backup = sys.path
    try:
        # Don't fail if first item on path does not exist
        sys.path = [str(tmp_path / "nonexistent"), str(tmp_path)]

        freezer = Freezer()
        assert freezer.getModulePath("nonexist") == None
        assert freezer.getModulePath("package1") == [str(package1)]
        assert freezer.getModulePath("package2") == [str(package2)]
        assert freezer.getModulePath("package1.submodule1") == None
        assert freezer.getModulePath("package1.nonexist") == None
        assert freezer.getModulePath("package2.submodule2") == None
        assert freezer.getModulePath("package2.nonexist") == None
        assert freezer.getModulePath("module1") == None
        assert freezer.getModulePath("module2") == None
        assert freezer.getModulePath("module3") == ['foobar']

        assert freezer.getModuleStar("nonexist") == None
        assert freezer.getModuleStar("package1") == ['submodule1']
        assert freezer.getModuleStar("package2") == ['submodule2']
        assert freezer.getModuleStar("package1.submodule1") == None
        assert freezer.getModuleStar("package1.nonexist") == None
        assert freezer.getModuleStar("package2.submodule2") == None
        assert freezer.getModuleStar("package2.nonexist") == None
        assert freezer.getModuleStar("module1") == None
        assert freezer.getModuleStar("module2") == None
        assert freezer.getModuleStar("module3") == ['test']
    finally:
        sys.path = backup


@pytest.mark.parametrize("use_console", (False, True))
def test_Freezer_generateRuntimeFromStub(tmp_path, use_console):
    try:
        # If installed as a wheel
        import panda3d_tools
        bin_dir = os.path.dirname(panda3d_tools.__file__)
    except:
        import panda3d
        bin_dir = os.path.join(os.path.dirname(os.path.dirname(panda3d.__file__)), 'bin')

    if sys.platform == 'win32':
        suffix = '.exe'
    else:
        suffix = ''

    if not use_console:
        stub_file = os.path.join(bin_dir, 'deploy-stubw' + suffix)

    if use_console or not os.path.isfile(stub_file):
        stub_file = os.path.join(bin_dir, 'deploy-stub' + suffix)

    if not os.path.isfile(stub_file):
        pytest.skip("Unable to find deploy-stub executable")

    target = str(tmp_path / ('stubtest' + suffix))

    freezer = Freezer()
    freezer.addModule('__main__', 'main.py', text='print("Hello world")')
    freezer.done(addStartupModules=True)
    freezer.generateRuntimeFromStub(target, open(stub_file, 'rb'), use_console)
