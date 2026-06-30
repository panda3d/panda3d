# Author: Epihaius
# Date: 2019-06-21
#
# This module contains classes to implement a "sizer" system, the purpose of
# which is to maintain the layout of widgets when resizing the window.

from panda3d.core import *


class SizerItem:

    def __init__(self, sizer, obj, obj_type, proportion=0., expand=False,
                 alignment="", borders=None):

        self._sizer = sizer
        self._obj = obj
        self._type = obj_type
        self._proportion = proportion
        self._expand = expand
        self._alignment = alignment

        if borders is None:
            outer_borders = (0, 0, 0, 0)
        else:
            outer_borders = borders

        l, r, b, t = self._borders = outer_borders

        x = y = 0

        if obj_type == "size":
            w, h = obj
        else:
            w, h = obj.get_min_size()

        w += l
        x = l
        w += r
        h += t
        y = t
        h += b

        self._obj_offset = (x, y)
        self._size = self._min_size = (w, h)

    def destroy(self):

        self._sizer = None

        if self._type != "size":
            self._obj.destroy()

        self._obj = None
        self._type = ""
        self._proportion = 0
        self._expand = False
        self._alignment = ""
        self._borders = (0, 0, 0, 0)
        self._obj_offset = (0, 0)
        self._size = self._min_size = (0, 0)

    def set_sizer(self, sizer):

        self._sizer = sizer

        if self._type == "sizer":
            self._obj.set_owner(sizer)

    def get_sizer(self):

        return self._sizer

    def get_type(self):

        return self._type

    def get_object(self):

        return self._obj

    def set_proportion(self, proportion):

        self._proportion = proportion

    def get_proportion(self):

        return self._proportion

    def set_expand(self, expand):

        self._expand = expand

    def get_expand(self):

        return self._expand

    def get_alignment(self):

        return self._alignment

    def get_borders(self):

        return self._borders

    def get_object_offset(self):

        return self._obj_offset

    def update_min_size(self):

        if self._type == "size":
            w, h = self._obj
        if self._type == "sizer":
            w, h = self._obj.update_min_size()
        if self._type == "widget":
            w, h = self._obj.get_min_size()

        l, r, b, t = self._borders
        w += l + r
        h += b + t
        self._min_size = (w, h)

        return self._min_size

    def get_min_size(self):

        return self._min_size

    def set_size(self, size):

        width, height = self._size = size
        x = y = 0
        l, r, b, t = self._borders

        if self._type != "size":

            width -= l + r
            height -= t + b
            x += l
            y += t
            stretch_dir = self._sizer.get_stretch_dir()
            w, h = self._obj.get_min_size()

            if stretch_dir == "horizontal":
                if not self._expand:
                    if self._alignment == "bottom":
                        y += height - h
                    elif self._alignment == "center_v":
                        y += (height - h) // 2
            elif stretch_dir == "vertical":
                if not self._expand:
                    if self._alignment == "right":
                        x += width - w
                    elif self._alignment == "center_h":
                        x += (width - w) // 2

            w_new, h_new = w, h

            if stretch_dir == "horizontal":
                if self._proportion > 0.:
                    w_new = width
                if self._expand:
                    h_new = height
            elif stretch_dir == "vertical":
                if self._proportion > 0.:
                    h_new = height
                if self._expand:
                    w_new = width

            new_size = (w_new, h_new)
            self._obj.set_size(new_size)

        self._obj_offset = (x, y)

    def get_size(self):

        return self._size


