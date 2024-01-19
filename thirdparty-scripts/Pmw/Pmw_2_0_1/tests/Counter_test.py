import tkinter
import Test
import Pmw

Test.initialise()

c = Pmw.Counter

_myValidators = {
    'hello' : (lambda s: s == 'hello', len),
}

kw_1 = {
    'labelpos' : 'w',
    'label_text' : 'Counter:',
    'buttonaspect': 2.0,
    'autorepeat': 0,
    'initwait': 1000,
    'padx': 5,
    'pady': 5,
    'repeatrate': 20,
    'entryfield_value': 'First value',
}
tests_1 = (
    (c.pack, (), {'padx' : 10, 'pady' : 10, 'fill' : 'both', 'expand' : 1}),
    (c.get, (), 'First value'),
    (Test.num_options, (), 12),
    ('Arrow_borderwidth', 10),
    ('hull_background', 'yellow'),
    ('Arrow_background', 'green'),
    ('label_background', 'blue'),
    ('hull_borderwidth', 10),
    ('entryfield_command', Test.callback),
    ('entryfield_errorbackground', 'red'),
    ('hull_cursor', 'gumby'),
    ('datatype', 'time'),
    ('datatype', 'numeric'),
    ('entry_borderwidth', 6),
    ('entry_relief', 'raised'),
    ('entry_exportselection', 0),
    ('entry_foreground', 'blue'),
    ('hull_highlightcolor', 'Red'),
    ('hull_highlightthickness', 2),
    ('increment', 1),
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
    (c.setentry, '69', Pmw.OK),
    ('entry_show', ''),
    ('entry_justify', 'right'),
    ('entry_justify', 'center'),
    ('entry_justify', 'left'),
    ('label_text', 'Label'),
    ('entry_relief', 'raised'),
    ('entry_relief', 'sunken'),
    ('entry_state', 'disabled'),
    ('entry_state', 'normal'),
    ('entry_background', 'GhostWhite'),
    ('entryfield_validate', 'alphabetic'),
    ('entryfield_validate', 'numeric'),
    ('entry_width', 30),
    ('relief', 'bogus', 'KeyError: Unknown option "relief" for Counter'),
    (c.invoke, (), 1),
    (c.interior, (), tkinter.Frame),
    (c.increment, ()),
    (c.get, (), '70'),
    ('increment', 5),
    (c.get, (), '70'),
    (c.decrement, ()),
    (c.get, (), '65'),
    (c.insert, ('end', 2)),
    (c.get, (), '652'),
    (c.invoke, (), 1),
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
    (c.insert, ('end', '100')),
    ('entryfield_validate', {'validator' : 'real', 'min' : 10}),
    (c.setentry, '50', Pmw.OK),
    (c.setentry, 'hello', Pmw.ERROR),
    ('entryfield_extravalidators', _myValidators),
    ('entryfield_validate', 'hello'),
    (c.setentry, 'hello', Pmw.OK),
    (c.setentry, 'foo', Pmw.ERROR),
    (c.valid, (), 1),
    (c.cget, 'entry_background', 'GhostWhite'),
    ('entry_textvariable', Test.stringvar),
    (c.checkentry, (), 0),
    (c.cget, 'entry_background', 'red'),
    ('entryfield_validate', {'validator' : 'date', 'fmt' : 'dmy'}),
    (c.valid, (), 0),
    ('datatype', {'counter' : 'date', 'format' : 'dmy', 'yyyy' : 1}),
    (c.setentry, '22/12/1999', Pmw.OK),
    ('increment', 10),
    (c.increment, ()),
    (c.get, (), '01/01/2000'),

    ('entryfield_validate', {'validator' : 'time', 'min' : '10:00:00'}),
    (c.valid, (), 0),
    ('increment', 60*60),
    ('datatype', {'counter' : 'time'}),
    (c.setentry, '11:00:00', Pmw.OK),
    (c.decrement, ()),
    (c.get, (), '10:00:00'),
    (c.decrement, ()),
    (c.get, (), '10:00:00'),

    ('entryfield_validate', {'validator' : 'time', 'separator' : '.'}),
    (c.valid, (), 0),
    ('datatype', {'counter' : 'time', 'separator' : '.'}),
    (c.setentry, '11.00.00', Pmw.OK),
    (c.decrement, ()),
    (c.get, (), '10.00.00'),

    ('entryfield_validate', {'validator' : 'date', 'fmt' : 'dmy'}),
    (c.valid, (), 0),
    ('increment', 1),
    ('datatype', {'counter' : 'date', 'format' : 'dmy'}),
    (c.setentry, '25/12/99', Pmw.OK),
    (c.decrement, ()),
    (c.get, (), '24/12/99'),

    ('entryfield_validate', {'validator' : 'date', 'separator' : '#@!',
            'max' : '99#@!12#@!26'}),
    (c.valid, (), 0),
    ('datatype', {'counter' : 'date', 'separator' : '#@!'}),
    (c.setentry, '99#@!12#@!25', Pmw.OK),
    (c.increment, ()),
    (c.get, (), '99#@!12#@!26'),
    ('increment', 10),
    (c.increment, ()),
    (c.get, (), '99#@!12#@!26'),  # max exceeded
    ('entryfield_validate', {'validator' : 'date', 'separator' : '#@!',
            'max' : '00#@!01#@!10'}),
    (c.increment, ()),
    (c.get, (), '00#@!01#@!05'),  # max not exceeded
    ('increment', 1),

    ('entryfield_validate', {'validator' : 'date', 'fmt' : 'ymd',
        'separator' : '-' }),
    (c.valid, (), 0),
    ('datatype', {'counter' : 'date', 'format' : 'ymd', 'yyyy' : 1,
        'separator' : '-' }),
    (c.setentry, '1999-12-22', 1),
    ('increment', 10),
    (c.increment, ()),
    (c.get, (), '2000-01-01'),
    ('increment', 1),
)

tests_2 = (
    (c.pack, (), {'padx' : 10, 'pady' : 10}),
)

alltests = [(tests_1, kw_1)]

poslist = ('nw', 'n', 'ne', 'en', 'e', 'es', 'se', 's', 'sw', 'ws', 'w', 'wn',)
for count in range(len(poslist)):
    pos = poslist[count]
    margin = count * 10
    kw_2 = {
      'entry_width' : 12,
      'labelpos' : pos,
      'labelmargin' : margin,
      'label_text' : 'Counter:',
    }
    alltests.append((tests_2, kw_2))

testData = ((c, alltests),)

if __name__ == '__main__':
    Test.runTests(testData)
