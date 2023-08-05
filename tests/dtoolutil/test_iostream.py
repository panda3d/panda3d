from panda3d.core import StringStream

import pytest


ISTREAM_DATA = b'abcdefghijklmnopqrstuvwxyz' * 500

@pytest.fixture
def istream():
    return StringStream(ISTREAM_DATA)


def test_istream_readall(istream):
    assert istream.readall() == ISTREAM_DATA
    assert istream.readall() == b''
    assert istream.readall() == b''
    assert istream.tellg() == len(ISTREAM_DATA)


def test_istream_read(istream):
    assert istream.read() == ISTREAM_DATA
    assert istream.read() == b''
    assert istream.read() == b''
    assert istream.tellg() == len(ISTREAM_DATA)


def test_istream_read_size(istream):
    assert istream.read(100) == ISTREAM_DATA[:100]
    assert istream.read(5000) == ISTREAM_DATA[100:5100]
    assert istream.read(5000) == ISTREAM_DATA[5100:10100]
    assert istream.read(5000) == ISTREAM_DATA[10100:15100]
    assert istream.read() == b''
    assert istream.tellg() == len(ISTREAM_DATA)


def test_istream_read1(istream):
    accumulated = b''
    data = istream.read1()
    while data:
        accumulated += data
        data = istream.read1()

    assert accumulated == ISTREAM_DATA
    assert istream.tellg() == len(ISTREAM_DATA)


def test_istream_read1_size(istream):
    accumulated = b''
    data = istream.read1(4000)
    while data:
        accumulated += data
        data = istream.read1(4000)

    assert accumulated == ISTREAM_DATA
    assert istream.tellg() == len(ISTREAM_DATA)


def test_istream_readinto(istream):
    ba = bytearray()
    assert istream.readinto(ba) == 0
    assert istream.tellg() == 0

    ba = bytearray(10)
    assert istream.readinto(ba) == 10
    assert ba == ISTREAM_DATA[:10]
    assert istream.tellg() == 10

    ba = bytearray(len(ISTREAM_DATA))
    assert istream.readinto(ba) == len(ISTREAM_DATA) - 10
    assert ba[:len(ISTREAM_DATA)-10] == ISTREAM_DATA[10:]
    assert istream.tellg() == len(ISTREAM_DATA)


def test_istream_readline():
    # Empty stream
    stream = StringStream(b'')
    assert stream.readline() == b''
    assert stream.readline() == b''

    # Single line without newline
    stream = StringStream(b'A')
    assert stream.readline() == b'A'
    assert stream.readline() == b''

    # Single newline
    stream = StringStream(b'\n')
    assert stream.readline() == b'\n'
    assert stream.readline() == b''

    # Line with text followed by empty line
    stream = StringStream(b'A\n\n')
    assert stream.readline() == b'A\n'
    assert stream.readline() == b'\n'
    assert stream.readline() == b''

    # Preserve null byte
    stream = StringStream(b'\x00\x00')
    assert stream.readline() == b'\x00\x00'


def test_istream_readlines():
    istream = StringStream(b'a')
    assert istream.readlines() == [b'a']
    assert istream.readlines() == []

    istream = StringStream(b'a\nb\nc\n')
    assert istream.readlines() == [b'a\n', b'b\n', b'c\n']

    istream = StringStream(b'\na\nb\nc')
    assert istream.readlines() == [b'\n', b'a\n', b'b\n', b'c']

    istream = StringStream(b'\n\n\n')
    assert istream.readlines() == [b'\n', b'\n', b'\n']


def test_istream_iter():
    istream = StringStream(b'a')
    assert tuple(istream) == (b'a',)
    assert tuple(istream) == ()

    istream = StringStream(b'a\nb\nc\n')
    assert tuple(istream) == (b'a\n', b'b\n', b'c\n')

    istream = StringStream(b'\na\nb\nc')
    assert tuple(istream) == (b'\n', b'a\n', b'b\n', b'c')

    istream = StringStream(b'\n\n\n')
    assert tuple(istream) == (b'\n', b'\n', b'\n')
