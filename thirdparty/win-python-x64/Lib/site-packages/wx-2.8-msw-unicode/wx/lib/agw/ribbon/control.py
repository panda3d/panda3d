"""
L{RibbonControl} serves as a base class for all controls which share the ribbon
charactertics of having a ribbon art provider, and (optionally) non-continous
resizing.


Description
===========

Despite what the name may imply, it is not the top-level control for creating a
ribbon interface - that is L{RibbonBar}. Ribbon controls often have a region which
is "transparent", and shows the contents of the ribbon page or panel behind it.

If implementing a new ribbon control, then it may be useful to realise that this
effect is done by the art provider when painting the background of the control,
and hence in the paint handler for the new control, you should call a draw background
method on the art provider (L{RibbonMSWArtProvider.DrawButtonBarBackground} and
L{RibbonMSWArtProvider.DrawToolBarBackground} typically just redraw what is behind the
rectangle being painted) if you want transparent regions. 

"""

import wx

class RibbonControl(wx.PyControl):

    def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize, style=0,
                 validator=wx.DefaultValidator, name="RibbonControl"):

        wx.PyControl.__init__(self, parent, id, pos, size, style, validator, name)
        self._art = None

        if isinstance(parent, RibbonControl):
            self._art = parent.GetArtProvider()
    

    def SetArtProvider(self, art):
        """
        Set the art provider to be used.

        In many cases, setting the art provider will also set the art provider on all
        child windows which extend L{RibbonControl}. In most cases, controls will not
        take ownership of the given pointer, with the notable exception being
        L{RibbonBar.SetArtProvider}.

        :param `art`: MISSING DESCRIPTION.

        """

        self._art = art


    def GetArtProvider(self):
        """
        Get the art provider to be used.

        Note that until an art provider has been set in some way, this function may
        return ``None``.

        """

        return self._art


    def IsSizingContinuous(self):
        """
        :returns: ``True`` if this window can take any size (greater than its minimum size),
         ``False`` if it can only take certain sizes.
        
        :see: L{GetNextSmallerSize}, L{GetNextLargerSize}
        """

        return True

    
    def DoGetNextSmallerSize(self, direction, size):
        """
        Implementation of L{GetNextSmallerSize}.

        Controls which have non-continuous sizing must override this virtual function
        rather than L{GetNextSmallerSize}.

        :param `direction`: MISSING DESCRIPTION;
        :param `relative_to`: MISSING DESCRIPTION.

        """

        # Dummy implementation for code which doesn't check for IsSizingContinuous() == true
        minimum = self.GetMinSize()
        
        if direction & wx.HORIZONTAL and size.x > minimum.x:
            size.x -= 1        
        if direction & wx.VERTICAL and size.y > minimum.y:
            size.y -= 1
        
        return size


    def DoGetNextLargerSize(self, direction, size):
        """
        Implementation of L{GetNextLargerSize}.

        Controls which have non-continuous sizing must override this virtual function
        rather than L{GetNextLargerSize}.

        :param `direction`: MISSING DESCRIPTION;
        :param `relative_to`: MISSING DESCRIPTION.

        """

        # Dummy implementation for code which doesn't check for IsSizingContinuous() == true
        if direction & wx.HORIZONTAL:
            size.x += 1
        if direction & wx.VERTICAL:
            size.y += 1
        
        return size


    def GetNextSmallerSize(self, direction, relative_to=None):
        """
        If sizing is not continuous, then return a suitable size for the control which
        is smaller than the given size.

        :param `direction`: The direction(s) in which the size should reduce;
        :param `relative_to`: The size for which a smaller size should be found.

        :returns: if there is no smaller size, otherwise a suitable size which is smaller
         in the given direction(s), and the same as in the other direction (if any).
         
        :see: L{IsSizingContinuous}, L{DoGetNextSmallerSize}
        """

        if relative_to is not None:
            return self.DoGetNextSmallerSize(direction, relative_to)

        return self.DoGetNextSmallerSize(direction, self.GetSize())


    def GetNextLargerSize(self, direction, relative_to=None):
        """
        If sizing is not continuous, then return a suitable size for the control which
        is larger then the given size.

        :param `direction`: The direction(s) in which the size should increase;
        :param `relative_to`: The size for which a larger size should be found.

        :returns: if there is no larger size, otherwise a suitable size which is larger
         in the given direction(s), and the same as in the other direction (if any).
         
        :see: L{IsSizingContinuous}, L{DoGetNextLargerSize}
        """

        if relative_to is not None:
            return self.DoGetNextLargerSize(direction, relative_to)

        return self.DoGetNextLargerSize(direction, self.GetSize())


    def Realize(self):
        """
        Perform initial size and layout calculations after children have been added,
        and/or realize children.
        """

        pass


    def Realise(self):
        """
        Alias for L{Realize}.
        """

        pass

    

