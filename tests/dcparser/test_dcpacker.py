import pytest

direct = pytest.importorskip("panda3d.direct")


def test_raw_pack_int8():
    for num in range(-128, 128):
        packer = direct.DCPacker()
        packer.raw_pack_int8(num)
        packer.set_unpack_data(packer.get_bytes())
        assert packer.raw_unpack_int8() == num


def test_raw_pack_uint8():
    for num in range(256):
        packer = direct.DCPacker()
        packer.raw_pack_uint8(num)
        packer.set_unpack_data(packer.get_bytes())
        assert packer.raw_unpack_uint8() == num


def test_raw_pack_int64():
    for num in (0, -1, 0x7fffffff, -0x80000000, 0x7fffffffffffffff, 0x7ffffffffffffffe, -0x8000000000000000, -0x7fffffffffffffff):
        packer = direct.DCPacker()
        packer.raw_pack_int64(num)
        packer.set_unpack_data(packer.get_bytes())
        assert packer.raw_unpack_int64() == num


def test_raw_pack_uint64():
    for num in (0, 1, 0x7fffffff, 0xffffffff, 0x7fffffffffffffff, 0xfffffffffffffffe, 0xffffffffffffffff):
        packer = direct.DCPacker()
        packer.raw_pack_uint64(num)
        packer.set_unpack_data(packer.get_bytes())
        assert packer.raw_unpack_uint64() == num


def test_raw_pack_string():
    for value in ("", "a", "hello", "x" * 1024, "embedded\x00null", "utf8 é中"):
        packer = direct.DCPacker()
        packer.raw_pack_string(value)
        packer.set_unpack_data(packer.get_bytes())
        assert packer.raw_unpack_string() == value


def _field(dc_file, class_name, field_name):
    cls = dc_file.get_class_by_name(class_name)
    assert cls is not None, class_name
    field = cls.get_field_by_name(field_name)
    assert field is not None, field_name
    return field


def test_pack_format_parse_roundtrip_atomic(dc_file):
    # Pack via pack_object, format to text, parse the text back, and verify
    # the bytes match.  Exercises both unpack_and_format() and the
    # parse_and_pack(string) overload that now moves its source by value.
    field = _field(dc_file, "Avatar", "setPos")
    packer = direct.DCPacker()
    packer.begin_pack(field)
    packer.pack_object((1, -2, 3))
    assert packer.end_pack()
    data = packer.get_bytes()

    formatted = field.format_data(data, False)
    again = field.parse_string(formatted)
    assert bytes(again) == bytes(data)


def test_pack_format_parse_roundtrip_string(dc_file):
    field = _field(dc_file, "Avatar", "setName")
    packer = direct.DCPacker()
    packer.begin_pack(field)
    packer.pack_object(("hello world",))
    assert packer.end_pack()
    data = packer.get_bytes()

    formatted = field.format_data(data, False)
    again = field.parse_string(formatted)
    assert bytes(again) == bytes(data)


def test_pack_object_roundtrip_atomic(dc_file):
    field = _field(dc_file, "Inventory", "setColor")
    packer = direct.DCPacker()
    packer.begin_pack(field)
    packer.pack_object((10, 20, 30))
    assert packer.end_pack()
    data = packer.get_bytes()

    packer = direct.DCPacker()
    packer.set_unpack_data(data)
    packer.begin_unpack(field)
    value = packer.unpack_object()
    assert packer.end_unpack()
    assert tuple(value) == (10, 20, 30)


def test_pack_object_roundtrip_blob(dc_file):
    field = _field(dc_file, "Inventory", "setBlob")
    payload = bytes(range(256)) + b"\x00\x01\x00\x02"
    packer = direct.DCPacker()
    packer.begin_pack(field)
    packer.pack_object((payload,))
    assert packer.end_pack()
    data = packer.get_bytes()

    packer = direct.DCPacker()
    packer.set_unpack_data(data)
    packer.begin_unpack(field)
    value = packer.unpack_object()
    assert packer.end_unpack()
    # unpack_object returns a tuple-ish container for atomic fields.
    assert bytes(value[0]) == payload


