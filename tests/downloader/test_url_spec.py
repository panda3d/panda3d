from panda3d.core import URLSpec, Filename


def test_urlspec_construct_empty():
    u = URLSpec()
    assert u.empty()
    assert not u
    assert u.get_url() == ""
    assert u.length() == 0
    assert len(u) == 0

    u = URLSpec("")
    assert u.empty()
    assert not u
    assert u.get_url() == ""
    assert u.length() == 0
    assert len(u) == 0


def test_urlspec_construct_url():
    u = URLSpec("http://example.com/path?q=1")
    assert not u.empty()
    assert bool(u)
    assert u.get_url() == "http://example.com/path?q=1"
    assert u.length() == len("http://example.com/path?q=1")


def test_urlspec_construct_server_name():
    # Without the hint a bare hostname is parsed as a path.
    plain = URLSpec("example.com")
    assert plain.get_path() == "example.com"
    assert not plain.has_server()

    # With the hint we get a server (and a leading "//" inserted).
    with_hint = URLSpec("example.com", True)
    assert with_hint.has_server()
    assert with_hint.get_server() == "example.com"
    assert with_hint.get_url().startswith("//example.com")


def test_urlspec_construct_append_path():
    base = URLSpec("http://example.com/a/")
    u = URLSpec(base, Filename("b/c"))
    assert u.get_path() == "/a/b/c"

    # Both ends have a slash: one is dropped.
    base = URLSpec("http://example.com/a/")
    u = URLSpec(base, Filename("/b"))
    assert u.get_path() == "/a/b"

    # Neither has a slash: one is inserted.
    base = URLSpec("http://example.com/a")
    u = URLSpec(base, Filename("b"))
    assert u.get_path() == "/a/b"

    # Empty path is noop.
    base = URLSpec("http://example.com/a/")
    u = URLSpec(base, Filename(""))
    assert u.get_path() == "/a/"


def test_urlspec_parse():
    u = URLSpec("http://user@example.com:8080/some/path?q=1&z=2")
    assert u.has_scheme()
    assert u.has_authority()
    assert u.has_username()
    assert u.has_server()
    assert u.has_port()
    assert u.has_path()
    assert u.has_query()
    assert u.get_scheme() == "http"
    assert u.get_authority() == "user@example.com:8080"
    assert u.get_username() == "user"
    assert u.get_server() == "example.com"
    assert u.get_server_and_port() == "example.com:8080"
    assert u.get_port() == 8080
    assert u.get_port_str() == "8080"
    assert u.get_path() == "/some/path"
    assert u.get_query() == "q=1&z=2"

    u = URLSpec("ftp://")
    assert u.has_scheme()
    assert u.get_scheme() == "ftp"
    assert u.has_authority()
    # No actual server, but the authority slot still exists (empty).
    assert u.get_server() == ""

    # Scheme is lowercased
    u = URLSpec("HTTP://example.com/")
    assert u.get_scheme() == "http"

    # Server is lowercased
    u = URLSpec("http://Example.COM/")
    assert u.get_server() == "example.com"
    assert u.get_server_and_port() == "example.com"

    # Trailing dot is stripped from server
    u = URLSpec("http://example.com./path")
    assert u.get_server() == "example.com"
    assert u.get_server_and_port() == "example.com"
    assert u.get_path() == "/path"

    # No leading slashes and no scheme: treated as a bare path.
    u = URLSpec("foo/bar")
    assert not u.has_authority()
    assert u.has_path()
    assert u.get_path() == "foo/bar"

    u = URLSpec("/just/a/path")
    assert not u.has_scheme()
    assert not u.has_authority()
    assert u.get_path() == "/just/a/path"

    # Query-only
    u = URLSpec("?q=1")
    assert not u.has_path()
    assert u.has_query()
    assert u.get_query() == "q=1"
    # Path defaults to "/" even when not explicitly present.
    assert u.get_path() == "/"
    assert u.get_path_and_query() == "/?q=1"

    # Converts backslashes to forward slashes
    u = URLSpec("http:\\\\example.com\\foo")
    assert u.get_server() == "example.com"
    assert u.get_path() == "/foo"

    u = URLSpec("http://example.com/foo?a\\b")
    assert u.get_path() == "/foo"
    # Backslashes after '?' are not normalized.
    assert u.get_query() == "a\\b"

    # %% is a literal escaped %, not a hex sequence.
    u = URLSpec("http://example.com/a%%b")
    assert u.get_path() == "/a%%b"

    # Trims whitespace
    u = URLSpec("   http://example.com/   ")
    assert u.get_url() == "http://example.com/"


