import pytest


@pytest.fixture
def world():
    ode = pytest.importorskip("panda3d.ode")
    return ode.OdeWorld()
