import os
import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.PanedWidget

def _setbackground(name, colour):
    w = Test.currentWidget()
    w.pane(name).configure(background = colour)

kw_1 = {'orient': 'horizontal', 'hull_width': 400, 'hull_height': 300}
tests_1 = (
  (c.pack, ()),
  (Test.num_options, (), 5),
  (c.add, 'left', {'min' : 100}, tkinter.Frame),
  (_setbackground, ('left', 'red')),
  (c.add, 'middle', {'max' : 100}, tkinter.Frame),
  (_setbackground, ('middle', 'green')),
  (c.add, 'right', {'size' : 100}, tkinter.Frame),
  (c.insert, ('first', 'middle'), {'size' : 100}, tkinter.Frame),
  (_setbackground, ('right', 'yellow')),
  (_setbackground, ('first', 'blue')),
  (c.delete, 'middle'),
  (c.pane, 'left', tkinter.Frame),
  (c.panes, (), ['left', 'first', 'right']),
  (c.configurepane, 'first', {'size' : 200}),
)

alltests = (
  (tests_1, kw_1),
)

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
