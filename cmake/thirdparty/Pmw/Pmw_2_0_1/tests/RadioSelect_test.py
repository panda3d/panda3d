import tkinter
import Test
import Pmw

Test.initialise()

if tkinter.TkVersion >= 8.4:
    expected1 = 'TclError: bad relief "bogus": must be '
else:
    expected1 = 'TclError: bad relief type "bogus": must be '

c = Pmw.RadioSelect

kw_1 = {'labelpos' : 'nw', 'label_text' : 'Radio Select:'}
tests_1 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
  (Test.num_options, (), 8),
  (c.index, Pmw.END, 'ValueError: RadioSelect has no buttons'),
  (c.add, ('Fruit',), tkinter.Button),
  (c.add, ('Vegetables',), tkinter.Button),
  (c.add, ('CornFlakes',), {'text': 'Cereals'}, tkinter.Button),
  (c.add, ('Legumes',), tkinter.Button),
  (c.add, ('Legumes',), 'ValueError: button "Legumes" already exists'),
  (c.index, 0, 0),
  (c.index, Pmw.END, 3),
  (c.index, 'Vegetables', 1),
  (c.index, 'Fruit', 0),
  (c.index, 12, 'ValueError: index "12" is out of range'),
  (c.index, 'bogus', 'ValueError: bad index "bogus": ' + \
      'must be a name, a number or Pmw.END'),
  ('hull_background', 'yellow'),
  ('hull_show', 'X', 'TclError: unknown option "-show"'),
  ('frame_relief', 'raised'),
  ('frame_borderwidth', 4),
  ('frame_borderwidth', 2),
  ('command', Test.callback1),
  (c.invoke, 'Vegetables', 'Vegetables'),
  ('hull_cursor', 'gumby'),
  ('Button_state', 'disabled'),
  ('Button_background', 'Green'),
  ('Button_cursor', 'watch'),
  ('Button_background', 'grey85'),
  ('label_foreground', 'Green'),
  ('label_foreground', 'Black'),
  ('label_highlightcolor', 'Red'),
  ('Fruit_background', 'red'),
  ('Vegetables_background', 'green'),
  ('CornFlakes_background', 'yellow'),
  ('Legumes_background', 'brown'),
  ('Legumes_foreground', 'white'),
  (c.add, ('Foo',), tkinter.Button),
  ('label_text', 'Label'),
  ('frame_relief', 'sunken'),
  ('frame_relief', 'bogus', expected1 + Test.reliefs),
  (c.deleteall, ()),
)

kw_2 = {
    'labelpos' : 'nw',
    'label_text' : 'Multiple:',
    'selectmode' : 'multiple',
}
tests_2 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
  (c.add, ('Fruit',), tkinter.Button),
  (c.add, ('Vegetables',), tkinter.Button),
  (c.add, ('CornFlakes',), {'text': 'Cereals'}, tkinter.Button),
  (c.add, ('Legumes',), tkinter.Button),
  ('command', Test.callback2),
  (c.getcurselection, (), ()),
  (c.invoke, 'Vegetables', ('Vegetables', 1)),
  (c.getcurselection, (), ('Vegetables',)),
  (c.invoke, 'Legumes', ('Legumes', 1)),
  (c.getcurselection, (), ('Vegetables', 'Legumes')),
  (c.invoke, 'Fruit', ('Fruit', 1)),
  (c.getcurselection, (), ('Vegetables', 'Legumes', 'Fruit')),
  (c.invoke, 'Legumes', ('Legumes', 0)),
  (c.getcurselection, (), ('Vegetables', 'Fruit')),
  (c.deleteall, ()),
  (c.add, ('Fruit',), tkinter.Button),
  (c.add, ('Vegetables',), tkinter.Button),
  (c.invoke, 'Vegetables', ('Vegetables', 1)),
  (c.getcurselection, (), ('Vegetables',)),
)

alltests = [
  (tests_1, kw_1),
  (tests_2, kw_2),
]


tests_3 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10}),
  (c.add, ('Foo',), tkinter.Button),
  (c.add, ('Bar',), tkinter.Button),
)

poslist = ('nw', 'n', 'ne', 'en', 'e', 'es', 'se', 's', 'sw', 'ws', 'w', 'wn',)
for pos in poslist:
    kw_3 = {
      'labelpos' : pos,
      'orient' : 'vertical',
      'padx' : 20,
      'pady' : 20,
      'label_text' : 'Radio Select',
    }
    alltests.append((tests_3, kw_3))

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
