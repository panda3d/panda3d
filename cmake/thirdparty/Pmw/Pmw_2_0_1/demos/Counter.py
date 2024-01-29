title = 'Pmw.Counter demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import string
import time
import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Need to use long ints here because on the Macintosh the maximum size
        # of an integer is smaller than the value returned by time.time().
        now = (int(time.time()) / 300) * 300

        # Create the Counters.
        self._date = Pmw.Counter(parent,
                labelpos = 'w',
                label_text = 'Date (4-digit year):',
                entryfield_value =
                        time.strftime('%d/%m/%Y', time.localtime(now)),
                entryfield_command = self.execute,
                entryfield_validate = {'validator' : 'date', 'format' : 'dmy'},
                datatype = {'counter' : 'date', 'format' : 'dmy', 'yyyy' : 1})

        self._isodate = Pmw.Counter(parent,
                labelpos = 'w',
                label_text = 'ISO-Date (4-digit year):',
                entryfield_value =
                        time.strftime('%Y-%m-%d', time.localtime(now)),
                entryfield_command = self.execute,
                entryfield_validate = {'validator' : 'date', 'format' : 'ymd',
                        'separator' : '-' },
                datatype = {'counter' : 'date', 'format' : 'ymd', 'yyyy' : 1,
                        'separator' : '-' })

        self._time = Pmw.Counter(parent,
                labelpos = 'w',
                label_text = 'Time:',
                entryfield_value =
                        time.strftime('%H:%M:%S', time.localtime(now)),
                entryfield_validate = {'validator' : 'time',
                        'min' : '00:00:00', 'max' : '23:59:59',
                        'minstrict' : 0, 'maxstrict' : 0},
                datatype = {'counter' : 'time', 'time24' : 1},
                increment=5*60)
        self._real = Pmw.Counter(parent,
                labelpos = 'w',
                label_text = 'Real (with comma)\nand extra\nlabel lines:',
                label_justify = 'left',
                entryfield_value = '1,5',
                datatype = {'counter' : 'real', 'separator' : ','},
                entryfield_validate = {'validator' : 'real',
                        'min' : '-2,0', 'max' : '5,0',
                        'separator' : ','},
                increment = 0.1)
        self._custom = Pmw.Counter(parent,
                labelpos = 'w',
                label_text = 'Custom:',
                entryfield_value = specialword[:4],
                datatype = _custom_counter,
                entryfield_validate = _custom_validate)
        self._int = Pmw.Counter(parent,
                labelpos = 'w',
                label_text = 'Vertical integer:',
                orient = 'vertical',
                entry_width = 2,
                entryfield_value = 50,
                entryfield_validate = {'validator' : 'integer',
                        'min' : 0, 'max' : 99}
        )

        counters = (self._date, self._isodate, self._time, self._real,
                self._custom)
        Pmw.alignlabels(counters)

        # Pack them all.
        for counter in counters:
            counter.pack(fill='both', expand=1, padx=10, pady=5)
        self._int.pack(padx=10, pady=5)

    def execute(self):
        print(('Return pressed, value is', self._date.get()))

specialword = 'Monti Python ik den Holie Grailen (Bok)'

def _custom_validate(text):
    if specialword.find(text) == 0:
        return 1
    else:
        return -1

def _custom_counter(text, factor, increment):
    # increment is ignored here.
    if specialword.find(text) == 0:
        length = len(text)
        if factor == 1:
            if length >= len(specialword):
                raise ValueError('maximum length reached')
            return specialword[:length + 1]
        else:
            if length == 0:
                raise ValueError('empty string')
            return specialword[:length - 1]
    else:
        raise ValueError('bad string ' + text)

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
