title = 'Pmw.EntryField demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import time
import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create and pack the EntryFields.
        self._any = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Any:',
                validate = None,
                command = self.execute)
        self._real = Pmw.EntryField(parent,
                labelpos = 'w',
                value = '55.5',
                label_text = 'Real (10.0 to 99.0):',
                validate = {'validator' : 'real',
                        'min' : 10, 'max' : 99, 'minstrict' : 0},
                modifiedcommand = self.changed)
        self._odd = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Odd length:',
                validate = self.custom_validate,
                value = 'ABC')
        self._date = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Date (in 2000):',
                value = '2000/2/29',
                validate = {'validator' : 'date',
                        'min' : '2000/1/1', 'max' : '2000/12/31',
                        'minstrict' : 0, 'maxstrict' : 0,
                        'fmt' : 'ymd'},
                )
        now = time.localtime(time.time())
        self._date2 = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Date (d.m.y):',
                value = '%d.%d.%d' % (now[2], now[1], now[0]),
                validate = {'validator' : 'date',
                        'fmt' : 'dmy', 'separator' : '.'},
                )
        self._time = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Time (24hr clock):',
                value = '8:00:00',
                validate = {'validator' : 'time',
                        'min' : '00:00:00', 'max' : '23:59:59',
                        'minstrict' : 0, 'maxstrict' : 0},
                )
        self._comma = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Real (with comma):',
                value = '123,456',
                validate = {'validator' : 'real', 'separator' : ','},
                )

        entries = (self._any, self._real, self._odd, self._date, self._date2,
                self._time, self._comma)

        for entry in entries:
            entry.pack(fill='x', expand=1, padx=10, pady=5)
        Pmw.alignlabels(entries)

        self._any.component('entry').focus_set()

    def changed(self):
        print(('Text changed, value is', self._real.getvalue()))

    def execute(self):
        print(('Return pressed, value is', self._any.getvalue()))

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
