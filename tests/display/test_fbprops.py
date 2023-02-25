from panda3d.core import FrameBufferProperties


def test_fbprops_copy_ctor():
    default = FrameBufferProperties.get_default()
    fbprops = FrameBufferProperties(default)
    assert fbprops == default


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
    req_color0 = FrameBufferProperties()
    req_color0.color_bits = 0

    req_color1 = FrameBufferProperties()
    req_color1.color_bits = 1

    req_color0_alpha0 = FrameBufferProperties()
    req_color0_alpha0.color_bits = 0
    req_color0_alpha0.alpha_bits = 0

    req_color1_alpha1 = FrameBufferProperties()
    req_color1_alpha1.color_bits = 1
    req_color1_alpha1.alpha_bits = 1

    req_rgb0 = FrameBufferProperties()
    req_rgb0.set_rgba_bits(0, 0, 0, 0)

    req_rgb1 = FrameBufferProperties()
    req_rgb1.set_rgba_bits(1, 1, 1, 0)

    req_rgb0_alpha0 = FrameBufferProperties()
    req_rgb0_alpha0.set_rgba_bits(0, 0, 0, 0)

    req_rgb1_alpha1 = FrameBufferProperties()
    req_rgb1_alpha1.set_rgba_bits(1, 1, 1, 1)

    fb_rgba8 = FrameBufferProperties()
    fb_rgba8.rgb_color = True
    fb_rgba8.set_rgba_bits(8, 8, 8, 8)

    fb_rgba16 = FrameBufferProperties()
    fb_rgba16.rgb_color = True
    fb_rgba16.set_rgba_bits(16, 16, 16, 16)

    assert fb_rgba8.get_quality(req_color0) > fb_rgba16.get_quality(req_color0)
    assert fb_rgba8.get_quality(req_color1) > fb_rgba16.get_quality(req_color1)
    assert fb_rgba8.get_quality(req_color0_alpha0) > fb_rgba16.get_quality(req_color0_alpha0)
    assert fb_rgba8.get_quality(req_color1_alpha1) > fb_rgba16.get_quality(req_color1_alpha1)
    assert fb_rgba8.get_quality(req_rgb0) > fb_rgba16.get_quality(req_rgb0)
    assert fb_rgba8.get_quality(req_rgb1) > fb_rgba16.get_quality(req_rgb1)
    assert fb_rgba8.get_quality(req_rgb0_alpha0) > fb_rgba16.get_quality(req_rgb0_alpha0)
    assert fb_rgba8.get_quality(req_rgb1_alpha1) > fb_rgba16.get_quality(req_rgb1_alpha1)


def test_fbquality_multi_samples():
    # Make sure that we get appropriate quality levels for different sample requests
    
    fb_0_samples = FrameBufferProperties()
    fb_0_samples.set_rgb_color(1)
    fb_0_samples.set_multisamples(0)
    
    fb_2_samples = FrameBufferProperties()
    fb_2_samples.set_rgb_color(1)
    fb_2_samples.set_multisamples(2)
    
    fb_4_samples = FrameBufferProperties()
    fb_4_samples.set_rgb_color(1)
    fb_4_samples.set_multisamples(4)
    
    fb_8_samples = FrameBufferProperties()
    fb_8_samples.set_rgb_color(1)
    fb_8_samples.set_multisamples(8)
    
    fb_16_samples = FrameBufferProperties()
    fb_16_samples.set_rgb_color(1)
    fb_16_samples.set_multisamples(16)
    
    req_0_samples = FrameBufferProperties()
    req_0_samples.set_multisamples(0)
    
    req_1_samples = FrameBufferProperties()
    req_1_samples.set_multisamples(1)
    
    req_2_samples = FrameBufferProperties()
    req_2_samples.set_multisamples(2)
    
    req_4_samples = FrameBufferProperties()
    req_4_samples.set_multisamples(4)
    
    req_8_samples = FrameBufferProperties()
    req_8_samples.set_multisamples(8)
    
    req_16_samples = FrameBufferProperties()
    req_16_samples.set_multisamples(16)

    # a fb which does not provide the requested number of samples should always 
    # have a lower quality than another
    assert fb_2_samples.get_quality(req_4_samples) < fb_2_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_4_samples) < fb_4_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_4_samples) < fb_8_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_4_samples) < fb_16_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_4_samples) < fb_16_samples.get_quality(req_16_samples)
    assert fb_8_samples.get_quality(req_16_samples) < fb_2_samples.get_quality(req_2_samples)
    
    # a fb which has more than the requested samples should have a 
    # lower quality than one that matches exactly
    assert fb_2_samples.get_quality(req_2_samples) > fb_4_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_2_samples) > fb_8_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_2_samples) > fb_16_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_2_samples) > fb_16_samples.get_quality(req_8_samples)
    
    # the more additional samples the fb provides rather than what is requested, 
    # the lower the quality should be
    assert fb_16_samples.get_quality(req_2_samples) < fb_8_samples.get_quality(req_2_samples)
    assert fb_8_samples.get_quality(req_2_samples) < fb_4_samples.get_quality(req_2_samples)
    
    # if the special value of 1 sample is requested, the fb with the highest samples should be 
    # in favour
    assert fb_16_samples.get_quality(req_1_samples) > fb_8_samples.get_quality(req_1_samples)
    assert fb_16_samples.get_quality(req_1_samples) > fb_4_samples.get_quality(req_1_samples)
    assert fb_16_samples.get_quality(req_1_samples) > fb_2_samples.get_quality(req_1_samples)
    assert fb_8_samples.get_quality(req_1_samples) > fb_4_samples.get_quality(req_1_samples)
    assert fb_8_samples.get_quality(req_1_samples) > fb_2_samples.get_quality(req_1_samples)
    
    # if 0 samples are requested, the fb with the highest samples should get a reduced quality level
    assert fb_16_samples.get_quality(req_0_samples) < fb_8_samples.get_quality(req_0_samples)
    assert fb_16_samples.get_quality(req_0_samples) < fb_4_samples.get_quality(req_0_samples)
    assert fb_16_samples.get_quality(req_0_samples) < fb_2_samples.get_quality(req_0_samples)
    assert fb_8_samples.get_quality(req_0_samples) < fb_4_samples.get_quality(req_0_samples)
    assert fb_8_samples.get_quality(req_0_samples) < fb_2_samples.get_quality(req_0_samples)

    # if samples are requested we prefer the ones with samples instead of having none
    assert fb_0_samples.get_quality(req_2_samples) < fb_2_samples.get_quality(req_4_samples)
    assert fb_0_samples.get_quality(req_2_samples) < fb_2_samples.get_quality(req_8_samples)
    assert fb_0_samples.get_quality(req_2_samples) < fb_2_samples.get_quality(req_16_samples)
    
    # we prefer buffers without samples if we don't request some
    assert fb_0_samples.get_quality(req_0_samples) > fb_2_samples.get_quality(req_0_samples)
    assert fb_0_samples.get_quality(req_0_samples) > fb_4_samples.get_quality(req_0_samples)
    assert fb_0_samples.get_quality(req_0_samples) > fb_8_samples.get_quality(req_0_samples)
    assert fb_0_samples.get_quality(req_0_samples) > fb_16_samples.get_quality(req_0_samples)


