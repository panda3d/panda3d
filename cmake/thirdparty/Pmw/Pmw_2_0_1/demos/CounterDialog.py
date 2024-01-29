title = 'Pmw.CounterDialog demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import string
import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create the dialog to prompt for the number of times to ring the bell.
        self.dialog = Pmw.CounterDialog(parent,
            label_text = 'Enter the number of times to\n' + \
                    'sound the bell (1 to 5)\n',
            counter_labelpos = 'n',
            entryfield_value = 2,
            counter_datatype = 'numeric',
            entryfield_validate =
                {'validator' : 'numeric', 'min' : 1, 'max' : 5},
            buttons = ('OK', 'Cancel'),
            defaultbutton = 'OK',
            title = 'Bell ringing',
            command = self.execute)
        self.dialog.withdraw()

        # Create button to launch the dialog.
        w = tkinter.Button(parent, text = 'Show counter dialog',
                command = self.dialog.activate)
        w.pack(padx = 8, pady = 8)

    def execute(self, result):
        if result is None or result == 'Cancel':
            print('Bell ringing cancelled')
            self.dialog.deactivate()
        else:
            count = self.dialog.get()
            if not self.dialog.valid():
                print(('Invalid entry: "' + count + '"'))
            else:
                print(('Ringing the bell ' + count + ' times'))
                for num in range(int(count)):
                    if num != 0:
                        self.dialog.after(200)
                    self.dialog.bell()
                self.dialog.deactivate()

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