def test_urlspec_parse_ipv6():
    u = URLSpec("http://[::1]:8080/path")
    assert u.get_server() == "::1"
    assert u.get_port() == 8080
    assert u.get_path() == "/path"

    u = URLSpec("http://[2001:db8::1]/")
    assert u.get_server() == "2001:db8::1"
    assert not u.has_port()

    u = URLSpec("http://[::1]:8080/")
    assert u.get_server_and_port() == "[::1]:8080"

    # The implementation only appends ":port" when has_port() is true
    # (i.e. the port was explicit in the URL), independent of IPv6 brackets.
    u = URLSpec("http://[::1]/")
    assert u.get_server_and_port() == "[::1]"

    u = URLSpec("http://[::1]:80/")
    assert u.get_server_and_port() == "[::1]:80"


def test_urlspec_default_port_for_scheme():
    assert URLSpec.get_default_port_for_scheme("http") == 80
    assert URLSpec.get_default_port_for_scheme("https") == 443
    assert URLSpec.get_default_port_for_scheme("socks") == 1080
    # Empty falls back to http.
    assert URLSpec.get_default_port_for_scheme("") == 80
    # Unknown scheme is 0.
    assert URLSpec.get_default_port_for_scheme("gopher") == 0

    assert URLSpec("http://example.com/").get_port() == 80
    assert URLSpec("https://example.com/").get_port() == 443
    assert URLSpec("socks://example.com/").get_port() == 1080


def test_urlspec_is_default_port():
    assert URLSpec("http://example.com/").is_default_port()
    assert URLSpec("http://example.com:80/").is_default_port()
    assert not URLSpec("http://example.com:8080/").is_default_port()
    assert URLSpec("https://example.com:443/").is_default_port()


def test_urlspec_is_ssl():
    assert not URLSpec("http://example.com/").is_ssl()
    assert URLSpec("https://example.com/").is_ssl()
    # The implementation treats any scheme ending in 's' as SSL, EXCEPT socks.
    assert not URLSpec("socks://example.com/").is_ssl()
    assert not URLSpec("ftp://example.com/").is_ssl()
    # No scheme at all isn't SSL.
    assert not URLSpec("//example.com/").is_ssl()


def test_urlspec_get_path_and_query():
    u = URLSpec("http://example.com")
    assert not u.has_path()
    assert u.get_path() == "/"
    assert u.get_path_and_query() == "/"

    u = URLSpec("http://example.com/foo")
    assert u.get_path_and_query() == "/foo"

    u = URLSpec("http://example.com?q=1")
    assert u.get_path_and_query() == "/?q=1"

    u = URLSpec("http://example.com/foo?q=1")
    assert u.get_path_and_query() == "/foo?q=1"


def test_urlspec_set_scheme():
    u = URLSpec("//example.com/foo")
    u.set_scheme("http")
    assert u.has_scheme()
    assert u.get_scheme() == "http"
    assert u.get_url() == "http://example.com/foo"

    # The setter is supposed to be tolerant of an appended ':' on the scheme.
    u = URLSpec("//example.com/foo")
    u.set_scheme("https:")
    assert u.get_scheme() == "https"
    assert u.get_url() == "https://example.com/foo"

    u = URLSpec("http://example.com/foo")
    u.set_scheme("ftp")
    assert u.get_scheme() == "ftp"
    assert u.get_url() == "ftp://example.com/foo"

    # Lowercases
    u = URLSpec("//example.com/foo")
    u.set_scheme("HTTP")
    assert u.get_scheme() == "http"


def test_urlspec_clear_scheme():
    u = URLSpec("http://example.com/foo")
    u.set_scheme("")
    assert not u.has_scheme()
    # The "//example.com/foo" remainder is preserved.
    assert u.get_url() == "//example.com/foo"
    assert u.has_authority()
    assert u.get_server() == "example.com"

    u = URLSpec("//example.com/foo")
    u.set_scheme("")
    assert u.get_url() == "//example.com/foo"


