# --------------------------------------------------------------------------------- #
# MULTIDIRDIALOG wxPython IMPLEMENTATION
#
# Andrea Gavana, @ 07 October 2008
# Latest Revision: 12 Sep 2010, 10.00 GMT
#
#
# TODO List
#
# 1) Implement an meaningful action for the "Make New Folder" button, but this
#    requires a strong integration with Explorer, at least on Windows;
#
# 2) Be more user-friendly with special folders as the Desktop, My Documents etc...
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
This class represents a possible replacement for `wx.DirDialog`, with the additional
ability of selecting multiple folders at once.


Description
===========

This class represents a possible replacement for `wx.DirDialog`, with the additional
ability of selecting multiple folders at once. It may be useful when you wish to
present to the user a directory browser which allows multiple folder selections.
MultiDirDialog sports the following features:

* Ability to select a single or mutliple folders, depending on the style passed;
* More colourful and eye-catching buttons;
* Good old Python code :-D .

And a lot more. Check the demo for an almost complete review of the functionalities.


Supported Platforms
===================

MultiDirDialog has been tested on the following platforms:
  * Windows (Windows XP).


Window Styles
=============

This class supports the following window styles:

===================== =========== ==================================================
Window Styles         Hex Value   Description
===================== =========== ==================================================
``DD_NEW_DIR_BUTTON``       0x000 Enable/disable the "Make new folder" button
``DD_DIR_MUST_EXIST``       0x200 The dialog will allow the user to choose only an existing folder. When this style is not given, a "Create new directory" button is added to the dialog (on Windows) or some other way is provided to the user to type the name of a new folder.
``DD_MULTIPLE``             0x400 Allows the selection of multiple folders.
===================== =========== ==================================================


Events Processing
=================

`No custom events are available for this class.`


License And Version
===================

MultiDirDialog is distributed under the wxPython license.

Latest Revision: Andrea Gavana @ 12 Sep 2010, 10.00 GMT

Version 0.3

