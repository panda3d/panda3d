title = 'Color spectrum demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import string
import tkinter
import Pmw

class Demo:
    def __init__(self, parent):
        parent = tkinter.Frame(parent)
        parent.pack(padx=10, pady=10, fill='both', expand=1)
        self.width = 350
        self.height = 250
        self.canvas = tkinter.Canvas(parent,
                width = self.width, height = self.height)
        self.canvas.grid(row = 0, column = 0, columnspan = 2, sticky = 'news')

        self.numColors = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Number of colors:',
                entry_width = 5,
                validate = 'numeric',
                command = Pmw.busycallback(self.execute))
        self.numColors.grid(row = 1, column = 0, sticky = 'ew')

        self.correction = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Correction:',
                validate = 'real',
                entry_width = 5,
                command = Pmw.busycallback(self.execute))
        self.correction.grid(row = 1, column = 1, sticky = 'ew')

        self.saturation = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Saturation:',
                validate = 'real',
                entry_width = 5,
                command = Pmw.busycallback(self.execute))
        self.saturation.grid(row = 2, column = 0, sticky = 'ew')

        self.intensity = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Intensity:',
                validate = 'real',
                entry_width = 5,
                command = Pmw.busycallback(self.execute))
        self.intensity.grid(row = 2, column = 1, sticky = 'ew')

        self.extraOrange = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Emphasize orange (0 or 1):',
                validate = {'validator' : 'numeric', 'min' : 0, 'max' : 1},
                entry_width = 5,
                command = Pmw.busycallback(self.execute))
        self.extraOrange.grid(row = 3, column = 0, sticky = 'ew')

        self.text = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Text:',
                entry_width = 20,
                command = Pmw.busycallback(self.execute))
        self.text.grid(row = 4, column = 0, sticky = 'ew')

        self.brightness = Pmw.EntryField(parent,
                labelpos = 'w',
                label_text = 'Brightness:',
                validate = 'real',
                entry_width = 5,
                command = Pmw.busycallback(self.execute))
        self.brightness.grid(row = 3, column = 1, sticky = 'ew')

        self.radiobuttons = Pmw.RadioSelect(parent,
                command = Pmw.busycallback(self.radio_cb),
        )
        self.radiobuttons.grid(row = 4, column = 1)
        self.radiobuttons.add('Use saturation\nand intensity')
        self.radiobuttons.add('Use\nbrightness')

        parent.grid_columnconfigure(0, weight = 1)
        parent.grid_columnconfigure(1, weight = 1)
        parent.grid_rowconfigure(0, weight = 1)

        Pmw.alignlabels((self.numColors, self.saturation, self.extraOrange))
        Pmw.alignlabels((self.correction, self.intensity, self.brightness))

        # Set initial values for all entries.
        self.numColors.setentry('64')
        self.correction.setentry('1.0')
        self.saturation.setentry('1.0')
        self.intensity.setentry('1.0')
        self.extraOrange.setentry('1')
        self.brightness.setentry('0.7')
        self.text.setentry('This is a test')
        self.radiobuttons.invoke('Use saturation\nand intensity')

        self.execute()

    def radio_cb(self, value):
        self.execute()

    def execute(self):
        try:
            numColors = int(self.numColors.get())
            correction = float(self.correction.get())
            saturation = float(self.saturation.get())
            intensity = float(self.intensity.get())
            extraOrange = float(self.extraOrange.get())
            brightness = float(self.brightness.get())
        except ValueError:
            self.numColors.bell()
            return

        if numColors <= 0:
            self.numColors.bell()
            return

        self.canvas.delete('all')

        colorList = Pmw.Color.spectrum(
                numColors, correction, saturation, intensity, extraOrange)
        extent = 360.0 / numColors

        useBrightness = \
                (self.radiobuttons.getcurselection() == 'Use\nbrightness')

        if numColors == 1:
            # Special case circle, since create_arc does not work when
            # extent is 360.
            background = colorList[0]
            if useBrightness:
                background = Pmw.Color.changebrightness(
                        self.canvas, background, brightness)
            self.canvas.create_oval(10, 10, self.width - 10, self.height - 10,
                fill = background, outline = background)

        for index in range(numColors):
            start = index * extent - extent / 2
            background = colorList[index]
            if useBrightness:
                background = Pmw.Color.changebrightness(
                        self.canvas, background, brightness)
            self.canvas.create_arc(10, 10, self.width - 10, self.height - 10,
                start = start, extent = extent,
                fill = background, outline = background)

        text = self.text.get()
        self.canvas.create_text(self.width / 2, self.height / 3, text = text)
        self.canvas.create_text(self.width / 2, self.height / 2, text = text)
        self.canvas.create_text(self.width / 2, 2 * self.height / 3, text = text)

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
