import tkinter
import Pmw

class ThresholdScale(Pmw.MegaWidget):
    """ Megawidget containing a scale and an indicator.
    """

    def __init__(self, parent = None, **kw):
        # Define the megawidget options.
        optiondefs = (
            ('colors',      ('green', 'red'), None),
            ('orient',      'vertical',       Pmw.INITOPT),
            ('labelmargin', 0,                Pmw.INITOPT),
            ('labelpos',    None,             Pmw.INITOPT),
            ('threshold',   (50,),            None),
            ('value',       None,             Pmw.INITOPT),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise base class (after defining options).
        Pmw.MegaWidget.__init__(self, parent)

        # Create the components.
        interior = self.interior()

        # Create the indicator component.
        self.indicator = self.createcomponent('indicator',
                (), None,
                tkinter.Frame, interior,
                        width = 16,
                        height = 16,
                        borderwidth = 2,
                        relief = 'raised')

        # Create the value component.
        self.value = self.createcomponent('value',
                (), None,
                tkinter.Label, interior,
                    width = 3)

        # Create the scale component.
        if self['orient'] == 'vertical':
            # The default scale range seems to be
            # the wrong way around - reverse it.
            from_ = 100
            to = 0
        else:
            from_ = 0
            to = 100

        self.scale = self.createcomponent('scale',
                (), None,
                tkinter.Scale, interior,
                        orient = self['orient'],
                        command = self._doCommand,
                        tickinterval = 20,
                        length = 200,
                        from_ = from_,
                        to = to,
                        showvalue = 0)

        value = self['value']
        if value is not None:
            self.scale.set(value)

        # Use grid to position all components
        if self['orient'] == 'vertical':
            self.indicator.grid(row = 1, column = 1)
            self.value.grid(row = 2, column = 1)
            self.scale.grid(row = 3, column = 1)
            # Create the label.
            self.createlabel(interior, childRows=3)
        else:
            self.indicator.grid(row = 1, column = 1)
            self.value.grid(row = 1, column = 2)
            self.scale.grid(row = 1, column = 3)
            # Create the label.
            self.createlabel(interior, childCols=3)

        # Check keywords and initialise options.
        self.initialiseoptions()

    def _doCommand(self, valueStr):
        valueInt = self.scale.get()
        colors = self['colors']
        thresholds = self['threshold']
        color = colors[-1]
        for index in range(len(colors) - 1):
            if valueInt <= thresholds[index]:
                color = colors[index]
                break
        self.indicator.configure(background = color)
        self.value.configure(text = valueStr)

Pmw.forwardmethods(ThresholdScale, tkinter.Scale, 'scale')

# Initialise Tkinter and Pmw.
root = Pmw.initialise()
root.title('Pmw ThresholdScale demonstration')

# Create and pack two ThresholdScale megawidgets.
mega1 = ThresholdScale(scale_showvalue = 1)
mega1.pack(side = 'left', padx = 10, pady = 10)

mega2 = ThresholdScale(
        colors = ('green', 'yellow', 'red'),
        threshold = (50, 75),
        value = 80,
        indicator_width = 32,
        scale_width = 25)
mega2.pack(side = 'left', padx = 10, pady = 10)

# Create and pack two horizontal ThresholdScale megawidgets.
mega3 = ThresholdScale(
        orient = 'horizontal',
        labelpos = 'n',
        label_text = 'Horizontal')
mega3.pack(side = 'top', padx = 10, pady = 10)
mega4 = ThresholdScale(orient = 'horizontal')
mega4.pack(side = 'top', padx = 10, pady = 10)

# Let's go.
root.mainloop()
