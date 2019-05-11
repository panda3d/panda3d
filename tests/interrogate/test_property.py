import sys
import pytest
from panda3d import core
from contextlib import contextmanager

if sys.version_info >= (3, 3):
    import collections.abc as collections_abc
else:
    import _abcoll as collections_abc


@contextmanager
def constant_refcount(var):
    """ with block that checks that the Python refcount remains the same. """
    rc = sys.getrefcount(var)
    yield
    assert sys.getrefcount(var) == rc


def test_property():
    # This is a property defined by MAKE_PROPERTY.
    np = core.PandaNode("")

    # Getter
    transform = np.get_transform()
    assert transform == np.transform

    # Setter
    new_transform = transform.set_pos((1, 0, 0))
    np.transform = new_transform
    assert np.transform == new_transform

    # Invalid assignments
    with pytest.raises(TypeError):
        np.transform = None
    with pytest.raises(TypeError):
        np.transform = "nonsense"
    with pytest.raises(TypeError):
        del np.transform


def test_property2():
    # This is a property defined by MAKE_PROPERTY2, that can be None.
    mat = core.Material()

    mat.ambient = (1, 0, 0, 1)
    assert mat.ambient == (1, 0, 0, 1)

    mat.ambient = None
    assert mat.ambient is None

    with pytest.raises(TypeError):
        mat.ambient = "nonsense"
    with pytest.raises(TypeError):
        del mat.ambient


# The next tests are for MAKE_SEQ_PROPERTY.
def seq_property(*items):
    """ Returns a sequence property initialized with the given items. """

    # It doesn't matter which property we use; I just happened to pick
    # CollisionNode.solids.
    cn = core.CollisionNode("")
    append = cn.add_solid
    for item in items:
        append(item)
    assert len(cn.solids) == len(items)
    return cn.solids

# Arbitrary items we can use as items for the above seq property.
item_a = core.CollisionSphere((0, 0, 0), 1)
item_b = core.CollisionSphere((0, 0, 0), 2)
item_c = core.CollisionSphere((0, 0, 0), 3)


def test_seq_property_abc():
    prop = seq_property()
    assert isinstance(prop, collections_abc.Container)
    assert isinstance(prop, collections_abc.Sized)
    assert isinstance(prop, collections_abc.Iterable)
    assert isinstance(prop, collections_abc.MutableSequence)
    assert isinstance(prop, collections_abc.Sequence)


def test_seq_property_empty():
    prop = seq_property()
    assert not prop
    assert len(prop) == 0

    with pytest.raises(IndexError):
        prop[0]
    with pytest.raises(IndexError):
        prop[-1]


def test_seq_property_iter():
    prop = seq_property(item_a, item_b, item_b)
    assert prop
    assert len(prop) == 3

    assert tuple(prop) == (item_a, item_b, item_b)
    assert item_a in prop
    assert item_c not in prop
    assert None not in prop


def test_seq_property_reversed():
    prop = seq_property(item_a, item_b, item_b)
    assert tuple(reversed(prop)) == tuple(reversed(tuple(prop)))


def test_seq_property_getitem():
    prop = seq_property(item_a, item_b, item_b)

    assert prop[0] == item_a
    assert prop[1] == item_b
    assert prop[2] == item_b

    # Reverse index
    assert prop[-1] == item_b
    assert prop[-2] == item_b
    assert prop[-3] == item_a

    # Long index
    if sys.version_info[0] < 3:
        assert prop[long(1)] == item_b
        assert prop[long(-1)] == item_b

    # Out of bounds access
    with pytest.raises(IndexError):
        prop[-4]
    with pytest.raises(IndexError):
        prop[3]
    with pytest.raises(IndexError):
        prop[2**63]

    # Invalid index
    with pytest.raises(TypeError):
        prop[None]
    with pytest.raises(TypeError):
        prop[item_a]
    with pytest.raises(TypeError):
        prop["nonsense"]

    # Reference count check
    i = 1
    with constant_refcount(i):
        prop[i]

    # Make sure it preserves refcount of invalid indices
    i = "nonsense"
    with constant_refcount(i):
        try:
            prop[i]
        except TypeError:
            pass


