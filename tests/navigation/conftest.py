import pytest


@pytest.fixture
def navigation():
    navmeshgen = pytest.importorskip("panda3d.navmeshgen")
    navigation = pytest.importorskip("panda3d.navigation")
    return navmeshgen, navigation
