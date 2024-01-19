title = 'Pmw.ComboBoxDialog demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create the dialog.
        self.dialog = Pmw.ComboBoxDialog(parent,
            title = 'My ComboBoxDialog',
            buttons = ('OK', 'Cancel'),
            defaultbutton = 'OK',
            combobox_labelpos = 'n',
            label_text = 'What do you think of Pmw?',
            scrolledlist_items = ('Cool man', 'Cool', 'Good', 'Bad', 'Gross'))
        self.dialog.withdraw()

        # Create button to launch the dialog.
        w = tkinter.Button(parent,
                text = 'Show combo box dialog',
                command = self.doit)
        w.pack(padx = 8, pady = 8)

    def doit(self):
        result = self.dialog.activate()
        print(('You clicked on', result, self.dialog.get()))

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