class Sizer:

    def __init__(self, stretch_dir=""):

        self._type = "sizer"
        self._sizer_type = "sizer"
        self._owner = None
        self._sizer_item = None
        self._stretch_dir = stretch_dir
        self._pos = (0, 0)
        # minimum size without any contents
        self._default_size = (0, 0)
        # minimum size needed to accommodate current contents, bigger than or
        # equal to default size
        self._min_size = (0, 0)
        self._is_min_size_stale = True
        # current size, bigger than or equal to minimum size needed for current
        # contents
        self._size = (0, 0)
        self._items = []

    def destroy(self):

        for item in self._items:
            item.destroy()

        self._items = []
        self._owner = None
        self._sizer_item = None

    def clear(self, destroy_items=False):

        if destroy_items:
            for item in self._items:
                item.destroy()

        self._items = []
        self.set_min_size_stale()

    def get_type(self):

        return self._type

    def get_sizer_type(self):

        return self._sizer_type

    def set_owner(self, owner):

        self._owner = owner

        if owner and owner.get_type() == "widget":
            self.set_default_size(owner.get_default_size())

    def get_owner(self):

        return self._owner

    def set_sizer_item(self, sizer_item):
        """ Create a reference to the SizerItem this subsizer is tracked by """

        self._sizer_item = sizer_item

    def get_sizer_item(self):
        """ Return the SizerItem this sizer is tracked by, in case it is a subsizer """

        return self._sizer_item

    def add(self, obj, proportion=0., expand=False, alignment="", borders=None, index=None):

        obj_type = "size" if type(obj) == tuple else obj.get_type()
        item = SizerItem(self, obj, obj_type, proportion, expand, alignment, borders)

        if index is None:
            self._items.append(item)
        else:
            self._items.insert(index, item)

        self.set_min_size_stale()

        if obj_type == "sizer":
            obj.set_owner(self)

        if obj_type != "size":
            obj.set_sizer_item(item)

        return item

    def add_item(self, item, index=None):

        item.set_sizer(self)

        if index is None:
            self._items.append(item)
        else:
            self._items.insert(index, item)

        self.set_min_size_stale()

    def remove_item(self, item, destroy=False):

        self._items.remove(item)
        item.set_sizer(None)

        if destroy:
            item.destroy()

        self.set_min_size_stale()

    def pop_item(self, index=None):

        item = self._items[-1 if index is None else index]
        self._items.remove(item)
        item.set_sizer(None)
        self.set_min_size_stale()

        return item

    def get_item_index(self, item):

        return self._items.index(item)

    def get_item(self, index):

        return self._items[index]

    def get_items(self):

        return self._items

    def get_item_count(self):

        return len(self._items)

    def get_widgets(self, include_children=True):

        widgets = []

        for item in self._items:

            if item.get_type() == "widget":

                widget = item.get_object()
                widgets.append(widget)

                if include_children:

                    sizer = widget.get_sizer()

                    if sizer:
                        widgets.extend(sizer.get_widgets())

            elif item.get_type() == "sizer":

                widgets.extend(item.get_object().get_widgets(include_children))

        return widgets

    def set_pos(self, pos):

        self._pos = pos

    def get_pos(self, from_root=False):

        x, y = self._pos

        if from_root:

            owner = self._owner

            while owner:

                if owner.get_type() == "widget":
                    x_o, y_o = owner.get_pos(from_root=True)
                    x += x_o
                    y += y_o
                    break

                owner = owner.get_owner()

        return (x, y)

    def get_stretch_dir(self):

        return self._stretch_dir

    def set_default_size(self, size):

        w_d, h_d = self._default_size = size
        w_min, h_min = self._min_size
        self._min_size = w_min, h_min = (max(w_d, w_min), max(h_d, h_min))
        w, h = self._size
        self._size = (max(w_min, w), max(h_min, h))

        if self._sizer_item:
            self._sizer_item.update_min_size()

        self.set_min_size_stale()

    def get_default_size(self):

        return self._default_size

    def set_min_size_stale(self, stale=True):

        self._is_min_size_stale = stale

        if stale and self._owner:

            if self._owner.get_type() == "sizer":

                self._owner.set_min_size_stale()

            elif self._owner.get_type() == "widget":

                item = self._owner.get_sizer_item()

                if item:

                    item_sizer = item.get_sizer()

                    if item_sizer:
                        item_sizer.set_min_size_stale()

    def update_min_size(self):

        if not self._is_min_size_stale:
            return self._min_size

        width = height = 0

        for item in self._items:

            if item.get_type() == "widget":

                sizer = item.get_object().get_sizer()

                if sizer:
                    sizer.update_min_size()

            w, h = item.update_min_size()

            if self._stretch_dir == "horizontal":
                width += w
            else:
                width = max(width, w)

            if self._stretch_dir == "vertical":
                height += h
            else:
                height = max(height, h)

        w_d, h_d = self._default_size
        self._min_size = width, height = (max(w_d, width), max(h_d, height))
        self._is_min_size_stale = False
        w, h = self._size
        self._size = (max(width, w), max(height, h))

        return self._min_size

    def get_min_size(self):

        return self._min_size

    def __check_proportions(self, items, total_size, sizes, dim):

        proportions = [i.get_proportion() for i in items]
        p_sum = sum(proportions)
        tmp_size = total_size

        for item, proportion in zip(items, proportions):

            s_min = item.get_min_size()[dim]
            s_new = int(round(tmp_size * proportion / p_sum))

            if s_new < s_min:
                items.remove(item)
                index = self._items.index(item)
                sizes[index] = s_min
                total_size -= s_min
                return True, total_size

            p_sum -= proportion
            tmp_size -= s_new

        return False, total_size

    def set_size(self, size, force=False):

        if force:
            self._size = size
            return

        width, height = size
        w_min, h_min = size_min = list(self.get_min_size())
        self._size = (max(w_min, width), max(h_min, height))
        dim = 0 if self._stretch_dir == "horizontal" else 1
        w_min, h_min = size_min
        width, height = (max(w_min, width), max(h_min, height))

        widths = heights = None

        if self._stretch_dir == "horizontal":

            widths = [0] * len(self._items)
            sizer_items = self._items[:]

            for index, item in enumerate(self._items):

                proportion = item.get_proportion()

                if proportion == 0.:
                    sizer_items.remove(item)
                    w_min = item.get_min_size()[0]
                    width -= w_min
                    widths[index] = w_min

            check_proportions = True

            while check_proportions:
                check_proportions, width = self.__check_proportions(sizer_items,
                                                                    width, widths, 0)

            proportions = [i.get_proportion() for i in sizer_items]
            p_sum = sum(proportions)
            sizer_items = [(i.get_min_size()[0], i) for i in sizer_items]
            last_item = sizer_items.pop() if sizer_items else None

            for w_min, item in sizer_items:

                proportion = item.get_proportion()
                index = self._items.index(item)
                w_new = int(round(width * proportion / p_sum))

                if w_new < w_min:
                    w_new = w_min

                p_sum -= proportion
                width -= w_new
                widths[index] = w_new

            if last_item:
                w_min, item = last_item
                index = self._items.index(item)
                widths[index] = width

        elif self._stretch_dir == "vertical":

            heights = [0] * len(self._items)
            sizer_items = self._items[:]

            for index, item in enumerate(self._items):

                proportion = item.get_proportion()

                if proportion == 0.:
                    sizer_items.remove(item)
                    h_min = item.get_min_size()[1]
                    height -= h_min
                    heights[index] = h_min

            check_proportions = True

            while check_proportions:
                check_proportions, height = self.__check_proportions(sizer_items,
                                                                     height, heights, 1)

            proportions = [i.get_proportion() for i in sizer_items]
            p_sum = sum(proportions)
            sizer_items = [(i.get_min_size()[1], i) for i in sizer_items]
            last_item = sizer_items.pop() if sizer_items else None

            for h_min, item in sizer_items:

                proportion = item.get_proportion()
                index = self._items.index(item)
                h_new = int(round(height * proportion / p_sum))

                if h_new < h_min:
                    h_new = h_min

                p_sum -= proportion
                height -= h_new
                heights[index] = h_new

            if last_item:
                h_min, item = last_item
                index = self._items.index(item)
                heights[index] = height

        if not (widths or heights):
            return

        if not widths:
            widths = [width] * len(self._items)

        if not heights:
            heights = [height] * len(self._items)

        for item, w, h in zip(self._items, widths, heights):
            item.set_size((w, h))

    def get_size(self):

        return self._size

    def calculate_positions(self, start_pos=(0, 0)):

        x, y = start_x, start_y = start_pos

        for item in self._items:

            obj = item.get_object()
            w, h = item.get_size()
            offset_x, offset_y = item.get_object_offset()
            pos = (x + offset_x, y + offset_y)

            if item.get_type() == "widget":

                obj.set_pos(pos)
                sizer = obj.get_sizer()

                if sizer:
                    sizer.calculate_positions()

            elif item.get_type() == "sizer":

                obj.set_pos(pos)
                obj.calculate_positions(pos)

            if self._stretch_dir == "horizontal":
                x += w

            if self._stretch_dir == "vertical":
                y += h

    def update(self, size=None):

        self.update_min_size()

        if size:
            self.set_size(size)
            self.calculate_positions()

    def update_mouse_region_frames(self):

        for item in self._items:
            if item.get_type() != "size":
                item.get_object().update_mouse_region_frames()
