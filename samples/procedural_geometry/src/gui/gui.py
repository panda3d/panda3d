# Author: Epihaius
# Date: 2019-06-21
#
# This package contains code to create a simple custom GUI.

from panda3d.core import *
from direct.showbase.DirectObject import DirectObject
from .widget import Widget
from .sizer import Sizer
from .button import Button


# The following class is responsible for controlling the GUI layout
class Window:

    def __init__(self, gui_root, mouse_watcher, hi_lights):

        self._gui_root = gui_root
        self._mouse_watcher = mouse_watcher
        self._hi_lights = hi_lights
        self._node = NodePath("window")
        self._sizer = Sizer("vertical")

    def get_ancestor(self, widget_type):

        if widget_type == "window":
            return self

    def get_gui_root(self):

        return self._gui_root

    def get_node(self):

        return self._node

    def get_mouse_watcher(self):

        return self._mouse_watcher

    def get_hi_lights(self):

        return self._hi_lights

    def add(self, *args, **kwargs):

        self._sizer.add(*args, **kwargs)

    def update(self, size):

        self._sizer.update(size)
        self._sizer.update_mouse_region_frames()

    def update_min_size(self):

        return self._sizer.update_min_size()

    def get_min_size(self):

        return self._sizer.get_min_size()


class GUI:

    def __init__(self, showbase):

        self._showbase = showbase
        root = NodePath("root")
        self._root = gui_root = root.attach_new_node("gui_root")
        CullBinManager.get_global_ptr().add_bin("gui", CullBinManager.BT_fixed, 41)
        gui_root.set_pos(-1., 0., 1.)
        gui_root.set_bin("gui", 0)
        gui_root.set_depth_test(False)
        gui_root.set_depth_write(False)
        gui_root.set_shader_auto()
        gui_mouse_watcher_node = MouseWatcher("gui")
        dr = showbase.win.make_display_region(0., 1., 0., 1.)
        dr.set_sort(100)
        dr.set_clear_depth(1000.)
        dr.set_clear_depth_active(True)
        gui_mouse_watcher_node.set_display_region(dr)
        input_ctrl = base.mouseWatcher.get_parent()
        mw = input_ctrl.attach_new_node(gui_mouse_watcher_node)
        gui_mouse_watcher_node.set_enter_pattern("gui_region_enter")
        gui_mouse_watcher_node.set_leave_pattern("gui_region_leave")
        self.mouse_watcher = gui_mouse_watcher_node
        btn_thrower_node = ButtonThrower("btn_thrower_gui")
        btn_thrower_node.set_prefix("gui_")
        mw.attach_new_node(btn_thrower_node)

        cam_node = Camera("gui_cam")
        gui_cam = root.attach_new_node(cam_node)
        gui_cam.set_y(-100.)
        self._lens = OrthographicLens()
        cam_node.set_lens(self._lens)
        cam_node.set_cull_bounds(OmniBoundingVolume())
        dr.set_camera(gui_cam)

        # set up a light source to reveal the 3D-nature of the widgets
        light_node = DirectionalLight("dir_light")
        light_node.set_color((1., 1., 1., 1.))
        self._light = gui_root.attach_new_node(light_node)
        self._light.set_hpr(-45., -45., 0.)
        gui_root.set_light(self._light)
        gui_root.set_shader_input("light", self._light)

        # set up 1 dynamic and 2 static light sources to highlight the widgets
        light_node = PointLight("hi_light0")
        light_node.set_color((.875, .895, .57, 1.))
        self._hi_light = hi_light0 = gui_root.attach_new_node(light_node)
        light_node = DirectionalLight("hi_light1")
        light_node.set_color((.3, .3, 0., 1.))
        hi_light1 = gui_root.attach_new_node(light_node)
        hi_light1.set_hpr(-45., -45., 0.)
        light_node = DirectionalLight("hi_light2")
        light_node.set_color((.6, .6, 0., 1.))
        hi_light2 = gui_root.attach_new_node(light_node)
        hi_light2.set_hpr(45., 45., 0.)
        hi_lights = (hi_light0, hi_light1, hi_light2)

        self._listener = listener = DirectObject()
        listener.accept("window-event", self.__handle_window_event)
        listener.accept("gui_region_enter", self.__on_region_enter)
        listener.accept("gui_region_leave", self.__on_region_leave)
        listener.accept("gui_mouse1", self.__on_left_down)
        listener.accept("gui_mouse1-up", self.__on_left_up)

        self.window = Window(gui_root, self.mouse_watcher, hi_lights)
        showbase.task_mgr.add(self.__update_hi_light, "update_hi_light")

    def add(self, *args, **kwargs):

        self.window.add(*args, **kwargs)

    def finalize(self):

        win_props = self._showbase.win.get_properties()
        w = win_props.get_x_size()
        h = win_props.get_y_size()
        self._window_size = (w, h)
        self.window.update((w, h))
        self.mouse_watcher.set_frame(0., w, -h, 0.)
        self._lens.set_film_size(w, h)
        self._lens.set_film_offset(w * .5 - 1., -h * .5 + 1.)

    def __update_hi_light(self, task):

        mouse_pointer = self._showbase.win.get_pointer(0)
        mouse_x = mouse_pointer.get_x()
        mouse_y = mouse_pointer.get_y()
        self._hi_light.set_pos(mouse_x, -60., -mouse_y)

        return task.cont

    def __on_region_enter(self, *args):

        name = args[0].get_name()

        if name.startswith("widget_"):
            widget_id = int(name.replace("widget_", ""))
            Widget.registry[widget_id].on_enter()

    def __on_region_leave(self, *args):

        name = args[0].get_name()

        if name.startswith("widget_"):

            widget_id = int(name.replace("widget_", ""))

            # the widget could already be destroyed and thus unregistered
            if widget_id in Widget.registry:
                Widget.registry[widget_id].on_leave()

    def __on_left_down(self):

        region = self.mouse_watcher.get_over_region()

        if not region:
            return

        name = region.get_name()

        if name.startswith("widget_"):
            widget_id = int(name.replace("widget_", ""))
            Widget.registry[widget_id].on_left_down()

    def __on_left_up(self):

        region = self.mouse_watcher.get_over_region()

        if not region:
            return

        name = region.get_name()

        if name.startswith("widget_"):
            widget_id = int(name.replace("widget_", ""))
            Widget.registry[widget_id].on_left_up()

    def __handle_window_event(self, window):

        win_props = window.get_properties()
        w, h = max(1, win_props.get_x_size()), max(1, win_props.get_y_size())

        if self._window_size != (w, h):

            self._window_size = (w, h)
            win_props = WindowProperties()
            win_props.set_size(w, h)
            window.request_properties(win_props)
            self._lens.set_film_size(w, h)
            self._lens.set_film_offset(w * .5 - 1., -h * .5 + 1.)

            w_min, h_min = self.window.update_min_size()
            self.mouse_watcher.set_frame(0., w, -h, 0.)

            if w < w_min:
                w = w_min

            if h < h_min:
                h = h_min

            self.window.update((w, h))
