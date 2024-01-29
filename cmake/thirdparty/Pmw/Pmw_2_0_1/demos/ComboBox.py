title = 'Pmw.ComboBox demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        parent.configure(background = 'white')

        # Create and pack the widget to be configured.
        self.target = tkinter.Label(parent,
                relief = 'sunken',
                padx = 20,
                pady = 20,
        )
        self.target.pack(fill = 'x', padx = 8, pady = 8)

        # Create and pack the simple ComboBox.
        words = ('Monti', 'Python', 'ik', 'den', 'Holie', 'Grailen', '(Bok)')
        simple = Pmw.ComboBox(parent,
                label_text = 'Simple ComboBox:',
                labelpos = 'nw',
                selectioncommand = self.changeText,
                scrolledlist_items = words,
                dropdown = 0,
        )
        simple.pack(side = 'left', fill = 'both',
                expand = 1, padx = 8, pady = 8)

        # Display the first text.
        first = words[0]
        simple.selectitem(first)
        self.changeText(first)

        # Create and pack the dropdown ComboBox.
        colours = ('cornsilk1', 'snow1', 'seashell1', 'antiquewhite1',
                'bisque1', 'peachpuff1', 'navajowhite1', 'lemonchiffon1',
                'ivory1', 'honeydew1', 'lavenderblush1', 'mistyrose1')
        dropdown = Pmw.ComboBox(parent,
                label_text = 'Dropdown ComboBox:',
                labelpos = 'nw',
                selectioncommand = self.changeColour,
                scrolledlist_items = colours,
        )
        dropdown.pack(side = 'left', anchor = 'n',
                fill = 'x', expand = 1, padx = 8, pady = 8)

        # Display the first colour.
        first = colours[0]
        dropdown.selectitem(first)
        self.changeColour(first)

    def changeColour(self, colour):
        print(('Colour: ' + colour))
        self.target.configure(background = colour)

    def changeText(self, text):
        print(('Text: ' + text))
        self.target.configure(text = text)

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