def test_urlspec_set_authority():
    # When inserting an authority and the existing URL has a path that doesn't
    # start with '/', the setter must insert one. This branch is exercised
    # primarily by the buffer-rebuild code path that replaced the old
    # `s + auth + s` concatenation.
    u = URLSpec("foo")
    assert u.has_path() and not u.has_authority()
    assert u.get_path() == "foo"
    u.set_authority("example.com")
    assert u.get_url() == "//example.com/foo"
    assert u.get_server() == "example.com"
    assert u.get_path() == "/foo"

    # Mirror of the above, but the path already starts with '/', so no extra
    # slash must be inserted.
    u = URLSpec("/foo")
    assert u.has_path() and not u.has_authority()
    u.set_authority("example.com")
    assert u.get_url() == "//example.com/foo"
    assert u.get_server() == "example.com"
    assert u.get_path() == "/foo"

    u = URLSpec("http://old.example.com:1234/foo?q=1")
    u.set_authority("new.example.com:5678")
    assert u.get_url() == "http://new.example.com:5678/foo?q=1"
    assert u.get_server() == "new.example.com"
    assert u.get_port() == 5678
    assert u.get_path() == "/foo"
    assert u.get_query() == "q=1"

    u = URLSpec("http://example.com/foo")
    u.set_authority("user@example.com:8080")
    assert u.has_username()
    assert u.get_username() == "user"
    assert u.get_server() == "example.com"
    assert u.get_port() == 8080
    assert u.get_path() == "/foo"

    u = URLSpec("http://example.com/foo")
    u.set_authority("")
    assert not u.has_authority()
    assert not u.has_server()
    assert not u.has_port()
    # The path is preserved.
    assert u.has_path()
    assert u.get_path() == "/foo"


def test_urlspec_set_username():
    u = URLSpec("http://old@example.com:8080/foo")
    u.set_username("new")
    assert u.get_username() == "new"
    assert u.get_server() == "example.com"
    assert u.get_port() == 8080
    assert u.get_path() == "/foo"

    u = URLSpec("http://example.com/foo")
    u.set_username("user")
    assert u.has_username()
    assert u.get_username() == "user"
    assert u.get_server() == "example.com"

    u = URLSpec("http://user@example.com/foo")
    u.set_username("")
    assert not u.has_username()
    assert u.get_server() == "example.com"

    u = URLSpec("http://example.com:8080/foo")
    u.set_server("other.example.com")
    assert u.get_server() == "other.example.com"
    assert u.get_port() == 8080
    assert u.get_path() == "/foo"


def test_urlspec_set_server():
    # set_server is allowed to accept a bare IPv6 address; it must add the
    # square brackets back automatically when rebuilding the authority.
    u = URLSpec("http://example.com:8080/foo")
    u.set_server("::1")
    assert u.get_server() == "::1"
    assert u.get_port() == 8080
    assert "[::1]" in u.get_url()


def test_urlspec_set_port():
    # Accepts string
    u = URLSpec("http://example.com/foo")
    u.set_port("8443")
    assert u.has_port()
    assert u.get_port() == 8443
    assert u.get_url() == "http://example.com:8443/foo"

    # Accepts int
    u = URLSpec("http://example.com/foo")
    u.set_port(9999)
    assert u.get_port() == 9999

    # Empty removes port
    u = URLSpec("http://example.com:1234/foo")
    u.set_port("")
    assert not u.has_port()
    # The default for http kicks in.
    assert u.get_port() == 80


def test_urlspec_set_server_and_port():
    u = URLSpec("http://example.com/foo")
    u.set_server_and_port("other:1234")
    assert u.get_server() == "other"
    assert u.get_port() == 1234

    u = URLSpec("http://example.com/foo")
    u.set_server_and_port("[::1]:1234")
    assert u.get_server() == "::1"
    assert u.get_port() == 1234


