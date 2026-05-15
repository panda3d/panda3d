from panda3d.core import Filename
import sys, os
import pytest


def test_filename_fspath():
    fn = Filename.from_os_specific(__file__)
    assert os.fspath(fn) == fn.to_os_specific_w()


def test_filename_open():
    fn = Filename.from_os_specific(__file__)
    open(fn, 'rb')


def test_filename_set_extension():
    fn = Filename('test.txt')
    fn.set_extension('abc')
    assert fn.get_fullpath() == 'test.abc'

    fn = Filename('test.a.b')
    fn.set_extension('c')
    assert fn.get_fullpath() == 'test.a.c'

    fn = Filename('test')
    fn.set_extension('ext')
    assert fn.get_fullpath() == 'test.ext'


def test_filename_ctor_pathlib():
    pathlib = pytest.importorskip('pathlib')

    path = pathlib.Path(__file__)
    fn = Filename(path)
    assert fn.to_os_specific_w().lower() == str(path).lower()
