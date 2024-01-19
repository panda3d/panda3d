import string
import tkinter
import Pmw

def aligngrouptags(groups):
    # Adjust the y position of the tags in /groups/ so that they all
    # have the height of the highest tag.

    maxTagHeight = 0
    for group in groups:
        if group._tag is None:
            #Python 3 conversion
            #height = (string.atoi(str(group._ring.cget('borderwidth'))) +
            #        string.atoi(str(group._ring.cget('highlightthickness'))))
            height = (int(str(group._ring.cget('borderwidth'))) +
                    int(str(group._ring.cget('highlightthickness'))))
        else:
            height = group._tag.winfo_reqheight()
        if maxTagHeight < height:
            maxTagHeight = height

    for group in groups:
        #Python 3 conversion
        #ringBorder = (string.atoi(str(group._ring.cget('borderwidth'))) +
        #        string.atoi(str(group._ring.cget('highlightthickness'))))
        ringBorder = (int(str(group._ring.cget('borderwidth'))) +
                int(str(group._ring.cget('highlightthickness'))))
        topBorder = maxTagHeight / 2 - ringBorder / 2
        group._hull.grid_rowconfigure(0, minsize = topBorder)
        group._ring.grid_rowconfigure(0,
                minsize = maxTagHeight - topBorder - ringBorder)
        if group._tag is not None:
            group._tag.place(y = maxTagHeight / 2)

class Group( Pmw.MegaWidget ):
    def __init__(self, parent = None, **kw):

        # Define the megawidget options.

                #TODO rename collapsedsize to collapsedheight
                #after adding collapsedwitdh (Pmw 1.3.3)
                #will both stay in place for compatibility...
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('collapsedsize',    6,         INITOPT),
                ('collapsedheight',     6,      INITOPT),
                ('collapsedwidth', 20, INITOPT),
            ('ring_borderwidth', 2,         None),
            ('ring_relief',      'groove',  None),
            ('tagindent',        10,        INITOPT),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.MegaWidget.__init__(self, parent)

        # Create the components.
        interior = Pmw.MegaWidget.interior(self)

        self._ring = self.createcomponent(
            'ring',
            (), None,
            tkinter.Frame, (interior,),
            )

        self._groupChildSite = self.createcomponent(
            'groupchildsite',
            (), None,
            tkinter.Frame, (self._ring,)
            )

        self._tag = self.createcomponent(
            'tag',
            (), None,
            tkinter.Label, (interior,),
            )

        ringBorder = (int(str(self._ring.cget('borderwidth'))) +
                int(str(self._ring.cget('highlightthickness'))))
        if self._tag is None:
            tagHeight = ringBorder
        else:
            tagHeight = self._tag.winfo_reqheight()
            self._tag.place(
                    x = ringBorder + self['tagindent'],
                    y = tagHeight / 2,
                    anchor = 'w')

        topBorder = tagHeight / 2 - ringBorder / 2
        self._ring.grid(column = 0, row = 1, sticky = 'nsew')
        interior.grid_columnconfigure(0, weight = 1)
        interior.grid_rowconfigure(1, weight = 1)
        interior.grid_rowconfigure(0, minsize = topBorder)

        self._groupChildSite.grid(column = 0, row = 1, sticky = 'nsew')
        self._ring.grid_columnconfigure(0, weight = 1)
        self._ring.grid_rowconfigure(1, weight = 1)
        self._ring.grid_rowconfigure(0,
                minsize = tagHeight - topBorder - ringBorder)

        self.showing = 1

        # Check keywords and initialise options.
        self.initialiseoptions()

    def toggle(self):
        if self.showing:
            self.collapse()
        else:
            self.expand()
        self.showing = not self.showing

    def expand(self):
        self._groupChildSite.grid(column = 0, row = 1, sticky = 'nsew')

    def collapse(self):
        self._groupChildSite.grid_forget()
        #Tracker item 1096289
        if self._tag is None:
            tagHeight = 0
        else:
            tagHeight = self._tag.winfo_reqheight()
            tagWidth = self._tag.winfo_reqwidth()
        self._ring.configure(height=(tagHeight / 2) + self['collapsedheight'],
                        width = tagWidth + self['collapsedwidth'])


    def interior(self):
        return self._groupChildSite
