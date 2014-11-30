import wx

from art_msw import RibbonMSWArtProvider
from art_aui import RibbonAUIArtProvider

if wx.Platform == "__WXMSW__":

    class RibbonDefaultArtProvider(RibbonMSWArtProvider):
        """ Default art provider on MSW. """

        def __init__(self, set_colour_scheme=True):

            RibbonMSWArtProvider.__init__(self, set_colour_scheme)


elif wx.Platform == "__WXGTK__":

    class RibbonDefaultArtProvider(RibbonAUIArtProvider):
        """ Default art provider on GTK. """

        def __init__(self):

            RibbonAUIArtProvider.__init__(self)


else:
    # MAC has still no art provider for a ribbon, so we'll use
    # The AUI one. Waiting for a RibbonOSXArtProvider :-D

    class RibbonDefaultArtProvider(RibbonAUIArtProvider):
        """ Default art provider on Mac. """

        def __init__(self):

            RibbonAUIArtProvider.__init__(self)

    
