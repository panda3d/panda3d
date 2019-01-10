from panda3d.core import StreamReader, StringStream
import pytest


def test_streamreader_string():
    # Empty string
    stream = StringStream(b'\x00\x00')
    reader = StreamReader(stream, False)
    assert reader.get_string() == ''

    # String size but no string contents
    stream = StringStream(b'\x01\x00')
    reader = StreamReader(stream, False)
    assert reader.get_string() == ''

    # String of length 1
    stream = StringStream(b'\x01\x00A')
    reader = StreamReader(stream, False)
    assert reader.get_string() == 'A'

    # String with excess data
    stream = StringStream(b'\x01\x00AB')
    reader = StreamReader(stream, False)
    assert reader.get_string() == 'A'

    # EOF before end of string
    stream = StringStream(b'\x03\x00AB')
    reader = StreamReader(stream, False)
    assert reader.get_string() == 'AB'

    # Preserves null bytes
    stream = StringStream(b'\x02\x00\x00\x00')
    reader = StreamReader(stream, False)
    assert reader.get_string() == '\x00\x00'


def test_streamreader_string32():
    # Empty string
    stream = StringStream(b'\x00\x00\x00\x00')
    reader = StreamReader(stream, False)
    assert reader.get_string32() == ''

    # String size but no string contents
    stream = StringStream(b'\x01\x00\x00\x00')
    reader = StreamReader(stream, False)
    assert reader.get_string32() == ''

    # String of length 1
    stream = StringStream(b'\x01\x00\x00\x00A')
    reader = StreamReader(stream, False)
    assert reader.get_string32() == 'A'

    # String with excess data
    stream = StringStream(b'\x01\x00\x00\x00AB')
    reader = StreamReader(stream, False)
    assert reader.get_string32() == 'A'

    # EOF before end of string
    stream = StringStream(b'\x04\x00\x00\x00AB')
    reader = StreamReader(stream, False)
    assert reader.get_string32() == 'AB'

    # Preserves null bytes
    stream = StringStream(b'\x02\x00\x00\x00\x00\x00')
    reader = StreamReader(stream, False)
    assert reader.get_string32() == '\x00\x00'


def test_streamreader_z_string():
    # Empty stream
    stream = StringStream(b'')
    reader = StreamReader(stream, False)
    assert reader.get_z_string() == ''

    # Empty string
    stream = StringStream(b'\x00')
    reader = StreamReader(stream, False)
    assert reader.get_z_string() == ''

    # String of length 1
    stream = StringStream(b'A\x00')
    reader = StreamReader(stream, False)
    assert reader.get_z_string() == 'A'

    # String with excess data
    stream = StringStream(b'ABC\x00AB')
    reader = StreamReader(stream, False)
    assert reader.get_z_string() == 'ABC'

    # EOF before end of string
    stream = StringStream(b'ABC')
    reader = StreamReader(stream, False)
    assert reader.get_z_string() == 'ABC'


def test_streamreader_fixed_string():
    # Zero-length string
    stream = StringStream(b'ABC')
    reader = StreamReader(stream, False)
    assert reader.get_fixed_string(0) == ''

    # Empty stream
    stream = StringStream(b'')
    reader = StreamReader(stream, False)
    assert reader.get_fixed_string(1) == ''

    # Empty string
    stream = StringStream(b'\x00')
    reader = StreamReader(stream, False)
    assert reader.get_fixed_string(1) == ''

    # String of length 1
    stream = StringStream(b'A')
    reader = StreamReader(stream, False)
    assert reader.get_fixed_string(1) == 'A'

    # String of length 1, excess data
    stream = StringStream(b'ABC\x00')
    reader = StreamReader(stream, False)
    assert reader.get_fixed_string(1) == 'A'

    # EOF before end of string
    stream = StringStream(b'AB')
    reader = StreamReader(stream, False)
    assert reader.get_fixed_string(4) == 'AB'


def test_streamreader_readline():
    # Empty stream
    stream = StringStream(b'')
    reader = StreamReader(stream, False)
    assert reader.readline() == b''
    assert reader.readline() == b''

    # Single line without newline
    stream = StringStream(b'A')
    reader = StreamReader(stream, False)
    assert reader.readline() == b'A'
    assert reader.readline() == b''

    # Single newline
    stream = StringStream(b'\n')
    reader = StreamReader(stream, False)
    assert reader.readline() == b'\n'
    assert reader.readline() == b''

    # Line with text followed by empty line
    stream = StringStream(b'A\n\n')
    reader = StreamReader(stream, False)
    assert reader.readline() == b'A\n'
    assert reader.readline() == b'\n'
    assert reader.readline() == b''

    # Preserve null byte
    stream = StringStream(b'\x00\x00')
    reader = StreamReader(stream, False)
    assert reader.readline() == b'\x00\x00'
