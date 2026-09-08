import pytest
from pathlib import Path


@pytest.fixture(scope="module")
def dc_file():
    direct = pytest.importorskip("panda3d.direct")
    dcf = direct.DCFile()
    path = Path(__file__).parent / "schema.dc"
    success = dcf.read(path)
    assert success, "failed to parse DC fixture"
    return dcf
