import pytest
import subprocess

# Currently only works when Panda was installed from wheel
panda3d_tools = pytest.importorskip("panda3d_tools")


@pytest.mark.skipif(not hasattr(panda3d_tools, 'bam_info'),
                    reason="requires bam-info")
def test_bam_info():
    output = subprocess.check_output(['bam-info', '-h'], stderr=subprocess.STDOUT).strip()
    assert output.startswith(b"This program scans one or more Bam files")


@pytest.mark.skipif(not hasattr(panda3d_tools, 'pzip'),
                    reason="requires pzip")
def test_pzip():
    output = subprocess.check_output(['pzip', '-h'], stderr=subprocess.STDOUT).strip()
    assert output.startswith(b"This program compresses the named file")
