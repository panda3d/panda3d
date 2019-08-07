from panda3d import core


def test_encrypt_string():
    # Test encrypt and then decrypt cycle
    enc = core.encrypt_string('abcdefg', '12345')
    assert len(enc) > 0

    dec = core.decrypt_string(enc, '12345')
    assert dec == 'abcdefg'

    # Test pre-encrypted string
    enc = b'[\x00\x10\x00d\x00\xb5\x7f\xc44Y\xb7\xd9\x15\xe3\xbd\xcf\xb3yK\xfb\xf6'
    assert 'test' == core.decrypt_string(enc, '98765')
