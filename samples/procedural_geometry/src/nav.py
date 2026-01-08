# Author: Epihaius
# Date: 2019-06-21
#
# This module contains code to create a simple navigation system.

from panda3d.core import *
from direct.showbase.DirectObject import DirectObject


class NavigationManager:

    def __init__(self, showbase, target_pos=Point3(), gui_mouse_watcher=None):

        self.showbase = showbase
        self.task_mgr = showbase.task_mgr
        self.mouse_watcher = showbase.mouseWatcherNode
        self.gui_mouse_watcher = gui_mouse_watcher
        self.cam = showbase.camera
        self.cam_target = showbase.render.attach_new_node("camera_target")
        self.cam_target.set_pos(target_pos)
        self.cam.reparent_to(self.cam_target)
        self.cam.set_y(-10.)
        self.mouse_prev = Point2()
        win_props = showbase.win.get_properties()
        w, h = win_props.get_x_size(), win_props.get_y_size()
        self.orbit_speed = (w * .15, h * .15)
        self.listener = listener = DirectObject()
        listener.accept("mouse1", self.start_orbiting)
        listener.accept("mouse1-up", self.stop_orbiting)
        listener.accept("wheel_up", self.zoom_step_in)
        listener.accept("wheel_down", self.zoom_step_out)

    def start_orbiting(self):

        # if the mouse is over a GUI widget, don't start navigating the scene
        if self.gui_mouse_watcher.get_over_region():
            return

        win_props = self.showbase.win.get_properties()
        w, h = win_props.get_x_size(), win_props.get_y_size()
        self.orbit_speed = (w * .15, h * .15)
        self.mouse_prev = Point2(self.mouse_watcher.get_mouse())
        self.task_mgr.add(self.orbit, "orbit")

    def stop_orbiting(self):

        self.task_mgr.remove("orbit")

    def orbit(self, task):
        """
        Orbit the camera about its target point by offsetting the orientation
        of the target node with the mouse motion.

        """

        if self.mouse_watcher.has_mouse():
            mouse_pos = self.mouse_watcher.get_mouse()
            speed_x, speed_y = self.orbit_speed
            d_h, d_p = (mouse_pos - self.mouse_prev)
            d_h *= speed_x
            d_p *= speed_y
            target = self.cam_target
            target.set_hpr(target.get_h() - d_h, target.get_p() + d_p, 0.)
            self.mouse_prev = Point2(mouse_pos)

        return task.cont

    def zoom_step_in(self):
        """Translate the camera along its positive local Y-axis to zoom in"""

        # if the mouse is over a GUI widget, don't zoom in
        if self.gui_mouse_watcher.get_over_region():
            return

        target_dist = self.cam.get_y()
        self.cam.set_y(self.cam, -target_dist * .1)

    def zoom_step_out(self):
        """Translate the camera along its negative local Y-axis to zoom out"""

        # if the mouse is over a GUI widget, don't zoom out
        if self.gui_mouse_watcher.get_over_region():
            return

        target_dist = self.cam.get_y()
        self.cam.set_y(self.cam, target_dist * .1)
