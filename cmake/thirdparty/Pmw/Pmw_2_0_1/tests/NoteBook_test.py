import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.NoteBook
class callbackCollector:
    def __init__(self):
        self.list = []
    def __call__(self, pageName):
        self.list.append(pageName)
    def get(self):
        rtn = self.list
        self.list = []
        return rtn

createCallback = callbackCollector()
raiseCallback = callbackCollector()
lowerCallback = callbackCollector()

def checkCallbacks(clear = 0):
    rtn = createCallback.get(), raiseCallback.get(), lowerCallback.get()
    if not clear:
        return rtn

def _populatePage(pageName):
    w = Test.currentWidget()
    page = w.page(pageName)
    text = tkinter.Text(page)
    text.pack()
    return w.pagenames()[w.index(pageName)]

def _getTopPageName():
    w = Test.currentWidget()
    return w._topPageName

kw_1 = {'tabpos' : None}
tests_1_common = (
  (Test.num_options, (), 7),
  ('createcommand', createCallback),
  ('raisecommand', raiseCallback),
  ('lowercommand', lowerCallback),
  (checkCallbacks, 1),
  (c.index, Pmw.END, 'ValueError: NoteBook has no pages'),
  (c.index, Pmw.SELECT, 'ValueError: NoteBook has no pages'),
  (c.setnaturalsize, ()),
  (c.getcurselection, ()),
  (c.insert, ('Temp', 0), {'page_pyclass' : tkinter.Canvas}, tkinter.Canvas),
  (checkCallbacks, (), (['Temp'], ['Temp'], [])),
  (c.getcurselection, (), 'Temp'),
  (c.setnaturalsize, ()),
  (c.delete, 'Temp'),
  (checkCallbacks, (), ([], [], [])),
  (c.getcurselection, ()),
  (c.insert, ('Temp', Pmw.END), tkinter.Frame),
  (checkCallbacks, (), (['Temp'], ['Temp'], [])),
  (c.delete, 'Temp'),
  (c.add, 'Start', tkinter.Frame),
  ('Start_background', 'green'),
  (c.insert, ('Final', Pmw.END), {'page_background' : 'blue'}, tkinter.Frame),
  (c.insert, ('Middle', 'Final'), tkinter.Frame),
  (c.index, Pmw.SELECT, 0),
  (c.insert, ('First', 'Start'), tkinter.Frame),
  (c.index, Pmw.SELECT, 1),
  (c.getcurselection, (), 'Start'),
  (c.selectpage, Pmw.END),
  (checkCallbacks, (), (['Start', 'Final'], ['Start', 'Final'], ['Start'])),
  (c.index, Pmw.SELECT, 3),
  (c.getcurselection, (), 'Final'),
  (c.recolorborders, ()),
  (c.selectpage, 2),
  (c.index, Pmw.SELECT, 2),
  (c.getcurselection, (), 'Middle'),
  (checkCallbacks, (), (['Middle'], ['Middle'], ['Final'])),
  (c.selectpage, 3),
  (c.selectpage, 2),
  (checkCallbacks, (), ([], ['Final', 'Middle'], ['Middle', 'Final'])),
  (c.selectpage, 'Final'),
  (c.index, Pmw.SELECT, 3),
  (c.getcurselection, (), 'Final'),
  (c.add, 'Last', tkinter.Frame),
  (c.pagenames, (), ['First', 'Start', 'Middle', 'Final', 'Last']),
  (c.setnaturalsize, ()),
  (_populatePage, Pmw.SELECT, 'Final'),
  (_populatePage, 'Middle', 'Middle'),
  (c.setnaturalsize, ()),
  (c.add, 'Start', 'ValueError: Page "Start" already exists.'),
  ('Page_background', 'yellow'),
  (c.index, 1, 1),
  (c.index, 10, 'ValueError: index "10" is out of range'),
  (c.index, Pmw.END, 4),
  (c.index, 'First', 0),
  (c.index, 'Middle', 2),
  (c.index, 'bogus', 'ValueError: bad index "bogus": ' + \
      'must be a name, a number, Pmw.END or Pmw.SELECT'),
  (c.previouspage, ()),
  (c.getcurselection, (), 'Middle'),
  (c.previouspage, 'Start'),
  (c.getcurselection, (), 'First'),
  (c.nextpage, ()),
  (c.getcurselection, (), 'Start'),
  (c.nextpage, 'Middle'),
  (c.getcurselection, (), 'Final'),
  (c.delete, ('First', 'Start', 'Middle', 'Final', 'Last')),
  (c.add, 'Temp', {'page_pyclass' : tkinter.Button}, tkinter.Button),
  (c.delete, 'Temp'),
  (c.add, 'Temp', {'page_pyclass' : tkinter.Text}, tkinter.Text),
  (c.delete, 'Temp'),
  (c.add, 'Temp', {'page_pyclass' : Pmw.ScrolledText,
      'page_vscrollmode' : 'static', 'page_text_state' : 'disabled'},
      Pmw.ScrolledText),
  ('Temp_text_background', 'red'),
  (c.page, 'Temp', Pmw.ScrolledText),
  (c.pagenames, (), ['Temp']),
  (c.getcurselection, (), 'Temp'),
  (c.delete, 'Temp'),
  (c.getcurselection, (), None),
  (c.add, 'Start', tkinter.Frame),
  (c.getcurselection, (), 'Start'),
)

tests_1 = tests_1_common + (
  (_getTopPageName, (), None),
  (c.pack, ()),
  (_getTopPageName, (), 'Start'),
  (c.delete, 'Start'),
) + tests_1_common + (
  (_getTopPageName, (), 'Start'),
  (c.delete, 'Start'),
  (c.pack_forget, ()),
) + tests_1_common + (
  (_getTopPageName, (), None),
  (c.pack, ()),
  (_getTopPageName, (), 'Start'),
)

kw_2 = {
    'tabpos' : None,
    'borderwidth' : 10,
    'pagemargin' : 10,
}

tests_2 = (
  (c.pack, ()),
  ('hull_relief', 'sunken'),
  ('hull_borderwidth', 20),
) + tests_1_common

kw_3 = {}

tests_3 = (
  (c.pack, ()),
) + tests_1_common + (
  ('Tab_background', 'red'),
  (c.add, 'One', tkinter.Frame),
  (c.tab, 'One', tkinter.Button),
)

alltests = (
  (tests_1, kw_1),
  (tests_1, kw_2),
  (tests_1, kw_3),
  (tests_2, kw_1),
  (tests_2, kw_3),
  (tests_3, kw_3),
)

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
