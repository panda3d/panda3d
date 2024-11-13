import pytest
import subprocess
import sys
import os
import tempfile
import panda3d

if sys.platform == "emscripten":
    pytest.skip(allow_module_level=True)

try:
    panda3d_tools = pytest.importorskip("panda3d_tools")
except:
    panda3d_tools = None


def get_tool(name):
    if sys.platform == 'win32':
        name += '.exe'

    if panda3d_tools:
        tools_dir = os.path.dirname(panda3d_tools.__file__)
    else:
        tools_dir = os.path.join(os.path.dirname(os.path.dirname(panda3d.__file__)), 'bin')

    path = os.path.join(tools_dir, name)
    if not os.path.isfile(path):
        pytest.skip(name + ' not found')

    return path


def test_bam_info():
    path = get_tool('bam-info')
    output = subprocess.check_output([path, '-h'], stderr=subprocess.STDOUT).strip()
    assert output.startswith(b"This program scans one or more Bam files")


def test_egg_trans():
    path = get_tool('egg-trans')
    output = subprocess.check_output([path, '-h'], stderr=subprocess.STDOUT).strip()
    assert output.startswith(b"egg-trans reads an egg file and writes")


def test_pzip():
    data = b'test \000 data'

    try:
        file = tempfile.NamedTemporaryFile(suffix='.bin', delete=False)
        file.write(data)
        file.close()

        path = get_tool('pzip')
        subprocess.check_output([path, file.name])

        zlib = pytest.importorskip('zlib')

        with open(file.name + '.pz', 'rb') as pz:
            assert zlib.decompress(pz.read(), 32 + 15, 4096) == data

    finally:
        if os.path.isfile(file.name):
            os.remove(file.name)

        if os.path.isfile(file.name + '.pz'):
            os.remove(file.name + '.pz')
