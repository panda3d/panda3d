# --------------------------------------------------------------------------------- #
# GENERICMESSAGEDIALOG wxPython IMPLEMENTATION
#
# Andrea Gavana, @ 07 October 2008
# Latest Revision: 12 Sep 2010, 10.00 GMT
#
#
# TODO List
#
# 1) ?
#
#
# For all kind of problems, requests of enhancements and bug reports, please
# write to me at:
#
# andrea.gavana@gmail.com
# gavana@kpo.kz
#
# Or, obviously, to the wxPython mailing list!!!
#
#
# End Of Comments
# --------------------------------------------------------------------------------- #

"""
This class is a possible, fancy replacement for `wx.MessageDialog`.


Description
===========

This class represents a dialog that shows a single or multi-line message,
with a choice of ``OK``, ``Yes``, ``No`` and ``Cancel`` buttons. It is a possible
replacement for the standard `wx.MessageDialog`, with these extra functionalities:

* Possibility to modify the dialog position;
* Custom themed generic bitmap & text buttons;
* Possibility to set an icon to the dialog;
* More visibility to the button getting the focus;
* Support for Aqua buttons or Gradient buttons instead of themed ones (see L{AquaButton}
  and L{GradientButton});
* Possibility to automatically wrap long lines of text;
* Good old Python code :-D .

And a lot more. Check the demo for an almost complete review of the functionalities.


Supported Platforms
===================

GenericMessageDialog has been tested on the following platforms:
  * Windows (Windows XP).


Window Styles
=============

This class supports the following window styles:

=========================== =========== ==================================================
Window Styles               Hex Value   Description
=========================== =========== ==================================================
``GMD_USE_AQUABUTTONS``            0x20 Uses `wx.lib.agw.aquabutton` buttons instead of generic buttons.
``GMD_USE_GRADIENTBUTTONS``        0x40 Uses `wx.lib.agw.gradientbutton` buttons instead of generic buttons.
=========================== =========== ==================================================


Events Processing
=================

`No custom events are available for this class.`


License And Version
===================

GenericMessageDialog is distributed under the wxPython license.

Latest Revision: Andrea Gavana @ 12 Sep 2010, 10.00 GMT

Version 0.5

"""

import wx
import wx.lib.wordwrap as wordwrap

import wx.lib.buttons as buttons
from wx.lib.embeddedimage import PyEmbeddedImage

# To use AquaButtons or GradientButtons instead of wx.lib.buttons
import aquabutton as AB
import gradientbutton as GB

# GenericMessageDialog styles
GMD_USE_AQUABUTTONS = 32
""" Uses `wx.lib.agw.aquabutton` buttons instead of generic buttons. """
GMD_USE_GRADIENTBUTTONS = 64
""" Uses `wx.lib.agw.gradientbutton` buttons instead of generic buttons. """

# Avoid 2.9 errors
BUTTON_SIZER_FLAGS = wx.OK | wx.CANCEL | wx.YES | wx.NO | wx.HELP | wx.NO_DEFAULT

_ = wx.GetTranslation

