# Based on iwidgets2.2.0/tests/entryfield.test code.

import tkinter
import Test
import Pmw

Test.initialise()

_myValidators = {
    'hello' : (lambda s: s == 'hello', len),
}

c = Pmw.EntryField

kw_1 = {'entry_width' : 12, 'labelpos' : 'n', 'label_text' : 'Entry Field:'}
tests_1 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
  (Test.num_options, (), 10),
  ('errorbackground', 'red'),
  ('hull_background', 'yellow'),
  ('label_background', 'yellow'),
  ('entry_background', 'yellow'),
  ('hull_show', 'X', 'TclError: unknown option "-show"'),
  ('entry_show', ''),
  ('entry_borderwidth', 4),
  ('entry_borderwidth', 2),
  ('command', Test.callback),
  ('hull_cursor', 'gumby'),
  ('entry_exportselection', 0),
  ('label_foreground', 'Green'),
  ('entry_foreground', 'Green'),
  ('label_foreground', 'Black'),
  ('entry_foreground', 'Black'),
  ('label_highlightcolor', 'Red'),
  ('entry_highlightcolor', 'Red'),
  ('entry_highlightthickness', 2),
  ('entry_insertbackground', 'Yellow'),
  ('entry_insertbackground', 'Black'),
  ('entry_insertborderwidth', 1),
  ('entry_insertborderwidth', 0),
  ('entry_insertofftime', 400),
  ('entry_insertontime', 700),
  ('entry_insertwidth', 3),
  ('invalidcommand', Test.callback),
  ('entry_justify', 'right'),
  ('entry_justify', 'center'),
  ('entry_justify', 'left'),
  ('label_text', 'Label'),
  ('entry_relief', 'raised'),
  ('entry_relief', 'sunken'),
  ('entry_state', 'disabled'),
  ('entry_state', 'normal'),
  ('entry_background', 'GhostWhite'),
  ('validate', 'numeric'),
  ('validate', 'alphabetic'),
  ('entry_width', 30),
  ('validate', 'bogus',
    "ValueError: bad validate value \"bogus\":  must be a function or one " +
        "of the standard validators ('alphabetic', 'alphanumeric', 'date', " +
        "'hexadecimal', 'integer', 'numeric', 'real', 'time') or extra " +
        "validators ()"),
  ('relief', 'bogus', 'KeyError: Unknown option "relief" for EntryField'),
  (c.invoke, (), 1),
  (c.interior, (), tkinter.Frame),
  (c.clear, ()),
  (c.get, (), ''),
  (c.insert, ('end', 'Test String')),
  (c.get, (), 'Test String'),
  (c.delete, (0, 'end')),
  (c.insert, ('end', 'Another Test')),
  (c.icursor, 'end'),
  (c.index, 'end', 12),
  (c.selection_from, 0),
  (c.selection_to, 'end'),
  (c.xview, '3'),
  (c.clear, ()),
  (c.insert, ('end', '100')),
  ('validate', {'validator' : 'real', 'min' : 10}),
  (c.setentry, '50', 1),
  (c.setentry, 'hello', 0),
  ('extravalidators', _myValidators),
  ('validate', 'hello'),
  (c.setentry, 'hello', 1),
  (c.setentry, 'foo', 0),
  (c.valid, (), 1),
  (c.cget, 'entry_background', 'GhostWhite'),
  ('entry_textvariable', Test.stringvar),
  (c.checkentry, (), 0),
  (c.cget, 'entry_background', 'red'),
)

tests_2 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10}),
)

alltests = [(tests_1, kw_1)]

poslist = ('nw', 'n', 'ne', 'en', 'e', 'es', 'se', 's', 'sw', 'ws', 'w', 'wn',)
for pos in poslist:
    kw_2 = {
      'labelpos' : pos,
      'label_text' : 'Entry Field',
    }
    alltests.append((tests_2, kw_2))

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
