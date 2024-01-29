import tkinter
import Pmw

class ThresholdScale(Pmw.MegaWidget):
    """ Megawidget containing a scale and an indicator.
    """

    def __init__(self, parent = None, **kw):

        # Define the megawidget options.
        optiondefs = (
            ('colors',    ('green', 'red'), None),
            ('threshold', 50,               None),
            ('value',     None,             Pmw.INITOPT),
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
        self.indicator.grid()

        # Create the scale component.
        self.scale = self.createcomponent('scale',
                (), None,
                tkinter.Scale, interior,
                        command = self._doCommand,
                        tickinterval = 20,
                        length = 200,
                        from_ = 100,
                        to = 0,
                        showvalue = 0)
        self.scale.grid()

        value = self['value']
        if value is not None:
            self.scale.set(value)

        # Check keywords and initialise options.
        self.initialiseoptions()

    def _doCommand(self, valueStr):
        if self.scale.get() > self['threshold']:
            color = self['colors'][1]
        else:
            color = self['colors'][0]
        self.indicator.configure(background = color)

Pmw.forwardmethods(ThresholdScale, tkinter.Scale, 'scale')

# Initialise Tkinter and Pmw.
root = Pmw.initialise()
root.title('Pmw ThresholdScale demonstration')

# Create and pack two ThresholdScale megawidgets.
mega1 = ThresholdScale()
mega1.pack(side = 'left', padx = 10, pady = 10)

mega2 = ThresholdScale(
        colors = ('green', 'yellow'),
        threshold = 75,
        value = 80,
        indicator_width = 32,
        scale_width = 25)
mega2.pack(side = 'left', padx = 10, pady = 10)

# Let's go.
root.mainloop()
