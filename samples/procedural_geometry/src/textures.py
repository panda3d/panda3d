from panda3d.core import *
from .gui.font import Font
from collections import deque
from random import random, randint


class Textures:

    uv_map = None
    cube_map = None
    hull_tex = None

    @classmethod
    def create(cls):

        cls.__create_uv_map()
        cls.__create_cube_map()
        cls.__create_hull_tex()

    @classmethod
    def __create_uv_map(cls):

        font = Font("fonts/Arimo-Bold.ttf", 56., 72, 56, 88)
        color = (0., 0., 0., 1.)
        back_colors = ((1., 0., 0., 1.), (0., 1., 0., 1.),
                      (1., 1., 0., 1.), (1., 0., 1., 1.),
                      (0., 1., 1., 1.), (1., 1., 1., 1.),
                      (1., .5, 1., 1.), (.5, 1., 1., 1.))
        d_max = 0
        txt_imgs = deque([])
        fill_brushes = deque([])
        pen = PNMBrush.make_spot(color, 3, False)

        for i, back_color in enumerate(back_colors):
            txt_img = font.create_image(str(i), color)
            txt_imgs.append(txt_img)
            d_max = max(d_max, txt_img.get_x_size(), txt_img.get_y_size())
            fill_brushes.append(PNMBrush.make_pixel(back_color))

        d_max += int(d_max * .5)
        image = PNMImage(d_max * 8, d_max * 8)
        image.fill(1., 1., 1.)
        painter = PNMPainter(image)
        painter.set_pen(pen)
        y = 0

        for i in range(8):

            x = 0

            for txt_img, brush in zip(txt_imgs, fill_brushes):
                painter.set_fill(brush)
                painter.draw_rectangle(x, y, x + d_max, y + d_max)
                offset_x = (d_max - txt_img.get_x_size()) // 2
                offset_y = (d_max - txt_img.get_y_size()) // 2
                image.blend_sub_image(txt_img, x + offset_x, y + offset_y, 0, 0)
                x += d_max

            txt_imgs.rotate(1)
            fill_brushes.rotate(1)
            y += d_max

        cls.uv_map = Texture("uv_map")
        cls.uv_map.load(image)

    @classmethod
    def __create_cube_map(cls):

        font = Font("fonts/Arimo-Bold.ttf", 56., 72, 56, 88)
        color = (0., 0., 0., 1.)
        w_max = 0
        txt_imgs = []
        side_ids = ("left", "back", "bottom", "right", "front", "top")

        for side_id in side_ids:
            txt_img = font.create_image(side_id, color)
            txt_imgs.append(txt_img)
            w_txt = txt_img.get_x_size()
            w_max = max(w_max, w_txt)

        w_max += int(w_max * .2)
        image = PNMImage(w_max * 6, w_max)
        image.fill(1., 1., 1.)
        x = 0
        y = (w_max - font.get_height()) // 2

        for txt_img in txt_imgs:
            offset = (w_max - txt_img.get_x_size()) // 2
            image.blend_sub_image(txt_img, x + offset, y, 0, 0)
            x += w_max

        cls.cube_map = Texture("cube_map")
        cls.cube_map.load(image)

    @classmethod
    def __create_hull_tex(cls):

        image = PNMImage(256, 256)
        image.fill(.8, .8, .8)
        pen = PNMBrush.make_transparent()
        painter = PNMPainter(image)
        painter.set_pen(pen)

        for i in range(50):
            offset = .2 - random() * .4
            color = (.8 + offset, .8 + offset, .8 + offset, 1.)
            fill_brush = PNMBrush.make_pixel(color)
            painter.set_fill(fill_brush)
            painter.draw_rectangle(randint(0, 255), randint(0, 255),
                                   randint(0, 255), randint(0, 255))

        cls.hull_tex = Texture("hull_tex")
        cls.hull_tex.load(image)
