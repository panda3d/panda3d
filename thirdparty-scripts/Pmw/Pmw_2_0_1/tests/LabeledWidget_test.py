# Based on iwidgets2.2.0/tests/labeledwidget.test code.

import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.LabeledWidget

def _addListbox():
    w = Test.currentWidget()
    lb = tkinter.Listbox(w.interior(), relief = 'sunken')
    lb.pack(padx = 10, pady = 10)

def _testalignLabels():
    w = Test.currentWidget()
    return Pmw.alignlabels((w,))

kw_1 = {'labelpos': 'nw'}
tests_1 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
  (_addListbox, ()),
  (Test.num_options, (), 3),
  ('label_textvariable', Test.stringvar),
  ('label_textvariable', ''),
  ('label_text', 'Label'),
  ('label_font', Test.font['small']),
  ('label_image', Test.flagup),
  ('label_image', ''),
  (c.interior, (), tkinter.Frame),
  (_testalignLabels, (), None),
)

kw_2 = {'label_text' : 'ListBox', 'labelpos' : 's'}
tests_2 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
  (_addListbox, ()),
)

alltests = (
  (tests_1, kw_1),
  (tests_2, kw_2),
)

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