def test_seq_property_setitem():
    prop = seq_property(item_c, item_c, item_c)

    prop[0] = item_a
    prop[1] = item_b
    assert tuple(prop) == (item_a, item_b, item_c)

    # Refcount of key and value stays the same?
    i = 0
    with constant_refcount(i):
        with constant_refcount(item_a):
            prop[0] = item_a

    # Reverse index
    prop[-1] = item_a
    prop[-2] = item_b
    prop[-3] = item_c
    assert tuple(prop) == (item_c, item_b, item_a)

    # Long index
    if sys.version_info[0] < 3:
        prop[long(1)] = item_b
        assert prop[1] == item_b
        prop[long(-1)] = item_b
        assert prop[-1] == item_b

    # Out of bounds access
    with pytest.raises(IndexError):
        prop[-4] = item_c
    with pytest.raises(IndexError):
        prop[3] = item_c
    with pytest.raises(IndexError):
        prop[2**63] = item_c

    # Invalid index
    with pytest.raises(TypeError):
        prop[None] = item_c
    with pytest.raises(TypeError):
        prop[item_a] = item_c
    with pytest.raises(TypeError):
        prop["nonsense"] = item_c

    # Invalid type
    with pytest.raises(TypeError):
        prop[0] = None
    with pytest.raises(TypeError):
        prop[0] = "nonsense"


def test_seq_property_delitem():
    prop = seq_property(item_a, item_b, item_c)

    # Out of bounds
    with pytest.raises(IndexError):
        prop[3]
    with pytest.raises(IndexError):
        prop[-4]

    # Positive index
    del prop[0]
    assert tuple(prop) == (item_b, item_c)

    # Negative index
    del prop[-2]
    assert tuple(prop) == (item_c,)

    # Invalid index
    with pytest.raises(TypeError):
        del prop[None]


def test_seq_property_index():
    prop = seq_property(item_a, item_b, item_b)

    assert prop.index(item_a) == 0
    assert prop.index(item_b) == 1

    with pytest.raises(ValueError):
        prop.index(item_c)
    with pytest.raises(ValueError):
        prop.index(None)
    with pytest.raises(ValueError):
        prop.index("nonsense")

    # Refcount is properly decreased
    with constant_refcount(item_b):
        prop.index(item_b)
    with constant_refcount(item_c):
        try:
            prop.index(item_c)
        except ValueError:
            pass

    nonsense = "nonsense"
    with constant_refcount(nonsense):
        try:
            prop.index(nonsense)
        except ValueError:
            pass


def test_seq_property_count():
    prop = seq_property(item_a, item_b, item_b)

    prop.count(item_a) == 1
    prop.count(item_b) == 2
    prop.count(item_c) == 0
    prop.count(None) == 0
    prop.count(("nonsense", -3.5)) == 0

    # Refcount does not leak
    with constant_refcount(item_b):
        prop.count(item_b)

    nonsense = "nonsense"
    with constant_refcount(nonsense):
        prop.count(nonsense)


def test_seq_property_clear():
    prop = seq_property(item_a, item_b, item_b)
    prop.clear()

    assert not prop
    assert len(prop) == 0
    assert tuple(prop) == ()


def test_seq_property_pop():
    prop = seq_property(item_a, item_b, item_c, item_b)

    # Test out of bounds pop
    with pytest.raises(IndexError):
        prop.pop(4)
    with pytest.raises(IndexError):
        prop.pop(-5)

    assert prop.pop(1) == item_b
    assert prop.pop(-1) == item_b
    assert prop.pop() == item_c
    assert prop.pop(0) == item_a

    # Wrong args
    with pytest.raises(TypeError):
        prop.pop(0, 0)

    # Pop on empty sequence
    with pytest.raises(IndexError):
        prop.pop()
    with pytest.raises(IndexError):
        prop.pop(0)
    with pytest.raises(IndexError):
        prop.pop(-1)


