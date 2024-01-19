# Based on iwidgets2.2.0/tests/buttonbox.test code.

import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.ButtonBox

kw_1 = {}
tests_1 = (
  (c.pack, ()),
  (c.add, 'Yes', tkinter.Button),
  (c.add, 'No', tkinter.Button),
  (c.setdefault, 'Yes'),
  (c.alignbuttons, ()),
  (Test.num_options, (), 5),
  # button_ options do not exist - there is no 'button' component
  # can not test on added buttons since the framework creates new objects
  # for testing purposes
  ('Button_activebackground', '#ececec'),
  ('Button_activeforeground', 'Black'),
  ('hull_background', '#d9d9d9'),
  ('hull_cursor', 'gumby'),
  ('Button_background', 'aliceblue'),
  ('Button_disabledforeground', '#a3a3a3'),
  ('Button_foreground', 'Black'),
  ('Button_highlightcolor', 'Black'),
  ('Button_highlightthickness', 2),
  (c.index, 0, 0),
  (c.index, Pmw.END, 1),
  (c.index, Pmw.DEFAULT, 0),
  (c.index, 'No', 1),
  (c.index, 'Yes', 0),
  (c.add, 'Maybe', tkinter.Button),
  (c.insert, ('Never', 0), {'text' : 'Never Never'}, tkinter.Button),
  (c.setdefault, 'Never'),
  (c.invoke, 'Yes', ''),
  (c.invoke, (), ''),
  (c.invoke, Pmw.DEFAULT, ''),
  (c.delete, 'Maybe'),
  ('Yes_text', 'YES'),
  (c.index, 12, 'ValueError: index "12" is out of range'),
  (c.index, 'bogus', 'ValueError: bad index "bogus": ' + \
      'must be a name, a number, Pmw.END or Pmw.DEFAULT'),
)

kw_2 = {
    'orient' : 'vertical',
    'padx' : 30,
    'pady' : 30,
    'labelpos' : 'w',
    'label_text' : 'Vertical\nButtonBox',
}
tests_2 = (
  (c.pack, ()),
  (c.add, 'Hello', tkinter.Button),
  (c.insert, ('GoodBye', Pmw.END), tkinter.Button),
  (c.setdefault, 'Hello'),
  (c.setdefault, 'GoodBye'),
  (c.setdefault, None),
  (c.index, Pmw.DEFAULT, 'ValueError: ButtonBox has no default'),
)

alltests = (
  (tests_1, kw_1),
  (tests_2, kw_2),
)

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
