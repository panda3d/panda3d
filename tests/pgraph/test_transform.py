from panda3d.core import Transform, Mat4, Mat3, NodePath, TransformState


def test_transform_identity():
    a = NodePath("a")
    a.set_pos(1, 2, 3)
    a.set_hpr(10, 20, 30)
    a.set_scale(40, -3, 3)

    b = NodePath("b")
    b.set_pos(100, 0.1, -5)
    b.set_hpr(45, 90, -180)
    b.set_scale(3, 2, 1)

    print(a.get_mat())
    print(b.get_mat())
    print(b.get_mat() * a.get_mat())

    print(TransformState.make_mat(a.get_mat()).compose(TransformState.make_mat(b.get_mat())).get_mat())

    print(Transform.make_mat(a.get_mat()).compose(Transform.make_mat(b.get_mat())).get_mat())

    print(Transform.make_mat(a.get_mat()).get_inverse().get_mat())

    m = Mat4()
    m.invert_from(a.get_mat())
    print(m)
    print
