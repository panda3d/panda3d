import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.ScrolledField

kw_1 = {'labelpos': 'nw'}
tests_1 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
  (Test.num_options, (), 4),
  ('text', 'Hello World'),
  ('label_textvariable', Test.stringvar),
  ('label_textvariable', ''),
  ('label_text', 'Label'),
  ('label_font', Test.font['small']),
  ('label_image', Test.flagup),
  ('label_image', ''),
  ('entry_foreground', 'red'),
  ('text', 'Foo'),
  ('entry_font', Test.font['small']),
)

alltests = (
  (tests_1, kw_1),
)

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
