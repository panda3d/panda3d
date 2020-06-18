from panda3d import core

# Some dummy lights we can use for our light attributes.
spot = core.NodePath(core.Spotlight("spot"))
point = core.NodePath(core.PointLight("point"))
ambient = core.NodePath(core.AmbientLight("ambient"))


def test_lightattrib_compose_add():
    # Tests a case in which a child node adds another light.
    lattr1 = core.LightAttrib.make()
    lattr1 = lattr1.add_on_light(spot)

    lattr2 = core.LightAttrib.make()
    lattr2 = lattr2.add_on_light(point)

    lattr3 = lattr1.compose(lattr2)
    assert lattr3.get_num_on_lights() == 2

    assert spot in lattr3.on_lights
    assert point in lattr3.on_lights


def test_lightattrib_compose_subtract():
    # Tests a case in which a child node disables a light.
    lattr1 = core.LightAttrib.make()
    lattr1 = lattr1.add_on_light(spot)
    lattr1 = lattr1.add_on_light(point)

    lattr2 = core.LightAttrib.make()
    lattr2 = lattr2.add_off_light(ambient)
    lattr2 = lattr2.add_off_light(point)

    lattr3 = lattr1.compose(lattr2)
    assert lattr3.get_num_on_lights() == 1

    assert spot in lattr3.on_lights
    assert point not in lattr3.on_lights
    assert ambient not in lattr3.on_lights


def test_lightattrib_compose_both():
    # Tests a case in which a child node both enables and disables a light.
    lattr1 = core.LightAttrib.make()
    lattr1 = lattr1.add_on_light(spot)
    lattr1 = lattr1.add_on_light(point)

    lattr2 = core.LightAttrib.make()
    lattr2 = lattr2.add_on_light(ambient)
    lattr2 = lattr2.add_on_light(spot)
    lattr2 = lattr2.add_off_light(point)

    lattr3 = lattr1.compose(lattr2)
    assert lattr3.get_num_on_lights() == 2

    assert spot in lattr3.on_lights
    assert point not in lattr3.on_lights
    assert ambient in lattr3.on_lights


def test_lightattrib_compose_alloff():
    # Tests a case in which a child node disables all lights.
    lattr1 = core.LightAttrib.make()
    lattr1 = lattr1.add_on_light(spot)
    lattr1 = lattr1.add_on_light(point)
    assert lattr1.get_num_on_lights() == 2

    lattr2 = core.LightAttrib.make_all_off()
    assert lattr2.has_all_off()

    lattr3 = lattr1.compose(lattr2)
    assert lattr3.get_num_on_lights() == 0
    assert lattr3.get_num_off_lights() == 0
    assert lattr3.has_all_off()


def test_lightattrib_compare():
    lattr1 = core.LightAttrib.make()
    lattr2 = core.LightAttrib.make()
    assert lattr1.compare_to(lattr2) == 0

    # All-off should not compare equal to empty
    lattr2 = core.LightAttrib.make_all_off()
    assert lattr1.compare_to(lattr2) != 0
    assert lattr2.compare_to(lattr1) != 0
    assert lattr2.compare_to(lattr1) == -lattr1.compare_to(lattr2)

    # If both have the same light, they are equal
    lattr1 = core.LightAttrib.make()
    lattr1 = lattr1.add_on_light(spot)
    lattr2 = core.LightAttrib.make()
    lattr2 = lattr2.add_on_light(spot)
    assert lattr1.compare_to(lattr2) == 0
    assert lattr2.compare_to(lattr1) == 0

    # Adding an extra light makes it unequal
    lattr2 = lattr2.add_on_light(point)
    assert lattr1.compare_to(lattr2) != 0
    assert lattr2.compare_to(lattr1) != 0
    assert lattr2.compare_to(lattr1) == -lattr1.compare_to(lattr2)

    # Different lights altogether is of course unequal
    lattr1 = core.LightAttrib.make()
    lattr1 = lattr1.add_on_light(point)
    lattr2 = core.LightAttrib.make()
    lattr2 = lattr2.add_on_light(spot)
    assert lattr1.compare_to(lattr2) != 0
    assert lattr2.compare_to(lattr1) != 0
    assert lattr2.compare_to(lattr1) == -lattr1.compare_to(lattr2)
