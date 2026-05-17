import time

from panda3d.core import HTTPDate


def test_http_date_default_invalid():
    d = HTTPDate()
    assert not d.is_valid()


def test_http_date_from_time_t():
    d = HTTPDate(0)
    assert d.is_valid()
    # Unix epoch
    assert d.get_string() == "Thu, 01 Jan 1970 00:00:00 GMT"


def test_http_date_known_epoch():
    # 21 Oct 2015 07:28:00 GMT == 1445412480
    d = HTTPDate(1445412480)
    assert d.is_valid()
    assert d.get_string() == "Wed, 21 Oct 2015 07:28:00 GMT"


def test_http_date_parse_rfc1123():
    d = HTTPDate("Wed, 21 Oct 2015 07:28:00 GMT")
    assert d.is_valid()
    assert d.get_time() == 1445412480


def test_http_date_parse_without_weekday():
    # The weekday is optional in get_token-based parser; the parser figures it out.
    d = HTTPDate("21 Oct 2015 07:28:00 GMT")
    assert d.is_valid()
    assert d.get_time() == 1445412480


def test_http_date_parse_lowercase_components():
    d = HTTPDate("wed, 21 oct 2015 07:28:00 gmt")
    assert d.is_valid()
    assert d.get_time() == 1445412480


def test_http_date_parse_mixed_separators():
    # The parser accepts a range of separators; commas and spaces are skipped.
    d = HTTPDate("Wed 21 Oct 2015 07:28:00 GMT")
    assert d.is_valid()
    assert d.get_time() == 1445412480


def test_http_date_parse_invalid_garbage():
    d = HTTPDate("not a real date")
    assert not d.is_valid()


def test_http_date_parse_missing_fields():
    # No year/hour/minute -> invalid.
    d = HTTPDate("Wed, 21 Oct")
    assert not d.is_valid()


def test_http_date_parse_out_of_range_month():
    # Month name not recognized.
    d = HTTPDate("Wed, 21 Foo 2015 07:28:00 GMT")
    assert not d.is_valid()


def test_http_date_get_string_roundtrip():
    original = HTTPDate(1445412480)
    s = original.get_string()
    reparsed = HTTPDate(s)
    assert reparsed.is_valid()
    assert reparsed.get_time() == original.get_time()
    assert reparsed.get_string() == s


def test_http_date_now_is_close():
    # HTTPDate::now() should be close to current time.
    before = int(time.time())
    d = HTTPDate.now()
    after = int(time.time())
    assert d.is_valid()
    assert before <= d.get_time() <= after


def test_http_date_comparison():
    early = HTTPDate(1000000)
    late = HTTPDate(2000000)
    assert early < late
    assert late > early
    assert early != late

    same = HTTPDate(1000000)
    assert early == same
    assert not (early < same)


def test_http_date_addition_subtraction():
    d = HTTPDate(1000)
    d += 60
    assert d.get_time() == 1060
    d -= 30
    assert d.get_time() == 1030

    assert (d + 10).get_time() == 1040
    assert (d - 10).get_time() == 1020


def test_http_date_difference():
    a = HTTPDate(1000)
    b = HTTPDate(1500)
    assert b - a == 500
