from panda3d.core import PNMImage, PNMImageHeader
from random import randint


def test_pixelspec_ctor():
    assert tuple(PNMImage.PixelSpec(1)) == (1, 1, 1, 0)
    assert tuple(PNMImage.PixelSpec(1, 2)) == (1, 1, 1, 2)
    assert tuple(PNMImage.PixelSpec(1, 2, 3)) == (1, 2, 3, 0)
    assert tuple(PNMImage.PixelSpec(1, 2, 3, 4)) == (1, 2, 3, 4)

    assert tuple(PNMImage.PixelSpec((1, 2, 3))) == (1, 2, 3, 0)
    assert tuple(PNMImage.PixelSpec((1, 2, 3), 4)) == (1, 2, 3, 4)

    # Copy constructor
    spec = PNMImage.PixelSpec(1, 2, 3, 4)
    assert tuple(PNMImage.PixelSpec(spec)) == (1, 2, 3, 4)


def test_pixelspec_coerce():
    img = PNMImage(1, 1, 4)
    img.set_pixel(0, 0, (1, 2, 3, 4))
    assert img.get_pixel(0, 0) == (1, 2, 3, 4)


def test_pnmimage_to_val():
    img = PNMImage(1, 1)
    assert img.to_val(-0.5) == 0
    assert img.to_val(0.0) == 0
    assert img.to_val(0.5) == 128
    assert img.to_val(1.0) == 255
    assert img.to_val(2.0) == 255


def test_pnmimage_from_val():
    img = PNMImage(1, 1)
    assert img.from_val(0) == 0.0
    assert img.to_val(img.from_val(128)) == 128
    assert img.from_val(255) == 1.0


def test_pnmimage_quantize():
    img = PNMImage(32, 32, 3)

    for x in range(32):
        for y in range(32):
            img.set_xel_val(x, y, randint(0, 100), randint(50, 100), randint(0, 1))

    hist = PNMImage.Histogram()
    img.make_histogram(hist)
    num_colors = hist.get_num_pixels()
    assert num_colors > 100

    img2 = PNMImage(img)
    img2.quantize(100)
    hist = PNMImage.Histogram()
    img2.make_histogram(hist)
    assert hist.get_num_pixels() <= 100

    # Make sure that this is reasonably close
    max_dist = 0
    for x in range(32):
        for y in range(32):
            diff = img.get_xel(x, y) - img2.get_xel(x, y)
            max_dist = max(max_dist, diff.length_squared())

            # Also make sure that they are not out of range of the original
            col = img2.get_xel_val(x, y)
            assert col.r <= 100
            assert col.g >= 50 and col.g <= 100
            assert col.b in (0, 1)

    assert max_dist < 0.1 ** 2