_cancel = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAA1dJ"
    "REFUOI11019oEwccB/Dv3eUuyZ2XpfljsmJ7JY01KZYWty6bdMwnp1X34JNS5sPAsmYruOnL"
    "3kTGcPg6Bdkexqql4EPdBuKbVG0xLmpoWjbW0D+S1Jg24RJzuSR3l58PtpsI/l5/fB5+3x9f"
    "AEDc7VauhMP3prq7q9+1t5/AW+aiLB+ZDocrU6HQk4tAFAC4s8Gg0uVyXTsZiw190Nsr6JnM"
    "kZAkrd6rVtOv4wuyfLS/rW3y6Oioq2tgILiRyXy4v1yexU979yaKIyNEiQRRsUjG2Bjddrtr"
    "532+k9v4B1kevu33l+vnzhFtbBAtL9OLS5douq9v0eZ1OPo8Xi8gSUClAls8jk+qVad148bP"
    "33s8TcY0K32mOTV07JhsP3UKKJUAy8IORYF3584erodopaGqh7qzWYEJBgGGgW3fPrQ/eyY0"
    "5uePewzjxIGDB0U5HgcsC1BV0MOH+GtiojF/9+433P1qNd1pGCvs5uawUijwbDAIWBZsAwPw"
    "5nJsRyBgc8fjYLZwK5lE6uZN88Hc3LdfmeYVDgDu12oLXUSrxvPnw8r6uo3z+cAQwRGJQOzv"
    "B0sEKhZhJRJI3rplJlKpM+OWdRkAuO2gZnQ93UO02CgUjr9bLHKCzweGZcGYJqhchp5I4NGd"
    "O9bjpaUvxol+2Xa211/FAKolSa0XySSq+TzYYBAAYGkaUKnA5LgWA6hvmP//PKgokx9tbspq"
    "Pg8NgL61c0gSJL8f73R04O9KRV9Mp0+PtlrX/zvhgigO749GJ4dKJVc9l0MTgAVAZBg4BQEk"
    "SeCcTjAAOhWF5/3+w7FsdvkPogXuR7f7s/d6eycPqKqrubKC+hZ28DxydnurzHFWwG5niefB"
    "CALYVgu7wmGe2toOfby2lrVFIpFrn9brcmNpCU0ALIAdooiMw9FI1etfkmGUbaY5EXY4JIth"
    "YAIw1tcxODgoEcddZeua9rQqCGB5HgwA0e3GmsdjPtH1s1/Xar+ON5vTi6p6+qmm6U5JAksE"
    "VhBQbzahl0p57n1Nm9kQxVhXINAucxzSLpeZLBTOxHX98nbAfxItxMrlVV4UD+/q7OTJ58Pc"
    "7Ow/uVTq81c1FYTo76HQo5k9expXnc6xt9X5OsuOPIhGtZndu//9DYgBwEt1gHq0YITgmAAA"
    "AABJRU5ErkJggg==")

#----------------------------------------------------------------------
_error = PyEmbeddedImage(
    "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")

#----------------------------------------------------------------------
_help = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAtxJ"
    "REFUOI2lk81LFHEYx5+ZndnWnV0bt8z2TYKpxYsdwpBWA62w2rxIdOgmdPAvCOnqMSiig2SB"
    "lVmeAhEsJt02SNdcXZXC1HYT3SRWXLd1HHdmduf3ezqUgrjQoef08MDzeZ7v8wLwn8aUCn69"
    "fNntWl9vsrrdR4pra2lDEN5Xf/qU/ScgMjR09PTjxw/sLS03+YYGFmw2QF0Hc2KisJXL9Sxf"
    "uNAZDAa1koBIJHJcFMUxKZOR+EwGVhHBQIQT5eVgzedBb26GX6o6kc1mL9XV1eV389hdxzU+"
    "/tzpdErfnU6IiaIunD+/6Gm5MhkXhLEvXm+6QAiUT02d87W1PTygY76pqT4jipjq6EBZlrfz"
    "eW1S1/U5TdPnVFWLxuOzTz7L8s5GfT2mRZEkPB7/vg7sq6vXiKLAyswMSNKpJUIIFosUCSEM"
    "pdQhSVJdQlXDRlUVmIrCWnO5q7sADgCAFgp+k1LQ/X4wjAKqO1qOs7B2lmVtCMBQgmYqlVo+"
    "K0mAlAItFj37OqAMkzMBwC8IIMvvOGISq6YZRUXVthQlvzX6fnSnoqJiAbNZMAHAtFi29wEM"
    "QYgQAODDYQj4jqfu37+3MTYeLZuNz1r7XjyHvme9ou3QoY5iNArkT8GP+4Y43dPDL9rtiW+h"
    "EC7FYnRhYeFee3t7sLa29szw8HAg/ObNYGsohK+fPsXk9etRRNxb/54T6+0NHm5oCPMcZyt2"
    "dQEbCCBbVbXDpNMOY2AANi9ehLsrK+Dz+e52d3d3lrzE6enpRlc8/pKV5WpEBCQEkFJASqGw"
    "ubn4qrn5dmxm5lFlZWV/f3//HYZh8MAvDLW22k86HDfsktTEWK3H6Pb2T3V+fvRHIjEYSiYN"
    "j8cTqKmp+WCxWG6NjIy8LfVMjNvtLlMUxWGapgMRbYhoRUT+79AJAHA8z6e8Xu8mVwKAjY2N"
    "RiwWo6qqaqZpcoQQy65clmUJx3Gmy+Uykslk4TdmSW8/+y1ZpAAAAABJRU5ErkJggg==")

#----------------------------------------------------------------------
_information = PyEmbeddedImage(
    "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")

