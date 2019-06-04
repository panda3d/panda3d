from panda3d import core

import pytest


@pytest.mark.skipif(not hasattr(core, 'IDecryptStream'), reason="Requires OpenSSL")
def test_decrypt_stream():
    encrypted = b'[\x00\x10\x00d\x00\x07K\x08\x03\xabS\x13L\xab\x93\x1b\x15\xe4\xeel\x80u o\xd0\x80aY_]\x10\x8a\xb5\xff\x9d1\xc9\xd3\xac\x95\x04\xd8\xdf\x10\xa1'
    decrypted = b'abcdefghijklmnopqrstuvwxyz'

    ss = core.StringStream(encrypted)
    ds = core.IDecryptStream(ss, False, '0123456789')

    assert ds.read(len(decrypted)) == decrypted
    assert ds.readall() == b''

    # Allow seeking back to the beginning
    ds.seekg(0)
    assert ds.readall() == decrypted
