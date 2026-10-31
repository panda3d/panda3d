from panda3d.core import NodePath, Vec3, Point3
from direct.gui.OnscreenGeom import OnscreenGeom


def test_onscreen_geom_transforms():
    parent = NodePath("parent")
    dummy = NodePath("dummy")

    # 1. Test Vec3 / Point3 (VBase3 subclasses)
    geom = OnscreenGeom(
        geom=dummy,
        parent=parent,
        pos=Point3(1, 2, 3),
        hpr=Vec3(45, 90, 180),
        scale=Vec3(2, 3, 4),
    )
    assert geom.getPos() == Point3(1, 2, 3)
    assert geom.getHpr() == Vec3(45, 90, 180)
    assert geom.getScale() == Vec3(2, 3, 4)

    # 2. Test updating with Vec3 via configure()
    geom.configure(pos=Vec3(4, 5, 6), hpr=Vec3(10, 20, 30), scale=Vec3(7, 8, 9))
    assert geom.getPos() == Vec3(4, 5, 6)
    assert geom.getHpr() == Vec3(10, 20, 30)
    assert geom.getScale() == Vec3(7, 8, 9)
    geom.destroy()

    # 3. Test tuples, lists, and uniform float scale
    geom2 = OnscreenGeom(
        geom=dummy,
        parent=parent,
        pos=(10, 20, 30),
        hpr=[15, 25, 35],
        scale=2.5,
    )
    assert geom2.getPos() == Point3(10, 20, 30)
    assert geom2.getHpr() == Vec3(15, 25, 35)
    assert geom2.getScale() == Vec3(2.5, 2.5, 2.5)
    geom2.destroy()