# Based on iwidgets2.2.0/tests/dialog.test code.

import sys
import tkinter
import Test
import Pmw

Test.initialise()

if tkinter.TkVersion >= 8.3:
    version = sys.version_info
    if (version[0] == 2 and version[1] > 0):
        expected1 = "AttributeError: Dialog instance has no attribute 'bogus'"
    elif version[0] >= 3:
        expected1 = "AttributeError: Dialog' object has no attribute 'bogus"
    else:
        expected1 = "AttributeError: 'Dialog' instance has no attribute 'bogus'"
else:
    expected1 = 'AttributeError: bogus'

c = Pmw.Dialog

def _addListbox():
    global _lb
    w = Test.currentWidget()
    _lb = tkinter.Listbox(w.interior(), relief = 'sunken')
    _lb.pack(fill = 'both', expand = 'yes')

def _addListEntry(text):
    _lb.insert('end', text)

def _test_deactivate(result):
    w = Test.currentWidget()
    w.after(Test.delay() + 4000,
        lambda widget=w, r=result: widget.deactivate(r))

def _createOtherToplevel():
    global tempToplevel
    Test.root.deiconify()
    Test.root.geometry('+0+0')
    tempToplevel = tkinter.Toplevel()
    tempToplevel.geometry('+0+0')
    label = tkinter.Label(tempToplevel, text =
        'The cursor should turn to a\n' +
        'clock over this window if the\n' +
        'blt busy command is available.\n' +
        'In any case, the button will be inactive\n' +
        'while the modal dialog is active.')
    label.pack(padx=100, pady=100)
    button = tkinter.Button(tempToplevel, text = 'Try to press me')
    button.pack(pady=100, expand=1)

def _hideOtherToplevel():
    global tempToplevel
    tempToplevel.withdraw()
    Test.root.withdraw()

def _bogus():
    w = Test.currentWidget()
    w.bogus()

kw_1 = {
  'buttons' : (),
  'buttonbox_padx': 30,
}
tests_1 = (
  ('buttons', ('OK',)),
  ('buttons', ('OK', 'Cancel',)),
  ('defaultbutton', 'OK'),
  (_addListbox, ()),
  (Test.num_options, (), 9),
  ('hull_background', '#d9d9d9'),
  ('buttons', ('A', 'B', 'C', 'D')),
  ('hull_cursor', 'gumby'),
  (c.title, 'Dialog Shell', ''),
  (c.interior, (), tkinter.Frame),
  ('buttons', ()),
  ('buttons', ('OK',)),
  ('buttons', ('OK', 'Cancel')),
  ('buttons', ('OK', 'Cancel', 'Help')),
  ('defaultbutton', 'OK'),
  ('buttons', ('Apply', 'OK', 'Cancel', 'Help')),
  ('buttons', ('Apply', 'OK', 'Cancel')),
  ('defaultbutton', 'Cancel'),
  (c.invoke, 'OK', 'None'),
  ('buttonbox_OK_text', 'OOOOOKKKKK'),
  (c.show, ()),
  (c.withdraw, (), ''),
  ('buttons', ('Apply', 'OK', 'Cancel', 'Foo')),
  ('buttons', ('Apply', 'OK', 'Cancel')),
  (c.show, ()),
  (c.withdraw, (), ''),
  (_createOtherToplevel, ()),
  (_addListEntry, 'Testing application activate/deactivate'),
  (_addListEntry, 'Please wait'),
  (_test_deactivate, 'Hello World'),
  (c.activate, (), 'Hello World'),
  ('defaultbutton', ''),
  (_addListEntry, 'Now testing global activate/deactivate'),
  (_addListEntry, 'Please wait a bit more'),
  (_test_deactivate, 'Hello World'),
  (c.activate, (1), 'Hello World'),
  (_hideOtherToplevel, ()),
  ('buttons', ('Apply', 'OK', 'Cancel', 'Foo', '1')),
  (c.show, (), {}),
  (_bogus, (), expected1),
)

kw_2 = {'buttonboxpos' : 'e', 'separatorwidth' : 5}
tests_2 = (
  ('buttons', ('OK',)),
  ('buttons', ('OK', 'Cancel',)),
  ('defaultbutton', 'OK'),
)

alltests = (
  (tests_1, kw_1),
  (tests_2, kw_2),
)

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
