from panda3d.core import HTTPEntityTag


def test_http_entity_tag_default():
    tag = HTTPEntityTag()
    assert tag.get_tag() == ""
    assert not tag.is_weak()


def test_http_entity_tag_explicit_strong():
    tag = HTTPEntityTag(False, "abc")
    assert tag.get_tag() == "abc"
    assert not tag.is_weak()


def test_http_entity_tag_explicit_weak():
    tag = HTTPEntityTag(True, "abc")
    assert tag.get_tag() == "abc"
    assert tag.is_weak()


def test_http_entity_tag_parse_unquoted():
    tag = HTTPEntityTag("abc")
    assert tag.get_tag() == "abc"
    assert not tag.is_weak()


def test_http_entity_tag_parse_quoted():
    tag = HTTPEntityTag('"abc"')
    assert tag.get_tag() == "abc"
    assert not tag.is_weak()


def test_http_entity_tag_parse_weak():
    tag = HTTPEntityTag('W/"abc"')
    assert tag.get_tag() == "abc"
    assert tag.is_weak()


def test_http_entity_tag_parse_weak_lowercase():
    # The original parser accepts a lowercase "w/" prefix as well.
    tag = HTTPEntityTag('w/"abc"')
    assert tag.get_tag() == "abc"
    assert tag.is_weak()


def test_http_entity_tag_parse_empty():
    tag = HTTPEntityTag("")
    assert tag.get_tag() == ""
    assert not tag.is_weak()


def test_http_entity_tag_parse_empty_quotes():
    tag = HTTPEntityTag('""')
    assert tag.get_tag() == ""
    assert not tag.is_weak()


def test_http_entity_tag_parse_backslash_escape():
    # Inside a quoted string, a backslash escapes the next character.
    tag = HTTPEntityTag(r'"a\"b"')
    assert tag.get_tag() == 'a"b'


def test_http_entity_tag_get_string_roundtrip_strong():
    tag = HTTPEntityTag(False, "abc")
    s = tag.get_string()
    assert s == '"abc"'

    parsed = HTTPEntityTag(s)
    assert parsed.get_tag() == "abc"
    assert not parsed.is_weak()


def test_http_entity_tag_get_string_roundtrip_weak():
    tag = HTTPEntityTag(True, "abc")
    s = tag.get_string()
    assert s == 'W/"abc"'

    parsed = HTTPEntityTag(s)
    assert parsed.get_tag() == "abc"
    assert parsed.is_weak()


def test_http_entity_tag_get_string_quotes_special_chars():
    tag = HTTPEntityTag(False, 'a"b\\c')
    s = tag.get_string()
    # Double-quotes and backslashes inside the tag must be backslash-escaped.
    assert s == r'"a\"b\\c"'

    parsed = HTTPEntityTag(s)
    assert parsed.get_tag() == 'a"b\\c'


def test_http_entity_tag_equality_strong_strong():
    a = HTTPEntityTag(False, "abc")
    b = HTTPEntityTag(False, "abc")
    assert a == b
    assert a.strong_equiv(b)
    assert a.weak_equiv(b)


def test_http_entity_tag_equality_different_tag():
    a = HTTPEntityTag(False, "abc")
    b = HTTPEntityTag(False, "xyz")
    assert a != b
    assert not a.strong_equiv(b)
    assert not a.weak_equiv(b)


def test_http_entity_tag_equality_weak_vs_strong():
    strong = HTTPEntityTag(False, "abc")
    weak = HTTPEntityTag(True, "abc")
    assert strong != weak
    # Strong equivalence requires both to be strong.
    assert not strong.strong_equiv(weak)
    # Weak equivalence allows weakness on either side.
    assert strong.weak_equiv(weak)


def test_http_entity_tag_ordering():
    weak_a = HTTPEntityTag(True, "abc")
    strong_a = HTTPEntityTag(False, "abc")
    strong_b = HTTPEntityTag(False, "xyz")

    # Strong sorts before weak when the tags differ in weakness.
    assert strong_a < weak_a
    # Same weakness orders by tag string.
    assert strong_a < strong_b
