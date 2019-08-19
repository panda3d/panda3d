# coding=utf-8
from direct.gui.DirectEntry import DirectEntry
import sys


def test_entry_destroy():
    entry = DirectEntry()
    entry.destroy()


def test_entry_get():
    entry = DirectEntry()
    assert isinstance(entry.get(), str)


def test_entry_auto_capitalize():
    # Now we can generate the DirectEntry component itself. In normal use, we
    # would pass "autoCapitalize=1" to DirectEntry's constructor in order for
    # DirectEntry._autoCapitalize() to be called upon typing into the entry
    # GUI, however in the case of this unit test where there is no GUI to type
    # into, we don't need to bother doing that; we're just calling the function
    # ourselves anyway.
    entry = DirectEntry()

    # Test DirectEntry._autoCapitalize(). The intended behavior would be that
    # the first letter of each word in the entry would be capitalized, so that
    # is what we will check for:
    entry.set('auto capitalize test')
    entry._autoCapitalize()
    assert entry.get() == 'Auto Capitalize Test'

    # Test DirectEntry._autoCapitalize() with a unicode object this time.
    entry.set(u'àütò çapítalízè ţèsţ')
    assert entry.get() == u'àütò çapítalízè ţèsţ'
    entry._autoCapitalize()
    assert entry.get() == u'Àütò Çapítalízè Ţèsţ'

    # Also test it with a UTF-8 encoded byte string in Python 2.
    if sys.version_info < (3, 0):
        entry.set(u'àütò çapítalízè ţèsţ'.encode('utf-8'))
        assert entry.get() == u'àütò çapítalízè ţèsţ'
        entry._autoCapitalize()
        assert entry.get() == u'Àütò Çapítalízè Ţèsţ'
