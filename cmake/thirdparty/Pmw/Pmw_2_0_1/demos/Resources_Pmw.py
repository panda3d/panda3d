title = 'Using Tk option database to configure Pmw megawidgets'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        self.parent = parent

        header = tkinter.Label(parent, text = 'Select some Tk option ' +
                'database values from\nthe lists, then click ' +
                '\'Create dialog\' to create\na MessageDialog with ' +
                'these values as defaults.')
        header.pack(padx = 10, pady = 10)

        # Create and pack the ComboBoxes to select options.
        buttons = (
            "('OK',)",
            "('Read', 'Write')",
            "('OK', 'Cancel')",
            "('OK', 'Apply', 'Cancel', 'Help')",
        )

        if tkinter.TkVersion >= 8.4:
            disabledState = 'readonly'
        else:
            disabledState = 'disabled'

        self._buttons = Pmw.ComboBox(parent, label_text = 'buttons:',
                labelpos = 'w',
                entry_state = disabledState,
                scrolledlist_items = buttons)
        self._buttons.pack(fill = 'x', expand = 1, padx = 8, pady = 8)
        self._buttons.selectitem(3)

        buttonboxpos = ('n', 's', 'e', 'w',)
        self._buttonboxpos = Pmw.ComboBox(parent, label_text = 'buttonboxpos:',
                labelpos = 'w',
                entry_state = disabledState,
                scrolledlist_items = buttonboxpos)
        self._buttonboxpos.pack(fill = 'x', expand = 1, padx = 8, pady = 8)
        self._buttonboxpos.selectitem(2)

        pad = ('0', '8', '20', '50',)
        self._pad = Pmw.ComboBox(parent, label_text = 'padx, pady:',
                labelpos = 'w',
                entry_state = disabledState,
                scrolledlist_items = pad)
        self._pad.pack(fill = 'x', expand = 1, padx = 8, pady = 8)
        self._pad.selectitem(1)

        Pmw.alignlabels((self._buttons, self._buttonboxpos, self._pad))

        # Create button to launch the dialog.
        w = tkinter.Button(parent, text = 'Create dialog',
                command = self._createDialog)
        w.pack(padx = 8, pady = 8)

        self.dialog = None

    def _createDialog(self):

        # Set the option database.
        buttons = self._buttons.get()
        buttonboxpos = self._buttonboxpos.get()
        pad = self._pad.get()
        self.parent.option_add('*MessageDialog.buttons', buttons)
        self.parent.option_add('*MessageDialog.buttonboxpos', buttonboxpos)
        self.parent.option_add('*ButtonBox.padx', pad)
        self.parent.option_add('*ButtonBox.pady', pad)

        # Create the dialog.
        if self.dialog is not None:
            self.dialog.destroy()

        text = ('This dialog was created by setting the Tk ' +
                'option database:\n\n  *MessageDialog.buttons: ' + buttons +
                '\n  *MessageDialog.buttonboxpos: ' + buttonboxpos +
                '\n  *ButtonBox.padx: ' + pad +
                '\n  *ButtonBox.pady: ' + pad)
        self.dialog = Pmw.MessageDialog(self.parent,
            defaultbutton = 0,
            title = 'Pmw option database demonstration',
            message_justify = 'left',
            message_text = text)
        self.dialog.iconname('Test dialog')

        # Return the defaults to normal, otherwise all other demos
        # will be affected.
        self.parent.option_add('*MessageDialog.buttons', "('OK',)")
        self.parent.option_add('*MessageDialog.buttonboxpos', 's')
        self.parent.option_add('*ButtonBox.padx', 8)
        self.parent.option_add('*ButtonBox.pady', 8)

######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = tkinter.Tk()
    Pmw.initialise(root, useTkOptionDb = 1)
    root.title(title)

    exitButton = tkinter.Button(root, text = 'Exit', command = root.destroy)
    exitButton.pack(side = 'bottom')
    widget = Demo(root)
    root.mainloop()
