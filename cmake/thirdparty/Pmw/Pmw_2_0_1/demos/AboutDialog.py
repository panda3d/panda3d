title = 'Pmw.AboutDialog demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create dialog.
        Pmw.aboutversion('9.9')
        Pmw.aboutcopyright('Copyright My Company 1999\nAll rights reserved')
        Pmw.aboutcontact(
            'For information about this application contact:\n' +
            '  My Help Desk\n' +
            '  Phone: +61 2 9876 5432\n' +
            '  email: help@my.company.com.au'
        )
        self.about = Pmw.AboutDialog(parent, applicationname = 'My Application')
        self.about.withdraw()

        # Create button to launch the dialog.
        w = tkinter.Button(parent, text = 'Show about dialog',
                command = self.execute)
        w.pack(padx = 8, pady = 8)

    def execute(self):
        self.about.show()

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
