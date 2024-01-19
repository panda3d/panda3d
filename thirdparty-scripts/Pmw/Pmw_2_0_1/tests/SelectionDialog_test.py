import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.SelectionDialog

kw_1 = {
    'scrolledlist_labelpos': 'n',
    'label_text' : 'Please select one',
    'buttons' : ('OK', 'Cancel'),
    'buttonbox_padx': 30,
}
tests_1 = (
  (Test.num_options, (), 11),
  ('hull_background', '#d9d9d9'),
  (c.insert, ('end', 'Calling', 'all', 'cars')),
  ('label_bitmap', 'warning'),
  ('hull_cursor', 'gumby'),
  ('label_image', Test.flagup),
  ('listbox_font', Test.font['variable']),
  ('listbox_foreground', 'red'),
  ('listbox_selectmode', 'multiple'),
  ('label_image', ''),
  ('label_bitmap', ''),
  (c.title, 'SelectionDialog 1: new title', ''),
  (c.interior, (), tkinter.Frame),
  ('defaultbutton', 'OK'),
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
)

kw_2 = {
    'buttons' : ('OK', 'Cancel'),
    'buttonboxpos': 'e',
    'scrolledlist_labelpos': 'n',
}
tests_2 = (
  (c.title, 'SelectionDialog 2', ''),
)

alltests = (
  (tests_1, kw_1),
  (tests_2, kw_2),
)

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
