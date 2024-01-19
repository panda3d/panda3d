import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.OptionMenu

kw_1 = {
    'labelpos' : 'nw',
    'label_text' : 'Option Menu:',
    'items' : ('Chips', 'Lollies', 'Junk', 'More junk'),
    'initialitem' : 1,
}
tests_1 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
  (Test.num_options, (), 6),
  (c.getcurselection, (), 'Lollies'),
  (c.index, 'Junk', 2),
  (c.index, 'Nowhere', 'ValueError: bad index "Nowhere": must be ' +
    'a name, a number, Pmw.END or Pmw.SELECT'),
  (c.index, Pmw.END, 3),
  (c.index, Pmw.SELECT, 1),
  (c.index, 1, 1),
  (c.invoke, 'Chips'),
  (c.getcurselection, (), 'Chips'),
  ('command', Test.callback1),
  (c.invoke, (), 'Chips'),
  (c.invoke, 'Lollies', 'Lollies'),
  (c.getcurselection, (), 'Lollies'),
  ('hull_background', 'yellow'),
  ('hull_show', 'X', 'TclError: unknown option "-show"'),
  (c.index, Pmw.SELECT, 1),
  (c.setitems, (('Chips', 'Junk', 'Lollies', 'More junk'),)),
  (c.index, Pmw.SELECT, 2),
  (c.setitems, (('Fruit', 'Vegetables', 'Cereals', 'Legumes'),)),
  (c.index, Pmw.SELECT, 0),
  (c.getcurselection, (), 'Fruit'),
  (c.setitems, (('Vegetables', 'Cereals', 'Legumes'), Pmw.END)),
  (c.getcurselection, (), 'Legumes'),
  (c.index, 'Vegetables', 0),
  (c.invoke, 'Legumes', 'Legumes'),
  ('hull_cursor', 'gumby'),
  ('label_foreground', 'Green'),
  ('label_foreground', 'Black'),
  ('label_highlightcolor', 'Red'),
  ('label_text', 'Label'),
)

testData = ((c, ((tests_1, kw_1),)),)

if __name__ == '__main__':
    Test.runTests(testData)