#----------------------------------------------------------------------
_no = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAelJ"
    "REFUOI21ksFL03EYxj/fX1tuWEtxW0ZulbbpKYutIYM6CIFEimh2CUFIKKhLpxCKSPwDunSI"
    "IOgQSh48dq5DB9EiVik/NaHBYrRm5HTG2u/pMAsTiRb0nJ735X0e3vfhhf+FC4dpT/WioWY6"
    "/8ngfT8reoXsXtYBd1XigTDnc/frJeW18iigoSOMAOavxEehZm4QOYXHymRmVV6b1PIlFPFy"
    "cKf5Xdsb9gD3gtdisdlcn3n+LEXm017inRniSx+iD5eY2D5vbS2uNBP8coDLnuRd46kpUiqV"
    "8O0p4YqNETrOudONxP9ocP0Y0y13uoAyTU0W+fxXmkIWsEF4pIsHSZ4CNTsa3G4n4TvjPkTd"
    "MPARv3+VbDZHJLIBpGFfD3Vn3Q1XW+lnS6A/iZnrw2mb7AYruRmNmJ7+TCJRC2wAa+DMsHDx"
    "BdEJGoD8rw2W+7gRvuUCyw8sAPMUCm8ZH39NNpsC5gEbLC+hmy6enGLst/PfDSKpVdJJSR2S"
    "OjQ1dUKRSESjo1FJbZJaJIUlBWQPIyAAwJseZuQYSQFJ+yU1SmpUWUHZi34Vv9VL8kmqleSR"
    "tFsSetnNIuB2GbDtQcWMexWMwRgLpM3dHNKOU6FykASq1OvfSQNes3mL31vdv5tiJdl8FZqd"
    "8QO3F9ZRFEVpTAAAAABJRU5ErkJggg==")

#----------------------------------------------------------------------
_ok = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAjdJ"
    "REFUOI2tksFLk3Ecxp+97975vmuve1dWuiUTNIy1JlsLpkZG0aXLbv0B0aVDUMfVQTp0jJpF"
    "EHl5LxUZgZcuQjAID4KUyWwyEU3d9m7O5d733dze97dfB1siJSn1nJ/P5+ELX+Afwx6YuAMB"
    "AVgwjcaBBdIovP2eyKMLPYNdM+7kNKZA9i3gR+ENCeF4Hx+8VigVBgrKWrXKGp/2JeCfwhsW"
    "Q/HTQiCaVTOYUiZtDuoMQqefrc1S9+uOEGNSRzqd+4j72/c1l4OOQNwn+aOFWg5TdBJEIKbH"
    "dI9zHLMt6H3lHrjScfU5x3DSmOXNrVUUxwFQ6S3vDdh9cZ/zTHSz8R0pMguGMKaRMuX5peQ9"
    "ZULPW8+PnB286L78zH/M76/DwCYtjSTefaAOQZjpEDofn5J8UR0qViqLoCpLql+IXFzS72IC"
    "eQCwssR2NFfOtNXsFZx09SLkDnfSlsYTluUy7a3Hz6mWMrLGKswiJaV0WS6Uyr9gAGC7It0L"
    "WrWYm99K9VdcqugSD8Pd6nG6RNeJCq9ZstwqNL1CMl/z8npdiRkPd2AAYJcTy41FcSVZt+lK"
    "na9FaLspCg4ehDew3qJgs6qStUxerhItlr+h74KB5iPNgVZuGkm6QpQWmy3i8AoiY7dA1XTy"
    "LZuVGYHGZi8t/gbvCABgDFS7vpVEgSgS29bv5CR7XtmQjxxyxt77En+Edwt+Svpua3MbRT5T"
    "a9QXPGL7gxc9L/eE98wwHWaG6JD1783/kB9qTvueLt8LjwAAAABJRU5ErkJggg==")

#----------------------------------------------------------------------
_question = PyEmbeddedImage(
    "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"
    "AElFTkSuQmCC")

