# Tests for Pmw color handling.

import tkinter
import Test
import Pmw

Test.initialise()
testData = ()

defaultPalette = Pmw.Color.getdefaultpalette(Test.root)

c = tkinter.Button

colors = ('red', 'orange', 'yellow', 'green', 'blue', 'purple', 'white')
normalcolors = list(map(Pmw.Color.changebrightness,
        (Test.root,) * len(colors), colors, (0.85,) * len(colors)))

kw = {}
tests = (
  (Pmw.Color.setscheme, (Test.root, normalcolors[0]), {'foreground' : 'white'}),
)
testData = testData + ((c, ((tests, kw),)),)

for color in normalcolors[1:]:
    kw = {'text' : color}
    tests = (
      (c.pack, ()),
      ('state', 'active'),
    )
    testData = testData + ((c, ((tests, kw),)),)

    kw = {}
    tests = (
      (Pmw.Color.setscheme, (Test.root, color), {'foreground' : 'red'}),
    )
    testData = testData + ((c, ((tests, kw),)),)

# Restore the default colors.
kw = {}
tests = (
  (Pmw.Color.setscheme, (Test.root,), defaultPalette),
)
testData = testData + ((c, ((tests, kw),)),)

if __name__ == '__main__':
    Test.runTests(testData)
