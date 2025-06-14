import pytest
pytest.importorskip('Pmw')
from direct.tkwidgets.EntryScale import EntryScale, EntryScaleGroup


def test_EntryScale(tk_toplevel):
    # Initialise Tkinter and Pmw.
    root = tk_toplevel
    root.title('Pmw EntryScale demonstration')

    # Dummy command
    def printVal(val):
        print(val)

    # Create and pack a EntryScale megawidget.
    mega1 = EntryScale(root, command = printVal)
    mega1.pack(side = 'left', expand = 1, fill = 'x')

    # These are things you can set/configure
    # Starting value for entryScale
    #mega1['value'] = 123.456
    #mega1['text'] = 'Drive delta X'
    #mega1['min'] = 0.0
    #mega1['max'] = 1000.0
    #mega1['resolution'] = 1.0
    # To change the color of the label:
    #mega1.label['foreground'] = 'Red'
    # Max change/update, default is 100
    # To have really fine control, for example
    # mega1['maxVelocity'] = 0.1
    # Number of digits to the right of the decimal point, default = 2
    # mega1['numDigits'] = 5

    # To create a entryScale group to set an RGBA value:
    group1 = EntryScaleGroup(root, dim = 4,
                          title = 'Simple RGBA Panel',
                          labels = ('R', 'G', 'B', 'A'),
                          Valuator_min = 0.0,
                          Valuator_max = 255.0,
                          Valuator_resolution = 1.0,
                          command = printVal)

    # Uncomment this if you aren't running in IDLE
    #root.mainloop()
