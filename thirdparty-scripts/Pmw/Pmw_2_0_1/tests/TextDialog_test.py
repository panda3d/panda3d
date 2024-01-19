import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.TextDialog

kw_1 = {
    'scrolledtext_labelpos': 'n',
    'label_text' : 'Here is the news',
    'buttons' : ('OK', 'Cancel'),
    'buttonbox_padx': 30,
}
tests_1 = (
  (Test.num_options, (), 11),
  ('text_wrap', 'none'),
  ('text_state', 'disabled'),
  ('hull_background', '#d9d9d9'),
  ('label_bitmap', 'warning'),
  ('hull_cursor', 'gumby'),
  ('label_image', Test.flagup),
  ('text_font', Test.font['variable']),
  ('text_foreground', 'red'),
  ('text_padx', 15),
  ('text_pady', 15),
  ('label_image', ''),
  ('label_bitmap', ''),
  (c.title, 'TextDialog 1: new title', ''),
  (c.interior, (), tkinter.Frame),
  ('defaultbutton', 'OK'),
  (c.clear, ()),
  (c.get, (), '\n'),
)

kw_2 = {
    'buttons' : ('OK', 'Cancel'),
    'buttonboxpos': 'e',
    'scrolledtext_labelpos': 'n',
}
tests_2 = (
  (c.title, 'TextDialog 2', ''),
)

alltests = (
  (tests_1, kw_1),
  (tests_2, kw_2),
)

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
