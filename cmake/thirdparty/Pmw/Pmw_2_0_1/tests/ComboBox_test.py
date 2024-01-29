# Based on iwidgets2.2.0/tests/combobox.test code.


import tkinter
import Test
import Pmw


    
Test.initialise()

if tkinter.TkVersion >= 8.4:
    expected1 = 'TclError: bad relief "bogus": must be '
    expected2 = 'TclError: bad state "bogus": must be ' + \
      'disabled, normal, or readonly'
elif tkinter.TkVersion >= 8.3:
    expected1 = 'TclError: bad relief "bogus": must be '
    expected2 = 'TclError: bad state "bogus": must be disabled or normal'
else:
    expected1 = 'TclError: bad relief type "bogus": must be '
    expected2 = 'TclError: bad state value "bogus": must be normal or disabled'

c = Pmw.ComboBox

kw_1a = {
  'labelpos' : 'w',
  'label_text' : 'Label:',
  'autoclear': 1,
  'listheight': 250,
  'scrolledlist_items': ('A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'),
}

kw_1b = {
  'dropdown': 0,
  'entry_state': 'disabled',
  'selectioncommand' : Test.callback1,
  'scrolledlist_items': ('A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'),
  'listheight' : 200,
  'label_text' : 'DropDown:',
  'labelpos' : 'w',
  'listbox_cursor' : 'hand1',
  'unique' : 1
}

tests_1 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
  (Test.num_options, (), 11),
  ('entry_width', 20),
  ('entry_textvariable', Test.stringvar),
  ('label_text', 'ComboBox:'),
  ('label_image', Test.flagup),
  ('label_image', ''),
  ('entry_borderwidth', 10),
  ('entry_borderwidth', 2),
  ('hull_background', 'steelblue'),
  ('entry_foreground', 'white'),
  ('hull_background', 'grey85'),
  ('entry_foreground', 'Black'),
  ('entry_textvariable', ''),
  ('entry_state', 'disabled'),
  ('entry_state', 'normal'),
  ('label_font', Test.font['variable']),
  ('entry_font', Test.font['large']),
  ('entry_font', Test.font['variable']),
  (c.invoke, ()),
  ('hull_cursor', ''),
  ('entry_relief', 'raised'),
  ('entry_relief', 'groove'),
  ('entry_relief', 'flat'),
  ('entry_relief', 'sunken'),
  ('entry_exportselection', 1),
  ('entry_exportselection', 0),
  ('entryfield_invalidcommand', Test.bell),
  ('listbox_cursor', 'hand1'),
  ('listbox_cursor', 'hand2'),
  ('entry_selectbackground', '#b2dfee'),
  ('listbox_selectbackground', 'steelblue'),
  ('entry_selectborderwidth', 1),
  ('entry_selectforeground', 'Black'),
  ('entry_background', 'white'),
  ('entryfield_validate', 'alphabetic'),
  (c.setentry, '1234', 0),
  (c.get, (), 'this is some text'),
  ('entryfield_validate', None),
  ('scrolledlist_hscrollmode', 'dynamic'),
  ('scrolledlist_hscrollmode', 'dynamic'),
  ('scrolledlist_vscrollmode', 'dynamic'),
  ('scrolledlist_vscrollmode', 'dynamic'),
  ('entry_borderwidth', 'bogus', 'TclError: bad screen distance "bogus"'),
  ('entry_cursor', 'bogus', 'TclError: bad cursor spec "bogus"'),
  ('entry_exportselection', 'bogus',
    'TclError: expected boolean value but got "bogus"'),
  ('scrolledlist_hscrollmode', 'bogus',
    'ValueError: bad hscrollmode option "bogus": should be static, dynamic, ' +
        'or none'),
  ('listbox_cursor', 'bogus', 'TclError: bad cursor spec "bogus"'),
  ('entry_relief', 'bogus', expected1 + Test.reliefs),
  ('entry_selectborderwidth', 'bogus', 'TclError: bad screen distance "bogus"'),
  ('entry_state', 'bogus', expected2),
  ('entryfield_validate', 'bogus',
    "ValueError: bad validate value \"bogus\":  must be a function or one of " +
        "the standard validators ('alphabetic', 'alphanumeric', 'date', " +
        "'hexadecimal', 'integer', 'numeric', 'real', 'time') or extra " +
        "validators ()"),
  ('scrolledlist_vscrollmode', 'bogus',
    'ValueError: bad vscrollmode option "bogus": should be static, dynamic, ' +
        'or none'),
  ('entry_width', 'bogus', 'TclError: expected integer but got "bogus"'),
  (c.interior, (), tkinter.Frame),
  (c.setentry, 'This is some text', 1),
  (c.get, (), 'This is some text'),
  (c.get, 2, 'C'),
  (c.get, (2, 4), ('C', 'D', 'E')),
  ('listbox_exportselection', 0),
  (c.selectitem, 3),
  (c.getcurselection, (), ('D',)),
  (c.get, (), 'D'),
  (c.selectitem, 'H'),
  (c.getcurselection, (), ('H',)),
  (c.get, (), 'H'),
  (c.setentry, '', 1),
  (c.size, (), 10),
  (c.get, (), ''),
  (c.delete, (0, 'end')),
  (c.insert, (0, 'Test1', 'Test2', 'Test3', 'Test4')),
  (c.insert, ('end', 'More Test')),
  (c.size, (), 5),
  (c.delete, (1),),
  (c.delete, (0, 2)),
  (c.size, (), 1),
  (c.get, 0, 'More Test'),
  (c.setentry, '', 1),
  (c.get, (), ''),
  (c.selectitem, 'More Test'),
  ( (c.curselection, (), (0,)) if Test.afterTk85() else (c.curselection, (), ('0',)) ),
  (c.delete, (0, 'end')),
  (c.size, (), 0),
  (c.getcurselection, (), ()),
  (c.insert, ('end', 'Test1', 'Test2', 'Really Long String Test')),
  (c.size, (), 3),
  (c.get, 0, 'Test1'),
  (c.get, 2, 'Really Long String Test'),
  (c.insert, (0, 'Test3', 'Test4', 'Really Long String Test')),
  (c.size, (), 6),
  (c.insert, (1, 'Test5', 'Test6', 'Really Long String Test')),
  (c.size, (), 9),
  (c.insert, (5, 'Test7', 'Test8', 'Really Long String Test')),
  (c.size, (), 12),
  (c.see, 0),
  (c.see, 11),
  (c.get, 'end', 'Really Long String Test'),
  (c.selectitem, 5),
  ( (c.curselection, (), (5,)) if Test.afterTk85() else (c.curselection, (), ('5',)) ),
  (c.clear, ()),
  (c.get, (), ''),
  (c.size, (), 0),
)

kw_2 = {
  'fliparrow': 1,
  'history': 1,
  'buttonaspect': 0.5,
  'arrowbutton_relief': 'groove',
  'selectioncommand' : Test.callback,
  'hull_background' : 'red',
  'scrolledlist_items' : (123, 456, 789, 101112),
  'listheight' : 50,
  'label_text' : 'Numeric Simple:',
  'labelpos' : 'w',
  'entryfield_validate' : 'numeric',
  'unique' : 0,
}
tests_2 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
)

alltests = (
  (tests_1, kw_1a),
  (tests_1, kw_1b),
  (tests_2, kw_2),
)

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