#----------------------------------------------------------------------
_reassign = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAAwAAAAMCAYAAABWdVznAAAABHNCSVQICAgIfAhkiAAAAcRJ"
    "REFUKJGdj01Ik3Ecxz///565KcvkcUwedtiWRFRUprTAxxZLq0MRzBy5IIoShF5cp6BLBF16"
    "oU4RnQK7BC2WtEOLGLJkvdGbBzOFEqpLRXhYbZLu+XWwt3Mf+B4/H/gqFlkOnAU+AGFgDlgJ"
    "zABBIC0ij5RSAoDH485lr52SocHdMv7giqR6e2SyeFn2JLrlWf6SWIHGj4VCoRXAAGhb17Yz"
    "urFTFnSTevn4KYnePqamp0j17+Xzl3esWdseFJFlwFsDQGmD4KrtyvzkZf/h05w/18GOXScw"
    "dA2UG9/tNwALADqTySQrlW/kcncYyd5ia3w9kZYaN65fpVwu43Y5gKCUUgA6mUzePHRwQIpj"
    "BZY2NXD0eBq7qwNrySxjo/eo1Ry01vzGABhKDyjmRsCoh9nvUP2KUZnm7pMJujZv41f8r5A+"
    "sk9ejWaVUddAIh4httrF5HuLwWMnsawWHMf5I+hSqWQXSzPqwvBzuvvPcHF4gvuvG1nReYBw"
    "OML8/A9EBBERAMO27YexWIzohnY8dS7GX/TRuilFfEsP1WoFr7cen8+H/vcIUAmFQmKappim"
    "Kc3NfvH7FxcIBASQfD4f5X/4CVPZn52VDH4mAAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
_warning = PyEmbeddedImage(
    "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"
    "7HCEfEAAAAAASUVORK5CYII=")

#----------------------------------------------------------------------
_yes = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAjdJ"
    "REFUOI2tksFLk3Ecxp+97975vmuve1dWuiUTNIy1JlsLpkZG0aXLbv0B0aVDUMfVQTp0jJpF"
    "EHl5LxUZgZcuQjAID4KUyWwyEU3d9m7O5d733dze97dfB1siJSn1nJ/P5+ELX+Afwx6YuAMB"
    "AVgwjcaBBdIovP2eyKMLPYNdM+7kNKZA9i3gR+ENCeF4Hx+8VigVBgrKWrXKGp/2JeCfwhsW"
    "Q/HTQiCaVTOYUiZtDuoMQqefrc1S9+uOEGNSRzqd+4j72/c1l4OOQNwn+aOFWg5TdBJEIKbH"
    "dI9zHLMt6H3lHrjScfU5x3DSmOXNrVUUxwFQ6S3vDdh9cZ/zTHSz8R0pMguGMKaRMuX5peQ9"
    "ZULPW8+PnB286L78zH/M76/DwCYtjSTefaAOQZjpEDofn5J8UR0qViqLoCpLql+IXFzS72IC"
    "eQCwssR2NFfOtNXsFZx09SLkDnfSlsYTluUy7a3Hz6mWMrLGKswiJaV0WS6Uyr9gAGC7It0L"
    "WrWYm99K9VdcqugSD8Pd6nG6RNeJCq9ZstwqNL1CMl/z8npdiRkPd2AAYJcTy41FcSVZt+lK"
    "na9FaLspCg4ehDew3qJgs6qStUxerhItlr+h74KB5iPNgVZuGkm6QpQWmy3i8AoiY7dA1XTy"
    "LZuVGYHGZi8t/gbvCABgDFS7vpVEgSgS29bv5CR7XtmQjxxyxt77En+Edwt+Svpua3MbRT5T"
    "a9QXPGL7gxc9L/eE98wwHWaG6JD1783/kB9qTvueLt8LjwAAAABJRU5ErkJggg==")


