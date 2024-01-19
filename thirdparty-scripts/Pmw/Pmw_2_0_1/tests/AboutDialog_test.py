# Based on iwidgets2.2.0/tests/messagedialog.test code.

import tkinter
import Test
import Pmw

Test.initialise()

Pmw.aboutversion('2.0')
Pmw.aboutcopyright('Copyright Really Good Software')
Pmw.aboutcontact('For information about this application contact\nyour' +
        'system administrator')

c = Pmw.AboutDialog

kw_1 = {
    'applicationname' : 'Really Good Application',
    'buttonbox_padx': 30,
}
tests_1 = (
  (Test.num_options, (), 14),
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
  (c.title, 'AboutDialog 1: new title', ''),
  (c.interior, (), tkinter.Frame),
  (Pmw.aboutcontact, ''),
)

kw_2 = {
    'applicationname' : 'Another Really Good Application',
    'buttonboxpos': 'e',
    'iconpos': 'n',
    'borderx': 15,
    'bordery': 15,
    'separatorwidth': 5,
}
tests_2 = (
  (c.title, 'AboutDialog 2', ''),
)

alltests = (
  (tests_1, kw_1),
  (tests_2, kw_2),
)

testData = ((c, alltests),)
if __name__ == '__main__':
    Test.runTests(testData)