"""

import os
import wx
import wx.lib.buttons as buttons

from wx.lib.embeddedimage import PyEmbeddedImage

# MultiDirDialog styles
DD_MULTIPLE = 1024
""" Allows the selection of multiple folders. """
DD_DEFAULT_STYLE = wx.DD_DEFAULT_STYLE
""" Equivalent to a combination of ``wx.DEFAULT_DIALOG_STYLE`` and ``wx.RESIZE_BORDER``. """
DD_DIR_MUST_EXIST = wx.DD_DIR_MUST_EXIST
""" The dialog will allow the user to choose only an existing folder. When this style""" \
""" is not given, a "Create new directory" button is added to the dialog (on Windows)""" \
""" or some other way is provided to the user to type the name of a new folder. """
DD_NEW_DIR_BUTTON = wx.DD_NEW_DIR_BUTTON

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
_cdrom = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAArRJ"
    "REFUOI11kc9rm3Ucx1/f5/eTLV2aJ2vqVseGWzeYDAbCCq2THQqiuB3mP+DBQ3ss3rysILLb"
    "2I5FhKHkNFmFHkrFoVDQDautI02ZWqGJ7WzWEkzy/M73u1NKbNj79Dl8Xi8+P+BQhoeHj09N"
    "Td1aWlr6s1qtNjY3N/dLpdIvExMTHwPG4f7/ZWRk5M35+fmnqidSSqWUUlEUqdnZ2W+B3Kv4"
    "wbm5uaeNRkO1220VRZEKw1D5vq/CMDwQTk9PfwVoffTk5ORMpVJR5XJZ1Wo1FYahCoJAtVot"
    "laapSpJEBUGgNjY2VLFYvNblDkzj4+PvJ0kCgJSSvb09tv7eiuv/1tMgDGg2m+zu7mKaJmNj"
    "Yx90uYOj5PP5k2ma4jgOuqbT/K/JvYd3n4+eOu9cH7s+lMiE/f19hBAUCoUzfYIkSYJ8Po+u"
    "69i2TZIk3Hz3w1MqUtT36iRpgu/7ZDIZfN+P+1ZYXV39bWBgANd1OZo9ilfwuDB0gYunL+K4"
    "Dq1WCyEEcRyztra22idYWFj4srxW9j3PQ0pJo9EADWzHxvM8juWO4doZln9c3llfX/+my+nd"
    "IgzDrUpceeftS1ffcHSX+os6Ukosy8I0THJHBnn87Cduf/H5/dZO++s+AcA/V2sfbYa/nmFb"
    "QwYamjJACWrbVVY2HvMDiyxXnvzMXyz2HRGw8ifJ+6N/sNi+QzE4jbd9Auu5g3Jh6OxrjGZP"
    "4HgUgh6oV2B++tZngxOXr2AbBpZpYGomujIR0kTFOqmQ/P56NVfiQb8gm80640fey9nPLKI4"
    "IkKhAKk6CDocHyqQcVyuFK8NlnhgAOnhCag36k6pdJ92u43ruliWhRACgWDmkxl27G2anVam"
    "93uih9dv3Lh569y5s5fjuCMNQ6BpIIROp9NB13XQ0R599+j7lZUnd7rQS0kMSYjIJmZ4AAAA"
    "AElFTkSuQmCC")

#----------------------------------------------------------------------
_computer = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAshJ"
    "REFUOI1dk91PXGUQxn/ve979YkHXmhZ325oUa9Wlxg9q8caoCRde9Hb/Bv4KQ8J/AXdceFF6"
    "YYJXNCYWE0xITAxt1cjXKpiYsnQpH3v2sOfMTC8WAZ2rmWSeZ56ZJ+MAzKwEXIHEQ5ELYedp"
    "QpKcV8Vise2cOwwAnU63sdFsTx0cnpiJoipkqkiaIaa0Wh2etQ4tyxRVo1xy0eefXf0G+DoA"
    "ZJlea/7VGRksF1B1iICIQwUyEb79boMfl/8khDy4wLVamdF3X33LzHwAUJQ47k82U1QVkX7e"
    "3u+y2XyB9w7THkZGlnkUNYDw705HHeXgqIdZH6wqmCq/r7XZPzBCroRKSvDKrZsVIt/HnREc"
    "x8bRcYaZoCKICCIZf2wcY65IFAIQeOdWhfdH30D1PwSeuOvYfS5wqkBEiOMeu3t6Oj2jXC4x"
    "+l6NblfO7Al9OMSJp9V2YJwe0XhxIPSyHFEAH2Vcvz5AL4vY2z85RV1YodM1dp8bDodI34nj"
    "Y4+LSkQuUCwYUcjz9z8ppYLiLipQlLiT0NpLCCEHOFQDIuCDxzRgTtnd6zt1+RL4KLqgQDP9"
    "6oscI28mmPVwPiKKgoUw4CLvyLLURFKX9nqc9E4oBCUfsnMbvfff3/lgoHK50vLLPy3zbLcV"
    "jdy48eHdjz75slAouidPnj7+7denj1wUpXc+HrPq1ZqrDlcfOec0AFQqlZ8bjcYvW1tbgyJy"
    "d3x8/F6pOHBlsPyKS9MeWZq+liS9oZWVlYcP7j/4YWJiYn92djY9e4xGoxEBQ8Db09PTC5ub"
    "m7a+vmZLS0u2uLhoq6urtr29bXNzc4+HhoY+BS6NjY3lgLNjMj8/Hy0sLBTb7fbtarV6r16v"
    "387n86+LiHfOHTabzfW1tbWHuVxueXh4uDMzM5M55+yM4GJMTU35OI7LOzs7AyLiarVaUq/X"
    "O5OTk+n/e18CKWqFGqiV9Q4AAAAASUVORK5CYII=")

#----------------------------------------------------------------------
_folder_close = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAcBJ"
    "REFUOI2Nk0FrE1EQx3+zL7vbmFKRQkE/gyfpISL4WYJHsQfPvXkQe6go+AnEi6dcBBEkhYgS"
    "oQfFeqo0VJRobaspm2R339sdD9u0IclC/zAwzLz5zbzhPeFUnU5HuYDq9brMBB+/+KQXVavV"
    "+jBZWxk79zffl3Z9dO8GQbCAiAAEM4DvvyI21m4SpyCiaK6ogqqiwN2nWzxbu0W1WpuBn00w"
    "ih3H/YwkcbgMnFOcU5K0yKdJTLVawzk3H3D8z9GPHKqKy3QGYK0Fiqkm5Y2do77l3ec+mQhD"
    "q+eWFgXjzr1ebzgXgBG2u0O+7A/JRYhzCjttqJrTbDb3G43G7blXyEQIlkI+dmNiPK5dqeBE"
    "sJoXO7CGdru9VbrEXDzCyyEisPPH8XOgrCwaFgysXl/lwcttLqWjmUd0BnCeh78UYgQQiJxy"
    "cpJj8gxcUZdbVw5Q47FYM1REESBTSJ0htYZkVCxChXKAXxGWq4bAnAPiDAbWMPCFEbD7bfew"
    "FPD34Hfa3TuKlmth4PsVycWTQYoeDp39EXnpVVh522pvTgPGI4V3Nt7E08lJvXr+ZP3g6+uH"
    "ZYB5EsCn+DwhEAHJ9KH/LOgEF+oT+k8AAAAASUVORK5CYII=")

#----------------------------------------------------------------------
_folder_open = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAvdJ"
    "REFUOI1tU01o23UYft7/75ekqXVJWsO6HaZWzDyIih4UCkJBGIroUATxWPEyLx6EDZzz5Dzr"
    "xB3FkygoCA7ZR0A3g60zWdutbrZLTNOvJI1pmvyT/8fv4/WQFFzwgZf36zm8HzyEeyEAEAAD"
    "gAexM/B6iEsDAwak4eZwzXk48/gTr7w5+04qlUoMkwEgnnnyuekPz372xbGXjr8GAKPp9OSL"
    "L796/PQnn57/5Y/lv9q+tp3A8KEHM48BqcT4o888CwA49taJ04vFeqPta22s5Wtz+cL5r77/"
    "YW3HdY213Pa1Xd5w+WK+Yr/NremzX17MLZVqrQu5m/MEgM6c+yb70btvzLR6RlmtEInGhKvg"
    "lDcb2Nr1jR+yTSbvc6YeGHGOpOMYjZAlInEhm7sm3/7g8+/em319ZqPR061OV5Z3Amq4yhoW"
    "eiI1JqYOJZ3JiTFxIMasNEMb0FbHU2OjI6LWaAdyYXHpTk8T1qpN+unmHo4emUT64LhzOCGd"
    "iNM/T3VPoWZCsk6Eo8KBUobageev77Q6cnE++2O+2DwVeBCuZ3g8GYfraax4/a9FRP8ZQgDM"
    "ISJSQAeKtN/tblabDal3ioW529XVp6fSmZqr+M62j0AzAYAjJKKOQlT2J2FmRIQBQE6n6QWl"
    "zXpTAvAvfX3u16MnP85U29r+vNIT9a5BIi4BADEBjEQIsQgBzBBEDCK5sFT5Z+VWZUsCwFpp"
    "9cr2bm+27QGtPUZUAnuDFYwBg4jAzIBmrZQlZlHZ5UTY9RwJAI3SQm61VPZk9P54CHCfvA/D"
    "VimjtCUCRGiYAKJY2Eg6brEuARD83crq8o1C9KEXpgOC0caQ0YqtNk5oyQGYwm6LwvpK26tc"
    "z7ul61ld//MytHdDDgSky7fmsgcPPz/t6Q4TWABA0Nwgv3Z7y/v7t5y/XrikGsWrAO4CsPvz"
    "yf1k++7vl+mp2hkVdE2wkV/2y/NXe+uFK/Ba8wDqw8IaePvf4gE58cj7ELEZAKP/o859qd+D"
    "fwFu65tDy5tMTAAAAABJRU5ErkJggg==")

#----------------------------------------------------------------------
_hd = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAxlJ"
    "REFUOI11ks1vG1UUxc+b98YzyfM4gxzSpHHttCFRbQto7YQgxRKIRRYIZRepEkQVzYJ/gAUi"
    "i/wlSLBhFSEWZMUiQkLCSmUk0o6rNiExcvNRR/Z0xh6P5/OxqFwlfNzVWZz7u0dXh+B/Zn19"
    "Pd1qtQpxHBeiKCoKId4aHR39dGdnx7zsY0Oxurq6FIbhZ4yxwgeVcnHuVuaarmvwPB/nL9r4"
    "/Y+neHrQ+BLA5mUAHYp8Pr8uSdLXD+7fu/nRh5XktckpKOoIONcwNzeP998r483x8bvKSPKb"
    "/f19Z7gnDYUsyz+nUiksLi4ioWqIBcFfBw/R/LOK1nkTCVVDpVJJLpbvfPWfCQzDON/e3v7k"
    "9szk9Z7VwUuzA4WnoaXS6LQ7CAYD2C/bODlr3c3O3Pq2Vqt1ryQghKDb7X7XPG9BH5/ExNQN"
    "DPo9nJ2+wMxsEfr4JPhYGkEYqrIsb/4rAQBwzp+HUfRF5no6MX1jFlOZmxAihtVuYpSnYDyq"
    "QdUm0O323i2Xy99Xq1XzCiCZTPqcp/K247192jxA4DmI4wDPT88xMZXF7NxtPDaeIZfLUdd1"
    "39jd3f2RXAYIIcjS0tLHy8vLP42NjUGWZTDGIEkS4jiGruuglIIQAtd1o5OTk3fYZQAhRGia"
    "Vi0Wi0/m5+fzhFzhAwBc14VlWbAsi5qmeZ/901AqlazDw8MfSqXSZiKRgOM4sG0bpmmi0+mg"
    "3++DUtpWFOWR53m/vT6xtbUl1et1cnR0JDUajTsrKyu/+L4/4nleGIZhw/O8x0EQPLQs69fj"
    "4+Mnuq73NjY2PLK2tkYNw6CmaTLP85jv+wnf99O5XO7zKIrMs7OzZ77vdwkhPiHEppSaiqLY"
    "09PTjmEYASkUCgnbtqnruiwIAjkMQyWKIkUIoQohZACyEIK+ehEJCCEOY8zmnPey2azHisVi"
    "ZBjGq15LkqCURmEY+nEc94UQVAgxLJuQJClkjAWqqvrpdDqo1WohrdfrotVqxbZtR47jRJzz"
    "kDEWaJrmqao64Jy7nHNX1/V+JpNxFxYWBnt7e/7FxUUEAH8DenV0NY9j5foAAAAASUVORK5C"
    "YII=")

#----------------------------------------------------------------------
_new = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAqpJ"
    "REFUOI1t0c1rXGUUx/Hvc+/czGtmEk1NTCYWY1BERAJVRMW4cCEIunHTnYjgQrduRHDTrX+F"
    "u+ZvEF2IRVCRRkpt1VSTmqQ1rzNz733uc855XDSRkfHsz+ec3zmO/6mrVz9vPjHTm52e6c23"
    "mq2lmpPFVE/6qrYUcXMON+2y7NP5Zy9/Wxtv/Gx9vXb5ynsfLczPvZnV0kfT+uycq6WdmO/V"
    "82GaNtsPucx5NAyqoDYD8B+gc2m53mk13pluZy9DgptK8b5kZ/sPkqzH4xdmiMUeopJU4jKA"
    "ZBwYDo4j0cRUiDESo3B8uMfmjV85Hea4GIgmqIRExJoTwGFd1LBgKpgZJp4qP6YoSqIJ0c4A"
    "DS5xNjURwfv7Fk28acC5Gi6MsGqIqUA0iIKZYKYuOEsngKOjFZMgXlVIkgBSIOIxOwMsoBIQ"
    "FSwGNxFhY2MjqkpQC2jwiB+gUiEqBA1UVYlIhYgSQmBiAwAViaqCaSCGHJGKO+6EnYMf+ObH"
    "67zYW2C50aXSB701wAEZ0HzjlbWLVfArKlOgHvTBNO1FwsIBh6OK1aLNQtImRmmdAy2gD3Sf"
    "ear/em/+ybWg+0g+4Pt7f7IzOkVmhXovoJmwuXeXraMDsE7jHPBAClwog8yS9ZJQ3qUoCm76"
    "Q/J+TqsraDPH0iF3yl2G96B2uvxvBDmL8fAoL6crVVxZEipBNDCo/qYq95HkmMoLeQVVaNKN"
    "uPEjeqCd+9C9+VfOonkyNS5al/Yu3J4qOJ3bJamarBw8x5R0bt0oTr4eB7aAzbIIa8lop4hp"
    "WSPJ9p+fX71tMf3p59+3Xy1j+kISUh5L+5tvP73+Qf+196+NAwp8d+u37ft+/5evWquLmrSa"
    "17uN/vbSpbfylz5Z+bg7eoQsNtIv/daVD9/94tr52/8BSS2agPSymFoAAAAASUVORK5CYII=")

#----------------------------------------------------------------------
_removable = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABHNCSVQICAgIfAhkiAAAAldJ"
    "REFUOI2lkbFOXFcQhr+Zc+7uXRZvFi2GOFHiSInA7CLFCnaRCokij0DjyElFh9K6sfwQeQEX"
    "rngAvwBNsCyniNCChCKFSCYOkWADd/fuvfeccUEgrCU3yS+NNNN88/8z8D8l14d+v38jTdv3"
    "fD3tqEpdRJxE1AzDYYCFoswGg8HL5eXPDwH8dYBzzR8/aM9855x778ZQC2TZ+TEwB6ATAK8r"
    "SZLgvefP316SZed47zl902fw+hXOOZxTfKKzz5//1Jpw8OSJaZIcfyFykeqPw1+QX7f5aOEb"
    "pP+Iqv4p1YdfUlUF3omUbtwB/r5ysLa2cztNG+nl3P36W5qzPaZ2HzL67BH15ceMxxnjYkiM"
    "FXOt5mSEznxn0aliZoSqRFXofNzjIHnA9M0F1HvG4yFVOQag0UhnJiIkkixEixTFiBgqQqg4"
    "G/xFdfY7+eicNG0g6nBaQ6SiVivmJgDiZKEsc8CIoSLGQNqc4cbS9zSmpvEuQZ1D1CFS4BJ/"
    "cwJQFKPFIRGwC6Aq6mp0Zm9hplQR1AzRSFUFLFhnAvBi5+e7z549Jc9zzGzy+WYYsHinS7N7"
    "wnyyxNIn9+evAKurqx5ke3Pzh68smkRMMDCLglz2iHOJjeKJNXw7HB0dvwCQlZWV5ODgoFkU"
    "RTOE0DSzGpCYmf9ngQfUzFREKqAQkaH3/qTb7b5xGxsb7O3tWVmWAPGyRCQApYgUIpKLyFBV"
    "z1X1zHs/aLfbp/v7+4X8G9NkfX1dd3d3XZZlmue5izFKjFFU1er1emi1WqHX64Wtra0oIu8c"
    "6j/qLUda/yKP2243AAAAAElFTkSuQmCC")

#----------------------------------------------------------------------


class MultiDirDialog(wx.Dialog):
    """
    A different implementation of `wx.DirDialog` which allows multiple
    folders to be selected at once.
    """

    def __init__(self, parent, message=_("Choose one or more folders:"), title=_("Browse For Folders"),
                 defaultPath="", style=wx.DD_DEFAULT_STYLE, agwStyle=DD_MULTIPLE, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, name="multidirdialog"):
        """
        Default class constructor.

        :param `parent`: the dialog parent widget;
        :param `message`: the message to show on the dialog;
        :param `title`: the dialog title;
        :param `defaultPath`: the default path, or the empty string;
        :param `style`: the underlying `wx.Dialog` window style;
        :param `agwStyle`: the AGW-specific dialog style; this can be a combination of the
         following bits:

         ===================== =========== ==================================================
         Window Styles         Hex Value   Description
         ===================== =========== ==================================================
         ``DD_NEW_DIR_BUTTON``       0x000 Enable/disable the "Make new folder" button
         ``DD_DIR_MUST_EXIST``       0x200 The dialog will allow the user to choose only an existing folder. When this style is not given, a "Create new directory" button is added to the dialog (on Windows) or some other way is provided to the user to type the name of a new folder.
         ``DD_MULTIPLE``             0x400 Allows the selection of multiple folders.
         ===================== =========== ==================================================

        :param `pos`: the dialog position;
        :param `size`: the dialog size;
        :param `name`: the dialog name.
        """

        wx.Dialog.__init__(self, parent, pos=pos, size=size, style=wx.DEFAULT_DIALOG_STYLE|wx.RESIZE_BORDER, name=name)

        self.agwStyle = agwStyle
        
        self.dirCtrl = wx.GenericDirCtrl(self, size=(300, 300), style=wx.DIRCTRL_3D_INTERNAL|wx.DIRCTRL_DIR_ONLY)
        self.folderText = wx.TextCtrl(self, -1, defaultPath, style=wx.TE_PROCESS_ENTER)
        self.CreateButtons()
        
        self.SetProperties(title)           
        # Setup the layout and frame properties        
        self.SetupDirCtrl(defaultPath)
        self.LayoutItems(message)
        self.BindEvents()
    
        if parent and pos == wx.DefaultPosition:
            self.CenterOnParent()
            

    def SetupDirCtrl(self, defaultPath):
        """
        Setup the internal `wx.GenericDirCtrl` (icons, labels, etc...).

        :param `defaultPath`: the default path for L{MultiDirDialog}, can be an
         empty string.
        """

        il = wx.ImageList(16, 16)

        # Add images to list. You need to keep the same order in order for
        # this to work!

        # closed folder:
        il.Add(_folder_close.GetBitmap())

        # open folder:
        il.Add(_folder_open.GetBitmap())

        # root of filesystem (linux):
        il.Add(_computer.GetBitmap())

        # drive letter (windows):
        il.Add(_hd.GetBitmap())

        # cdrom drive:
        il.Add(_cdrom.GetBitmap())

        # removable drive on win98:
        il.Add(_removable.GetBitmap())

        # removable drive (floppy, flash, etc):
        il.Add(_removable.GetBitmap())

        # assign image list:
        treeCtrl = self.dirCtrl.GetTreeCtrl()
        treeCtrl.AssignImageList(il)

        if self.agwStyle & DD_MULTIPLE:
            treeCtrl.SetWindowStyle(treeCtrl.GetWindowStyle() | wx.TR_MULTIPLE)

        if not defaultPath.strip():
            return
        
        # Set the wx.GenericDirCtrl default path
        self.dirCtrl.ExpandPath(defaultPath)
        self.dirCtrl.SetDefaultPath(defaultPath)
        self.dirCtrl.SetPath(defaultPath)

        self.folderText.SetValue(treeCtrl.GetItemText(treeCtrl.GetSelections()[0]))
        

    def SetProperties(self, title):
        """
        Sets few properties for the dialog.

        :param `title`: the dialog title.
        """

        self.SetTitle(title)
        self.okButton.SetDefault()

        if self.agwStyle & wx.DD_DIR_MUST_EXIST:
            self.newButton.Enable(False)


    def LayoutItems(self, message):
        """ Layout the widgets using sizers. """

        mainSizer = wx.BoxSizer(wx.VERTICAL)
        textSizer = wx.BoxSizer(wx.HORIZONTAL)
        bottomSizer = wx.BoxSizer(wx.HORIZONTAL)
        
        staticText = wx.StaticText(self, -1, message)
        f = staticText.GetFont()
        f.SetWeight(wx.BOLD)
        staticText.SetFont(f)

        # Add the main wx.GenericDirCtrl        
        mainSizer.Add(staticText, 0, wx.EXPAND|wx.ALL, 10)
        mainSizer.Add(self.dirCtrl, 1, wx.EXPAND|wx.ALL, 10)

        label = wx.StaticText(self, -1, _("Folder:"))
        textSizer.Add(label, 0, wx.LEFT|wx.RIGHT|wx.ALIGN_CENTER_VERTICAL, 10)
        textSizer.Add(self.folderText, 1, wx.RIGHT|wx.EXPAND|wx.ALIGN_CENTER_VERTICAL, 10)
        mainSizer.Add(textSizer, 0, wx.EXPAND|wx.BOTTOM, 10)

        # Add the fancy buttons
        bottomSizer.Add(self.newButton, 0, wx.ALL, 10)
        bottomSizer.Add((0, 0), 1, wx.EXPAND)
        bottomSizer.Add(self.okButton, 0, wx.TOP|wx.BOTTOM, 10)
        bottomSizer.Add(self.cancelButton, 0, wx.TOP|wx.BOTTOM|wx.RIGHT, 10)
        mainSizer.Add(bottomSizer, 0, wx.EXPAND)

        # Layout the dialog
        self.SetSizer(mainSizer)
        mainSizer.Layout()
        mainSizer.Fit(self)
        mainSizer.SetSizeHints(self)


    def GetPaths(self):
        """ Returns the folders selected by the user, or the default path. """

        # Retrieve the tree control and the selections the
        # user has made
        treeCtrl = self.dirCtrl.GetTreeCtrl()
        selections = treeCtrl.GetSelections()

        folders = []

        # Loop recursively over the selected folder and its sub-direcories
        for select in selections:
            itemText = treeCtrl.GetItemText(select)
            # Recurse on it.
            folder = self.RecurseTopDir(treeCtrl, select, itemText)
            folders.append(os.path.normpath(folder))

        return folders


    def RecurseTopDir(self, treeCtrl, item, itemText):
        """
        Recurse a directory tree to include the parent-folder.

        :param `treeCtrl`: the tree control associated with teh internal `wx.GenericDirCtrl`;
        :param `item`: the selected tree control item;
        :param `itemText`: the selected tree control item text.
        """

        # Get the item parent        
        parent = treeCtrl.GetItemParent(item)
        if parent != treeCtrl.GetRootItem():
            # Not the root item, recurse again on it
            itemText = treeCtrl.GetItemText(parent) + "/" + itemText
            itemText = self.RecurseTopDir(treeCtrl, parent, itemText)

        return itemText
    

    def BindEvents(self):
        """ Binds the events to specific methods. """

        self.Bind(wx.EVT_BUTTON, self.OnOk, self.okButton)
        self.Bind(wx.EVT_BUTTON, self.OnCancel, self.cancelButton)
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.Bind(wx.EVT_CHAR_HOOK, self.OnKeyUp)
        self.dirCtrl.GetTreeCtrl().Bind(wx.EVT_TREE_SEL_CHANGED, self.OnSelChanged)


    def CreateButtons(self):
        """ Creates the ``OK``, ``Cancel`` and ``Make New Folder`` bitmap buttons. """
        
        # Build a couple of fancy buttons
        self.newButton = buttons.ThemedGenBitmapTextButton(self, wx.ID_NEW, _new.GetBitmap(),
                                                           _("Make New Folder"), size=(-1, 28))
        self.okButton = buttons.ThemedGenBitmapTextButton(self, wx.ID_OK, _ok.GetBitmap(),
                                                          _("OK"), size=(-1, 28))
        self.cancelButton = buttons.ThemedGenBitmapTextButton(self, wx.ID_CANCEL, _cancel.GetBitmap(),
                                                              _("Cancel"), size=(-1, 28))


    def OnOk(self, event):
        """
        Handles the ``wx.EVT_BUTTON`` event for the dialog.

        :param `event`: a `wx.CommandEvent` event to be processed.

        :note: This method handles the ``OK`` button press. 
        """

        self.EndModal(wx.ID_OK)


    def OnCancel(self, event):
        """
        Handles the ``wx.EVT_BUTTON`` event for the dialog.

        :param `event`: a `wx.CommandEvent` event to be processed.

        :note: This method handles the ``Cancel`` button press. 
        """

        self.OnClose(event)


    def OnClose(self, event):
        """
        Handles the ``wx.EVT_CLOSE`` event for the dialog.

        :param `event`: a `wx.CloseEvent` event to be processed.
        """

        self.EndModal(wx.ID_CANCEL)


    def OnKeyUp(self, event):
        """
        Handles the ``wx.EVT_CHAR_HOOK`` event for the dialog.

        :param `event`: a `wx.KeyEvent` event to be processed.
        """
        
        if event.GetKeyCode() == wx.WXK_ESCAPE:
            # Close the dialog, no action
            self.OnClose(event)
        elif event.GetKeyCode() in [wx.WXK_RETURN, wx.WXK_NUMPAD_ENTER]:
            # Close the dialog, the user wants to continue
            self.OnOk(event)

        event.Skip()


    def OnSelChanged(self, event):
        """
        Handles the ``wx.EVT_TREE_SEL_CHANGED`` event for the tree control associated
        with L{MultiDirDialog}.

        :param `event`: a `wx.TreeEvent` event to be processed.        
        """

        if self.IsBeingDeleted():
            # We are being destroyed...
            event.Skip()
            return
        
        item = event.GetItem()
        if not item.IsOk():
            # Bad item?
            event.Skip()
            return
        
        treeCtrl = self.dirCtrl.GetTreeCtrl()
        text = treeCtrl.GetItemText(item)
        # Set the item name into the text control
        self.folderText.SetValue(text)
        self.folderText.Refresh()

        event.Skip()
        
