# Tests for basic Tkinter widgets.

import tkinter
import Test

Test.initialise()
testData = ()

if tkinter.TkVersion >= 8.0:
    button_num = 31
    frame_num = 16
    menu_num = 20
    menubutton_num = 32
else:
    button_num = 30
    frame_num = 15
    menu_num = 19
    menubutton_num = 31

c = tkinter.Button
tests = (
  (c.pack, ()),
  (Test.num_options, (), button_num),
  ('text', 'Hello World'),
  ('background', 'lightsteelblue1'),
  ('foreground', 'seagreen4'),
  ('command', Test.callback),
  (c.flash, ()),
  (c.invoke, (), '1'),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Canvas
tests = (
  (c.pack, ()),
  (Test.num_options, (), 27),
  ('background', 'aliceblue'),
  (c.create_oval, (100, 100, 200, 200),
      {'fill' : 'lightsteelblue1', 'tags' : 'circle'}, 1),
  (c.create_rectangle, (200, 100, 300, 200),
      {'fill' : 'lightsteelblue2', 'tags' : 'square'}, 2),
  (c.create_text, (0, 200),
      {'text' : 'Hello, world', 'tags' : 'words', 'anchor' : 'w'}, 3),
  (c.addtag_withtag, ('lightsteelblue1', 'circle')),
  (c.bbox, ('circle', 'square'), (99, 99, 301, 201)),
  (c.tag_bind, ('circle', '<1>', Test.callback)),
  (c.tag_bind, 'circle', '<Button-1>'),
  (c.tag_unbind, ('circle', '<1>')),
  (c.canvasx, 100, 100.0),
  (c.canvasy, 100, 100.0),
  (c.coords, 'circle', [100.0, 100.0, 200.0, 200.0]),
  (c.coords, ('circle', 0, 0, 300, 300), []),
  (c.coords, 'circle', [0.0, 0.0, 300.0, 300.0]),
  (c.find_withtag, 'lightsteelblue1', (1,)),
  (c.focus, 'circle', ''),
  (c.gettags, 'circle', ('circle', 'lightsteelblue1')),
  (c.icursor, ('words', 7)),
  (c.index, ('words', 'insert'), 7),
  (c.insert, ('words', 'insert', 'cruel ')),
  (c.itemconfigure, 'circle', {'fill': 'seagreen4'}),
  (c.itemcget, ('circle', 'fill'), 'seagreen4'),
  (c.lower, 'words'),
  (c.move, ('square', -50, -50)),
  (c.tkraise, ('words', 'circle')),
  (c.scale, ('circle', 150, 150, 1.0, 0.5)),
  (c.select_from, ('words', 0)),
  (c.select_to, ('words', 'end')),
  (c.delete, 'square'),
  (c.type, 'circle', 'oval'),
  (c.dtag, 'lightsteelblue1'),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Checkbutton
tests = (
  (c.pack, ()),
  (Test.num_options, (), 36),
  ('text', 'Hello World'),
  ('background', 'lightsteelblue1'),
  ('foreground', 'seagreen4'),
  ('command', Test.callback),
  (c.flash, ()),
  (c.invoke, (), '1'),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Entry
tests = (
  (c.pack, ()),
  (Test.num_options, (), 28),
  ('background', 'lightsteelblue1'),
  (c.insert, ('insert', 'Hello, Brian!')),
  (c.delete, (7, 12)),
  (c.icursor, 7),
  (c.insert, ('insert', 'world')),
  (c.get, (), 'Hello, world!'),
  (c.index, 'insert', 12),
  (c.selection_from, 7),
  (c.selection_to, '12'),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Frame
tests = (
  (c.pack, ()),
  (Test.num_options, (), frame_num),
  ('background', 'lightsteelblue1'),
  ('width', 300),
  ('height', 50),
  ('background', 'lightsteelblue1'),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Label
tests = (
  (c.pack, ()),
  (Test.num_options, (), 25),
  ('text', 'Hello World'),
  ('background', 'lightsteelblue1'),
  ('foreground', 'seagreen4'),
  ('image', Test.earthris),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Listbox
tests = (
  (c.pack, ()),
  (Test.num_options, (), 23),
  ('background', 'lightsteelblue1'),
  ('foreground', 'seagreen4'),
  (c.insert, (0, 'ABC', 'DEF', 'GHI', 'XXXXXXXXXXXX')),
  (c.activate, 1),
  (c.select_set, (2, 3)),
  (c.curselection, (), ('2', '3')),
  (c.delete, 1),
  (c.get, 1, 'GHI'),
  (c.get, (0, 1), ('ABC', 'GHI')),
  (c.index, 'end', 3),
  (c.nearest, 1, 0),
  (c.see, 1),
  (c.size, (), 3),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Menu
tests = (
  (Test.num_options, (), menu_num),
  ('background', 'lightsteelblue1'),
  ('foreground', 'seagreen4'),
  (c.add_command, (),
      {'background': 'lightsteelblue2', 'label': 'Hello World'}),
  (c.add_checkbutton, (),
      {'background': 'lightsteelblue2', 'label': 'Charm'}),
  (c.post, (100, 100)),
  (c.activate, 1),
  (c.entryconfigure, 'Hello World', {'background': 'aliceblue'}),
  (c.entrycget, ('Hello World', 'background'), 'aliceblue'),
  (c.index, 'end', 2),
  ('tearoff', 0),
  (c.index, 'end', 1),
  (c.insert_radiobutton, 'Charm',
      {'background': 'lightsteelblue2', 'label': 'Niceness',
          'command': Test.callback}),
  (c.invoke, 'Niceness', '1'),
  (c.delete, 'Charm'),
  (c.type, 'Hello World', 'command'),
  (c.yposition, 'Hello World', 2),
  (c.unpost, ()),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Menubutton
tests = (
  (c.pack, ()),
  (Test.num_options, (), menubutton_num),
  ('text', 'Hello World'),
  ('background', 'lightsteelblue1'),
  ('foreground', 'seagreen4'),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Message
tests = (
  (c.pack, ()),
  (Test.num_options, (), 21),
  ('text', 'Hello World'),
  ('background', 'lightsteelblue1'),
  ('foreground', 'seagreen4'),
  ('text', 'Hello\nCruel Cruel World'),
  ('borderwidth', 100),
  ('justify', 'center'),
  ('justify', 'right'),
  ('justify', 'left'),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Radiobutton
tests = (
  (c.pack, ()),
  (Test.num_options, (), 35),
  ('text', 'Hello World'),
  ('value', 'Foo Bar'),
  ('variable', Test.stringvar),
  ('background', 'lightsteelblue1'),
  ('foreground', 'seagreen4'),
  ('text', 'Hello\nCruel Cruel World'),
  ('command', Test.callback),
  (c.select, ()),
  (Test.stringvar.get, (), 'Foo Bar'),
  (c.flash, ()),
  (c.invoke, (), '1'),
  (c.deselect, ()),
  (Test.stringvar.get, (), ''),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Scale
tests = (
  (c.pack, ()),
  (Test.num_options, (), 33),
  ('showvalue', 1),
  ('orient', 'horizontal'),
  ('from', 100.0),
  ('to', 200.0),
  ('variable', Test.floatvar),
  ('background', 'lightsteelblue1'),
  ('foreground', 'seagreen4'),
  ('command', Test.callback1),
  (c.set, 150.0),
  (c.get, (), 150.0),
  (c.get, 123, 'TypeError: too many arguments; expected 1, got 2'),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Scrollbar
tests = (
  (c.pack, (), {'fill': 'x'}),
  (Test.num_options, (), 20),
  ('orient', 'horizontal'),
  (Test.set_geom, (300, 50)),
  (c.set, (0.3, 0.7)),
  ('background', 'lightsteelblue1'),
  ('troughcolor', 'aliceblue'),
  (c.get, (), (0.3, 0.7)),
  (c.activate, 'slider'),
  (c.set, (0.5, 0.9)),
  (c.delta, (0, 0), 0),
  (c.fraction, (0, 0), 0),
)
testData = testData + ((c, ((tests, {}),)),)

c = tkinter.Text
tests = (
  (c.pack, ()),
  (Test.num_options, (), 35),
  ('background', 'lightsteelblue1'),
  (c.insert, ('end', 'This little piggy is bold.', 'bold', '\n')),
  (c.insert, ('end', 'This little piggy is in green.', 'green', '\n')),
  (c.insert, ('end', 'This line is a mistake.\n')),
  (c.insert, ('end', 'This little piggy is crossed out.', 'overstrike', '\n')),
  (c.insert, ('end', 'This little piggy is raised.', 'raised', '\n')),
  (c.insert, ('end', 'This little piggy is underlined.', 'underline', '\n')),
  (c.tag_configure, 'bold', {'font': Test.font['variable']}),
  (c.tag_configure, 'green', {'background': 'seagreen1'}),
  (c.tag_configure, 'overstrike', {'overstrike': 1}),
  (c.tag_configure, 'raised',
      {'background': 'aliceblue', 'borderwidth': 2, 'relief': 'raised'}),
  (c.tag_configure, 'underline', {'underline': 1}),
  (c.compare, ('2.0', '<', 'end'), 1),
  (c.delete, ('3.0', '4.0')),
  (c.get, ('1.0', '1.4'), 'This'),
  (c.index, 'end', '7.0'),
  (c.mark_set, ('my_mark', '4.9')),
  (c.mark_gravity, ('my_mark', 'right'), ''),
  (c.mark_gravity, 'my_mark', 'right'),
  (c.mark_names, (), ('my_mark', 'insert', 'current')),
  (c.mark_unset, 'my_mark'),
  (c.insert, ('end', '\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n')),
  (c.insert, ('end', 'This is the last line.')),
  (c.scan_mark, (0, 20)),
  (c.scan_dragto, (0, 0)),
  (c.scan_dragto, (0, 20)),
  (c.tag_add, ('green', '1.0', '1.4')),
  (c.tag_cget, ('raised', 'background'), 'aliceblue'),
  (c.tag_lower, 'green'),
  (c.tag_names, (),
      ('green', 'sel', 'bold', 'overstrike', 'raised', 'underline')),
  (c.tag_nextrange, ('raised', '0.0'), ('4.0', '4.28')),
  (c.tag_raise, 'green'),
  (c.tag_ranges, 'green', ('1.0', '1.4', '2.0', '2.30')),
  (c.tag_remove, ('green', '1.0', '1.4')),
  (c.tag_ranges, 'green', ('2.0', '2.30')),
  (c.tag_delete, 'green'),
  (c.search, ('Gre.n', '0.0'), {'regexp': 1, 'nocase': 1}, '2.24'),
  (c.search, ('Gre.n', '3.0', 'end'), {'regexp': 1, 'nocase': 1}, ''),
  (c.see, 'end'),
  (c.see, '0.0'),
)
testData = testData + ((c, ((tests, {}),)),)

#=============================================================================

# Grid command

def _makeGridButtons():
    w = Test.currentWidget()
    b1 = tkinter.Button(w, text = 'Button 1')
    b2 = tkinter.Button(w, text = 'Button 2')
    b3 = tkinter.Button(w, text = 'Button 3')
    b4 = tkinter.Button(w, text = 'Button 4')
    b5 = tkinter.Button(w, text = 'Button 5')
    b6 = tkinter.Button(w, text = 'Button 6')
    b7 = tkinter.Button(w, text = 'Button 7')
    b8 = tkinter.Button(w, text = 'Button 8')

    b1.grid(column=0, row=0)
    b2.grid(column=1, row=0)
    b3.grid(column=2, row=0, ipadx=50, ipady=50, padx=50, pady=50, sticky='nsew')
    b4.grid(column=3, row=0)
    b5.grid(column=0, row=1)
    b6.grid(column=2, row=1, columnspan=2, rowspan=2, sticky='nsew')
    b7.grid(column=0, row=2)
    b8.grid(column=0, row=3, columnspan=4, padx=50, sticky='ew')

def _checkGridSlaves():
    w = Test.currentWidget()
    return len(w.grid_slaves())

def _checkGridInfo():
    w = Test.currentWidget()
    b8 = w.grid_slaves(column=0, row=3)[0]
    info = b8.grid_info()
    if info['in'] == w:
        rtn = {}
        for key, value in list(info.items()):
            if key != 'in':
                rtn[key] = value
        return rtn
    return 'BAD'

def _checkGridForget():
    w = Test.currentWidget()
    b8 = w.grid_slaves(column=0, row=3)[0]
    b8.grid_forget()
    return w.grid_size()

# The -pad grid option was added in Tk 4.2.
# Could not do columnconfigure(0) before Tk 4.2.
if tkinter.TkVersion >= 4.2:
    padTest = {'pad': 25}
    colTest = {'minsize': 100, 'pad': 25, 'weight': 1}
    rowTest = {'minsize': 100, 'pad': 0, 'weight': 1}
else:
    padTest = {'minsize': 100}
    colTest = 'TclError: wrong # args: should be "grid columnconfigure master index ?-option value...?"'
    rowTest = 'TclError: wrong # args: should be "grid rowconfigure master index ?-option value...?"'

c = tkinter.Frame
tests = (
  (c.pack, (), {'fill': 'both', 'expand': 1}),
  (_makeGridButtons, ()),
  # (c.grid_bbox, (1, 2), (85, 268, 85, 34)),
  (c.grid_columnconfigure, (0, 'minsize'), 0),
  (c.grid_columnconfigure, (0, 'weight'), 0),
  (c.grid_columnconfigure, 0, {'minsize': 100, 'weight': 1}),
  (c.grid_columnconfigure, 0, padTest),
  (c.grid_columnconfigure, 0, {}, colTest),
  (c.grid_columnconfigure, (0, 'minsize'), 100),
  (c.grid_columnconfigure, (0, 'weight'), 1),
  (c.location, (200, 100), (2, 0)),
  (c.grid_propagate, (), 1),
  (c.grid_propagate, 0),
  (c.grid_propagate, (), 0),
  (c.grid_rowconfigure, (0, 'minsize'), 0),
  (c.grid_rowconfigure, (0, 'weight'), 0),
  (c.grid_rowconfigure, 0, {'minsize': 100, 'weight': 1}),
  (c.grid_rowconfigure, 0, {}, rowTest),
  (c.grid_size, (), (4, 4)),
  (_checkGridSlaves, (), 8),
  (_checkGridInfo, (), {}, {'column': '0', 'columnspan': '4',
    'ipadx': '0', 'ipady': '0', 'padx': '50', 'pady': '0',
    'row': '3', 'rowspan': '1', 'sticky': 'ew',
  }),
  (_checkGridForget, (), (4, 3)),
  (_checkGridSlaves, (), 7),
)

testData = testData + ((c, ((tests, {}),)),)

if __name__ == '__main__':
        #Test.setverbose(1)
        #Test.setdelay(1000)
    Test.runTests(testData)
