title = 'Subclassing Pmw.EntryField'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import string
import time
import types
import tkinter
import Pmw

class SpecialEntry(Pmw.EntryField):

    def __init__(self, parent=None , **kw):

        kw['extravalidators'] = _myValidators
        Pmw.EntryField.__init__(*(self, parent), **kw)
        self._converter = None

    def setentry(self, text):
        # Override Pmw.EntryField.setentry to pass string through
        # the appropriate converter.

        val = self['validate']
        if type(val) == dict:
            val = val['validator']
        if val in _converters:
            text = _converters[val](text, output = 0)
        Pmw.EntryField.setentry(self, text)

    def getentry(self):
        text = self.get()
        val = self['validate']
        if type(val) == dict:
            val = val['validator']
        if val in _converters:
            return _converters[val](text, output = 1)
        else:
            return text

def _date(text):
    return Pmw.datevalidator(text, 'dmy', '.')

def _real(text):
    return Pmw.realvalidator(text, ',')

def _dateconv(text, output = 0):
    # On output, convert from dd.mm.yy to mm-dd-yy.  On input, convert
    # mm-dd-yy to dd.mm.yy and also from +NN+ or -NN- to date NN days
    # before or after today.

    if len(text) == 0:
        return ''
    if output:
        try:
            d = text.split('.')
            return d[1] + '-' + d[0] + '-' + d[2]
        except:
            return text
    else:
        if text[-1] == '+' or text[-1] == '-':
            text = text[:-1]
        if text[0] == '+' or text[0] == '-':
            secondsAhead = int(text) * 3600 * 24
            return time.strftime('%d.%m.%Y',
                    time.localtime(time.time() + secondsAhead))
        try:
            d = text.split('-')
            return d[1] + '.' + d[0] + '.' + d[2]
        except:
            return text

def _realconv(text, output = 0):
    # Convert between DD.DD and DD,DD.

    if output:
        index = text.find(',')
        if index >= 0:
            return text[:index] + '.' + text[index + 1:]
        else:
            return text
    else:
        index = text.find('.')
        if index >= 0:
            return text[:index] + ',' + text[index + 1:]
        else:
            return text


_converters = {
    'real' : _realconv,
    'float8' : _realconv,
    'date' : _dateconv
}

_myValidators = {
    'date' : (_date, lambda s: Pmw.datestringtojdn(s, 'dmy', '.')),
    'real' : (_real, lambda s: float(_realconv(s, 1))),
    'int4' : ('numeric', 'numeric'),
    'oid' : ('int4', 'int4'),
    'float8' : ('real', 'real'),
    'varchar' : ('alphanumeric', 'alphanumeric'),
    'text' : ('alphanumeric', 'alphanumeric'),
}

class Demo:
    def __init__(self, parent):
        # Create and pack the SpecialEntry megawidgets.
        self._any = SpecialEntry(parent,
                labelpos = 'w',
                label_text = 'Text (max 10 chars):',
                validate = {'validator' : 'text', 'max' : 10},
                command = self.execute)
        self._any.setentry('abc')
        self._int = SpecialEntry(parent,
                labelpos = 'w',
                label_text = 'Int4:',
                validate = 'int4')
        self._int.setentry(1)
        self._real = SpecialEntry(parent,
                labelpos = 'w',
                label_text = 'Real (max 2,5e+9):',
                validate = {'validator' : 'real', 'max' : +2.5e+9},
                )
        self._real.setentry('+2.5e+6')
        self._date = SpecialEntry(parent,
                labelpos = 'w',
                label_text = 'Date (dd.mm.yy):',
                validate = 'date',
                modifiedcommand = self.changed
                )
        # Set entry to one week from now, using special intput format.
        self._date.setentry('+7+')

        entries = (self._any, self._int, self._real, self._date)

        for entry in entries:
            entry.pack(fill='x', expand=1, padx=10, pady=5)
        Pmw.alignlabels(entries)

        self._any.component('entry').focus_set()

    def changed(self):
        print(('Text changed, converted value is', self._date.getentry()))

    def execute(self):
        print(('Return pressed, value is', self._any.get()))

    # This implements a custom validation routine.  It simply checks
    # if the string is of odd length.
    def custom_validate(self, text):
        print(('text:', text))
        if len(text) % 2 == 0:
            return -1
        else:
            return 1

######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = tkinter.Tk()
    Pmw.initialise(root)
    root.title(title)

    exitButton = tkinter.Button(root, text = 'Exit', command = root.destroy)
    exitButton.pack(side = 'bottom')
    widget = Demo(root)
    root.mainloop()
