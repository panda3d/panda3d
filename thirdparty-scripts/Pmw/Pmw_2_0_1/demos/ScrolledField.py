title = 'Pmw.ScrolledField demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create and pack the ScrolledField.
        self._field = Pmw.ScrolledField(parent, entry_width = 30,
                entry_relief='groove', labelpos = 'n',
                label_text = 'Scroll the field using the\nmiddle mouse button')
        self._field.pack(fill = 'x', expand = 1, padx = 10, pady = 10)

        # Create and pack a button to change the ScrolledField.
        self._button = tkinter.Button(parent, text = 'Change field',
                command = self.execute)
        self._button.pack(padx = 10, pady = 10)

        self._index = 0
        self.execute()

    def execute(self):
        self._field.configure(text = lines[self._index % len(lines)])
        self._index = self._index + 1

lines = (
  'Alice was beginning to get very tired of sitting by her sister',
  'on the bank, and of having nothing to do:  once or twice she had',
  'peeped into the book her sister was reading, but it had no',
  'pictures or conversations in it, "and what is the use of a book,"',
  'thought Alice "without pictures or conversation?"',
  'Alice\'s Adventures in Wonderland',
  'Lewis Carroll',
)

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
