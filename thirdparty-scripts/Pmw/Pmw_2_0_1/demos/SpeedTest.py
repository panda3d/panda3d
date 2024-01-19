title = 'Test of the speed of creating Pmw megawidgets'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import time
import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        self.parent = parent

        message = 'This is a test of the time\n' + \
                'it takes to create 20 Pmw\nEntryField megawidgets.\n' + \
                'Click on the button to create them.'
        w = tkinter.Label(parent, text = message)
        w.pack(padx = 8, pady = 8)

        # Create button to run speed test.
        w = tkinter.Button(parent,
                text = 'Create 20 EntryFields',
                command = self.createEntries)
        w.pack(padx = 8, pady = 8)

    def createEntries(self):
        entryTop = tkinter.Toplevel(self.parent)

        startClock = time.clock()
        fields = []
        for num in range(20):
            field = Pmw.EntryField(entryTop,
                    labelpos = 'w',
                    label_text='*' + ('*' * num),
                    hull_background = 'lightsteelblue',
                    label_background = 'lightsteelblue',
                    hull_highlightbackground = 'lightsteelblue',
                    label_highlightbackground = 'lightsteelblue',
                    entry_highlightbackground = 'lightsteelblue',
                    entry_background = 'aliceblue')
            field.pack()
            fields.append(field)

        Pmw.alignlabels(fields)
        print(('Time to create 20 EntryFields:', \
                time.clock() - startClock, 'seconds'))

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
