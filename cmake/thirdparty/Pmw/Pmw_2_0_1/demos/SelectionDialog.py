title = 'Pmw.SelectionDialog demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create the dialog.
        self.dialog = Pmw.SelectionDialog(parent,
            title = 'My SelectionDialog',
            buttons = ('OK', 'Cancel'),
            defaultbutton = 'OK',
            scrolledlist_labelpos = 'n',
            label_text = 'What do you think of Pmw?',
            scrolledlist_items = ('Cool man', 'Cool', 'Good', 'Bad', 'Gross'),
            command = self.execute)
        self.dialog.withdraw()

        # Create button to launch the dialog.
        w = tkinter.Button(parent, text = 'Show selection dialog',
                command = self.dialog.activate)
        w.pack(padx = 8, pady = 8)

    def execute(self, result):
        sels = self.dialog.getcurselection()
        if len(sels) == 0:
            print(('You clicked on', result, '(no selection)'))
        else:
            print(('You clicked on', result, sels[0]))
        self.dialog.deactivate(result)

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
