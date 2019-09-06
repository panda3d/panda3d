from panda3d.core import Filename
import sys, os
import pytest


@pytest.mark.skipif(sys.version_info < (3, 6), reason="Requires Python 3.6")
def test_filename_fspath():
    fn = Filename.from_os_specific(__file__)
    assert os.fspath(fn) == fn.to_os_specific_w()


@pytest.mark.skipif(sys.version_info < (3, 6), reason="Requires Python 3.6")
def test_filename_open():
    fn = Filename.from_os_specific(__file__)
    open(fn, 'rb')


@pytest.mark.skipif(sys.version_info < (3, 4), reason="Requires Python 3.4")
def test_filename_ctor_pathlib():
    pathlib = pytest.importorskip('pathlib')

    path = pathlib.Path(__file__)
    fn = Filename(path)
    assert fn.to_os_specific_w() == str(path)
