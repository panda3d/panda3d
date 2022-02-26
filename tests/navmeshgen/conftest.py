import pytest


@pytest.fixture
def navmeshgen():
    navmeshgen = pytest.importorskip("panda3d.navmeshgen")
    return navmeshgen
