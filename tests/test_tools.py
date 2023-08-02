import pytest
import subprocess
import sys
import os

# Currently only works when Panda was installed from wheel
panda3d_tools = pytest.importorskip("panda3d_tools")

def get_tool(name):
    if sys.platform == 'win32':
        name += '.exe'

    tools_dir = os.path.dirname(panda3d_tools.__file__)
    path = os.path.join(tools_dir, name)
    if not os.path.isfile(path):
        pytest.skip(name + ' not found')

    return path


def test_bam_info():
    path = get_tool('bam-info')
    output = subprocess.check_output([path, '-h'], stderr=subprocess.STDOUT).strip()
    assert output.startswith(b"This program scans one or more Bam files")


def test_pzip():
    path = get_tool('pzip')
    output = subprocess.check_output([path, '-h'], stderr=subprocess.STDOUT).strip()
    assert output.startswith(b"This program compresses the named file")