class StdDialogButtonSizer(wx.BoxSizer):
    """ wxWidgets standard dialog button sizer. """

    def __init__(self):
        """ Default class constructor. """

        wx.BoxSizer.__init__(self, wx.HORIZONTAL)
        
        self._buttonAffirmative = None
        self._buttonApply = None
        self._buttonNegative = None
        self._buttonCancel = None
        self._buttonHelp = None


    def AddButton(self, mybutton):
        """
        Add a button to the sizer.

        :param `mybutton`: the button to add.
        """

        buttonId = mybutton.GetId()
        
        if buttonId in [wx.ID_OK, wx.ID_YES, wx.ID_SAVE]:    
            self._buttonAffirmative = mybutton

        elif buttonId == wx.ID_APPLY:
            self._buttonApply = mybutton
        elif buttonId == wx.ID_NO:
            self._buttonNegative = mybutton
        elif buttonId in [wx.ID_CANCEL, wx.ID_CLOSE]:
            self._buttonCancel = mybutton
        elif buttonId in [wx.ID_HELP, wx.ID_CONTEXT_HELP]:
            self._buttonHelp = mybutton
            

    def SetAffirmativeButton(self, button):
        """
        Sets the affirmative button.

        :param `button`: the button to set as affirmative one.
        """

        self._buttonAffirmative = button


    def SetNegativeButton(self, button):
        """
        Sets the negative button.

        :param `button`: the button to set as negative one.
        """

        self._buttonNegative = button


    def SetCancelButton(self, button):
        """
        Sets the cancel button.

        :param `button`: the button to set as cancel one.
        """

        self._buttonCancel = button


    def Realize(self):
        """ Realizes the sizer depending on the platform. """

        if wx.Platform == "__WXMAC__":
            
            self.Add((0, 0), 0, wx.LEFT, 6)
            if self._buttonHelp:
                self.Add(self._buttonHelp, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, 6)

            if self._buttonNegative:
                # HIG POLICE BULLETIN - destructive buttons need extra padding
                # 24 pixels on either side
                self.Add(self._buttonNegative, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, 12)
            
            # extra whitespace between help/negative and cancel/ok buttons
            self.Add((0, 0), 1, wx.EXPAND, 0)

            if self._buttonCancel:
                self.Add(self._buttonCancel, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, 6)
                # Cancel or help should be default
                # self._buttonCancel.SetDefaultButton()
            
            # Ugh, Mac doesn't really have apply dialogs, so I'll just
            # figure the best place is between Cancel and OK
            if self._buttonApply:
                self.Add(self._buttonApply, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, 6)

            if self._buttonAffirmative:
                self.Add(self._buttonAffirmative, 0, wx.ALIGN_CENTRE | wx.LEFT, 6)
                if self._buttonAffirmative.GetId() == wx.ID_SAVE:
                    # these buttons have set labels under Mac so we should use them
                    self._buttonAffirmative.SetLabel(_("Save"))
                    if self._buttonNegative:
                        self._buttonNegative.SetLabel(_("Don't Save"))
                
            # Extra space around and at the right
            self.Add((12, 24))
            
        elif wx.Platform == "__WXGTK__":

            self.Add((0, 0), 0, wx.LEFT, 9)
            if self._buttonHelp:
                self.Add(self._buttonHelp, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, 3)

            # extra whitespace between help and cancel/ok buttons
            self.Add((0, 0), 1, wx.EXPAND, 0)

            if self._buttonNegative:
                self.Add(self._buttonNegative, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, 3)
            
            # according to HIG, in explicit apply windows the order is:
            # [ Help                     Apply   Cancel   OK ]
            if self._buttonApply:
                self.Add(self._buttonApply, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, 3)

            if self._buttonCancel:
                self.Add(self._buttonCancel, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, 3)
                # Cancel or help should be default
                # self._buttonCancel.SetDefaultButton()
            
            if self._buttonAffirmative:
                self.Add(self._buttonAffirmative, 0, wx.ALIGN_CENTRE | wx.LEFT, 6)
                
        elif wx.Platform == "__WXMSW__":
            # Windows
            # center-justify buttons
            self.Add((0, 0), 1, wx.EXPAND, 0)

            if self._buttonAffirmative:
                self.Add(self._buttonAffirmative, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, self._buttonAffirmative.ConvertDialogSizeToPixels(wx.Size(2, 0)).x)

            if self._buttonNegative:
                self.Add(self._buttonNegative, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, self._buttonNegative.ConvertDialogSizeToPixels(wx.Size(2, 0)).x)
            
            if self._buttonCancel:
                self.Add(self._buttonCancel, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, self._buttonCancel.ConvertDialogSizeToPixels(wx.Size(2, 0)).x)
            
            if self._buttonApply:
                self.Add(self._buttonApply, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, self._buttonApply.ConvertDialogSizeToPixels(wx.Size(2, 0)).x)

            if self._buttonHelp:
                self.Add(self._buttonHelp, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, self._buttonHelp.ConvertDialogSizeToPixels(wx.Size(2, 0)).x)

            self.Add((0, 0), 1, wx.EXPAND, 0)

        else:
            # GTK+1 and any other platform

            if self._buttonHelp:
                self.Add(self._buttonHelp, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, self._buttonHelp.ConvertDialogSizeToPixels(wx.Size(4, 0)).x)

            # extra whitespace between help and cancel/ok buttons
            self.Add((0, 0), 1, wx.EXPAND, 0)

            if self._buttonApply:
                self.Add(self._buttonApply, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, self._buttonApply.ConvertDialogSizeToPixels(wx.Size(4, 0)).x)

            if self._buttonAffirmative:
                self.Add(self._buttonAffirmative, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, self._buttonAffirmative.ConvertDialogSizeToPixels(wx.Size(4, 0)).x)
            
            if self._buttonNegative:
                self.Add(self._buttonNegative, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, self._buttonNegative.ConvertDialogSizeToPixels(wx.Size(4, 0)).x)
            
            if self._buttonCancel:
                self.Add(self._buttonCancel, 0, wx.ALIGN_CENTRE | wx.LEFT | wx.RIGHT, self._buttonCancel.ConvertDialogSizeToPixels(wx.Size(4, 0)).x)
                # Cancel or help should be default
                # self._buttonCancel.SetDefaultButton()
            

