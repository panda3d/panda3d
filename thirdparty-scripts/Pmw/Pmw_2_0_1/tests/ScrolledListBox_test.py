# Based on iwidgets2.2.0/tests/scrolledlistbox.test code.

import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.ScrolledListBox

def _testYView(doBottom):
    w = Test.currentWidget()
    top, bottom = w.yview()
    if type(top) != type(0.0) or type(bottom) != type(0.0):
        return 'bad type ' + str(top) + ' ' + str(bottom)
    if doBottom:
        if bottom != 1.0:
            return 'bottom is ' + str(bottom)
    else:
        if top != 0.0:
            return 'top is ' + str(top)

kw_1 = {
  'labelpos': 'n',
  'label_text': 'Start',
  'listbox_height' : 20,
  'listbox_width' : 40
}
tests_1 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
  (Test.num_options, (), 9),
  ('label_text', 'ScrolledListBox'),
  ('listbox_height', 6),
  ('listbox_width', 20),
  ('listbox_borderwidth', 3),
  ('hscrollmode', 'none'),
  ('hscrollmode', 'static'),
  ('hscrollmode', 'dynamic'),
  (c.delete, (0, 'end')),
  (c.insert, ('end', 'Hello', 'World')),
  ('listbox_relief', 'raised'),
  ('listbox_relief', 'sunken'),
  ('Scrollbar_width', 20),
  ('Scrollbar_width', 15),
  ('listbox_background', 'GhostWhite'),
  ('listbox_selectborderwidth', 3),
  ('listbox_selectforeground', 'blue'),
  ('listbox_selectmode', 'browse'),
  ('listbox_selectmode', 'extended'),
  ('listbox_selectmode', 'single'),
  ('listbox_selectmode', 'multiple'),
  ('listbox_font', Test.font['small']),
  ('vscrollmode', 'none'),
  ('vscrollmode', 'static'),
  ('vscrollmode', 'dynamic'),
  ('listbox_width', 30),
  ('listbox_height', 20),
  ('vscrollmode', 'bogus', 'ValueError: bad vscrollmode option "bogus": ' + \
    'should be static, dynamic, or none'),
  ('hscrollmode', 'bogus', 'ValueError: bad hscrollmode option "bogus": ' + \
      'should be static, dynamic, or none'),
  (c.cget, 'vscrollmode', 'bogus'),
  (c.cget, 'hscrollmode', 'bogus'),
  ('vscrollmode', 'dynamic'),
  ('hscrollmode', 'dynamic'),
  (c.insert, (0, 'Test', 'Test', 'Test', 'Test')),
  (c.insert, ('end', 'More Test')),
  (c.delete, 1),
  (c.delete, (0, 3)),
  ('listbox_exportselection', 0),
  (c.select_set, 0),
  (c.select_set, (0, 1)),
  (c.getcurselection, (), ('World', 'More Test')),
  (c.select_clear, (0, 'end')),
  (c.getcurselection, (), ()),
  (c.delete, (0, 'end')),
  (c.get, (0, 'end'), ()),
  (c.insert, ('end', 'Test', 'Test', 'Long String Test')),
  (c.get, (0, 'end'), ('Test', 'Test', 'Long String Test')),
  (c.insert, (0, 'Test', 'Test A')),
  (c.get, (0, 'end'), ('Test', 'Test A', 'Test', 'Test', 'Long String Test')),
  (c.insert, (1, 'Test', 'Test', 'Long String Test')),
  (c.get, (0, 4), ('Test', 'Test', 'Test', 'Long String Test', 'Test A')),
  (c.insert, (5, 'Test', 'Test',
    'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX')),
  (c.get, 7, 'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX'),
  (c.get, 'end', 'Long String Test'),
  (c.size, (), 11),
  (c.delete, (3, 2)),
  (c.size, (), 11),
  (c.delete, (3, 3)),
  (c.size, (), 10),
  (c.clear, ()),
  (c.size, (), 0),
  (c.get, (), ()),
  (c.yview, ('moveto', 0.0)),
  (_testYView, 0),
  (c.insert, ('end', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10')),
  (c.insert, ('end', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10')),
  (c.insert, ('end', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10')),
  (c.insert, ('end', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10')),
  (_testYView, 0),
  (c.yview, ('moveto', 0.02)),
  (c.yview, ('moveto', 0.04)),
  (c.yview, ('moveto', 0.06)),
  (c.yview, ('moveto', 0.08)),
  (c.yview, ('moveto', 0.10)),
  (c.yview, ('moveto', 0.12)),
  (c.yview, ('moveto', 0.14)),
  (c.yview, ('moveto', 0.16)),
  (c.yview, ('moveto', 0.18)),
  (c.yview, ('moveto', 0.20)),
  (c.yview, ('moveto', 0.22)),
  (c.yview, ('moveto', 0.24)),
  (c.yview, ('moveto', 0.26)),
  (c.yview, ('moveto', 0.28)),
  (c.yview, ('moveto', 0.98)),
  (_testYView, 1),
  (c.yview, ('scroll', -1, 'page')),
  (c.yview, ('scroll', -1, 'page')),
  (c.yview, ('scroll', -1, 'page')),
  (_testYView, 0),
  (c.yview, ('scroll', 1, 'page')),
  (c.yview, ('scroll', 1, 'page')),
  (c.yview, ('scroll', 1, 'page')),
  (_testYView, 1),
)

tests_2 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
)

alltests = [(tests_1, kw_1)]

poslist = ('nw', 'n', 'ne', 'en', 'e', 'es', 'se', 's', 'sw', 'ws', 'w', 'wn',)
for pos in poslist:
    kw_2 = {
      'listbox_selectmode' : 'extended',
      'items' : ('Hello', 'Out There', 'World'),
      'vscrollmode' : 'static',
      'hscrollmode' : 'dynamic',
      'label_text' : 'List',
      'labelpos' : pos,
      'scrollmargin': 10,
    }
    alltests.append((tests_2, kw_2))

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
