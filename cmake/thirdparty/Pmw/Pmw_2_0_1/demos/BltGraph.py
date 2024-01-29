title = 'Blt Graph demonstration'

# Import Pmw from this directory tree.
import sys
sys.path[:0] = ['../../..']

import string
import tkinter
import Pmw

# Simple random number generator.
rand = 12345
def random():
    global rand
    rand = (rand * 125) % 2796203
    return rand

class GraphDemo(Pmw.MegaToplevel):

    def __init__(self, parent=None, **kw):

        # Define the megawidget options.
        optiondefs = (
            ('size',      10,   Pmw.INITOPT),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.MegaToplevel.__init__(self, parent)

        # Create the graph.
        self.createWidgets()

        # Check keywords and initialise options.
        self.initialiseoptions()

    def createWidgets(self):
        # Create vectors for use as x and y data points.
        self._numElements = 7
        self._vectorSize = self['size']
        self._vector_x = Pmw.Blt.Vector()
        self._vector_y = []
        for y in range(self._numElements):
            self._vector_y.append(Pmw.Blt.Vector())
        for index in range(self._vectorSize):
            self._vector_x.append(index)
            for y in range(self._numElements):
                self._vector_y[y].append(random() % 100)

        interior = self.interior()

        controlFrame = tkinter.Frame(interior)
        controlFrame.pack(side = 'bottom', fill = 'x', expand = 0)

        # Create an option menu for the kind of elements to create.
        elementtype = Pmw.OptionMenu(controlFrame,
                labelpos = 'nw',
                label_text = 'Element type',
                items = ['bars', 'lines', 'mixed', 'none'],
                command = self._setelementtype,
                menubutton_width = 8,
        )
        elementtype.pack(side = 'left')

        # Create an option menu for the barmode option.
        barmode = Pmw.OptionMenu(controlFrame,
                labelpos = 'nw',
                label_text = 'Bar mode',
                items = ['normal', 'stacked', 'aligned', 'overlap'],
                command = self._setbarmode,
                menubutton_width = 8,
        )
        barmode.pack(side = 'left')

        # Create an option menu for the smooth option.
        self.smooth = Pmw.OptionMenu(controlFrame,
                labelpos = 'nw',
                label_text = 'Smooth',
                items = ['linear', 'step', 'natural', 'quadratic'],
                command = self._setsmooth,
                menubutton_width = 9,
        )
        self.smooth.pack(side = 'left')

        # Create an option menu to reverse sort the elements.
        sortelements = Pmw.OptionMenu(controlFrame,
                labelpos = 'nw',
                label_text = 'Order',
                items = ['normal', 'reverse'],
                command = self._setsortelements,
                menubutton_width = 8,
        )
        sortelements.pack(side = 'left')

        # Create an option menu for the bufferelements option.
        bufferelements = Pmw.OptionMenu(controlFrame,
                labelpos = 'nw',
                label_text = 'Buffering',
                items = ['buffered', 'unbuffered'],
                command = self._setbufferelements,
                menubutton_width = 10,
        )
        bufferelements.pack(side = 'left')

        # Create a button to add a point to the vector.
        addpoint = tkinter.Button(controlFrame, text = 'Add point',
                command = Pmw.busycallback(self._addpoint))
        addpoint.pack(side = 'left', fill = 'x', expand = 0)

        # Create a button to close the window
        close = tkinter.Button(controlFrame, text = 'Close',
                command = Pmw.busycallback(self.destroy))
        close.pack(side = 'left', fill = 'x', expand = 0)

        # Create the graph and its elements.
        self._graph = Pmw.Blt.Graph(interior)
        self._graph.pack(expand = 1, fill = 'both')
        self._graph.yaxis_configure(command=self.yaxisCommand)
        elementtype.invoke('mixed')
        bufferelements.invoke('buffered')

    def yaxisCommand(self, graph, value):
        try:
            num = int(value)
            return '%d      %3d' % (num * 3, num)
        except ValueError:
            num = float(value)
            return '%g      %3g' % (num * 3, num)

    def _setelementtype(self, type):
        elements = self._graph.element_names()
        self._graph.element_delete(*elements)

        if type == 'none':
            return

        colorList = Pmw.Color.spectrum(self._numElements)
        for elem in range(self._numElements):
            if elem == 0:
                hue = None
            else:
                hue = (elem + 1.0) / self._numElements * 6.28318
            foreground = colorList[elem]
            background = Pmw.Color.changebrightness(self, foreground, 0.8)
            if type == 'mixed':
                if elem < self._numElements / 2:
                    bar = 0
                else:
                    bar = 1
            elif type == 'bars':
                bar = 1
            else:
                bar = 0
            if bar:
                self._graph.bar_create(
                    'var' + str(elem),
                    xdata=self._vector_x,
                    ydata=self._vector_y[elem],
                    foreground = foreground,
                    background = background)
            else:
                self._graph.line_create(
                    'var' + str(elem),
                    linewidth = 4,
                    xdata=self._vector_x,
                    ydata=self._vector_y[elem],
                    smooth = self.smooth.getcurselection(),
                    color = foreground)

    def _setbarmode(self, tag):
        self._graph.configure(barmode = tag)

    def _setsmooth(self, tag):
        for element in self._graph.element_show():
            if self._graph.element_type(element) == 'line':
                self._graph.element_configure(element, smooth = tag)

    def _setbufferelements(self, tag):
        self._graph.configure(bufferelements = (tag == 'buffered'))

    def _setsortelements(self, tag):
        element_list = list(self._graph.element_show())
        if len(element_list) > 1:
            if (tag == 'normal') == (element_list[-1] != 'var0'):
                element_list.reverse()
                self._graph.element_show(element_list)

    def _addpoint(self):
        self._vector_x.append(self._vectorSize)
        for y in range(self._numElements):
            self._vector_y[y].append(random() % 100)
        self._vectorSize = self._vectorSize + 1

class Demo:
    def __init__(self, parent):
        if not Pmw.Blt.haveblt(parent):
            message = 'Sorry\nThe BLT package has not been\n' + \
                    'installed on this system.\n' + \
                    'Please install it and try again.'
            w = tkinter.Label(parent, text = message)
            w.pack(padx = 8, pady = 8)
            return

        message = 'This is a simple demonstration of the\n' + \
                'BLT graph widget.\n' + \
                'Select the number of points to display and\n' + \
                'click on the button to display the graph.'
        w = tkinter.Label(parent, text = message)
        w.pack(padx = 8, pady = 8)

        # Create combobox to select number of points to display.
        self.combo = Pmw.ComboBox(parent,
                scrolledlist_items = ('10', '25', '50', '100', '300'),
                entryfield_value = '10')
        self.combo.pack(padx = 8, pady = 8)

        # Create button to start blt graph.
        start = tkinter.Button(parent,
                text = 'Show BLT graph',
                command = Pmw.busycallback(self.showGraphDemo))
        start.pack(padx = 8, pady = 8)

        self.parent = parent

    def showGraphDemo(self):
        size = int(self.combo.get())
        demo = GraphDemo(self.parent, size = size)
        demo.focus()

######################################################################

# Create demo in root window for testing.
if __name__ == '__main__':
    root = tkinter.Tk()
    Pmw.initialise(root)
    Pmw.Blt.setBltDisable(root, False)
    root.title(title)

    exitButton = tkinter.Button(root, text = 'Exit', command = root.destroy)
    exitButton.pack(side = 'bottom')
    widget = Demo(root)
    root.mainloop()