class GenericMessageDialog(wx.Dialog):
    """
    Main class implementation, L{GenericMessageDialog} is a possible replacement
    for the standard `wx.MessageDialog`.
    """

    def __init__(self, parent, message, caption, agwStyle,
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.DEFAULT_DIALOG_STYLE|wx.WANTS_CHARS,
                 wrap=-1):
        """
        Default class constructor.

        :param `parent`: the L{GenericMessageDialog} parent (if any);
        :param `message`: the message in the main body of the dialog;
        :param `caption`: the dialog title;
        :param `agwStyle`: the AGW-specific dialog style; it can be one of the
         following bits:

         =========================== =========== ==================================================
         Window Styles               Hex Value   Description
         =========================== =========== ==================================================
         0                                     0 Uses normal generic buttons
         ``GMD_USE_AQUABUTTONS``            0x20 Uses L{AquaButton} buttons instead of generic buttons.
         ``GMD_USE_GRADIENTBUTTONS``        0x40 Uses L{GradientButton} buttons instead of generic buttons.
         =========================== =========== ==================================================

        :param `pos`: the dialog position on screen;
        :param `size`: the dialog size;
        :param `style`: the underlying `wx.Dialog` style;
        :param `wrap`: if set greater than zero, wraps the string in `message` so that
         every line is at most `wrap` pixels long.
        """

        wx.Dialog.__init__(self, parent, wx.ID_ANY, caption, pos, size, style)

        self._agwStyle = agwStyle

        if wrap > 0:
            message = self.WrapMessage(message, wrap)

        topsizer = wx.BoxSizer(wx.VERTICAL)
        icon_text = wx.BoxSizer(wx.HORIZONTAL)

        case = self._agwStyle & wx.ICON_MASK
        
        if case == wx.ICON_ERROR:
            bitmap = _error

        elif case == wx.ICON_INFORMATION:
            bitmap = _information

        elif case == wx.ICON_WARNING:
            bitmap = _warning

        elif case == wx.ICON_QUESTION:
            bitmap = _question

        # Populate the sizers...
        icon = wx.StaticBitmap(self, -1, bitmap.GetBitmap())
        icon_text.Add(icon, 0, wx.ALIGN_CENTER_HORIZONTAL)
        icon_text.Add(self.CreateTextSizer(message), 1, wx.ALIGN_CENTER | wx.LEFT, 10)

        topsizer.Add(icon_text, 1, wx.EXPAND | wx.LEFT | wx.RIGHT | wx.TOP, 10)

        center_flag = wx.EXPAND
        if self._agwStyle & wx.YES_NO:
            center_flag |= wx.ALIGN_CENTRE

        sizerBtn = self.CreateSeparatedButtonSizer(self._agwStyle & BUTTON_SIZER_FLAGS)
        if sizerBtn:
            topsizer.Add(sizerBtn, 0, center_flag | wx.ALL, 10)

        self.SetAutoLayout(True)
        self.SetSizer(topsizer)

        topsizer.SetSizeHints(self)
        topsizer.Fit(self)
        size = self.GetSize()
        
        if size.x < size.y*3/2:
            size.x = size.y*3/2
            self.SetSize(size)

        self.Layout()
        self.Centre(wx.BOTH | wx.CENTER_FRAME)

        self.Bind(wx.EVT_BUTTON, self.OnYes, id=wx.ID_YES)
        self.Bind(wx.EVT_BUTTON, self.OnOk, id=wx.ID_OK)
        self.Bind(wx.EVT_BUTTON, self.OnNo, id=wx.ID_NO)
        self.Bind(wx.EVT_BUTTON, self.OnCancel, id=wx.ID_CANCEL)

        for child in self.GetChildren():
            if isinstance(child, wx.lib.buttons.ThemedGenBitmapTextButton) or \
               isinstance(child, AB.AquaButton) or isinstance(child, GB.GradientButton):
                # Handles the key down for the buttons...
                child.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
                if isinstance(child, wx.lib.buttons.ThemedGenBitmapTextButton):
                    child.SetUseFocusIndicator(False)

        self.Bind(wx.EVT_NAVIGATION_KEY, self.OnNavigation)
        self.SwitchFocus()
        
        
    def OnYes(self, event):
        """ L{GenericMessageDialog} had received a ``wx.ID_YES`` answer. """

        self.EndModal(wx.ID_YES)


    def OnOk(self, event):
        """ L{GenericMessageDialog} had received a ``wx.ID_OK`` answer. """

        self.EndModal(wx.ID_OK)


    def OnNo(self, event):
        """ L{GenericMessageDialog} had received a ``wx.ID_NO`` answer. """

        self.EndModal(wx.ID_NO)


    def OnCancel(self, event):
        """ L{GenericMessageDialog} had received a ``wx.ID_CANCEL`` answer. """

        # Allow cancellation via ESC/Close button except if
        # only YES and NO are specified.
        
        if self._agwStyle & wx.YES_NO != wx.YES_NO or self._agwStyle & wx.CANCEL:
            self.EndModal(wx.ID_CANCEL)


    def OnKeyDown(self, event):
        """
        Handles the ``wx.EVT_KEY_DOWN`` event for L{GenericMessageDialog}.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """

        key = event.GetKeyCode()
        if key == wx.WXK_ESCAPE:
            self.OnCancel(None)

        ids = []
        for child in self.GetChildren():
            if isinstance(child, wx.lib.buttons.ThemedGenBitmapTextButton) or \
               isinstance(child, AB.AquaButton) or isinstance(child, GB.GradientButton):
                ids.append(child.GetId())

        if key in [ord("y"), ord("Y")] and wx.ID_YES in ids:
            self.OnYes(None)
        elif key in [ord("n"), ord("N")] and wx.ID_NO in ids:
            self.OnNo(None)
        elif key in [ord("c"), ord("C")] and wx.ID_CANCEL in ids:
            self.OnCancel(None)
        elif key in [ord("o"), ord("O")] and wx.ID_OK in ids:
            self.OnOk(None)
            
        event.Skip()


    def OnNavigation(self, event):
        """
        Handles the ``wx.EVT_NAVIGATION_KEY`` event for L{GenericMessageDialog}.

        :param `event`: a `wx.NavigationKeyEvent` event to be processed.
        """

        # Switch the focus between buttons...
        if wx.GetKeyState(wx.WXK_LEFT) or wx.GetKeyState(wx.WXK_RIGHT) or \
           wx.GetKeyState(wx.WXK_DOWN) or wx.GetKeyState(wx.WXK_UP) or \
           wx.GetKeyState(wx.WXK_NUMPAD_LEFT) or wx.GetKeyState(wx.WXK_NUMPAD_RIGHT) or \
           wx.GetKeyState(wx.WXK_NUMPAD_DOWN) or wx.GetKeyState(wx.WXK_NUMPAD_UP) or \
           event.IsFromTab():
            event.Skip()
            wx.CallAfter(self.SwitchFocus)
            return

        button = wx.Window.FindFocus()
        buttonId = button.GetId()
        self.EndModal(buttonId)
            

    def SwitchFocus(self):
        """ Switch focus between buttons. """

        current = wx.Window.FindFocus()
        font = self.GetFont()
        boldFont = wx.Font(font.GetPointSize(), font.GetFamily(), font.GetStyle(), wx.BOLD,
                           font.GetUnderlined(), font.GetFaceName())
        
        for child in self.GetChildren():
            if isinstance(child, wx.lib.buttons.ThemedGenBitmapTextButton) or \
               isinstance(child, AB.AquaButton) or isinstance(child, GB.GradientButton):
                if child == current:
                    # Set a bold font for the current focused button
                    child.SetFont(boldFont)
                else:
                    # Restore the other buttons...
                    child.SetFont(font)
                child.Refresh()                

        
    def CreateButtonSizer(self, flags):
        """
        Creates a sizer with standard buttons.

        :param `flags`: a bit list of the following flags:

         ================= ========= ==========================
         Flags             Hex Value Description
         ================= ========= ==========================
         ``wx.YES``              0x2 Show a ``Yes`` button
         ``wx.OK``               0x4 Show an ``OK`` button
         ``wx.NO``               0x8 Show a ``No`` button
         ``wx.CANCEL``          0x10 Show a ``Cancel`` button
         ``wx.NO_DEFAULT``      0x80 Used with ``wx.YES`` and ``wx.NO``, makes ``No`` button the default
         ``wx.HELP``          0x8000 Show a ``Help`` button
         ================= ========= ==========================
        
        :note: The sizer lays out the buttons in a manner appropriate to the platform.
        """

        sizer = self.CreateStdDialogButtonSizer(flags)

        return sizer


    def CreateSeparatedButtonSizer(self, flags):
        """
        Creates a sizer with standard buttons using L{CreateButtonSizer} separated
        from the rest of the dialog contents by a horizontal `wx.StaticLine`.

        :param `flags`: the button sizer flags.

        :see: L{CreateButtonSizer} for a list of valid flags.
        """

        sizer = self.CreateButtonSizer(flags)

        # Mac Human Interface Guidelines recommend not to use static lines as
        # grouping elements
        if wx.Platform != "__WXMAC__":
            topsizer = wx.BoxSizer(wx.VERTICAL)
            topsizer.AddF(wx.StaticLine(self), wx.SizerFlags().Expand().DoubleBorder(wx.BOTTOM))
            topsizer.AddF(sizer, wx.SizerFlags().Expand())
            sizer = topsizer
            
        return sizer


    def CreateStdDialogButtonSizer(self, flags):
        """
        Creates a L{StdDialogButtonSizer} with standard buttons.

        :param `flags`: the button sizer flags.

        :see: L{CreateButtonSizer} for a list of valid flags.

        :note: The sizer lays out the buttons in a manner appropriate to the platform.
        """
        
        sizer = StdDialogButtonSizer()
        no = yes = ok = None
        if flags & wx.OK:
            # Remove unwanted flags...
            flags &= ~(wx.YES | wx.NO | wx.NO_DEFAULT)

        size=(-1, 28)
        if self._agwStyle & GMD_USE_AQUABUTTONS:
            klass = AB.AquaButton
            size=(-1, -1)
        elif self._agwStyle & GMD_USE_GRADIENTBUTTONS:
            klass = GB.GradientButton
        else:
            klass = buttons.ThemedGenBitmapTextButton
            
        if flags & wx.OK:
            ok = klass(self, wx.ID_OK, _ok.GetBitmap(), _("OK"), size=size)
            sizer.AddButton(ok)

        if flags & wx.CANCEL:
            cancel = klass(self, wx.ID_CANCEL, _cancel.GetBitmap(), _("Cancel"), size=size)
            sizer.AddButton(cancel)
        
        if flags & wx.YES:
            yes = klass(self, wx.ID_YES, _yes.GetBitmap(), _("Yes"), size=size)
            sizer.AddButton(yes)
        
        if flags & wx.NO:
            no = klass(self, wx.ID_NO, _no.GetBitmap(), _("No"), size=size)
            sizer.AddButton(no)
                
        if flags & wx.HELP:
            help = klass(self, wx.ID_HELP, _help.GetBitmap(), _("Help"), size=size)
            sizer.AddButton(help)
        
        if flags & wx.NO_DEFAULT:
            if no:
                no.SetDefault()
                no.SetFocus()
        else:
            if ok:
                ok.SetDefault()
                ok.SetFocus()
            elif yes:
                yes.SetDefault()
                yes.SetFocus()
            
        if flags & wx.OK:
            self.SetAffirmativeId(wx.ID_OK)
        elif flags & wx.YES:
            self.SetAffirmativeId(wx.ID_YES)

        sizer.Realize()

        return sizer


    def WrapMessage(self, message, wrap):
        """
        Wraps the input message to multi lines so that the resulting new message
        is at most `wrap` pixels wide.

        :param `message`: the original input message;
        :param `wrap`: wraps the string in `message` so that every line is at most
         `wrap` pixels long.

        :return: a new message wrapped at maximum `wrap` pixels wide.
        
        :todo: Estabilish if wrapping all messages by default is a better idea than
         provide a keyword parameter to L{GenericMessageDialog}. A default maximum
         line width might be the wxMac one, at 360 pixels.
        """
        
        dc = wx.ClientDC(self)
        dc.SetFont(self.GetFont())

        newMessage = wordwrap.wordwrap(message, wrap, dc, False)
        return newMessage
    
