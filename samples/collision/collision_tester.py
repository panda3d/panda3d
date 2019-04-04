#!/usr/bin/env python

import sys

from panda3d.core import KeyboardButton
from panda3d.core import MouseButton
from panda3d.core import Point2
from panda3d.core import Point3
from panda3d.core import CollisionNode
from panda3d.core import CollisionTraverser
from panda3d.core import CollisionHandlerQueue
from panda3d.core import Plane
#from panda3d.core import Parabolaf
from panda3d.core import CollisionBox
from panda3d.core import CollisionCapsule
from panda3d.core import CollisionFloorMesh
from panda3d.core import CollisionInvSphere
from panda3d.core import CollisionLine
from panda3d.core import CollisionParabola
from panda3d.core import CollisionPlane
from panda3d.core import CollisionPolygon
from panda3d.core import CollisionRay
from panda3d.core import CollisionSegment
from panda3d.core import CollisionSphere
from panda3d.core import CollisionTube

from direct.showbase.ShowBase import ShowBase


class ColliderSolid:
    def __init__(self, name, solid, cam_shot=False, origin=True):
        self.name = name
        self.solid = solid
        self.cam_shot = cam_shot
        self.origin = origin


solids_into = [
    ColliderSolid("Sphere", CollisionSphere((0, 0, 0), 1.5)),
    ColliderSolid("Capsule", CollisionCapsule((-1, 0, 0), (1, 0, 0), 1)),
    ColliderSolid("Box", CollisionBox((-1, -1, -1), (1, 1, 1))),
    ColliderSolid("Plane", CollisionPlane(Plane((1, 0, 0), (0, 0, 0)))),
    ColliderSolid("InvSphere", CollisionInvSphere((0, 0, 0), 2)),
]
solids_from = [
#    ColliderSolid(
#        "Sphere",
#        CollisionSphere((0, 0, 0), 1.5),
#    ),
#    ColliderSolid(
#        "Capsule",
#        CollisionCapsule((-1, 0, 0), (1, 0, 0), 1),
#    ),
#    ColliderSolid(
#        "Box",
#        CollisionBox((-1, -1, -1), (1, 1, 1)),
#    ),
    ColliderSolid(
        "Segment",
        CollisionSegment((0, 0, 0), (0, 25, 0)),
        cam_shot=True, origin=False,
    ),
    ColliderSolid(
        "Ray",
        CollisionRay((0, 0, 0), (0, 1, 0)),
        cam_shot=True, origin=False,
    ),
    ColliderSolid(
        "Line",
        CollisionLine((0, 0, 0), (0, 1, 0)),
        cam_shot=True, origin=False,
    ),
]


