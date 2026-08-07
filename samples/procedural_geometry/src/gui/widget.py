# Author: Epihaius
# Date: 2019-06-21
#
# This module contains the base class of all GUI widgets.

from panda3d.core import *


class Widget:

    registry = {}
    _count = 0

    def __init__(self, widget_type, parent, borders, stretch_dir="", has_mouse_region=True):

        self._type = "widget"
        self._widget_type = widget_type
        self._parent = parent
        self._node = parent.get_node().attach_new_node("widget")
        self._inner_borders = borders
        self._stretch_dir = stretch_dir
        border_left, border_right, border_bottom, border_top = borders
        w = border_left + border_right
        h = border_bottom + border_top
        self._size = self._default_size = self._min_size = (w, h)
        self._sizer = None
        self._sizer_item = None

        self._widget_id = Widget._count
        Widget._count += 1

        if has_mouse_region:
            name = "widget_{:d}".format(self._widget_id)
            self._mouse_region = MouseWatcherRegion(name, 0., 0., 0., 0.)
            if parent:
                self.get_mouse_watcher().add_region(self._mouse_region)
        else:
            self._mouse_region = None

        Widget.registry[self._widget_id] = self

    def destroy(self):

        if self._widget_id not in Widget.registry:
            return False

        if self._node:
            self._node.remove_node()
            self._node = None

        if self._sizer:
            self._sizer.destroy()
            self._sizer = None

        self._sizer_item = None

        if self._parent:

            if self._mouse_region:
                self.get_mouse_watcher().remove_region(self._mouse_region)
                self._mouse_region = None

            self._parent = None

        del Widget.registry[self._widget_id]

        return True

    def get_type(self):

        return self._type

    def get_widget_type(self):

        return self._widget_type

    def get_ancestor(self, widget_type):

        if self._widget_type == widget_type:
            return self

        if self._parent:
            return self._parent.get_ancestor(widget_type)

    def get_widget_id(self):

        return self._widget_id

    def set_parent(self, parent):

        if parent:
            self._node.reparent_to(parent.get_node())
        else:
            self._node.detach_node()

        self._parent = parent

    def get_parent(self):

        return self._parent

    def get_inner_borders(self):

        return self._inner_borders

    def get_mouse_region(self):

        return self._mouse_region

    def get_node(self):

        return self._node

    def get_gui_root(self):

        return self._parent.get_gui_root()

    def set_pos(self, pos):

        x, y = pos
        self.get_node().set_pos(x, 0, -y)
        x, z = self.get_pos(from_root=True)
        self.model.set_pos(x, 0., -z)

    def get_pos(self, ref_node=None, from_root=False):

        node = self.get_node()

        if ref_node:
            x, y, z = node.get_pos(ref_node)
        elif from_root:
            x, y, z = node.get_pos(node.get_top())
        else:
            x, y, z = node.get_pos(self.get_parent().get_node())

        y = -z

        return (int(x), int(y))

    def set_sizer(self, sizer):

        if sizer:
            sizer.set_owner(self)

        self._sizer = sizer

    def get_sizer(self):

        return self._sizer

    def set_sizer_item(self, sizer_item):

        self._sizer_item = sizer_item

    def get_sizer_item(self):

        return self._sizer_item

    def get_stretch_dir(self):

        return self._stretch_dir

    def get_min_size(self):

        return self._sizer.get_min_size() if self._sizer else self._min_size

    def set_size(self, size, includes_borders=True, is_min=False):

        width, height = size
        w_def, h_def = self._default_size
        l, r, b, t = self._inner_borders
        borders_h = l + r
        borders_v = b + t

        if not includes_borders:
            w_def -= borders_h
            h_def -= borders_v

        width = max(w_def, width)
        height = max(h_def, height)

        if is_min:
            w_new, h_new = width, height
        else:
            w_new, h_new = self.get_min_size()
            width, height = (max(w_new, width), max(h_new, height))

        if includes_borders:
            w_new = max(width, borders_h)
        else:
            w_new = width + borders_h

        if includes_borders:
            h_new = max(height, borders_v)
        else:
            h_new = height + borders_v

        new_size = (w_new, h_new)

        if self._sizer:
            self._sizer.set_size(new_size)

        self._size = new_size

        if is_min:
            self._min_size = new_size

        return new_size

    def get_default_size(self):

        return self._default_size

    def get_size(self):

        return self._sizer.get_size() if self._sizer else self._size

    def update_mouse_region_frames(self, recurse=True):

        if self._sizer and recurse:
            self._sizer.update_mouse_region_frames()

        if not self._mouse_region:
            return

        w, h = self.get_size()
        x, y = self.get_pos(from_root=True)
        l = x
        r = x + w
        b = -y - h
        t = -y
        self._mouse_region.set_frame(l, r, b, t)

    def get_mouse_watcher(self):

        return self._parent.get_mouse_watcher()

    def get_hi_lights(self):

        return self._parent.get_hi_lights()

    def on_enter(self): pass

    def on_leave(self): pass

    def on_left_down(self): pass

    def on_left_up(self): pass

    def on_right_down(self): pass

    def on_right_up(self): pass
