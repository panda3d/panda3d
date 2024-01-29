import Test
import Pmw

Test.initialise()

c = Pmw.ScrolledFrame

def _createInterior():
    w = Test.currentWidget()
    for i in range(3):
        lb = Pmw.ScrolledListBox(w.interior(),
                items = list(range(20)), listbox_height = 6)
        lb.pack(padx = 10, pady = 10)

def _testYView(doBottom):
    w = Test.currentWidget()
    top, bottom = w.yview()
    if type(top) != type(0.0) or type(bottom) != type(0.0):
        return 'bad type ' + str(top) + ' ' + str(bottom)
    if doBottom:
        if bottom != 1.0:
            return 'bottom is ' + str(bottom)
    else:
        if top != 0.0:
            return 'top is ' + str(top)

kw_1 = {'labelpos': 'n', 'label_text': 'ScrolledFrame'}
tests_1 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
  (Test.num_options, (), 11),
  (_createInterior, ()),
  ('hull_background', 'aliceblue'),
  ('Scrollbar_borderwidth', 3),
  ('hull_cursor', 'gumby'),
  ('label_text', 'Label'),
  ('Scrollbar_repeatdelay', 200),
  ('Scrollbar_repeatinterval', 105),
  ('vscrollmode', 'none'),
  ('vscrollmode', 'static'),
  ('vscrollmode', 'dynamic'),
  ('hscrollmode', 'none'),
  ('hscrollmode', 'static'),
  ('hscrollmode', 'dynamic'),
  ('Scrollbar_width', 20),
  ('vscrollmode', 'bogus', 'ValueError: bad vscrollmode ' +
    'option "bogus": should be static, dynamic, or none'),
  ('hscrollmode', 'bogus', 'ValueError: bad hscrollmode ' +
    'option "bogus": should be static, dynamic, or none'),
  (c.cget, 'vscrollmode', 'bogus'),
  (c.cget, 'hscrollmode', 'bogus'),
  ('vscrollmode', 'dynamic'),
  ('hscrollmode', 'dynamic'),
  (_testYView, 0),
  (c.yview, ('moveto', 0.02)),
  (c.yview, ('moveto', 0.04)),
  (c.yview, ('moveto', 0.06)),
  (c.yview, ('moveto', 0.08)),
  (c.yview, ('moveto', 0.10)),
  (c.yview, ('moveto', 0.12)),
  (c.yview, ('moveto', 0.14)),
  (c.yview, ('moveto', 0.16)),
  (c.yview, ('moveto', 0.18)),
  (c.yview, ('moveto', 0.20)),
  (c.yview, ('moveto', 0.22)),
  (c.yview, ('moveto', 0.24)),
  (c.yview, ('moveto', 0.26)),
  (c.yview, ('moveto', 0.28)),
  (c.yview, ('moveto', 0.98)),
  (_testYView, 1),
  (c.yview, ('scroll', -1, 'page')),
  (c.yview, ('scroll', -1, 'page')),
  (c.yview, ('scroll', -1, 'page')),
  (_testYView, 0),
  (c.yview, ('scroll', 1, 'page')),
  (c.yview, ('scroll', 1, 'page')),
  (c.yview, ('scroll', 1, 'page')),
  (_testYView, 1),
)

kw_2 = {
  'hscrollmode' : 'dynamic',
  'label_text' : 'Label',
  'labelpos' : 'n',
  'scrollmargin': 20,
}
tests_2 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
)

alltests = (
  (tests_1, kw_1),
  (tests_2, kw_2),
)

testData = ((Pmw.ScrolledFrame, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
