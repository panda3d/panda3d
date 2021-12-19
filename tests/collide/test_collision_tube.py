from panda3d import core


def test_collision_tube_alias():
    assert hasattr(core, 'CollisionCapsule')
    assert hasattr(core, 'CollisionTube')
    assert core.CollisionTube is core.CollisionCapsule


def test_collision_tube_write_old():
    buffer = core.DatagramBuffer()
    writer = core.BamWriter(buffer)
    assert writer.get_file_major_ver() == 6
    writer.set_file_minor_ver(43)

    capsule = core.CollisionCapsule((0, 0, -1), (0, 0, 1), 0.5)
    writer.init()
    writer.write_object(capsule)
    writer.flush()

    data = buffer.data
    assert b'CollisionTube' in data
    assert b'CollisionCapsule' not in data


def test_collision_tube_write_new():
    buffer = core.DatagramBuffer()
    writer = core.BamWriter(buffer)
    assert writer.get_file_major_ver() == 6
    writer.set_file_minor_ver(44)

    capsule = core.CollisionCapsule((0, 0, -1), (0, 0, 1), 0.5)
    writer.init()
    writer.write_object(capsule)
    writer.flush()

    data = buffer.data
    assert b'CollisionTube' not in data
    assert b'CollisionCapsule' in data


def test_collision_tube_read_old():
    # Make sure we can read an older file that contains CollisionTube.
    buffer = core.DatagramBuffer(b'\x06\x00\x00\x00\x06\x00+\x00\x01\x00\xd6\x00\x00\x00\x00j\x01\r\x00CollisionTube\x01h\x01\x0e\x00CollisionSolid\x01B\x00\x11\x00CopyOnWriteObject\x01A\x00!\x00CachedTypedWritableReferenceCount\x01=\x00\x1b\x00TypedWritableReferenceCount\x02<\x00\r\x00TypedWritable\x01\x03\x00\x0b\x00TypedObject\x00\x07\x00\x0e\x00ReferenceCount\x00\x01\x00\x15\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xbf\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80?\x00\x00\x00?\x01\x00\x00\x00\x01')
    reader = core.BamReader(buffer)
    reader.init()
    assert reader.file_version == (6, 43)

    capsule = reader.read_object()
    reader.resolve()
    assert isinstance(capsule, core.CollisionCapsule)


def test_collision_tube_read_new():
    # Make sure we can read a newer file that contains CollisionCapsule.
    buffer = core.DatagramBuffer(b'\x06\x00\x00\x00\x06\x00,\x00\x01\x00\xd9\x00\x00\x00\x00j\x01\x10\x00CollisionCapsule\x01h\x01\x0e\x00CollisionSolid\x01B\x00\x11\x00CopyOnWriteObject\x01A\x00!\x00CachedTypedWritableReferenceCount\x01=\x00\x1b\x00TypedWritableReferenceCount\x02<\x00\r\x00TypedWritable\x01\x03\x00\x0b\x00TypedObject\x00\x07\x00\x0e\x00ReferenceCount\x00\x01\x00\x15\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xbf\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80?\x00\x00\x00?\x01\x00\x00\x00\x01')
    reader = core.BamReader(buffer)
    reader.init()
    assert reader.file_version == (6, 44)

    capsule = reader.read_object()
    reader.resolve()
    assert isinstance(capsule, core.CollisionCapsule)