def test_pack_object_roundtrip_nested_struct_array(dc_file):
    field = _field(dc_file, "Inventory", "setItems")
    items = [
        (1, "sword", 1),
        (2, "potion", 5),
        (3, "", 0),
    ]
    packer = direct.DCPacker()
    packer.begin_pack(field)
    packer.pack_object((items,))
    assert packer.end_pack()
    data = packer.get_bytes()

    packer = direct.DCPacker()
    packer.set_unpack_data(data)
    packer.begin_unpack(field)
    unpacked = packer.unpack_object()
    assert packer.end_unpack()
    out = [tuple(item) for item in unpacked[0]]
    assert out == items


def test_pack_range_error(dc_file):
    # setRanged restricts int16 to (-100, 100); an out-of-range value must
    # be flagged as an error rather than silently truncated.
    field = _field(dc_file, "Inventory", "setRanged")
    packer = direct.DCPacker()
    packer.begin_pack(field)
    packer.pack_object((9999,))
    # end_pack() returns False once the packer is in the error state, so we
    # don't assert on it here.  had_error() is the signal we care about.
    packer.end_pack()
    assert packer.had_error()


def test_pack_seek_by_name_repack(dc_file):
    # setItem(Item item) gives us a packable field whose nested struct fields
    # (itemId, label, quantity) can be reached by name via DCPacker::seek().
    field = _field(dc_file, "Inventory", "setItem")

    # Pack an initial Item value.
    packer = direct.DCPacker()
    packer.begin_pack(field)
    packer.pack_object(((42, "potion", 5),))
    assert packer.end_pack()
    data = packer.get_bytes()

    # Re-open in repack mode and seek by (local) name to overwrite just
    # 'quantity'.  The catalog indexes both fully-qualified ("item.quantity")
    # and local ("quantity") names, so either spelling must resolve.
    packer = direct.DCPacker()
    packer.set_unpack_data(data)
    packer.begin_repack(field)
    assert packer.seek("quantity"), "seek by local name failed"
    packer.pack_object(99)
    assert packer.end_repack()
    new_data = packer.get_bytes()

    # Unpack everything and verify only 'quantity' changed.
    packer = direct.DCPacker()
    packer.set_unpack_data(new_data)
    packer.begin_unpack(field)
    unpacked = packer.unpack_object()
    assert packer.end_unpack()
    item = tuple(unpacked[0])
    assert item == (42, "potion", 99)

    # Verify the qualified-name spelling resolves too.  We can't chain two
    # seeks in a single repack cycle (the first seek leaves _current_field
    # set), so run a fresh repack on the original data.
    packer = direct.DCPacker()
    packer.set_unpack_data(data)
    packer.begin_repack(field)
    assert packer.seek("item.quantity"), "seek by qualified name failed"
    packer.pack_object(7)
    assert packer.end_repack()
    qualified_data = packer.get_bytes()

    packer = direct.DCPacker()
    packer.set_unpack_data(qualified_data)
    packer.begin_unpack(field)
    unpacked = packer.unpack_object()
    assert packer.end_unpack()
    assert tuple(unpacked[0]) == (42, "potion", 7)


def test_pack_seek_by_name_missing(dc_file):
    field = _field(dc_file, "Inventory", "setItem")
    packer = direct.DCPacker()
    packer.begin_pack(field)
    packer.pack_object(((1, "x", 1),))
    assert packer.end_pack()
    data = packer.get_bytes()

    packer = direct.DCPacker()
    packer.set_unpack_data(data)
    packer.begin_repack(field)
    # A non-existent name, a substring of a real name, and an empty string
    # should all fail the seek rather than silently land on something.
    assert not packer.seek("nonexistent")
    assert not packer.seek("quant")
    assert not packer.seek("item.quanti")
    assert not packer.seek("")
    # A valid seek after the failed ones must still work--the failures
    # shouldn't have left the packer in a bad state.
    assert packer.seek("quantity")

    # And the qualified spelling also recovers from failed seeks, on a
    # fresh repack cycle (chained seeks in one cycle aren't allowed).
    packer = direct.DCPacker()
    packer.set_unpack_data(data)
    packer.begin_repack(field)
    assert not packer.seek("nonexistent")
    assert packer.seek("item.quantity")
