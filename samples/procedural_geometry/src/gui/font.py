# Author: Epihaius
# Date: 2019-06-21
#
# This module contains a class to procedurally create text images.

from panda3d.core import *


class Font:

    def __init__(self, path, pixel_size, height, y, line_spacing):

        self._text_maker = text_maker = PNMTextMaker(path, 0)
        text_maker.set_pixel_size(pixel_size)
        text_maker.set_scale_factor(1.)
        self._height = height
        self._y = y
        self._line_spacing = max(height, line_spacing)

    def get_height(self):

        return self._height

    def get_line_spacing(self):

        return self._line_spacing

    def calc_width(self, text):

        return self._text_maker.calc_width(text)

    def __create_image(self, text, text_color, back_color):

        text_maker = self._text_maker
        w = text_maker.calc_width(text)
        h = self._height
        image = PNMImage(w, h, 4)

        if back_color is not None:
            r, g, b, a = back_color
            image.fill(r, g, b)
            image.alpha_fill(a)

        text_maker.set_fg(text_color)
        text_maker.generate_into(text, image, 0, self._y)
        image.unpremultiply_alpha()

        return image

    def create_image(self, text, text_color=(0., 0., 0., 1.), back_color=None):

        lines = text.split("\n")
        line_count = len(lines)

        if line_count == 1:
            return self.__create_image(text, text_color, back_color)

        line_imgs = []
        width = 0

        for line in lines:
            line_img = self.__create_image(line, text_color, back_color)
            line_imgs.append(line_img)
            width = max(width, line_img.get_x_size())

        img = PNMImage(width, self._line_spacing * (line_count - 1) + self._height, 4)

        for i, line_img in enumerate(line_imgs):
            img.copy_sub_image(line_img, 0, i * self._line_spacing, 0, 0)

        return img