def test_seq_property_remove():
    prop = seq_property(item_a, item_b, item_c, item_b)

    with constant_refcount(item_b):
        prop.remove(item_b)

    assert tuple(prop) == (item_a, item_c, item_b)

    prop.remove(item_b)
    assert tuple(prop) == (item_a, item_c)

    with pytest.raises(ValueError):
        prop.remove(item_b)
    with pytest.raises(ValueError):
        prop.remove(None)
    with pytest.raises(ValueError):
        prop.remove("nonsense")


def test_seq_property_append():
    prop = seq_property(item_a, item_b)

    with constant_refcount(item_c):
        prop.append(item_c)

    assert tuple(prop) == (item_a, item_b, item_c)

    with pytest.raises(TypeError):
        prop.append(None)
    with pytest.raises(TypeError):
        prop.append("nonsense")


def test_seq_property_insert():
    # Adding at the beginning
    prop = seq_property(item_a, item_a, item_a)
    with constant_refcount(item_b):
        prop.insert(0, item_b)

    assert tuple(prop) == (item_b, item_a, item_a, item_a)

    # Adding in the middle
    prop = seq_property(item_a, item_a, item_a)
    with constant_refcount(item_b):
        prop.insert(2, item_b)

    assert tuple(prop) == (item_a, item_a, item_b, item_a)

    # Adding at the end
    prop = seq_property(item_a, item_a, item_a)
    with constant_refcount(item_b):
        prop.insert(len(prop), item_b)

    assert tuple(prop) == (item_a, item_a, item_a, item_b)

    # Adding with negative index
    prop = seq_property(item_a, item_a, item_a)
    with constant_refcount(item_b):
        prop.insert(-2, item_b)

    assert tuple(prop) == (item_a, item_b, item_a, item_a)

    # Adding at the end with overflowing index
    prop = seq_property(item_a, item_a, item_a)
    with constant_refcount(item_b):
        prop.insert(2345, item_b)

    assert tuple(prop) == (item_a, item_a, item_a, item_b)

    # Adding at the beginning with negative overflowing index
    prop = seq_property(item_a, item_a, item_a)
    with constant_refcount(item_b):
        prop.insert(-2345, item_b)

    assert tuple(prop) == (item_b, item_a, item_a, item_a)


def test_seq_property_extend():
    prop = seq_property(item_a)

    with constant_refcount(item_b):
        prop.extend((item_b, item_c))

    assert tuple(prop) == (item_a, item_b, item_c)

    with pytest.raises(TypeError):
        prop.extend(None)
    with pytest.raises(TypeError):
        prop.extend("nonsense")
    with pytest.raises(TypeError):
        prop.extend(item_a)
    with pytest.raises(TypeError):
        prop.extend(item_a, item_b)
    with pytest.raises(TypeError):
        prop.extend()
    with pytest.raises(TypeError):
        prop.extend((item_a, None))
    with pytest.raises(TypeError):
        prop.extend(["nonsense"])


# The next tests are for MAKE_MAP_PROPERTY.
def map_property(**items):
    """ Returns a mapping property initialized with the given values. """

    # It doesn't matter which property we use; I just happened to pick
    # NodePath.tags.
    np = core.NodePath("")
    for k, v in items.items():
        np.set_tag(k, v)
    return np.tags


def test_map_property_abc():
    prop = map_property()
    assert isinstance(prop, collections_abc.Container)
    assert isinstance(prop, collections_abc.Sized)
    assert isinstance(prop, collections_abc.Iterable)
    assert isinstance(prop, collections_abc.MutableMapping)
    assert isinstance(prop, collections_abc.Mapping)


def test_map_property_empty():
    prop = map_property()
    assert not prop
    assert len(prop) == 0

    with pytest.raises(KeyError):
        prop.popitem()

    with pytest.raises(KeyError):
        prop['nonsense']


