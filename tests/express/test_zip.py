from panda3d.core import ZipArchive, IStreamWrapper, StringStream, Filename
from direct.stdpy.file import StreamIOWrapper
import zipfile
from io import BytesIO


EMPTY_ZIP = b'PK\x05\x06' + b'\x00' * 18


def test_zip_read_empty():
    stream = StringStream(EMPTY_ZIP)
    wrapper = IStreamWrapper(stream)

    zip = ZipArchive()
    zip.open_read(wrapper)

    assert zip.is_read_valid()
    assert not zip.is_write_valid()
    assert not zip.needs_repack()

    assert zip.get_num_subfiles() == 0

    zip.close()


def test_zip_write_empty():
    stream = StringStream()
    zip = ZipArchive()
    zip.open_write(stream)

    assert not zip.is_read_valid()
    assert zip.is_write_valid()
    assert not zip.needs_repack()

    zip.close()

    assert stream.data == EMPTY_ZIP
    with zipfile.ZipFile(StreamIOWrapper(stream), 'r') as zf:
        assert zf.testzip() is None


def test_zip_read_extract(tmp_path):
    stream = StringStream()
    zf = zipfile.ZipFile(StreamIOWrapper(stream), mode='w', allowZip64=True)
    zf.writestr("test.txt", b"test stored", compress_type=zipfile.ZIP_STORED)
    zf.writestr("test2.txt", b"test deflated", compress_type=zipfile.ZIP_DEFLATED)
    zf.writestr("dir/dtest.txt", b"test in dir")
    zf.writestr("dir1/dir2/test.txt", b"test nested dir")
    zf.writestr("emptydir/", b"", compress_type=zipfile.ZIP_STORED)
    zf.close()

    wrapper = IStreamWrapper(stream)
    zip = ZipArchive()
    zip.open_read(wrapper)

    assert zip.is_read_valid()
    assert not zip.is_write_valid()
    assert not zip.needs_repack()

    assert zip.verify()

    assert zip.find_subfile("nonexistent.txt") == -1

    sf = zip.find_subfile("test.txt")
    assert sf >= 0
    assert zip.read_subfile(sf) == b"test stored"
    assert zip.extract_subfile(sf, tmp_path / "test.txt")
    assert open(tmp_path / "test.txt", 'rb').read() == b"test stored"

    sf = zip.find_subfile("test2.txt")
    assert sf >= 0
    assert zip.read_subfile(sf) == b"test deflated"
    assert zip.extract_subfile(sf, tmp_path / "test2.txt")
    assert open(tmp_path / "test2.txt", 'rb').read() == b"test deflated"


def test_zip_write():
    stream = StringStream()
    zip = ZipArchive()
    zip.open_write(stream)
    zip.add_subfile("test.txt", StringStream(b"test deflated"), 6)
    zip.add_subfile("test2.txt", StringStream(b"test stored"), 0)
    zip.close()

    with zipfile.ZipFile(StreamIOWrapper(stream), 'r') as zf:
        assert zf.testzip() is None

    assert tuple(sorted(zf.namelist())) == ("test.txt", "test2.txt")


def test_zip_replace_subfile(tmp_path):
    stream = StringStream()
    zf = zipfile.ZipFile(StreamIOWrapper(stream), mode='w', allowZip64=True)
    zf.writestr("test1.txt", b"contents of first file")
    zf.writestr("test2.txt", b"")
    zf.writestr("test3.txt", b"contents of third file")
    zf.close()

    zip = ZipArchive()
    zip.open_read_write(stream)

    assert zip.is_read_valid()
    assert zip.is_write_valid()
    assert not zip.needs_repack()

    assert zip.verify()

    sf = zip.find_subfile("test2.txt")
    assert sf >= 0
    zip.add_subfile("test2.txt", StringStream(b"contents of second file"), 6)
    zip.close()

    with zipfile.ZipFile(StreamIOWrapper(stream), 'r') as zf:
        assert zf.testzip() is None
        assert zf.read("test1.txt") == b"contents of first file"
        assert zf.read("test2.txt") == b"contents of second file"
        assert zf.read("test3.txt") == b"contents of third file"


def test_zip_remove_subfile(tmp_path):
    stream = StringStream()
    zf = zipfile.ZipFile(StreamIOWrapper(stream), mode='w', allowZip64=True)
    zf.writestr("test1.txt", b"contents of first file")
    zf.writestr("test2.txt", b"contents of second file")
    zf.writestr("test3.txt", b"contents of third file")
    zf.close()

    zip = ZipArchive()
    zip.open_read_write(stream)

    assert zip.is_read_valid()
    assert zip.is_write_valid()
    assert not zip.needs_repack()

    assert zip.verify()

    removed = zip.remove_subfile("test2.txt")
    assert removed
    zip.close()

    with zipfile.ZipFile(StreamIOWrapper(stream), 'r') as zf:
        assert zf.testzip() is None
        names = zf.namelist()
        assert "test1.txt" in names
        assert "test2.txt" not in names
        assert "test3.txt" in names


def test_zip_repack(tmp_path):
    zip_path = tmp_path / "test_zip_repack.zip"
    zf = zipfile.ZipFile(zip_path, mode='w', allowZip64=True)
    zf.writestr("test1.txt", b"contents of first file")
    zf.writestr("test2.txt", b"contents of second file")
    zf.close()

    zip = ZipArchive()
    zip.open_read_write(zip_path)

    assert zip.is_read_valid()
    assert zip.is_write_valid()
    assert not zip.needs_repack()

    assert zip.verify()

    removed = zip.remove_subfile("test2.txt")
    assert removed

    zip.add_subfile("test3.txt", StringStream(b"contents of third file"), 6)

    assert zip.needs_repack()
    result = zip.repack()
    assert result
    assert not zip.needs_repack()

    assert zip.verify()

    zip.close()

    with zipfile.ZipFile(zip_path, 'r') as zf:
        assert zf.testzip() is None
        assert zf.read("test1.txt") == b"contents of first file"
        assert "test2.txt" not in zf.namelist()
        assert zf.read("test3.txt") == b"contents of third file"
