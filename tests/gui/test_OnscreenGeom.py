from panda3d.core import NodePath, Vec3
from direct.gui.OnscreenGeom import OnscreenGeom


def test_onscreen_geom_vec3():
    parent = NodePath("parent")
    dummy = NodePath("dummy")

    geom = OnscreenGeom(
        geom=dummy,
        parent=parent,
        pos=Vec3(1, 2, 3),
        hpr=Vec3(45, 90, 180),
        scale=Vec3(2, 3, 4),
    )

    assert geom.getPos() == Vec3(1, 2, 3)
    assert geom.getHpr() == Vec3(45, 90, 180)
    assert geom.getScale() == Vec3(2, 3, 4)
    geom.destroy()