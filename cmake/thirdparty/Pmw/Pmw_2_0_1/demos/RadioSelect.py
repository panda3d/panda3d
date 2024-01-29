title = 'Pmw.RadioSelect demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        # Create and pack a horizontal RadioSelect widget.
        horiz = Pmw.RadioSelect(parent,
                labelpos = 'w',
                command = self.callback,
                label_text = 'Horizontal',
                frame_borderwidth = 2,
                frame_relief = 'ridge'
        )
        horiz.pack(fill = 'x', padx = 10, pady = 10)

        # Add some buttons to the horizontal RadioSelect.
        for text in ('Fruit', 'Vegetables', 'Cereals', 'Legumes'):
            horiz.add(text)
        horiz.invoke('Cereals')

        # Create and pack a multiple selection RadioSelect widget.
        self.multiple = Pmw.RadioSelect(parent,
                labelpos = 'w',
                command = self.multcallback,
                label_text = 'Multiple\nselection',
                frame_borderwidth = 2,
                frame_relief = 'ridge',
                selectmode = 'multiple',
        )
        self.multiple.pack(fill = 'x', padx = 10)

        # Add some buttons to the multiple selection RadioSelect.
        for text in ('Apricots', 'Eggplant', 'Rice', 'Lentils'):
            self.multiple.add(text)
        self.multiple.invoke('Rice')

        # Create and pack a vertical RadioSelect widget, with checkbuttons.
        self.checkbuttons = Pmw.RadioSelect(parent,
                buttontype = 'checkbutton',
                orient = 'vertical',
                labelpos = 'w',
                command = self.checkbuttoncallback,
                label_text = 'Vertical,\nusing\ncheckbuttons',
                hull_borderwidth = 2,
                hull_relief = 'ridge',
        )
        self.checkbuttons.pack(side = 'left', expand = 1, padx = 10, pady = 10)

        # Add some buttons to the checkbutton RadioSelect.
        for text in ('Male', 'Female'):
            self.checkbuttons.add(text)
        self.checkbuttons.invoke('Male')
        self.checkbuttons.invoke('Female')

        # Create and pack a RadioSelect widget, with radiobuttons.
        radiobuttons = Pmw.RadioSelect(parent,
                buttontype = 'radiobutton',
                orient = 'vertical',
                labelpos = 'w',
                command = self.callback,
                label_text = 'Vertical,\nusing\nradiobuttons',
                hull_borderwidth = 2,
                hull_relief = 'ridge',
        )
        radiobuttons.pack(side = 'left', expand = 1, padx = 10, pady = 10)

        # Add some buttons to the radiobutton RadioSelect.
        for text in ('Male', 'Female', 'Both', 'Neither'):
            radiobuttons.add(text)
        radiobuttons.invoke('Both')

    def callback(self, tag):
        # This is called whenever the user clicks on a button
        # in a single select RadioSelect widget.
        print(('Button', tag, 'was pressed.'))

    def multcallback(self, tag, state):
        # This is called whenever the user clicks on a button
        # in the multiple select RadioSelect widget.
        if state:
            action = 'pressed.'
        else:
            action = 'released.'

        print(('Button', tag, 'was', action, \
                'Selection:', self.multiple.getcurselection()))

    def checkbuttoncallback(self, tag, state):
        # This is called whenever the user clicks on a button
        # in the checkbutton RadioSelect widget.
        if state:
            action = 'pressed.'
        else:
            action = 'released.'

        print(('Button', tag, 'was', action, \
                'Selection:', self.checkbuttons.getcurselection()))

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
