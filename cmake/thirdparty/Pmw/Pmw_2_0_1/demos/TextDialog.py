title = 'Pmw.TextDialog demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create the dialog.
        dialog = Pmw.TextDialog(parent, scrolledtext_labelpos = 'n',
                title = 'My TextDialog',
                defaultbutton = 0,
                label_text = 'Lawyer jokes')
        dialog.withdraw()
        dialog.insert('end', jokes)
        dialog.configure(text_state = 'disabled')

        # Create button to launch the dialog.
        w = tkinter.Button(parent, text = 'Show text dialog',
                command = dialog.activate)
        w.pack(padx = 8, pady = 8)

jokes = """
Q: What do you call 5000 dead lawyers at the bottom of the ocean?
A: A good start!

Q: How can you tell when a lawyer is lying?
A: His lips are moving.

Q: Why won't sharks attack lawyers?
A: Professional courtesy.

Q: What do have when a lawyer is buried up to his neck in sand?
A: Not enough sand.

Q: How do you get a lawyer out of a tree?
A: Cut the rope.

Q: What is the definition of a shame (as in "that's a shame")?
A: When a bus load of lawyers goes off a cliff.

Q: What is the definition of a "crying shame"?
A: There was an empty seat.

Q: What do you get when you cross the Godfather with a lawyer?
A: An offer you can't understand.

Q. What do lawyers use as contraceptives?
A. Their personalities.

Q. What's brown and black and looks good on a lawyer?
A. A doberman.

Q. Why are lawyers buried 12 feet underground?
A. Deep down their good.

Q. What's the difference between a catfish and a lawyer?
A. One's a slimy scum-sucking scavenger, the other is just a fish.

"""
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
