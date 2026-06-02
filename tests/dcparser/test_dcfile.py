import pytest

direct = pytest.importorskip("panda3d.direct")


def test_dcfile_get_class_by_name(dc_file):
    avatar = dc_file.get_class_by_name("Avatar")
    inventory = dc_file.get_class_by_name("Inventory")
    item = dc_file.get_class_by_name("Item")
    assert avatar is not None and avatar.get_name() == "Avatar"
    assert inventory is not None and inventory.get_name() == "Inventory"
    assert item is not None and item.get_name() == "Item"

    assert dc_file.get_class_by_name("Nonexistent") is None
    assert dc_file.get_class_by_name("") is None

    assert dc_file.get_class_by_name("Avata") is None
    assert dc_file.get_class_by_name("AvatarX") is None
    assert dc_file.get_class_by_name("Inv") is None
    assert dc_file.get_class_by_name("Inventory ") is None


def test_dcfile_get_field_by_name(dc_file):
    avatar = dc_file.get_class_by_name("Avatar")
    field = avatar.get_field_by_name("setName")
    assert field is not None
    assert field.get_name() == "setName"

    field = avatar.get_field_by_name("setHealth")
    assert field is not None and field.get_name() == "setHealth"

    # Inventory inherits from Avatar; setName lives on the parent but
    # get_field_by_name() should still find it.
    inventory = dc_file.get_class_by_name("Inventory")
    field = inventory.get_field_by_name("setName")
    assert field is not None
    assert field.get_name() == "setName"

    # And the child's own fields.
    field = inventory.get_field_by_name("setItems")
    assert field is not None and field.get_name() == "setItems"

    assert avatar.get_field_by_name("noSuchField") is None
    assert avatar.get_field_by_name("") is None

    assert avatar.get_field_by_name("set") is None
    assert avatar.get_field_by_name("setNam") is None
    assert avatar.get_field_by_name("setNameX") is None
    assert avatar.get_field_by_name("etName") is None


def test_dcfile_get_field_index_roundtrip(dc_file):
    # A field's index returned by get_field_by_name() should resolve back
    # to the same field via get_field_by_index().
    inventory = dc_file.get_class_by_name("Inventory")
    for name in ("setItems", "setColor", "setBlob", "setName", "setPos"):
        field = inventory.get_field_by_name(name)
        assert field is not None, name
        same = dc_file.get_field_by_index(field.get_number())
        assert same is not None
        assert same.get_name() == name
