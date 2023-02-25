from panda3d import core

import pytest


@pytest.mark.skipif(not hasattr(core, 'encrypt_string'), reason="Requires OpenSSL")
def test_encrypt_string():
    # Test encrypt and then decrypt cycle
    for algorithm in ('', 'bf-cbc', 'aes-256-cbc'):
        enc = core.encrypt_string('abcdefg', '12345', algorithm)
        assert len(enc) > 0

        dec = core.decrypt_string(enc, '12345')
        assert dec == 'abcdefg'

    # Test pre-encrypted bf-cbc string
    enc = b'[\x00\x10\x00d\x00\xb5\x7f\xc44Y\xb7\xd9\x15\xe3\xbd\xcf\xb3yK\xfb\xf6'
    assert 'test' == core.decrypt_string(enc, '98765')

    # Test pre-encrypted aes-256-cbc string
    enc = b'\xab\x01 \x00d\x00\xf1WP\xb0\x96h\xf8\xc5\xf4\x8d\x0b>q0\xf15\x185\xf8+\x1b\xe4\xae8\x88\xf2\x91\x15\xb8\x8fh\x88'
    assert 'test' == core.decrypt_string(enc, '98765')
