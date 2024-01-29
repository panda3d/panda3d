"""
I needed a simple gauge, so I've made on with Pmw.
It might be useful for others to use as a base to develop more comples
gauges with.

Is it worth cleaning up and submitting?

cheers and thanks

chris

Dr. Chris Wright
Intensive Care Unit
Monash Medical Centre
Clayton. VIC Australia
"""

import sys
import tkinter
import Pmw
import time


if sys.platform == 'win32':
    # MS-Windows specific fonts
    label_font = "-family Ariel  -size 12"
    value_font = "-family Ariel  -size 12"
    small_font = "-family {MS Sans Serif} -size 9 -weight bold"
    header_font = "-family {MS Sans Serif} -weight bold"
else:
    # X-Windows specific fonts
    label_font = "-*-helvetica-*-r-*-*-*-160-*-*-*-*-*-*"
    value_font = "-*-courier-*-r-*-*-*-160-*-*-*-*-*-*"
    small_font = "-*-helvetica-*-r-*-*-*-130-*-*-*-*-*-*"
    header_font = "-*-helvetica-bold-r-*-*-*-150-*-*-*-*-*-*"

class VerticalGauge(Pmw.MegaWidget):
    """Vertical gauge with actual and desired settings"""

    def __init__(self, parent = None, **kw):
        optiondefs = (
            ('min', 0, None),
            ('max', 100, None),
            ('majortickinterval', 10, None),
            ('minortickinterval', 5, None),
            ('units', '', None),
            ('bg', 'grey', self._backgroundSet),
            ('actualvalue', 50, self._actualSet),
            ('desiredvalue', 50, self._desiredSet),
            ('actualcolour', 'yellow1', None),
            ('desiredcolour', 'turquoise1', None),
            ('label', 'Label', None),
            )
        self.defineoptions(kw, optiondefs)
        Pmw.MegaWidget.__init__(self, parent)

        interior = self.interior()
        interior.grid_rowconfigure(1, weight = 1)
        for r in range(3):
            interior.grid_columnconfigure(r, weight = 1)

        self.actuallabel = self.createcomponent('actualLabel',
                                                (), None,
                                                tkinter.Label, (interior,),
                                                text = '',
                                                width = 3,
                                                relief = 'sunken',
                                                bd = 1,
                                                fg = self['actualcolour'],
                                                font = value_font)
        self.actuallabel.grid(sticky = "nswe", row = 0, column = 0)

        self.label = self.createcomponent('label',
                                          (), None,
                                          tkinter.Label, (interior,),
                                          text = self['label'],
                                          relief = 'raised',
                                          font = label_font,
                                          fg = 'navy',
                                          bd = 2)
        self.label.grid(sticky = "nsew", row = 0, column = 1)

        self.desiredlabel = self.createcomponent('desiredLabel',
                                                 (), None,
                                                 tkinter.Label, (interior,),
                                                 text = '',
                                                 width = 3,
                                                 relief = 'sunken',
                                                 bd = 1,
                                                 fg = self['desiredcolour'],
                                                 font = value_font)
        self.desiredlabel.grid(sticky = "nswe", row = 0, column = 2)

        self.canvas = self.createcomponent('canvas',
                                           (), None,
                                           tkinter.Canvas, (interior,),
                                           width = 100,
                                           height = 300,
                                           bg = 'grey')

        self.canvas.grid(sticky = "nsew", columnspan = 3, pady = 1)
        self.canvas.bind("<Configure>", self._createGaugeAxes)

        self._createGaugeAxes()

        self.initialiseoptions()

    def _createGaugeAxes(self, event = None):
        min = self['min']
        max = self['max']
        units = self['units']
        majortickinterval = self['majortickinterval']

        gauge_range = max - min

        c = self.canvas
        c.delete("all")
        if event:
            h, w = event.height, event.width
        else:
            h = int(c.configure("height")[4])
            w = int(c.configure("width")[4])

        self.lower = h - 15
        self.upper = 15
        self.middle = w / 2
        c.create_line(self.middle, self.lower, self.middle, self.upper)

        majortickcount = int((max - min) / majortickinterval)
        self.axislength = self.lower - self.upper
        self.majortickdistance = float(self.axislength) / majortickcount
        self.majortickwidth = w / 5
        labeloffset = (w / 4) + 10

        for i in range(majortickcount + 1):
            v = min + i * majortickinterval
            d = self.lower - i * self.majortickdistance
            c.create_line(self.middle, d, self.middle + self.majortickwidth, d)
            c.create_text(self.middle + labeloffset, d, font = small_font, text = str(v))

        self._desiredSet(event)
        self._actualSet(event)

    def _backgroundSet(self):
        self.canvas.configure(bg = self['bg'])

    def _desiredSet(self, event = None):
        c = self.canvas
        desired = self['desiredvalue']
        desiredcolour = self['desiredcolour']

        min = self['min']
        max = self['max']

        if desired > max: desired = max
        if desired < min: desired = min
        gauge_range = max - min

        c = self.canvas
        if event:
            h, w = event.height, event.width
        else:
            h = int(c.configure("height")[4])
            w = int(c.configure("width")[4])


        desired_y = self.lower - (float(desired - min) / gauge_range) * self.axislength

        try:
            c.delete('desiredBar')
        except:
            pass

        c.create_line(self.middle - self.majortickwidth, desired_y,
                      self.middle + self.majortickwidth, desired_y,
                      fill = desiredcolour, stipple = 'gray50',
                      width = 10, tag = 'desiredBar')
        self.desiredlabel.configure(text = desired)

    def setActual(self, value):
        self.configure(actualvalue = value)

    def getActual(self):
        return self.cget('actualvalue')

    def _actualSet(self, event = None):
        c = self.canvas
        actual = self['actualvalue']
        actualcolour = self['actualcolour']

        min = self['min']
        max = self['max']

        if actual > max: actual = max
        if actual < min: actual = min
        gauge_range = max - min

        c = self.canvas
        if event:
            h, w = event.height, event.width
        else:
            h = int(c.configure("height")[4])
            w = int(c.configure("width")[4])

        actual_y = self.lower - (float(actual - min) / gauge_range) * self.axislength

        try:
            c.delete('actualPointer')
        except:
            pass

        triangle = ((self.middle, actual_y),
                    (self.middle - 1.4 * self.majortickwidth, actual_y - self.majortickwidth / 2),
                    (self.middle - 1.4 * self.majortickwidth, actual_y + self.majortickwidth / 2))

        c.create_polygon(triangle, fill = actualcolour, tag = 'actualPointer')
        self.actuallabel.configure(text = actual)


Pmw.forwardmethods(VerticalGauge, tkinter.Canvas, 'canvas')

if __name__ == '__main__':


    # Initialise Tkinter and Pmw.
    root = Pmw.initialise()
    root.title('Pmw VerticalGauge demonstration')


    def increase():
        av = g1.getActual()
        g1.setActual(av + 1)

    def decrease():
        av = g1.getActual()
        g1.setActual(av - 1)

    g1 = VerticalGauge(min = 0,
                       max = 30,
                       actualvalue = 15,
                       desiredvalue = 22,
                       majortickinterval = 2,
                       label = "Pms")
    g1.grid(sticky = "nsew")
    root.grid_rowconfigure(0, weight = 1)
    root.grid_columnconfigure(0, weight = 1)
    b1 = tkinter.Button(text = "Increase", command = increase)
    b1.grid()
    b2 = tkinter.Button(text = "Decrease", command = decrease)
    b2.grid()

    # Let's go.
    root.mainloop()
