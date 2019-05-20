# coding=utf-8
from direct.gui.DirectEntry import DirectEntry
from panda3d.core import NodePath
import sys
reload(sys)
sys.setdefaultencoding('utf-8')
if sys.version_info >= (3, 0):
    import builtins
else:
    import __builtin__ as builtins


def test_auto_capitalize():
    # DirectEntry expects a global "hidden" NodePath to exist for its
    # onscreenText component, so:
    builtins.hidden = NodePath('hidden')

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

    # Test DirectEntry._autoCapitalize() again, this time with a UTF-8 string.
    entry.set('àütò çapítalízè ţèsţ')
    entry._autoCapitalize()
    assert entry.get() == 'Àütò Çapítalízè Ţèsţ'
