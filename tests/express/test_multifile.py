from panda3d.core import Multifile, StringStream, IStreamWrapper


def test_multifile_read_empty():
    stream = StringStream(b'pmf\x00\n\r\x01\x00\x01\x00\x01\x00\x00\x00\xdb\x9d7\\\x00\x00\x00\x00')
    wrapper = IStreamWrapper(stream)

    m = Multifile()
    assert m.open_read(wrapper)
    assert m.is_read_valid()
    assert m.get_num_subfiles() == 0
    m.close()


def test_multifile_password():
    m = Multifile()

    m.set_encryption_password('Panda3D rocks!')
    assert m.get_encryption_password() == 'Panda3D rocks!'

    m.set_encryption_password(b'Panda3D is awesome!')
    assert m.get_encryption_password() == 'Panda3D is awesome!'

    m.set_encryption_password(b'\xc4\x97\xa1\x01\x85\xb6')
    assert m.get_encryption_password() == b'\xc4\x97\xa1\x01\x85\xb6'
