title = 'Modal dialog nesting demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create button to launch the dialog.
        w = tkinter.Button(parent, text = 'Show first dialog',
                command = self.showFirstDialog)
        w.pack(padx = 8, pady = 8)

        self.timerId = None

        self.dialog1 = Pmw.MessageDialog(parent,
                message_text = 'This is the first modal dialog.\n' +
                        'You can see how dialogs nest by\n' +
                        'clicking on the "Next" button.',
                title = 'Dialog 1',
                buttons = ('Next', 'Cancel'),
                defaultbutton = 'Next',
                command = self.next_dialog)
        self.dialog1.withdraw()

        self.dialog2 = Pmw.Dialog(self.dialog1.interior(),
                title = 'Dialog 2',
                buttons = ('Cancel',),
                deactivatecommand = self.cancelTimer,
                defaultbutton = 'Cancel')
        self.dialog2.withdraw()
        w = tkinter.Label(self.dialog2.interior(),
            text = 'This is the second modal dialog.\n' +
                'It will automatically disappear shortly')
        w.pack(padx = 10, pady = 10)

    def showFirstDialog(self):
        self.dialog1.activate()

    def cancelTimer(self):
        if self.timerId is not None:
            self.dialog2.after_cancel(self.timerId)
            self.timerId = None

    def deactivateSecond(self):
        self.timerId = None
        self.dialog2.deactivate()

    def next_dialog(self, result):
        if result != 'Next':
            self.dialog1.deactivate()
            return

        self.timerId = self.dialog2.after(3000, self.deactivateSecond)
        self.dialog2.activate()

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
