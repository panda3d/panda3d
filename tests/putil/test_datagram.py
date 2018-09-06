import pytest
from panda3d import core
import sys
import tempfile

# Fixtures for generating interesting datagrams (and verification functions) on
# the fly...

@pytest.fixture(scope='module',
                params=[False, True],
                ids=['stdfloat_float', 'stdfloat_double'])
def datagram_small(request):
    """Returns a small datagram, along with a verification function."""
    dg = core.Datagram()

    dg.set_stdfloat_double(request.param)

    dg.add_uint8(3)
    dg.add_uint16(14159)
    dg.add_uint32(0xDEADBEEF)
    dg.add_uint64(0x0123456789ABCDEF)

    dg.add_int8(-77)
    dg.add_int16(-1)
    dg.add_int32(-972965890)
    dg.add_int64(-1001001001001001)

    dg.add_string('this is a string')
    dg.add_string32('this is another string')
    dg.add_string('this is yet a third string')

    dg.add_blob(b'blob data \x00\xf2\xa0\x00\x00')
    dg.add_blob32(b'\xc9\x8f\x00 test blob32')

    dg.add_stdfloat(800.2)
    dg.add_stdfloat(3.1415926)
    dg.add_stdfloat(2.7182818)

    def readback_function(dgi):
        assert dgi.get_remaining_size() > 0

        assert dgi.get_uint8() == 3
        assert dgi.get_uint16() == 14159
        assert dgi.get_uint32() == 0xDEADBEEF
        assert dgi.get_uint64() == 0x0123456789ABCDEF

        assert dgi.get_int8() == -77
        assert dgi.get_int16() == -1
        assert dgi.get_int32() == -972965890
        assert dgi.get_int64() == -1001001001001001

        assert dgi.get_string() == 'this is a string'
        assert dgi.get_string32() == 'this is another string'
        assert dgi.get_string() == 'this is yet a third string'

        assert dgi.get_blob() == b'blob data \x00\xf2\xa0\x00\x00'
        assert dgi.get_blob32() == b'\xc9\x8f\x00 test blob32'

        assert dgi.get_stdfloat() == pytest.approx(800.2)
        assert dgi.get_stdfloat() == pytest.approx(3.1415926)
        assert dgi.get_stdfloat() == pytest.approx(2.7182818)

        assert dgi.get_remaining_size() == 0

    return dg, readback_function

@pytest.fixture(scope='module')
def datagram_large():
    """Returns a big datagram, along with a verification function."""

    dg = core.Datagram()
    for x in range(2000000):
        dg.add_uint32(x)
        dg.add_string('the magic words are squeamish ossifrage')

    def readback_function(dgi):
        assert dgi.get_remaining_size() > 0

        for x in range(2000000):
            assert dgi.get_uint32() == x
            assert dgi.get_string() == 'the magic words are squeamish ossifrage'

        assert dgi.get_remaining_size() == 0

    return dg, readback_function

@pytest.mark.skipif(sys.version_info < (3, 0), reason="Requires Python 3")
def test_datagram_bytes():
    """Tests that we can put and get a bytes object on Datagram."""
    dg = core.Datagram(b'abc\x00')
    dg.append_data(b'\xff123')
    assert bytes(dg) == b'abc\x00\xff123'

    dgi = core.DatagramIterator(dg)
    dgi.get_remaining_bytes() == b'abc\x00\xff123'


def test_datagram_get_message():
    dg = core.Datagram(b'abc\x00')
    dg.append_data(b'\xff123')
    assert dg.get_message() == b'abc\x00\xff123'


def test_iterator(datagram_small):
    """This tests Datagram/DatagramIterator, and sort of serves as a self-check
    of the test fixtures too."""
    dg, verify = datagram_small

    dgi = core.DatagramIterator(dg)
    verify(dgi)


# This tests the copy constructor:
def test_copy(datagram_small):
    dg, verify = datagram_small

    dg2 = core.Datagram(dg)
    dgi = core.DatagramIterator(dg2)
    verify(dgi)


def test_assign(datagram_small):
    dg, verify = datagram_small

    dg2 = core.Datagram()
    dg2.assign(dg)
    dgi = core.DatagramIterator(dg2)
    verify(dgi)


# These test DatagramInputFile/DatagramOutputFile:

def do_file_test(dg, verify, filename):
    dof = core.DatagramOutputFile()
    dof.open(filename)
    dof.put_datagram(dg)
    dof.close()

    dg2 = core.Datagram()
    dif = core.DatagramInputFile()
    dif.open(filename)
    assert dif.get_datagram(dg2)
    dif.close()

    # This is normally saved by the DatagramOutputFile header. We cheat here.
    dg2.set_stdfloat_double(dg.get_stdfloat_double())

    dgi = core.DatagramIterator(dg2)
    verify(dgi)

def test_file_small(datagram_small):
    """This tests DatagramOutputFile/DatagramInputFile on small datagrams."""
    dg, verify = datagram_small

    file = tempfile.NamedTemporaryFile(suffix='.bin')
    filename = core.Filename.from_os_specific(file.name)
    filename.make_true_case()

    do_file_test(dg, verify, filename)

def test_file_large(datagram_large):
    """This tests DatagramOutputFile/DatagramInputFile on very large datagrams."""
    dg, verify = datagram_large

    file = tempfile.NamedTemporaryFile(suffix='.bin')
    filename = core.Filename.from_os_specific(file.name)
    filename.make_true_case()

    do_file_test(dg, verify, filename)

def test_file_corrupt(datagram_small):
    """This tests DatagramInputFile's handling of a corrupt size header."""
    dg, verify = datagram_small

    file = tempfile.NamedTemporaryFile(suffix='.bin')
    filename = core.Filename.from_os_specific(file.name)
    filename.make_true_case()

    dof = core.DatagramOutputFile()
    dof.open(filename)
    dof.put_datagram(dg)
    dof.close()

    # Corrupt the size header to 1GB
    file.seek(0)
    file.write(b'\xFF\xFF\xFF\x4F')
    file.flush()

    dg2 = core.Datagram()
    dif = core.DatagramInputFile()
    dif.open(filename)
    assert not dif.get_datagram(dg2)
    dif.close()

    # Truncate the file
    for size in [12, 8, 4, 3, 2, 1, 0]:
        file.truncate(size)

        dg2 = core.Datagram()
        dif = core.DatagramInputFile()
        dif.open(filename)
        assert not dif.get_datagram(dg2)
        dif.close()

    # Should we test that dg2 is unmodified?
