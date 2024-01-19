# Based on iwidgets2.2.0/tests/promptdialog.test code.

import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.PromptDialog

kw_1 = {
    'entryfield_labelpos': 'n',
    'label_text' : 'Please enter your password',
    'buttons' : ('OK', 'Cancel', 'Help'),
}
tests_1 = (
  (Test.num_options, (), 11),
  (c.title, 'PromptDialog 1', ''),
  ('hull_background', '#d9d9d9'),
  ('hull_cursor', 'gumby'),
  ('entry_exportselection', 1),
  ('entry_foreground', 'Black'),
  ('entry_background', 'GhostWhite'),
  ('entry_insertbackground', 'Black'),
  ('entry_insertborderwidth', 1),
  ('entry_insertborderwidth', 0),
  ('entry_insertofftime', 400),
  ('entry_insertontime', 700),
  ('entry_insertwidth', 3),
  ('label_text', 'Label'),
  ('entry_justify', 'left'),
  ('entry_relief', 'sunken'),
  ('entry_state', 'disabled'),
  ('entry_state', 'normal'),
  ('entry_background', 'GhostWhite'),
  ('entryfield_validate', 'numeric'),
  ('entryfield_validate', 'alphabetic'),
  ('entryfield_validate', 'alphanumeric'),
  ('entry_width', 30),
  (c.interior, (), tkinter.Frame),
  (c.insertentry, ('end', 'Test String')),
  (c.get, (), 'Test String'),
  (c.deleteentry, (0, 'end')),
  (c.insertentry, ('end', 'Another Test')),
  (c.icursor, 'end'),
  (c.indexentry, 'end', 12),
  (c.selection_from, 0),
  (c.selection_to, 'end'),
  (c.xview, 3),
  (c.clear, ()),
  (c.get, (), ''),
  ('label_bitmap', 'warning'),
  ('label_image', Test.flagup),
  ('entry_font', Test.font['variable']),
  ('entry_foreground', 'red'),
  ('label_image', ''),
  ('label_bitmap', ''),
  (c.title, 'PromptDialog 1: new title', ''),
  ('defaultbutton', 'OK'),
)

kw_2 = {
    'buttonboxpos': 'e',
    'entryfield_labelpos': 'w',
    'label_text' : 'Please enter your password',
    'buttonbox_pady': 25,
    'buttons' : ('OK', 'Cancel'),
}
tests_2 = (
  (c.title, 'PromptDialog 2', ''),
)

alltests = (
  (tests_1, kw_1),
  (tests_2, kw_2),
)

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
