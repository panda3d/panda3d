# Tests for Pmw megawidgets

import tkinter
import Test
import Pmw

Test.initialise()

class TestWidget(Pmw.MegaWidget):

    def __init__(self, parent = None, **kw):
        # Define the megawidget options.
        optiondefs = ()
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.MegaWidget.__init__(self, parent)

        # Create the components.
        interior = self.interior()
        self._label = self.createcomponent('label',
                (), None,
                tkinter.Label, (interior,), text = 'test')
        self._label.pack(side='left', padx=2)

        # Check keywords and initialise options.
        self.initialiseoptions(TestWidget)

    def addComponent(self, nickname):
        self.createcomponent(nickname,
                (), None,
                TestComponent, (self.interior(),), status = 'create')

    def addTestWidget(self, widget, option, value):
        w = self.createcomponent('test',
                (), None,
                widget, (self.interior(),))
        self.configure(*(), **{'test_' + option : value})
        if w.__class__.__name__ not in ('Menu', 'Toplevel'):
            w.pack()
        if hasattr(widget, 'geometry'):
            w.geometry('+100+100')
        return str(w.cget(option))

    def deleteTestWidget(self):
        w = self.component('test')
        if hasattr(widget, 'pack'):
            w.pack_forget()
        self.destroycomponent('test')

    def packComponent(self, nickname):
        self.component(nickname).pack()

    def getStatusList(self, nickname):
        return self.component(nickname)._statusList

    def componentOption(self, nickname, option):
        return self.component(nickname).cget(option)

class TestComponent(Pmw.MegaWidget):

    def __init__(self, parent = None, **kw):
        # Define the megawidget options.
        optiondefs = (
            ('status', '', self._status),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.MegaWidget.__init__(self, parent)

        # Create the components.
        interior = self.interior()
        self._label = self.createcomponent('label',
                (), None,
                tkinter.Label, (interior,), text = 'test')
        self._label.pack(side='left', padx=2)

        self._statusList = []

        # Check keywords and initialise options.
        self.initialiseoptions()

    def action(self, info):
        self._statusList.append(info)

    def _status(self):
        self._statusList.append(self['status'])

c = TestWidget
tests = (
  (c.pack, ()),
  (c.components, (), ['hull', 'label']),

  (c.addComponent, 'k0'),
  (c.getStatusList, 'k0', ['create']),
  (c.packComponent, 'k0'),
  ('k0_status', 'foo'),
  (c.getStatusList, 'k0', ['create', 'foo']),

  (c.addComponent, 'k1'),
  (c.packComponent, 'k1'),
  ('k1_status', 'bar'),
  (c.getStatusList, 'k1', ['create', 'bar']),

  (c.addComponent, 'k2'),
  (c.packComponent, 'k2'),
  ('k2_label_foreground', 'green'),
  ('hull_cursor', 'gumby'),
  ('k2_label_cursor', 'gumby'),
  (c.componentOption, ('k2', 'label_foreground'), 'green'),
  (c.componentOption, ('k2', 'label_cursor'), 'gumby'),

  (c.addComponent, 'k3'),
  (c.packComponent, 'k3'),
  ('k3_label_foreground', 'red'),
  ('hull_background', 'white'),
  ('k3_label_background', 'white'),
  ('hull_cursor', 'dot'),
  ('k3_label_cursor', 'dot'),
  (c.componentOption, ('k3', 'label_foreground'), 'red'),
  (c.componentOption, ('k3', 'label_background'), 'white'),
  (c.componentOption, ('k3', 'label_cursor'), 'dot'),
  ('label_background', 'white'),
  (c.destroycomponent, 'k0'),
  (c.destroycomponent, 'k1'),
  (c.destroycomponent, 'k2'),
  (c.destroycomponent, 'k3'),
)

# Test each of the standard widgets as components.
for widget in [tkinter.Button, tkinter.Checkbutton, tkinter.Entry, tkinter.Label, tkinter.Listbox, \
  tkinter.Menu, tkinter.Menubutton, tkinter.Message, tkinter.Radiobutton, tkinter.Scale, tkinter.Text]:
    tests = tests + (
      (c.addTestWidget, (widget, 'foreground', 'blue'), 'blue'),
      (c.deleteTestWidget, ())
    )

for widget in [tkinter.Canvas, tkinter.Frame, tkinter.Scrollbar, tkinter.Toplevel]:
    tests = tests + (
      (c.addTestWidget, (widget, 'background', 'grey80'), 'grey80'),
      (c.deleteTestWidget, ())
    )

# Test the logical fonts.
for fontName in Pmw.logicalfontnames():
    for sizeIncr in (-2, 0, 1, 2, 4, 8):
        font = Pmw.logicalfont(fontName, sizeIncr)
        tests = tests + (
          ('label_text', 'Testing font\n' + fontName + ' ' + str(sizeIncr)),
          ('label_font', font),
        )
fontList = (
  (('Helvetica', 0), {}),
  (('Times', 0), {}),
  (('Typewriter', 0), {}),
  (('Typewriter', 0), {'width' : 'condensed'}),
  (('Typewriter', -1), {'width' : 'condensed'}),
  (('Fixed', 0), {}),
  (('Fixed', 0), {'width' : 'condensed'}),
  (('Fixed', -1), {'width' : 'condensed'}),
  (('Helvetica', 2), {'slant' : 'italic'}),
  (('Helvetica', 0), {'size' : 18}),
  (('Helvetica', 0), {'weight' : 'bold'}),
  (('Helvetica', 12), {'weight' : 'bold', 'slant' : 'italic'}),
  (('Typewriter', 0), {'size' : 8, 'weight' : 'bold'}),
  (('Fixed', 0), {'size' : 8, 'weight' : 'bold'}),
  (('Times', 0), {'size' : 24, 'weight' : 'bold', 'slant' : 'italic'}),
)

for args, dict in fontList:
    font = Pmw.logicalfont(*args, **dict)
    tests = tests + (
        ('label_text', 'Testing font\n' + str(args) + '\n' + str(dict)),
        ('label_font', font),
    )

testData = ((c, ((tests, {'label_text' : 'Testing Pmw base classes'}),)),)

if __name__ == '__main__':
    Test.runTests(testData)
