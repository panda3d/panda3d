title = 'Subclassing Pmw.Counter'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import string
import time
import types
import tkinter
import Pmw

class LabeledDateCounter(Pmw.Counter):

    def __init__(self, parent=None , **kw):

        # Need to use long ints here because on the Macintosh the maximum size
        # of an integer is smaller than the value returned by time.time().
        now = (int(time.time()) / 300) * 300
        text = time.strftime('%y/%m/%d', time.localtime(now))

        kw['datatype'] = 'date'
        kw['entryfield_validate'] = 'date'
        kw['entryfield_value'] = text
        kw['labelpos'] = 'w'

        Pmw.Counter.__init__(*(self, parent), **kw)

class LabeledRealCounter(Pmw.Counter):

    def __init__(self, parent=None , **kw):

        # Define the validate option dictionary.
        validate = {'validator' : 'real', 'min' : 0.0, 'max' : 100.0}

        kw['datatype'] = 'real'
        kw['entryfield_validate'] = validate
        kw['entryfield_value'] = 50.0
        kw['labelpos'] = 'w'

        Pmw.Counter.__init__(*(self, parent), **kw)

class Demo:
    def __init__(self, parent):
        # Create and pack some LabeledDateCounters and LabeledRealCounter.
        self._date1 = LabeledDateCounter(parent, label_text = 'Date:')
        self._date2 = LabeledDateCounter(parent, label_text = 'Another Date:')
        self._real1 = LabeledRealCounter(parent, label_text = 'Real:')
        self._real2 = LabeledRealCounter(parent, label_text = 'Another Real:')

        counters = (self._date1, self._date2, self._real1, self._real2)

        for counter in counters:
            counter.pack(fill='x', expand=1, padx=10, pady=5)
        Pmw.alignlabels(counters)

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
