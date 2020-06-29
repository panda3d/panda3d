from panda3d.core import CullFaceAttrib


def test_cullfaceattrib_compare():
    clockwise1 = CullFaceAttrib.make()
    clockwise2 = CullFaceAttrib.make()
    reverse1 = CullFaceAttrib.make_reverse()
    reverse2 = CullFaceAttrib.make_reverse()

    assert clockwise1.compare_to(clockwise2) == 0
    assert clockwise2.compare_to(clockwise1) == 0

    assert reverse1.compare_to(reverse2) == 0
    assert reverse2.compare_to(reverse1) == 0

    assert reverse1.compare_to(clockwise1) != 0
    assert clockwise1.compare_to(reverse1) != 0
    assert reverse1.compare_to(clockwise1) == -clockwise1.compare_to(reverse1)
