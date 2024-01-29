import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.CounterDialog

kw_1 = {
    'counter_labelpos': 'n',
    'counter_buttonaspect': 2.0,
    'counter_autorepeat': 0,
    'counter_initwait': 1000,
    'counter_padx': 5,
    'counter_pady': 5,
    'counter_repeatrate': 20,
    'label_text' : 'Counter:',
    'buttons' : ('OK', 'Cancel', 'Help'),
}
tests_1 = (
    (Test.num_options, (), 11),
    ('counter_Arrow_borderwidth', 10),
    ('counter_hull_background', 'yellow'),
    ('command', Test.callback1),
    ('hull_cursor', 'gumby'),
    ('counter_datatype', 'time'),
    ('counter_datatype', 'numeric'),
    ('entry_borderwidth', 6),
    ('entry_relief', 'raised'),
    ('entry_exportselection', 0),
    ('entry_foreground', 'blue'),
    ('hull_highlightcolor', 'Red'),
    ('hull_highlightthickness', 2),
    ('counter_increment', 1),
    ('entry_insertbackground', 'Yellow'),
    ('entry_insertbackground', 'Black'),
    ('entry_insertborderwidth', 1),
    ('entry_insertborderwidth', 0),
    ('entry_insertofftime', 400),
    ('entry_insertontime', 700),
    ('entry_insertwidth', 3),
    ('entryfield_invalidcommand', Test.callback),
    ('entry_show', '*'),
    ('entry_background', 'red'),
    (c.setentry, '69', 1),
    ('entry_justify', 'right'),
    ('entry_justify', 'center'),
    ('entry_justify', 'left'),
    ('label_text', 'Label'),
    ('entry_relief', 'raised'),
    ('entry_relief', 'sunken'),
    ('entry_state', 'disabled'),
    ('entry_state', 'normal'),
    ('entry_background', 'GhostWhite'),
    ('entryfield_validate', 'numeric'),
    ('entryfield_validate', 'alphabetic'),
    ('entry_width', 30),
    ('relief', 'bogus', 'KeyError: Unknown option "relief" for CounterDialog'),
    (c.interior, (), tkinter.Frame),
    (c.insert, ('end', 69)),
    (c.increment, ()),
    (c.decrement, ()),
    ('defaultbutton', 1),
    (c.invoke, (), 'Cancel'),
    (c.clear, ()),
    (c.get, (), ''),
    (c.insert, ('end', 'Test String')),
    (c.get, (), 'Test String'),
    (c.delete, (0, 'end')),
    (c.insert, ('end', 'Another Test')),
    (c.icursor, 'end'),
    (c.index, 'end', 12),
    (c.selection_from, 0),
    (c.selection_to, 'end'),
    (c.xview, '3'),
    (c.clear, ()),
)

kw_2 = {
    'buttonboxpos': 'e',
    'counter_labelpos': 'w',
    'label_text' : 'Another counter',
    'buttonbox_pady': 25,
    'buttons' : ('OK', 'Cancel'),
}
tests_2 = (
    (c.title, 'CounterDialog 2', ''),
)

alltests = (
    (tests_1, kw_1),
    (tests_2, kw_2),
)

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
