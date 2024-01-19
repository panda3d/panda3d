# Based on iwidgets2.2.0/tests/scrolledtext.test code.

import Test
import Pmw

Test.initialise()

c = Pmw.ScrolledText

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

kw_1 = {'labelpos': 'n', 'label_text': 'ScrolledText'}
tests_1 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
  (Test.num_options, (), 10),
  (c.importfile, 'ScrolledText_test.py'),
  ('hull_background', 'aliceblue'),
  ('text_borderwidth', 3),
  ('Scrollbar_borderwidth', 3),
  ('hull_cursor', 'gumby'),
  ('text_exportselection', 0),
  ('text_exportselection', 1),
  ('text_foreground', 'Black'),
  ('text_height', 10),
  ('text_width', 20),
  ('text_insertbackground', 'Black'),
  ('text_insertborderwidth', 1),
  ('text_insertofftime', 200),
  ('text_insertontime', 500),
  ('text_insertwidth', 3),
  ('label_text', 'Label'),
  ('text_relief', 'raised'),
  ('text_relief', 'sunken'),
  ('Scrollbar_repeatdelay', 200),
  ('Scrollbar_repeatinterval', 105),
  ('vscrollmode', 'none'),
  ('vscrollmode', 'static'),
  ('vscrollmode', 'dynamic'),
  ('hscrollmode', 'none'),
  ('hscrollmode', 'static'),
  ('hscrollmode', 'dynamic'),
  ('Scrollbar_width', 20),
  ('text_selectborderwidth', 2),
  ('text_state', 'disabled'),
  ('text_state', 'normal'),
  ('text_background', 'GhostWhite'),
  ('text_wrap', 'char'),
  ('text_wrap', 'none'),
  ('vscrollmode', 'bogus', 'ValueError: bad vscrollmode ' +
    'option "bogus": should be static, dynamic, or none'),
  ('hscrollmode', 'bogus', 'ValueError: bad hscrollmode ' +
    'option "bogus": should be static, dynamic, or none'),
  (c.cget, 'vscrollmode', 'bogus'),
  (c.cget, 'hscrollmode', 'bogus'),
  ('vscrollmode', 'dynamic'),
  ('hscrollmode', 'dynamic'),
  (c.insert, ('end', 'Hello there\n')),
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
  (c.yview, ('scroll', -50, 'page')),
  (_testYView, 0),
  (c.yview, ('scroll', 1, 'page')),
  (c.yview, ('scroll', 50, 'page')),
  (_testYView, 1),
  (c.clear, ()),
  (c.get, (), '\n'),
)

kw_2 = {
  'hscrollmode' : 'dynamic',
  'label_text' : 'Label',
  'labelpos' : 'n',
  'scrollmargin': 20,
}
tests_2 = (
  (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
  (c.importfile, 'ScrolledText_test.py'),
  ('text_relief', 'raised'),
  ('text_relief', 'sunken'),
)

alltests = (
  (tests_1, kw_1),
  (tests_2, kw_2),
)

testData = ((Pmw.ScrolledText, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
