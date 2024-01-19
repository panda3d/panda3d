# Based on iwidgets2.2.0/tests/messagedialog.test code.

import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.MessageDialog

kw_1 = {
    'message_text' : 'Are you sure you want to do that?',
    'buttons' : ('OK', 'Cancel'),
    'icon_bitmap' : 'questhead',
    'iconmargin': '60',
    'iconpos' : 'w',
    'buttonbox_padx': 30,
}
tests_1 = (
  (Test.num_options, (), 13),
  ('message_anchor', 'center'),
  ('message_justify', 'center'),
  ('message_wraplength', 0),
  ('hull_background', '#d9d9d9'),
  ('icon_bitmap', 'warning'),
  ('hull_cursor', 'gumby'),
  ('icon_image', Test.flagup),
  ('message_font', Test.font['variable']),
  ('message_foreground', 'red'),
  ('message_padx', 15),
  ('message_pady', 15),
  ('icon_image', ''),
  (c.title, 'MessageDialog 1: new title', ''),
  (c.interior, (), tkinter.Frame),
  ('defaultbutton', 'OK'),
)

kw_2 = {
    'message_text' : 'On the left',
    'buttons' : ('OK', 'Cancel'),
    'buttonboxpos': 'e',
    'borderx': 55,
    'bordery': 55,
    'separatorwidth': 5,
}
tests_2 = (
  (c.title, 'MessageDialog 2', ''),
)

alltests = (
  (tests_1, kw_1),
  (tests_2, kw_2),
)

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
