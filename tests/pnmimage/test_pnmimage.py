from panda3d.core import PNMImage, PNMImageHeader


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
