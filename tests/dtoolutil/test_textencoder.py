import sys
import pytest
from panda3d.core import TextEncoder

if sys.version_info >= (3, 0):
    unichr = chr
    xrange = range


def valid_characters():
    """Generator yielding all valid Unicode code points."""

    for i in xrange(0xd800):
        yield unichr(i)

    for i in xrange(0xe000, sys.maxunicode + 1):
        if i != 0xfeff and i & 0xfffe != 0xfffe:
            yield unichr(i)


def test_text_decode_iso8859():
    encoder = TextEncoder()
    encoder.set_encoding(TextEncoder.E_iso8859)

    for i in xrange(255):
        enc = unichr(i).encode('latin-1')
        assert len(enc) == 1

        dec = encoder.decode_text(enc)
        assert len(dec) == 1
        assert ord(dec) == i


def test_text_decode_utf8():
    encoder = TextEncoder()
    encoder.set_encoding(TextEncoder.E_utf8)

    for c in valid_characters():
        enc = c.encode('utf-8')
        assert len(enc) <= 4

        dec = encoder.decode_text(enc)
        assert len(dec) == 1
        assert dec == c


def test_text_decode_utf16be():
    encoder = TextEncoder()
    encoder.set_encoding(TextEncoder.E_utf16be)

    for c in valid_characters():
        enc = c.encode('utf-16be')

        dec = encoder.decode_text(enc)
        assert len(c) == len(dec)
        assert c == dec


def test_text_encode_iso8859():
    encoder = TextEncoder()
    encoder.set_encoding(TextEncoder.E_iso8859)

    for i in xrange(255):
        c = unichr(i)
        enc = encoder.encode_wtext(c)
        assert enc == c.encode('latin-1')


def test_text_encode_utf8():
    encoder = TextEncoder()
    encoder.set_encoding(TextEncoder.E_utf8)

    for c in valid_characters():
        enc = encoder.encode_wtext(c)
        assert enc == c.encode('utf-8')


def test_text_encode_utf16be():
    encoder = TextEncoder()
    encoder.set_encoding(TextEncoder.E_utf16be)

    for c in valid_characters():
        enc = encoder.encode_wtext(c)
        assert enc == c.encode('utf-16-be')


def test_text_append_unicode_char():
    encoder = TextEncoder()
    encoder.set_encoding(TextEncoder.E_iso8859)

    code_points = []
    for code_point in [0, 1, 127, 128, 255, 256, 0xfffd, 0x10000, 0x10ffff]:
        if code_point <= sys.maxunicode:
            code_points.append(code_point)
            encoder.append_unicode_char(code_point)

    encoded = encoder.get_wtext()
    assert len(encoded) == len(code_points)

    for a, b in zip(code_points, encoded):
        assert a == ord(b)
