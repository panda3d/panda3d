from panda3d.core import FrameBufferProperties


def test_fbquality_depth():
    # We check common framebuffer depth configurations to make sure they
    # are rated predictably with respect to each other when requesting 1 bit.
    # In particular, we make sure that we don't get a 16-bit depth buffer if
    # the driver only gives extra depth in combination with a stencil buffer.
    req = FrameBufferProperties()
    req.depth_bits = 1

    #NB. we need to set rgb_color=True when testing the quality of framebuffer
    # properties, lest it return a quality of 0.
    fb_d16s8 = FrameBufferProperties()
    fb_d16s8.rgb_color = True
    fb_d16s8.depth_bits = 16
    fb_d16s8.stencil_bits = 8

    fb_d24s8 = FrameBufferProperties()
    fb_d24s8.rgb_color = True
    fb_d24s8.depth_bits = 24
    fb_d24s8.stencil_bits = 8

    fb_d32 = FrameBufferProperties()
    fb_d32.rgb_color = True
    fb_d32.depth_bits = 32

    fb_d32s8 = FrameBufferProperties()
    fb_d32s8.rgb_color = True
    fb_d32s8.depth_bits = 32
    fb_d32s8.stencil_bits = 8

    # 16-bit depth is terrible for most applications and should not be chosen.
    assert fb_d16s8.get_quality(req) < fb_d24s8.get_quality(req)
    assert fb_d16s8.get_quality(req) < fb_d32.get_quality(req)
    assert fb_d16s8.get_quality(req) < fb_d32s8.get_quality(req)

    # Getting extra depth should be better than getting an unwanted bitplane.
    assert fb_d32.get_quality(req) > fb_d16s8.get_quality(req)
    assert fb_d32.get_quality(req) > fb_d24s8.get_quality(req)

    # If we're getting stencil anyway, we'll prefer to maximize our depth.
    assert fb_d32s8.get_quality(req) > fb_d24s8.get_quality(req)

    # However, unnecessary stencil bits are still a waste.
    assert fb_d32s8.get_quality(req) < fb_d32.get_quality(req)


def test_fbquality_rgba64():
    # Make sure that we don't get a 64-bit configuration if we request
    # an unspecific number of color bits.  See:
    #   https://www.panda3d.org/forums/viewtopic.php?t=20192
    # This issue occurs if we are requesting 1 bit, not if we are requesting
    # a specific amount.  There are several ways to do that, so we want to
    # assert that none of them will yield a 64-bit color buffer.
    req_color = FrameBufferProperties()
    req_color.color_bits = 1

    req_color_alpha = FrameBufferProperties()
    req_color_alpha.color_bits = 1
    req_color_alpha.alpha_bits = 1

    req_rgb = FrameBufferProperties()
    req_rgb.set_rgba_bits(1, 1, 1, 0)

    req_rgb_alpha = FrameBufferProperties()
    req_rgb_alpha.set_rgba_bits(1, 1, 1, 1)

    fb_rgba8 = FrameBufferProperties()
    fb_rgba8.rgb_color = True
    fb_rgba8.set_rgba_bits(8, 8, 8, 8)

    fb_rgba16 = FrameBufferProperties()
    fb_rgba16.rgb_color = True
    fb_rgba16.set_rgba_bits(16, 16, 16, 16)

    assert fb_rgba8.get_quality(req_color) > fb_rgba16.get_quality(req_color)
    assert fb_rgba8.get_quality(req_color_alpha) > fb_rgba16.get_quality(req_color_alpha)
    assert fb_rgba8.get_quality(req_rgb) > fb_rgba16.get_quality(req_rgb)
    assert fb_rgba8.get_quality(req_rgb_alpha) > fb_rgba16.get_quality(req_rgb_alpha)
