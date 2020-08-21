from panda3d import core

def luminance(col):
    return 0.2126 * col[0] + 0.7152 * col[1] + 0.0722 * col[2]


def test_light_colortemp():
    # Default is all white, assuming a D65 white point.
    light = core.PointLight("light")
    assert light.color == (1, 1, 1, 1)
    assert light.color_temperature == 6500

    # When setting color temp, it should preserve luminance.
    for temp in range(2000, 15000):
        light.color_temperature = temp
        assert abs(luminance(light.color) - 1.0) < 0.001

    # Setting it to the white point will make a white color.
    light.color_temperature = 6500
    assert light.color.almost_equal((1, 1, 1, 1), 0.001)