def test_urlspec_set_path():
    u = URLSpec("http://example.com")
    u.set_path("/foo")
    assert u.has_path()
    assert u.get_path() == "/foo"
    assert u.get_url() == "http://example.com/foo"

    u = URLSpec("http://example.com")
    u.set_path("foo")
    assert u.get_path() == "/foo"

    u = URLSpec("http://example.com/old?q=1")
    u.set_path("/new")
    assert u.get_path() == "/new"
    assert u.get_query() == "q=1"
    assert u.get_url() == "http://example.com/new?q=1"

    u = URLSpec("http://example.com/old?q=1")
    u.set_path("new")
    assert u.get_path() == "/new"
    assert u.get_url() == "http://example.com/new?q=1"

    u = URLSpec("http://example.com/foo")
    u.set_path("")
    assert not u.has_path()


def test_urlspec_set_query():
    u = URLSpec("http://example.com/foo")
    u.set_query("q=1")
    assert u.has_query()
    assert u.get_query() == "q=1"
    assert u.get_url() == "http://example.com/foo?q=1"

    u = URLSpec("http://example.com/foo?old=1")
    u.set_query("new=2")
    assert u.get_query() == "new=2"
    assert u.get_url() == "http://example.com/foo?new=2"

    u = URLSpec("http://example.com/foo?q=1")
    u.set_query("")
    assert not u.has_query()
    assert u.get_url() == "http://example.com/foo"


def test_urlspec_set_url():
    u = URLSpec("http://old.example.com/foo")
    u.set_url("https://new.example.com:8443/bar?baz=1")
    assert u.get_scheme() == "https"
    assert u.get_server() == "new.example.com"
    assert u.get_port() == 8443
    assert u.get_path() == "/bar"
    assert u.get_query() == "baz=1"

    u = URLSpec("http://example.com/foo")
    u.set_url("")
    assert u.empty()
    assert not u.has_scheme()
    assert not u.has_authority()
    assert not u.has_path()

    u = URLSpec()
    u.set_url("example.com:1234", True)
    assert u.has_server()
    assert u.get_server() == "example.com"
    assert u.get_port() == 1234


def test_urlspec_quote():
    assert URLSpec.quote("abcXYZ0189_,.-") == "abcXYZ0189_,.-"

    # By default '/' is in the safe set; space is not.
    assert URLSpec.quote("a b") == "a%20b"

    assert URLSpec.quote("a/b") == "a/b"

    # With an empty safe set, '/' is escaped.
    assert URLSpec.quote("a/b", "") == "a%2fb"


def test_urlspec_quote_plus():
    assert URLSpec.quote_plus("a b") == "a+b"

    # '&' is not safe so it's escaped, while space becomes '+'.
    assert URLSpec.quote_plus("a&b c", "") == "a%26b+c"


def test_urlspec_unquote():
    assert URLSpec.unquote("a%20b") == "a b"
    # Case-insensitive hex.
    assert URLSpec.unquote("a%2Fb") == "a/b"
    assert URLSpec.unquote("a%2fb") == "a/b"

    assert URLSpec.unquote("hello") == "hello"

    raw = "Hello, World! 100% / safe?"
    assert URLSpec.unquote(URLSpec.quote(raw, "")) == raw


def test_urlspec_unquote_plus():
    assert URLSpec.unquote_plus("a+b") == "a b"
    assert URLSpec.unquote_plus("a%2Bb") == "a+b"

    raw = "key=value&other=thing with spaces"
    assert URLSpec.unquote_plus(URLSpec.quote_plus(raw, "")) == raw


def test_urlspec_equality():
    a = URLSpec("http://example.com/foo")
    b = URLSpec("http://example.com/foo")
    assert a == b
    assert not (a != b)

    a = URLSpec("http://example.com/foo")
    b = URLSpec("http://example.com/bar")
    assert a != b

    # compare_to does cmp_nocase on the scheme.
    a = URLSpec("http://example.com/")
    b = URLSpec("HTTP://example.com/")
    assert a == b

    a = URLSpec("http://example.com/")
    b = URLSpec("http://EXAMPLE.com/")
    assert a == b


def test_urlspec_ordering():
    a = URLSpec("ftp://example.com/")
    b = URLSpec("http://example.com/")
    assert a < b
    assert not (b < a)


def test_urlspec_hash():
    a = URLSpec("http://example.com/")
    b = URLSpec("HTTP://Example.COM/")
    # Equality already covers this, but the hash must match equal-objects too.
    assert a.get_hash() == b.get_hash()


def test_urlspec_indexing():
    u = URLSpec("http://example.com/")
    assert u[0] == "h"
    assert u[4] == ":"
