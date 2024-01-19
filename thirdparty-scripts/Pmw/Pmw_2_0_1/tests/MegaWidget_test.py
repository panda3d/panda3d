# Based on itk2.2/tests/widget.test code.

import tkinter
import Test
import Pmw

Test.initialise()

class TestWidget(Pmw.MegaWidget):

    def __init__(self, parent = None, **kw):

        # Define the megawidget options.
        optiondefs = (
            ('status',         '',          self._status),
            ('background',     'linen',     None),
            ('borderwidth',    2,           None),
            ('foreground',     'navy',      None),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.MegaWidget.__init__(self, parent)

        # Create the components.
        interior = self.interior()
        self._label = self.createcomponent('label',
                (), None,
                tkinter.Label, (interior,))
        self._label.pack(side='left', padx=2)

        self._button = self.createcomponent('button',
                (), 'Mygroup',
                tkinter.Button, (interior,), text = 'Push Me',
                activebackground = 'white', background = 'ivory')
        self._button.pack(side='right', fill='x', padx=2)

        # Initialise instance variables.
        self._statusList = []

        # Check keywords and initialise options.
        self.initialiseoptions()

    def statusList(self, val=None):
        if val is None:
            return self._statusList
        else:
            self._statusList = val

    def action(self, info):
        self._statusList.append(info)

    def _status(self):
        self._statusList.append(self['status'])

def _componentOption(component, option):
    w = Test.currentWidget()
    return w.component(component).cget(option)

def _componentInvoke(component):
    w = Test.currentWidget()
    w.component(component).invoke()

def _addComponent():
    w = Test.currentWidget()
    label2 = w.createcomponent('label2',
            (), 'Mygroup',
            tkinter.Label, (w.interior(),),
            text = 'Temporary', background = 'yellow')
    label2.pack(fill = 'x')
    return label2.cget('text')

expectedOptions = {
    'background': ('background', 'background', 'Background', 'linen', 'linen'),
    'borderwidth': ('borderwidth', 'borderwidth', 'Borderwidth', 2, 2),
    'foreground': ('foreground', 'foreground', 'Foreground', 'navy', 'navy'),
    'status': ('status', 'status', 'Status', '', ''),
}

c = TestWidget
tests = (
  # Set status list to a known state, since configure(status) may have
  # been called during contruction.
  (c.statusList, ([''])),
  (c.pack, ()),
  (c.configure, (), {}, expectedOptions),
  (c.configure, ('background'), expectedOptions['background']),
  (c.configure, ('borderwidth'), expectedOptions['borderwidth']),
  (c.configure, ('foreground'), expectedOptions['foreground']),
  (c.configure, ('status'), expectedOptions['status']),
  ('hull_background', 'red'),
  ('label_background', 'red'),
  ('borderwidth', 1),
  ('button_command', Test.callback),
  ('hull_cursor', 'trek'),
  ('label_cursor', 'trek'),
  ('Mygroup_foreground', 'IndianRed'),
  ('button_activebackground', 'MistyRose'),
  ('button_background', 'MistyRose2'),
  ('status', 'test message'),
  ('label_text', 'Label:'),
  (c.components, (), ['button', 'hull', 'label']),
  (c.component, ('hull'), tkinter.Frame),
  (c.component, ('label'), tkinter.Label),
  (c.component, ('button'), tkinter.Button),
  (_componentOption, ('hull', 'cursor'), 'trek'),
  (_componentOption, ('label', 'cursor'), 'trek'),
  (_componentOption, ('hull', 'background'), 'red'),
  (_componentOption, ('label', 'background'), 'red'),
  (_componentOption, ('button', 'background'), 'MistyRose2'),
  (_componentOption, ('label', 'text'), 'Label:'),
  (_componentOption, ('button', 'text'), 'Push Me'),
  (c.statusList, (), ['', 'test message']),
  ('button_command', Test.actioncallback),
  (c.statusList, ([])),
  (_componentInvoke, 'button'),
  ('status', 'in between'),
  (_componentInvoke, 'button'),
  (c.statusList, (), ['button press', 'in between', 'button press']),
  (_addComponent, (), 'Temporary'),
  (c.components, (), ['button', 'hull', 'label', 'label2']),
  (_componentOption, ('label2', 'background'), 'yellow'),
  (c.destroycomponent, ('label2')),
  (c.components, (), ['button', 'hull', 'label']),
)
testData = ((c, ((tests, {}),)),)

if __name__ == '__main__':
    Test.runTests(testData)