def test_fbquality_coverage_samples():
    # Make sure that we get appropriate quality levels for different sample requests
    
    fb_2_samples = FrameBufferProperties()
    fb_2_samples.set_rgb_color(1)
    fb_2_samples.set_coverage_samples(2)
    
    fb_4_samples = FrameBufferProperties()
    fb_4_samples.set_rgb_color(1)
    fb_4_samples.set_coverage_samples(4)
    
    fb_8_samples = FrameBufferProperties()
    fb_8_samples.set_rgb_color(1)
    fb_8_samples.set_coverage_samples(8)
    
    fb_16_samples = FrameBufferProperties()
    fb_16_samples.set_rgb_color(1)
    fb_16_samples.set_coverage_samples(16)
    
    req_0_samples = FrameBufferProperties()
    req_0_samples.set_coverage_samples(0)
    
    req_1_samples = FrameBufferProperties()
    req_1_samples.set_coverage_samples(1)
    
    req_2_samples = FrameBufferProperties()
    req_2_samples.set_coverage_samples(2)
    
    req_4_samples = FrameBufferProperties()
    req_4_samples.set_coverage_samples(4)
    
    req_8_samples = FrameBufferProperties()
    req_8_samples.set_coverage_samples(8)
    
    req_16_samples = FrameBufferProperties()
    req_16_samples.set_coverage_samples(16)

    # a fb which does not provide the requested number of samples should always 
    # have a lower quality than another
    assert fb_2_samples.get_quality(req_4_samples) < fb_2_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_4_samples) < fb_4_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_4_samples) < fb_8_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_4_samples) < fb_16_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_4_samples) < fb_16_samples.get_quality(req_16_samples)
    assert fb_8_samples.get_quality(req_16_samples) < fb_2_samples.get_quality(req_2_samples)
    
    # a fb which has more than the requested samples should have a 
    # lower quality than one that matches exactly
    assert fb_2_samples.get_quality(req_2_samples) > fb_4_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_2_samples) > fb_8_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_2_samples) > fb_16_samples.get_quality(req_2_samples)
    assert fb_2_samples.get_quality(req_2_samples) > fb_16_samples.get_quality(req_8_samples)
    
    # the more additional samples the fb provides rather than what is requested, 
    # the lower the quality should be
    assert fb_16_samples.get_quality(req_2_samples) < fb_8_samples.get_quality(req_2_samples)
    assert fb_8_samples.get_quality(req_2_samples) < fb_4_samples.get_quality(req_2_samples)
    
    # if the special value of 1 sample is requested, the fb with the highest samples should be 
    # in favour
    assert fb_16_samples.get_quality(req_1_samples) > fb_8_samples.get_quality(req_1_samples)
    assert fb_16_samples.get_quality(req_1_samples) > fb_4_samples.get_quality(req_1_samples)
    assert fb_16_samples.get_quality(req_1_samples) > fb_2_samples.get_quality(req_1_samples)
    assert fb_8_samples.get_quality(req_1_samples) > fb_4_samples.get_quality(req_1_samples)
    assert fb_8_samples.get_quality(req_1_samples) > fb_2_samples.get_quality(req_1_samples)
    
    # if 0 samples are requested, the fb with the highest samples should get a reduced quality level
    assert fb_16_samples.get_quality(req_0_samples) < fb_8_samples.get_quality(req_0_samples)
    assert fb_16_samples.get_quality(req_0_samples) < fb_4_samples.get_quality(req_0_samples)
    assert fb_16_samples.get_quality(req_0_samples) < fb_2_samples.get_quality(req_0_samples)
    assert fb_8_samples.get_quality(req_0_samples) < fb_4_samples.get_quality(req_0_samples)
    assert fb_8_samples.get_quality(req_0_samples) < fb_2_samples.get_quality(req_0_samples)
