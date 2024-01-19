title = 'Pmw.MessageDialog demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        self.parent = parent

        # Create dialog 1.
        self.dialog1 = Pmw.MessageDialog(parent,
            title = 'Simple message dialog',
            defaultbutton = 0,
            message_text = 'A simple message dialog\nwith no callback.')
        self.dialog1.iconname('Simple message dialog')
        self.dialog1.withdraw()

        # Create dialog 2.
        self.dialog2 = Pmw.MessageDialog(parent,
            title = 'Bell ringing dialog',
            message_text = 'This message dialog\nwill ring the bell ' +
                'when\nyou click on the buttons.',
            iconpos = 'w',
            icon_bitmap = 'error',
            command = self.execute2,
            buttons = ('One', 'Two', 'Three', 'Close'))
        self.dialog2.iconname('Bell ringing dialog')
        self.dialog2.withdraw()

        # Create dialog 3.
        self.dialog3 = Pmw.MessageDialog(parent,
            title = 'Vertical button dialog',
            message_text = 'This message dialog\nhas the buttons on the\n' +
                'right hand side.',
            buttonboxpos = 'e',
            iconpos = 'n',
            icon_bitmap = 'warning',
            buttons = ('Goodbye', 'Au revoir', 'Sayonara', 'Close'),
            defaultbutton = 'Close')
        self.dialog3.iconname('Vertical button dialog')
        self.dialog3.withdraw()

        # Create some buttons to launch the dialogs.
        w = tkinter.Button(parent, text = 'Simple dialog',
                command = lambda self = self:
                        self.dialog1.activate(geometry = 'first+100+100'))
        w.pack(padx = 8, pady = 8)

        w = tkinter.Button(parent, text = 'Bell ringing dialog',
                command = self.dialog2.activate)
        w.pack(padx = 8, pady = 8)

        w = tkinter.Button(parent, text = 'Vertical buttons',
                command = self.dialog3.activate)
        w.pack(padx = 8, pady = 8)

        w = tkinter.Button(parent, text = 'On the fly dialog',
                command = self._createOnTheFly)
        w.pack(padx = 8, pady = 8)

    def execute2(self, result):
        print(('You clicked on', result))
        if result is None:
            self.dialog2.deactivate(result)
        elif result == 'Close':
            self.dialog2.deactivate(result)
        else:
            for count in range({'One': 1, 'Two': 2, 'Three': 3}[result]):
                if count != 0:
                    self.dialog2.after(200)
                self.dialog2.bell()

    def _createOnTheFly(self):
        dialog = Pmw.MessageDialog(self.parent,
            title = 'On the fly dialog',
            defaultbutton = 0,
            buttons = ('OK', 'Apply', 'Cancel', 'Help'),
            message_text = 'This dialog was created when you clicked ' +
                'on the button.')
        dialog.iconname('Simple message dialog')
        result = dialog.activate()

        print(('You selected', result))



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
