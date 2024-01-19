title = 'Pmw.MessageBar demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create and pack the MessageBar.
        self._messagebar = Pmw.MessageBar(parent,
                entry_width = 40,
                entry_relief='groove',
                labelpos = 'w',
                label_text = 'Status:')
        self._messagebar.pack(side = 'bottom', fill = 'x',
                expand = 1, padx = 10, pady = 10)

        # Create and pack the ScrolledListBox to change the MessageBar.
        self.box = Pmw.ScrolledListBox(parent,
                listbox_selectmode='single',
                items=('state', 'help', 'userevent', 'systemevent',
                        'usererror', 'systemerror', 'busy',),
                label_text='Message type',
                labelpos='n',
                selectioncommand=self.selectionCommand)
        self.box.pack(fill = 'both', expand = 'yes', padx = 10, pady = 10)

        self._index = 0
        self._stateCounter = 0

    def selectionCommand(self):
        sels = self.box.getcurselection()
        if len(sels) > 0:
            self._index = self._index + 1
            messagetype = sels[0]
            if messagetype == 'state':
                self._stateCounter = (self._stateCounter + 1) % 3
                text = stateMessages[self._stateCounter]
                if text != '':
                    text = text + ' (' + messagetype + ')'
                self._messagebar.message('state', text)
            else:
                text = messages[messagetype]
                text = text + ' (' + messagetype + ')'
                self._messagebar.message(messagetype, text)
                if messagetype == 'busy':
                    Pmw.showbusycursor()
                    self.box.after(2000)
                    Pmw.hidebusycursor()
                    self._messagebar.resetmessages('busy')
                    text = 'All files successfully removed'
                    text = text + ' (userevent)'
                    self._messagebar.message('userevent', text)


messages = {
    'help': 'Save current file',
    'userevent': 'Saving file "foo"',
    'busy': 'Busy deleting all files from file system ...',
    'systemevent': 'File "foo" saved',
    'usererror': 'Invalid file name "foo/bar"',
    'systemerror': 'Failed to save file: file system full',
}

stateMessages = {
    0: '',
    1: 'Database is down',
    2: 'Waiting for reply from database',
}

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
