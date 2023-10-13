from direct.dist.FreezeTool import Freezer, PandaModuleFinder
import sys


def test_Freezer_moduleSuffixes():
    freezer = Freezer()

    for suffix, mode, type in freezer.mf.suffixes:
        if type == 2: # imp.PY_SOURCE
            assert mode == 'rb'


def test_Freezer_getModulePath_getModuleStar(tmp_path):
    # Package 1 can be imported
    package1 = tmp_path / "package1"
    package1.mkdir()
    (package1 / "submodule1.py").write_text("")
    (package1 / "__init__.py").write_text("")

    # Package 2 can not be imported
    package2 = tmp_path / "package2"
    package2.mkdir()
    (package2 / "submodule2.py").write_text("")
    (package2 / "__init__.py").write_text("raise ImportError\n")

    # Module 1 can be imported
    (tmp_path / "module1.py").write_text("")

    # Module 2 can not be imported
    (tmp_path / "module2.py").write_text("raise ImportError\n")

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

        assert freezer.getModuleStar("nonexist") == None
        assert freezer.getModuleStar("package1") == ['submodule1']
        assert freezer.getModuleStar("package2") == ['submodule2']
        assert freezer.getModuleStar("package1.submodule1") == None
        assert freezer.getModuleStar("package1.nonexist") == None
        assert freezer.getModuleStar("package2.submodule2") == None
        assert freezer.getModuleStar("package2.nonexist") == None
        assert freezer.getModuleStar("module1") == None
        assert freezer.getModuleStar("module2") == None
    finally:
        sys.path = backup
