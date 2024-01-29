title = 'Pmw.PromptDialog demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

# This may demonstrate a bug in Tk.  Click on Cancel in the confirm
# dialog and then click on OK in the password dialog.  Under Solaris
# 2.5 and python 1.5, the Cancel button in the confirm dialog is still
# displayed active, that is, it has a lighter background.

class Demo:
    def __init__(self, parent):
        # Create the dialog to prompt for the password.
        self.dialog = Pmw.PromptDialog(parent,
            title = 'Password',
            label_text = 'Password:',
            entryfield_labelpos = 'n',
            entry_show = '*',
            defaultbutton = 0,
            buttons = ('OK', 'Cancel'),
            command = self.execute)
        self.dialog.withdraw()

        # Create the confirmation dialog.
        self.confirm = Pmw.MessageDialog(
            title = 'Are you sure?',
            message_text = 'Are you really sure?',
            defaultbutton = 0,
            buttons = ('OK', 'Cancel'))
        self.confirm.withdraw()

        # Create button to launch the dialog.
        w = tkinter.Button(parent, text = 'Show prompt dialog',
                command = self.dialog.activate)
        w.pack(padx = 8, pady = 8)

    def execute(self, result):
        if result is None or result == 'Cancel':
            print('Password prompt cancelled')
            self.dialog.deactivate(result)
        else:
            result = self.confirm.activate()
            if result == 'OK':
                print(('Password entered ' + self.dialog.get()))
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
