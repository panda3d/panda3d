from panda3d.core import TypeRegistry
import pytest


def test_get_type_name():
    # We test the get_type_name macro by checking for the existence of a type
    # that has been registered using it, and that it has the name we expect
    # (without weird extra mangling characters)

    # The type we're testing is registered by EggNode.
    pytest.importorskip("panda3d.egg")

    registry = TypeRegistry.ptr()

    found = False
    for type in registry.typehandles:
        if type.name.startswith('RefCountObj<') and 'LMatrix4d' in type.name:
            found = True
            break

    # This is an assert, not a skip, so that we are sure to change the example
    # if it ever becomes invalid
    assert found

    assert type.name == 'RefCountObj<LMatrix4d>'
    assert any(parent.name == 'LMatrix4d' for parent in type.parent_classes)