class Base(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        base.disableMouse()
        self.accept("escape", sys.exit)

        # Camera
        self.camera_gimbal = base.render.attach_new_node("Camera gimbal")
        base.camera.reparent_to(self.camera_gimbal)
        base.camera.set_pos(0, -10, 0)
        self.rotation_mode = False
        self.mouse_pos = None
        self.accept("mouse3", self.set_rotation_mode, [True])
        self.accept("mouse3-up", self.set_rotation_mode, [False])
        self.accept("wheel_up", self.move_camera_distance, [-1])
        self.accept("wheel_down", self.move_camera_distance, [1])
        base.taskMgr.add(self.move_camera, "Move camera")

        self.from_coll = FromCollider(solids_from)
        self.accept("1", self.from_coll.switch_solid)
        self.into_coll = IntoCollider(solids_into)
        self.accept("2", self.into_coll.switch_solid)

    def set_rotation_mode(self, mode):
        self.rotation_mode = mode
        if base.mouseWatcherNode.has_mouse():
            self.mouse_pos = base.mouseWatcherNode.get_mouse()

    def move_camera_distance(self, direction):
        base.camera.set_pos(base.camera.get_pos() * (1 + 0.1 * direction))

    def move_camera(self, task):
        rot = globalClock.get_dt() * 360.0 / 3.0
        up_down = 0
        left_right = 0
        if base.mouseWatcherNode.is_button_down(KeyboardButton.up()):
            up_down -= 1
        if base.mouseWatcherNode.is_button_down(KeyboardButton.down()):
            up_down += 1
        if base.mouseWatcherNode.is_button_down(KeyboardButton.left()):
            left_right -=1
        if base.mouseWatcherNode.is_button_down(KeyboardButton.right()):
            left_right +=1
        if self.rotation_mode and base.mouseWatcherNode.has_mouse():
            mouse_pos = base.mouseWatcherNode.get_mouse()
            mouse_delta = mouse_pos - self.mouse_pos
            self.mouse_pos = Point2(mouse_pos)
            up_down += mouse_delta.get_y() * 50
            left_right += mouse_delta.get_x() * -50
        self.camera_gimbal.set_h(self.camera_gimbal.get_h() + left_right * rot)
        new_pitch = self.camera_gimbal.get_p() + up_down * rot
        self.camera_gimbal.set_p(min(max(new_pitch, -89), 89))
        return task.cont


class Collider:
    def __init__(self, solids):
        self.solids = solids
        self.solid_idx = 0

        self.np = base.render.attach_new_node("from")
        self.model = base.loader.load_model("models/zup-axis")
        self.model.reparent_to(self.np)
        self.model.set_scale(0.1)
        if not self.solids[self.solid_idx].origin:
            self.model.hide()
        self.coll = CollisionNode("coll")
        self.coll_np = self.np.attach_new_node(self.coll)
        self.coll_np.show()
        self.coll.add_solid(self.solids[self.solid_idx].solid)

    def switch_solid(self):
        self.coll.remove_solid(0)
        self.solid_idx = (self.solid_idx + 1) % len(self.solids)
        self.coll.add_solid(self.solids[self.solid_idx].solid)
        if self.solids[self.solid_idx].origin:
            self.model.show()
        else:
            self.model.hide()


class FromCollider(Collider):
    def __init__(self, solids):
        Collider.__init__(self, solids)
        self.coll.set_into_collide_mask(0)

        self.traverser = CollisionTraverser("traverser")
        self.traverser.show_collisions(base.render)
        self.queue = CollisionHandlerQueue()
        self.traverser.add_collider(self.coll_np, self.queue)
        base.task_mgr.add(self.collide, "collide")
        base.task_mgr.add(self.adjust_solid_pos, "adjust solid pos")

    def adjust_solid_pos(self, task):
        lmb = base.mouseWatcherNode.is_button_down(MouseButton.one())
        if lmb and self.solids[self.solid_idx].cam_shot:
            self.np.reparent_to(base.cam)
            self.np.set_pos(0, 0, 0)
            mpos = base.mouseWatcherNode.getMouse()
            mpos = Point3(mpos.x, mpos.y, 0)
            cam_space_vec = Point3()
            base.cam.node().get_lens().extrude_depth(mpos, cam_space_vec)
            cam_space_vec /= cam_space_vec.length()
            self.np.look_at(cam_space_vec)
            self.np.wrt_reparent_to(base.render)
        return task.cont

    def collide(self, task):
        self.traverser.traverse(base.render)
        return task.cont


class IntoCollider(Collider):
    def __init__(self, solids):
        Collider.__init__(self, solids)
        self.coll.set_from_collide_mask(0)
        #base.task_mgr.add(self.wobble, "wobble")

    def wobble(self, task):
        if self.wobble_type == STILL:
            pass
        elif self.wobble_type == LEFTRIGHT:
            self.np.set_x(sin((task.time*2*pi)/10) * 5)
        elif self.wobble_type == ROTATE:
            self.np.set_hpr(
                task.time*360/10,
                task.time*360/20,
                task.time*360/30,
            )
        return task.cont


if __name__ == '__main__':
    app = Base()
    app.run()