def test_map_property_getitem():
    key = 'key'
    value = 'value'
    prop = map_property(**{key: value})

    with constant_refcount(key):
        with constant_refcount(value):
            assert prop[key] == value

    with pytest.raises(KeyError):
        prop['nonsense']
    with pytest.raises(TypeError):
        prop[None]


def test_map_property_setitem():
    key = 'key'
    value = 'value'
    prop = map_property()

    # Setting new key
    with constant_refcount(key):
        with constant_refcount(value):
            prop[key] = value

    assert prop[key] == value

    # Setting existing key
    with constant_refcount(key):
        with constant_refcount(value):
            prop[key] = value

    assert prop[key] == value

    # Unknown key
    with pytest.raises(TypeError):
        prop[None] = value

    # Unknown value
    with pytest.raises(TypeError):
        prop[key] = None


def test_map_property_delitem():
    key = 'key'
    value = 'value'
    prop = map_property(**{key: value})

    with constant_refcount(key):
        with constant_refcount(value):
            del prop[key]

    with pytest.raises(KeyError):
        assert prop[key]

    # Nonexistent key
    with pytest.raises(KeyError):
        del prop['nonsense']

    # Invalid type key
    with pytest.raises(TypeError):
        del prop[None]


def test_map_property_contains():
    prop = map_property(key='value')

    assert 'key' in prop
    assert None not in prop
    assert 'value' not in prop


def test_map_property_get():
    key = 'key'
    value = 'value'
    prop = map_property(**{key: value})

    default = 'default'
    with constant_refcount(key):
        with constant_refcount(default):
            assert prop.get(key) == value

    with constant_refcount(key):
        with constant_refcount(default):
            assert prop.get(key, default) == value

    assert prop.get('unknown') is None

    with constant_refcount(default):
        assert prop.get('unknown', default) == default


def test_map_property_pop():
    key = 'key'
    value = 'value'
    prop = map_property(**{key: value})

    assert prop.pop('nonsense', None) is None
    assert prop.pop('nonsense', 'default') == 'default'

    assert prop.pop('key', 'default') == 'value'
    assert 'key' not in prop


def test_map_property_popitem():
    key = 'key'
    value = 'value'
    prop = map_property(**{key: value})

    assert prop.popitem() == (key, value)

    with pytest.raises(KeyError):
        assert prop.popitem()


def test_map_property_clear():
    prop = map_property(key='value', key2='value2')

    prop.clear()
    assert len(prop) == 0


def test_map_property_setdefault():
    prop = map_property(key='value')

    # Don't change value of key that already exists
    prop.setdefault('key', 'value2')
    assert prop['key'] == 'value'

    # Change values of nonexistent key
    prop.setdefault('key2', 'value2')
    assert prop['key2'] == 'value2'

    # These should error because you can't set None on this property
    with pytest.raises(TypeError):
        prop.setdefault('key3', None)
    with pytest.raises(TypeError):
        prop.setdefault('key3')


def test_map_property_update():
    prop = map_property()

    # Empty update
    prop.update()

    # Passing in dictionary
    prop.update({'key': 'value'})

    # Passing in keywords
    prop.update(key2='value2')

    # Don't pass in both!
    with pytest.raises(TypeError):
        prop.update({}, k='v')

    assert prop['key'] == 'value'
    assert prop['key2'] == 'value2'


def test_map_property_keys():
    prop = map_property(key='value', key2='value2')

    assert isinstance(prop.keys(), collections_abc.MappingView)
    assert frozenset(prop.keys()) == frozenset(('key', 'key2'))


def test_map_property_values():
    prop = map_property(key='value', key2='value2')

    assert isinstance(prop.values(), collections_abc.ValuesView)
    assert frozenset(prop.values()) == frozenset(('value', 'value2'))


def test_map_property_items():
    prop = map_property(key='value', key2='value2')

    assert isinstance(prop.items(), collections_abc.MappingView)
    assert frozenset(prop.items()) == frozenset((('key', 'value'), ('key2', 'value2')))
