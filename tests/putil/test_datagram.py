import pytest
from panda3d import core

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

def test_iterator(datagram_small):
    """This tests Datagram/DatagramIterator, and sort of serves as a self-check
    of the test fixtures too."""
    dg, verify = datagram_small

    dgi = core.DatagramIterator(dg)
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

def test_file_small(datagram_small, tmpdir):
    """This tests DatagramOutputFile/DatagramInputFile on small datagrams."""
    dg, verify = datagram_small

    p = tmpdir.join('datagram.bin')
    filename = core.Filename.from_os_specific(str(p))

    do_file_test(dg, verify, filename)

def test_file_large(datagram_large, tmpdir):
    """This tests DatagramOutputFile/DatagramInputFile on very large datagrams."""
    dg, verify = datagram_large

    p = tmpdir.join('datagram.bin')
    filename = core.Filename.from_os_specific(str(p))

    do_file_test(dg, verify, filename)

def test_file_corrupt(datagram_small, tmpdir):
    """This tests DatagramInputFile's handling of a corrupt size header."""
    dg, verify = datagram_small

    p = tmpdir.join('datagram.bin')
    filename = core.Filename.from_os_specific(str(p))

    dof = core.DatagramOutputFile()
    dof.open(filename)
    dof.put_datagram(dg)
    dof.close()

    # Corrupt the size header to 4GB
    with p.open(mode='wb') as f:
        f.seek(0)
        f.write(b'\xFF\xFF\xFF\xFF')

    dg2 = core.Datagram()
    dif = core.DatagramInputFile()
    dif.open(filename)
    assert not dif.get_datagram(dg2)
    dif.close()

    # Should we test that dg2 is unmodified?
