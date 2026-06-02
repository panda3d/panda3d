import pytest
from panda3d import core


# HTTPCookie is only available when OpenSSL support is compiled in.
HTTPCookie = getattr(core, "HTTPCookie", None)
URLSpec = core.URLSpec
HTTPDate = core.HTTPDate

pytestmark = pytest.mark.skipif(HTTPCookie is None,
                                reason="Requires OpenSSL")


def _url(s="http://example.com/some/path"):
    return URLSpec(s)


def test_http_cookie_default_construct():
    c = HTTPCookie()
    assert c.get_name() == ""
    assert c.get_value() == ""
    assert c.get_path() == ""
    assert c.get_domain() == ""
    assert not c.get_secure()
    assert c.get_samesite() == HTTPCookie.SS_unspecified


def test_http_cookie_construct_with_name_path_domain():
    c = HTTPCookie("sessionid", "/foo", "example.com")
    assert c.get_name() == "sessionid"
    assert c.get_path() == "/foo"
    assert c.get_domain() == "example.com"


def test_http_cookie_parse_simple_name_value():
    c = HTTPCookie("k=v", _url())
    assert c.get_name() == "k"
    assert c.get_value() == "v"
    # The path defaults to the URL path.
    assert c.get_path() == "/some/path"
    # The domain defaults to the URL server.
    assert c.get_domain() == "example.com"


def test_http_cookie_parse_empty_value():
    c = HTTPCookie("k=", _url())
    assert c.get_name() == "k"
    assert c.get_value() == ""


def test_http_cookie_parse_no_equals_in_first_param():
    # A first parameter without '=' is treated as the name with empty value.
    c = HTTPCookie("flag", _url())
    assert c.get_name() == "flag"
    assert c.get_value() == ""


def test_http_cookie_parse_path_attribute():
    c = HTTPCookie("k=v; path=/api", _url())
    assert c.get_path() == "/api"


def test_http_cookie_parse_domain_attribute_adds_leading_dot():
    # RFC 2965: if domain doesn't start with a dot, the user agent adds one.
    c = HTTPCookie("k=v; domain=example.com", _url())
    assert c.get_domain() == ".example.com"


def test_http_cookie_parse_domain_attribute_preserves_leading_dot():
    c = HTTPCookie("k=v; domain=.example.com", _url())
    assert c.get_domain() == ".example.com"


def test_http_cookie_parse_domain_lowercased():
    c = HTTPCookie("k=v; domain=Example.COM", _url())
    assert c.get_domain() == ".example.com"


def test_http_cookie_parse_secure_flag():
    c = HTTPCookie("k=v; secure", _url())
    assert c.get_secure()


def test_http_cookie_parse_without_secure_flag():
    c = HTTPCookie("k=v", _url())
    assert not c.get_secure()


def test_http_cookie_parse_samesite_lax():
    c = HTTPCookie("k=v; samesite=Lax", _url())
    assert c.get_samesite() == HTTPCookie.SS_lax


def test_http_cookie_parse_samesite_strict():
    c = HTTPCookie("k=v; samesite=Strict", _url())
    assert c.get_samesite() == HTTPCookie.SS_strict


def test_http_cookie_parse_samesite_none():
    c = HTTPCookie("k=v; samesite=None", _url())
    assert c.get_samesite() == HTTPCookie.SS_none


def test_http_cookie_parse_samesite_unknown_kept_unspecified():
    c = HTTPCookie("k=v; samesite=Bogus", _url())
    assert c.get_samesite() == HTTPCookie.SS_unspecified


def test_http_cookie_parse_expires():
    c = HTTPCookie("k=v; expires=Wed, 21 Oct 2015 07:28:00 GMT", _url())
    # Note: the embedded comma after "Wed" is itself a token separator in
    # parse_set_cookie, so we don't worry here whether the weekday was
    # consumed -- the resulting date must just be the one we wrote.
    assert c.has_expires()
    assert c.get_expires().get_time() == 1445412480


def test_http_cookie_parse_all_attributes_together():
    raw = ("sid=abc123; path=/api; domain=example.com; "
           "secure; samesite=Strict")
    c = HTTPCookie(raw, _url())
    assert c.get_name() == "sid"
    assert c.get_value() == "abc123"
    assert c.get_path() == "/api"
    assert c.get_domain() == ".example.com"
    assert c.get_secure()
    assert c.get_samesite() == HTTPCookie.SS_strict


def test_http_cookie_parse_leading_whitespace_between_params():
    c = HTTPCookie("k=v;  path=/api ;  secure", _url())
    assert c.get_name() == "k"
    assert c.get_path() == "/api "  # Trailing space inside value is preserved
    assert c.get_secure()


def test_http_cookie_setters_update_fields():
    c = HTTPCookie()
    c.set_name("foo")
    c.set_value("bar")
    c.set_path("/abc")
    c.set_domain(".example.com")
    c.set_secure(True)
    c.set_samesite(HTTPCookie.SS_lax)
    assert c.get_name() == "foo"
    assert c.get_value() == "bar"
    assert c.get_path() == "/abc"
    assert c.get_domain() == ".example.com"
    assert c.get_secure()
    assert c.get_samesite() == HTTPCookie.SS_lax


def test_http_cookie_matches_url_same_domain():
    c = HTTPCookie("k=v; domain=example.com", _url("http://example.com/"))
    assert c.matches_url(URLSpec("http://example.com/anything"))


def test_http_cookie_matches_url_path_prefix():
    c = HTTPCookie("k=v; path=/api", _url("http://example.com/"))
    assert c.matches_url(URLSpec("http://example.com/api/v1/items"))
    assert not c.matches_url(URLSpec("http://example.com/other"))


def test_http_cookie_matches_url_secure_requires_https():
    c = HTTPCookie("k=v; secure", _url("https://example.com/"))
    assert c.matches_url(URLSpec("https://example.com/anything"))
    assert not c.matches_url(URLSpec("http://example.com/anything"))


def test_http_cookie_is_expired():
    c = HTTPCookie("k=v; expires=Wed, 21 Oct 2015 07:28:00 GMT", _url())
    # 2015 is comfortably in the past.
    assert c.is_expired()


def test_http_cookie_not_expired_when_no_expiry():
    c = HTTPCookie("k=v", _url())
    # is_expired() is only true when an expiry is set AND past.
    assert not c.is_expired()


def test_http_cookie_ordering_by_domain_then_path():
    a = HTTPCookie("k=v; domain=a.example.com; path=/", _url())
    b = HTTPCookie("k=v; domain=b.example.com; path=/", _url())
    assert a < b

    # Same domain: paths are sorted in reverse lexicographic order, so a more
    # specific (longer) sub-path of a common prefix sorts before its parent.
    longer = HTTPCookie("k=v; domain=x.example.com; path=/api/v1/items", _url())
    shorter = HTTPCookie("k=v; domain=x.example.com; path=/api", _url())
    assert longer < shorter
