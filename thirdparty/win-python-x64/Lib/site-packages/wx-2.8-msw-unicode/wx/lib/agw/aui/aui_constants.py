"""
This module contains all the constants used by wxPython-AUI.

Especially important and meaningful are constants for AuiManager, AuiDockArt and
AuiNotebook.
"""

__author__ = "Andrea Gavana <andrea.gavana@gmail.com>"
__date__ = "31 March 2009"


import wx
from wx.lib.embeddedimage import PyEmbeddedImage

# ------------------------- #
# - AuiNotebook Constants - #
# ------------------------- #

# For tabart
# --------------

vertical_border_padding = 4
""" Border padding used in drawing tabs. """

if wx.Platform == "__WXMAC__":
    nb_close_bits = "\xFF\xFF\xFF\xFF\x0F\xFE\x03\xF8\x01\xF0\x19\xF3" \
                    "\xB8\xE3\xF0\xE1\xE0\xE0\xF0\xE1\xB8\xE3\x19\xF3" \
                    "\x01\xF0\x03\xF8\x0F\xFE\xFF\xFF"
    """ AuiNotebook close button image on wxMAC. """
    
elif wx.Platform == "__WXGTK__":
    nb_close_bits = "\xff\xff\xff\xff\x07\xf0\xfb\xef\xdb\xed\x8b\xe8" \
                    "\x1b\xec\x3b\xee\x1b\xec\x8b\xe8\xdb\xed\xfb\xef" \
                    "\x07\xf0\xff\xff\xff\xff\xff\xff"
    """ AuiNotebook close button image on wxGTK. """
    
else:
    nb_close_bits = "\xff\xff\xff\xff\xff\xff\xff\xff\xe7\xf3\xcf\xf9" \
                    "\x9f\xfc\x3f\xfe\x3f\xfe\x9f\xfc\xcf\xf9\xe7\xf3" \
                    "\xff\xff\xff\xff\xff\xff\xff\xff"
    """ AuiNotebook close button image on wxMSW. """

nb_left_bits = "\xff\xff\xff\xff\xff\xff\xff\xfe\x7f\xfe\x3f\xfe\x1f" \
               "\xfe\x0f\xfe\x1f\xfe\x3f\xfe\x7f\xfe\xff\xfe\xff\xff" \
               "\xff\xff\xff\xff\xff\xff"
""" AuiNotebook left button image. """

nb_right_bits = "\xff\xff\xff\xff\xff\xff\xdf\xff\x9f\xff\x1f\xff\x1f" \
                "\xfe\x1f\xfc\x1f\xfe\x1f\xff\x9f\xff\xdf\xff\xff\xff" \
                "\xff\xff\xff\xff\xff\xff"
""" AuiNotebook right button image. """

nb_list_bits = "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x0f" \
               "\xf8\xff\xff\x0f\xf8\x1f\xfc\x3f\xfe\x7f\xff\xff\xff" \
               "\xff\xff\xff\xff\xff\xff"
""" AuiNotebook windows list button image. """


#----------------------------------------------------------------------
tab_active_center = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAAbCAYAAAC9WOV0AAAABHNCSVQICAgIfAhkiAAAADNJ"
    "REFUCJltzMEJwDAUw9DHX6OLdP/Bop4KDc3F2EIYrsFtrZow8GnH6OD1zvRTajvY2QMHIhNx"
    "jUhuAgAAAABJRU5ErkJggg==")
""" Center active tab image for the Chrome tab art. """

#----------------------------------------------------------------------
tab_active_left = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAA8AAAAbCAYAAACjkdXHAAAABHNCSVQICAgIfAhkiAAAAglJ"
    "REFUOI2Nkk9rE0EYh5/J7mpW06xE2iSmeFHxEoqIAc/FQ5CKgn4DP4KlIQG/QVsQbBEKgop+"
    "Anvy4rV4bLT2JCGJPVXqwaZJd+f1kN26WTfJDrzszDLPPL/5o0jeFGAC54A0YKmEYAo4DzjA"
    "LHAZmElqtIGrhmEsvtzcfPNtb6/V6524SWALKBiGsfhxe/uzFhGth5XEmgVubWxsvA1Az68k"
    "1nngYbPZ7ASg69c06wxwe3V9/b3reVqHwGmwCZRs2370fX//wIuA0+CLwEKj0XilZTSu602G"
    "FcP7vLe7+7XlRaCgPw62gGv5fP6p63raiwFdLWKOgdNArl6vV1UqpQgcYdcYbwooAPfb7c7h"
    "mTWmUjGwCWTL5fL1K6VSLiqQyMTYyLVa/UEwe9IC0chFYKnb/XnkeiIDV+Q0UsG/qNkCnEql"
    "crNQLDpaxpskJnYayD1bXl4S/xrDoPLHKjQOmsHwlCuHv44+ZJ2sLTrGGqzg7zEc+VK1Wl1w"
    "HMcG0DFxw6sFsRVwAZhdWak9FoRJ+w2HCKzzwN3jXv+daVmGDkdWoMKb9fumHz0DFFfX1p5Y"
    "lmXo6N0G48jzVEDOt97pdA9ezOXzGU+PzBmN6VuDqyoDN3Z2vjyfKxQynhYkJuJ/L02Ara3X"
    "n3602r8HrpaTUy3HAy1/+hNq8O+r+q4WETirmFMNBwm3v+gdmytKNIUpAAAAAElFTkSuQmCC")
""" Left active tab image for the Chrome tab art. """

#----------------------------------------------------------------------
tab_active_right = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAA8AAAAbCAYAAACjkdXHAAAABHNCSVQICAgIfAhkiAAAAkpJ"
    "REFUOI2NlM1rU0EUxX9zZ5KaWq3GKKnGutC0FEWCWAWLRUOxBetK/wdp6Re6F6TFXXGhuFdw"
    "b7dCQUUpiFt1XbB2q7Uf1iTvunjzkpe0afNgmLnDnHvOPe/OWCALtAFC+Cktfha4CRwBDnhg"
    "BQhaSrK19bf89dv35WfPX7y01haBbiAFmH3BlUA1Gm8WFt75BFkg0TK4VAl0Y3NL5+efvgIK"
    "wOH92EVjxRljGBi4VgTOeLDbk7kcqEZju1TWX7/Xgtm5J6+BS8ChvdilLhAhkUya4eFbxVQq"
    "1e3ZbUtgg8GKJd/Tk70/NjYCHCPsgX1kV8K5VA70z8amfvy0tAwMAcebSRfijikY8ez5/OlM"
    "JrOncbIjp4K1lmRb0sw8eDgCpAm7rwlz46YIzjpGb48WveyDNPhDfCOuHmNwzpHL5dK9fX3n"
    "mkmvaxJiayOCWMvM1PSdZtJrhiloLJMYIeESDFwf7Acyu0mXGLYmX0PpYi3ZbFdnoVDoBTpp"
    "uCxCjFob1tYKzlnGJyZHd5Mu6uVGkqvMCmCwzjE4eOMqcALoINauUic37hjhLXPWcTSdThWL"
    "QxcJX5yqdGk4H/cP9a4755iYnLpL+M/b8e0qjafrekb9TUskuNx/5TzQ5Y1zO9yOZEd1R7OI"
    "JdXebh/Pzt3zCToAMZv/AjU1orDWWKAGVJVSqcTqysp6X+/ZaeAL8KNac9wsVQ8yNeOsdZw8"
    "let4/2HpEdAPXDAb20HLj7xqeHT158ra4uLbz2bdg03krmetxrH9KDAmHP8Bn0j1t/01UV0A"
    "AAAASUVORK5CYII=")
""" Right active tab image for the Chrome tab art. """

#----------------------------------------------------------------------
tab_close = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAAwAAAAMCAYAAABWdVznAAAABHNCSVQICAgIfAhkiAAAAI9J"
    "REFUKJG90MEKAWEUxfEfM4rxAFIommzZzNb7v4BsLJTsiGQlYjHfME3flrO75/xvnXv5p/qY"
    "R/wcWTUktWCKFbrYB6/AAhecmwunAI/RwQAjbLGpoFakwjLATxzqMLQjC68A3/FohkljLkKN"
    "Ha4YKg8+VkBag3Pll9a1GikmuPk+4qMMs0jFMXoR/0d6A9JRFV/jxY+iAAAAAElFTkSuQmCC")
""" Normal close button image for the Chrome tab art. """

#----------------------------------------------------------------------
tab_close_h = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAAwAAAAMCAYAAABWdVznAAAABHNCSVQICAgIfAhkiAAAAOlJ"
    "REFUKJGVkiFuw0AQRd849hUS7iPUwGEllhyjYJ+gaK9Q4CsY9QTFIY4shQQucI8Q7l6h3Z0S"
    "r7UgjdrPZvVm52k0wpJLWe4y51qgVpECQFQnYPzabN4ra2cAAbgWxZMmyavAkTtROIn33fM0"
    "fcilLHep92+/wXHTd5K8JJlzbYD3w8C2aVZo2zTsh4FF5Zg516ZAHYBb35MbszbkxnDr+3hQ"
    "napIIUv1eT6vYPggvAGoSJE88r6XVFQnRA7BOdYIk8IUUZ1SYAQOsXOskRsT1+P/11pZO4v3"
    "ncLpESzed5W1c1jQn0/jBzPfck1qdmfjAAAAAElFTkSuQmCC")
""" Hover close button image for the Chrome tab art. """

#----------------------------------------------------------------------
tab_close_p = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAAwAAAAMCAYAAABWdVznAAAABHNCSVQICAgIfAhkiAAAASxJ"
    "REFUKJF9kbFLQlEYxX/nvbs55OAkiJAE7k7Nibo9xf+hrTlyr3Boipb+BCGq0bApJEQcG0Ms"
    "aQ0Lmq5+Dc+nDtbZ7uHce37fd8VSlWwh50PfRKqClWJXI8y6bu5uHj5e3wEEcJDP75txLBSx"
    "RYbdS7QfJ5PnsJIt5BbB4hQjkrQtjxlFILOXyvQDH/qmUCSJznDAYetkFTxsndAZDggkhCIf"
    "+qaLmWP1bu8oN+qrC+VGnd7t3bpKqrp4wBjl+ux8FUweSLwlXCnYCv2PHGgE1BLmTYykad2i"
    "kcOsi1TbZN7EKDfq67NZV5VsIeedvzQjCv5YK8R/4bw7Cl+/P7920+kJkBEq/hWWaPem45cQ"
    "YDybTfdSmf5CizckwHaAH9ATZldu7i560/ELwC+6RXdU6KzezAAAAABJRU5ErkJggg==")
""" Pressed close button image for the Chrome tab art. """

#----------------------------------------------------------------------
tab_inactive_center = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAAbCAYAAAC9WOV0AAAABHNCSVQICAgIfAhkiAAAAElJ"
    "REFUCJlVyiEOgDAUBNHp3qmX5iYkyMpqBAaFILRdDGn4qybZB98yy3ZZrRu1PpABAQiDSLN+"
    "h4NLEU8CBAfoPHZUywr3M/wCTz8c3/qQrUcAAAAASUVORK5CYII=")
""" Center inactive tab image for the Chrome tab art. """

#----------------------------------------------------------------------
tab_inactive_left = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAA8AAAAbCAYAAACjkdXHAAAABHNCSVQICAgIfAhkiAAAAf5J"
    "REFUOI2llE1rE1EUhp8bZwyhaZomk5DaD40hSWPQVkTd6KIIEUWlLqTEhTaLulBQ6sfKjeBC"
    "ECULXQku/Alx7d6/U1EQae45LjJpJ5NOnOKBgYG5z33Px3sG/iPMIc87QAmYBZKHgdOu69a2"
    "3/W2yrVGK5vPLTlxFV3Xrb3+8v1Ntd5oiSpWBmnEidKT972tar3R6ovSt4qoxoIdoFipNlpW"
    "B6AVRYFEHNWn3a8dz/PK1rIHEgN2UpnMseVTK7fUGBME48CFe88+3sh5+SXr1xmMSbABvJXz"
    "l9siYAVGWJ0Mu/OVZr5Q8CpWfFWzD2Imj2qu/fhtG4wRVUIZg0bDBsgtn15dt6qIKKBDQZ81"
    "kWmnzly6OZ+ZzhSt7jfK6CBjFMwEk5TWOy82AVQGhzVUb5RJEkC2fLK6JgIiPhioeZJJUhev"
    "3j2RTqdzooqge2ojCxwxqrnrG4/uq4Ida3HgAjMOJ4CZSq1+RVBUzCgQinDDstfa282jyeTU"
    "rhUGF4CJgMPKhbXbmw9VFfG7fBA4LCao7AAzi8cXz1kF0dENMqH38KgWnnd7nSMJxxE5wI4+"
    "MHyCaeeAYvPshQ0RJby3wVSDHxxgAVh99elb9/evndmfP3boW2FsqGNhMMCdBy8/fJ5KZ6at"
    "qL+3Q1dEzFkNGMX82ZWh18e0/vVT/wuFmdYVv/ruKgAAAABJRU5ErkJggg==")
""" Left inactive tab image for the Chrome tab art. """

#----------------------------------------------------------------------
tab_inactive_right = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAA8AAAAbCAYAAACjkdXHAAAABHNCSVQICAgIfAhkiAAAAhBJ"
    "REFUOI2llM9rE1EQxz8zb1dSTKNuYtW01kQDRoKFWi9FEEq1IooUUWoPokWCtVqkR69KsSBU"
    "8OJRPOhBxZNe/At6FBER/HFUPEq1IGn3ecgm2ZjdJODCHPY9vvP9fufNDPzHZ4DDQBrYBKwB"
    "ftfoJys/Kw9ef/1y8/6rh67rHgKS3WLl6cqqtcCGD58+vn+zdPXorUql8g5Y7wTWdd+y4Vus"
    "teQK+yfKi8/KwM5umBXAAgioCIP54gTQBzgdwTbsQZR0JpOfXXw+0w27hn9EBGMcyRcPnulJ"
    "pbKd2JvACKgKnpcePH99+TSwvT3YEphusKsqB4ZHp4FMNWUn5loSEVSFbZ63b8eeUhpwu5Md"
    "JBFRjHHk7LXb08CuNuAaZTgEEaFQHJoEvDjpakOYmnURUFWSvam+0ujJfqAnmlnABhG2jlTZ"
    "j19YuEzMm7dUu34hihrDQG7vGLCViPq0VruuvdquyWSvN3xsKhclvbXaoUQiihFlfLJ8iYiq"
    "O/EtUC2xGGF3vjAObAnI6stCsZbYCLwnEonNY+dulALvHWSH2YN2PXLq4hz/9HpjnmOs18DZ"
    "bP9IIL0+afV5juqzRgLFcV1n9u6LGWAgWnaMBFHBOIbi0MgU1S3jAcjyyw9xqpvzWou1Pj++"
    "f/t8b/7EAvBW5u48agU37abWs99rv1YfL81fkT8V34YxbZ696d4CfwEszZSZx6Z26wAAAABJ"
    "RU5ErkJggg==")
""" Right inactive tab image for the Chrome tab art. """

# For auibook
# -----------

AuiBaseTabCtrlId = 5380
""" Base window identifier for AuiTabCtrl. """

AUI_NB_TOP                 = 1 << 0
""" With this style, tabs are drawn along the top of the notebook. """
AUI_NB_LEFT                = 1 << 1  # not implemented yet
""" With this style, tabs are drawn along the left of the notebook.
Not implemented yet. """
AUI_NB_RIGHT               = 1 << 2  # not implemented yet
""" With this style, tabs are drawn along the right of the notebook.
Not implemented yet. """
AUI_NB_BOTTOM              = 1 << 3
""" With this style, tabs are drawn along the bottom of the notebook. """
AUI_NB_TAB_SPLIT           = 1 << 4
""" Allows the tab control to be split by dragging a tab. """
AUI_NB_TAB_MOVE            = 1 << 5
""" Allows a tab to be moved horizontally by dragging. """
AUI_NB_TAB_EXTERNAL_MOVE   = 1 << 6
""" Allows a tab to be moved to another tab control. """
AUI_NB_TAB_FIXED_WIDTH     = 1 << 7
""" With this style, all tabs have the same width. """
AUI_NB_SCROLL_BUTTONS      = 1 << 8
""" With this style, left and right scroll buttons are displayed. """
AUI_NB_WINDOWLIST_BUTTON   = 1 << 9
""" With this style, a drop-down list of windows is available. """
AUI_NB_CLOSE_BUTTON        = 1 << 10
""" With this style, a close button is available on the tab bar. """
AUI_NB_CLOSE_ON_ACTIVE_TAB = 1 << 11
""" With this style, a close button is available on the active tab. """
AUI_NB_CLOSE_ON_ALL_TABS   = 1 << 12
""" With this style, a close button is available on all tabs. """
AUI_NB_MIDDLE_CLICK_CLOSE  = 1 << 13
""" Allows to close `AuiNotebook` tabs by mouse middle button click. """
AUI_NB_SUB_NOTEBOOK        = 1 << 14
""" This style is used by `AuiManager` to create automatic `AuiNotebooks`. """
AUI_NB_HIDE_ON_SINGLE_TAB  = 1 << 15
""" Hides the tab window if only one tab is present. """
AUI_NB_SMART_TABS          = 1 << 16
""" Use `Smart Tabbing`, like ``Alt`` + ``Tab`` on Windows. """
AUI_NB_USE_IMAGES_DROPDOWN = 1 << 17
""" Uses images on dropdown window list menu instead of check items. """
AUI_NB_CLOSE_ON_TAB_LEFT   = 1 << 18
""" Draws the tab close button on the left instead of on the right
(a la Camino browser). """
AUI_NB_TAB_FLOAT           = 1 << 19
""" Allows the floating of single tabs.
Known limitation: when the notebook is more or less full screen, tabs 
cannot be dragged far enough outside of the notebook to become 
floating pages. """
AUI_NB_DRAW_DND_TAB        = 1 << 20
""" Draws an image representation of a tab while dragging. """
AUI_NB_ORDER_BY_ACCESS     = 1 << 21
""" Tab navigation order by last access time. """
AUI_NB_NO_TAB_FOCUS        = 1 << 22
""" Don't draw tab focus rectangle. """

AUI_NB_DEFAULT_STYLE = AUI_NB_TOP | AUI_NB_TAB_SPLIT | AUI_NB_TAB_MOVE | \
                       AUI_NB_SCROLL_BUTTONS | AUI_NB_CLOSE_ON_ACTIVE_TAB | \
                       AUI_NB_MIDDLE_CLICK_CLOSE | AUI_NB_DRAW_DND_TAB
""" Default `AuiNotebook` style. """

#----------------------------------------------------------------------
Mondrian = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAABHNCSVQICAgIfAhkiAAAAHFJ"
    "REFUWIXt1jsKgDAQRdF7xY25cpcWC60kioI6Fm/ahHBCMh+BRmGMnAgEWnvPpzK8dvrFCCCA"
    "coD8og4c5Lr6WB3Q3l1TBwLYPuF3YS1gn1HphgEEEABcKERrGy0E3B0HFJg7C1N/f/kTBBBA"
    "+Vi+AMkgFEvBPD17AAAAAElFTkSuQmCC")
""" Default icon for the Smart Tabbing dialog. """

# -------------------------- #
# - FrameManager Constants - #
# -------------------------- #

# Docking Styles
AUI_DOCK_NONE = 0
""" No docking direction. """
AUI_DOCK_TOP = 1
""" Top docking direction. """
AUI_DOCK_RIGHT = 2
""" Right docking direction. """
AUI_DOCK_BOTTOM = 3
""" Bottom docking direction. """
AUI_DOCK_LEFT = 4
""" Left docking direction. """
AUI_DOCK_CENTER = 5
""" Center docking direction. """
AUI_DOCK_CENTRE = AUI_DOCK_CENTER
""" Centre docking direction. """
AUI_DOCK_NOTEBOOK_PAGE = 6
""" Automatic AuiNotebooks docking style. """

# Floating/Dragging Styles
AUI_MGR_ALLOW_FLOATING           = 1 << 0
""" Allow floating of panes. """
AUI_MGR_ALLOW_ACTIVE_PANE        = 1 << 1
""" If a pane becomes active, "highlight" it in the interface. """
AUI_MGR_TRANSPARENT_DRAG         = 1 << 2
""" If the platform supports it, set transparency on a floating pane
while it is dragged by the user. """
AUI_MGR_TRANSPARENT_HINT         = 1 << 3
""" If the platform supports it, show a transparent hint window when
the user is about to dock a floating pane. """
AUI_MGR_VENETIAN_BLINDS_HINT     = 1 << 4
""" Show a "venetian blind" effect when the user is about to dock a
floating pane. """
AUI_MGR_RECTANGLE_HINT           = 1 << 5
""" Show a rectangle hint effect when the user is about to dock a
floating pane. """
AUI_MGR_HINT_FADE                = 1 << 6
""" If the platform supports it, the hint window will fade in and out. """
AUI_MGR_NO_VENETIAN_BLINDS_FADE  = 1 << 7
""" Disables the "venetian blind" fade in and out. """
AUI_MGR_LIVE_RESIZE              = 1 << 8
""" Live resize when the user drag a sash. """
AUI_MGR_ANIMATE_FRAMES           = 1 << 9
""" Fade-out floating panes when they are closed (all platforms which support
frames transparency) and show a moving rectangle when they are docked
(Windows < Vista and GTK only). """
AUI_MGR_AERO_DOCKING_GUIDES      = 1 << 10
""" Use the new Aero-style bitmaps as docking guides. """
AUI_MGR_PREVIEW_MINIMIZED_PANES  = 1 << 11
""" Slide in and out minimized panes to preview them. """
AUI_MGR_WHIDBEY_DOCKING_GUIDES   = 1 << 12
""" Use the new Whidbey-style bitmaps as docking guides. """
AUI_MGR_SMOOTH_DOCKING           = 1 << 13
""" Performs a "smooth" docking of panes (a la PyQT). """
AUI_MGR_USE_NATIVE_MINIFRAMES    = 1 << 14
""" Use miniframes with native caption bar as floating panes instead or custom
drawn caption bars (forced on wxMac). """
AUI_MGR_AUTONB_NO_CAPTION        = 1 << 15
""" Panes that merge into an automatic notebook will not have the pane
caption visible. """


AUI_MGR_DEFAULT = AUI_MGR_ALLOW_FLOATING | AUI_MGR_TRANSPARENT_HINT | \
                  AUI_MGR_HINT_FADE | AUI_MGR_NO_VENETIAN_BLINDS_FADE
""" Default `AuiManager` style. """

# Panes Customization
AUI_DOCKART_SASH_SIZE = 0
""" Customizes the sash size. """
AUI_DOCKART_CAPTION_SIZE = 1
""" Customizes the caption size. """
AUI_DOCKART_GRIPPER_SIZE = 2
""" Customizes the gripper size. """
AUI_DOCKART_PANE_BORDER_SIZE = 3
""" Customizes the pane border size. """
AUI_DOCKART_PANE_BUTTON_SIZE = 4
""" Customizes the pane button size. """
AUI_DOCKART_BACKGROUND_COLOUR = 5
""" Customizes the background colour. """
AUI_DOCKART_BACKGROUND_GRADIENT_COLOUR = 6
""" Customizes the background gradient colour. """
AUI_DOCKART_SASH_COLOUR = 7
""" Customizes the sash colour. """
AUI_DOCKART_ACTIVE_CAPTION_COLOUR = 8
""" Customizes the active caption colour. """
AUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR = 9
""" Customizes the active caption gradient colour. """
AUI_DOCKART_INACTIVE_CAPTION_COLOUR = 10
""" Customizes the inactive caption colour. """
AUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR = 11
""" Customizes the inactive gradient caption colour. """
AUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR = 12
""" Customizes the active caption text colour. """
AUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR = 13
""" Customizes the inactive caption text colour. """
AUI_DOCKART_BORDER_COLOUR = 14
""" Customizes the border colour. """
AUI_DOCKART_GRIPPER_COLOUR = 15
""" Customizes the gripper colour. """
AUI_DOCKART_CAPTION_FONT = 16
""" Customizes the caption font. """
AUI_DOCKART_GRADIENT_TYPE = 17
""" Customizes the gradient type (no gradient, vertical or horizontal). """
AUI_DOCKART_DRAW_SASH_GRIP = 18
""" Draw a sash grip on the sash. """

# Caption Gradient Type
AUI_GRADIENT_NONE = 0
""" No gradient on the captions. """
AUI_GRADIENT_VERTICAL = 1
""" Vertical gradient on the captions. """
AUI_GRADIENT_HORIZONTAL = 2
""" Horizontal gradient on the captions. """

# Pane Button State
AUI_BUTTON_STATE_NORMAL = 0
""" Normal button state. """
AUI_BUTTON_STATE_HOVER = 1 << 1
""" Hovered button state. """
AUI_BUTTON_STATE_PRESSED = 1 << 2
""" Pressed button state. """
AUI_BUTTON_STATE_DISABLED = 1 << 3
""" Disabled button state. """
AUI_BUTTON_STATE_HIDDEN   = 1 << 4
""" Hidden button state. """
AUI_BUTTON_STATE_CHECKED  = 1 << 5
""" Checked button state. """

# Pane minimize mode
AUI_MINIMIZE_POS_SMART    = 0x01
""" Minimizes the pane on the closest tool bar. """
AUI_MINIMIZE_POS_TOP      = 0x02
""" Minimizes the pane on the top tool bar. """
AUI_MINIMIZE_POS_LEFT     = 0x03
""" Minimizes the pane on its left tool bar. """
AUI_MINIMIZE_POS_RIGHT    = 0x04
""" Minimizes the pane on its right tool bar. """
AUI_MINIMIZE_POS_BOTTOM   = 0x05
""" Minimizes the pane on its bottom tool bar. """
AUI_MINIMIZE_POS_MASK     = 0x07
""" Mask to filter the position flags. """
AUI_MINIMIZE_CAPT_HIDE    = 0
""" Hides the caption of the minimized pane. """
AUI_MINIMIZE_CAPT_SMART   = 0x08
""" Displays the caption in the best rotation (horz or clockwise). """
AUI_MINIMIZE_CAPT_HORZ    = 0x10
""" Displays the caption horizontally. """
AUI_MINIMIZE_CAPT_MASK    = 0x18
""" Mask to filter the caption flags. """

# Button kind
AUI_BUTTON_CLOSE = 101
""" Shows a close button on the pane. """
AUI_BUTTON_MAXIMIZE_RESTORE = 102
""" Shows a maximize/restore button on the pane. """
AUI_BUTTON_MINIMIZE = 103
""" Shows a minimize button on the pane. """
AUI_BUTTON_PIN = 104
""" Shows a pin button on the pane. """
AUI_BUTTON_OPTIONS = 105
""" Shows an option button on the pane (not implemented). """
AUI_BUTTON_WINDOWLIST = 106
""" Shows a window list button on the pane (for AuiNotebook). """
AUI_BUTTON_LEFT = 107
""" Shows a left button on the pane (for AuiNotebook). """
AUI_BUTTON_RIGHT = 108
""" Shows a right button on the pane (for AuiNotebook). """
AUI_BUTTON_UP = 109
""" Shows an up button on the pane (not implemented). """
AUI_BUTTON_DOWN = 110
""" Shows a down button on the pane (not implemented). """
AUI_BUTTON_CUSTOM1 = 201
""" Shows a custom button on the pane. """
AUI_BUTTON_CUSTOM2 = 202
""" Shows a custom button on the pane. """
AUI_BUTTON_CUSTOM3 = 203
""" Shows a custom button on the pane. """
AUI_BUTTON_CUSTOM4 = 204
""" Shows a custom button on the pane. """
AUI_BUTTON_CUSTOM5 = 205
""" Shows a custom button on the pane. """
AUI_BUTTON_CUSTOM6 = 206
""" Shows a custom button on the pane. """
AUI_BUTTON_CUSTOM7 = 207
""" Shows a custom button on the pane. """
AUI_BUTTON_CUSTOM8 = 208
""" Shows a custom button on the pane. """
AUI_BUTTON_CUSTOM9 = 209
""" Shows a custom button on the pane. """

# Pane Insert Level
AUI_INSERT_PANE = 0
""" Level for inserting a pane. """
AUI_INSERT_ROW = 1
""" Level for inserting a row. """
AUI_INSERT_DOCK = 2
""" Level for inserting a dock. """

# Action constants
actionNone = 0
""" No current action. """
actionResize = 1
""" Resize action. """
actionClickButton = 2
""" Click on a pane button action. """
actionClickCaption = 3
""" Click on a pane caption action. """
actionDragToolbarPane = 4
""" Drag a floating toolbar action. """
actionDragFloatingPane = 5
""" Drag a floating pane action. """

# Drop/Float constants
auiInsertRowPixels = 10
""" Number of pixels between rows. """
auiNewRowPixels = 40
""" Number of pixels for a new inserted row. """
auiLayerInsertPixels = 40
""" Number of pixels between layers. """
auiLayerInsertOffset = 5
""" Number of offset pixels between layers. """
auiToolBarLayer = 10
""" AUI layer for a toolbar. """

# some built in bitmaps

if wx.Platform == "__WXMAC__":

    close_bits = "\xFF\xFF\xFF\xFF\x0F\xFE\x03\xF8\x01\xF0\x19\xF3\xB8\xE3\xF0" \
                 "\xE1\xE0\xE0\xF0\xE1\xB8\xE3\x19\xF3\x01\xF0\x03\xF8\x0F\xFE\xFF\xFF"
    """ Close button bitmap for a pane on wxMAC. """

elif wx.Platform == "__WXGTK__":

    close_bits = "\xff\xff\xff\xff\x07\xf0\xfb\xef\xdb\xed\x8b\xe8\x1b\xec\x3b\xee" \
                 "\x1b\xec\x8b\xe8\xdb\xed\xfb\xef\x07\xf0\xff\xff\xff\xff\xff\xff"
    """ Close button bitmap for a pane on wxGTK. """

else:

    close_bits = "\xff\xff\xff\xff\xff\xff\xff\xff\xcf\xf3\x9f\xf9\x3f\xfc\x7f\xfe" \
                 "\x3f\xfc\x9f\xf9\xcf\xf3\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
    """ Close button bitmap for a pane on wxMSW. """

pin_bits     = '\xff\xff\xff\xff\xff\xff\x1f\xfc\xdf\xfc\xdf\xfc\xdf\xfc\xdf\xfc' \
               '\xdf\xfc\x0f\xf8\x7f\xff\x7f\xff\x7f\xff\xff\xff\xff\xff\xff\xff'
""" Pin button bitmap for a pane. """

max_bits     = '\xff\xff\xff\xff\xff\xff\x07\xf0\xf7\xf7\x07\xf0\xf7\xf7\xf7\xf7' \
               '\xf7\xf7\xf7\xf7\xf7\xf7\x07\xf0\xff\xff\xff\xff\xff\xff\xff\xff'
""" Maximize button bitmap for a pane. """

restore_bits = '\xff\xff\xff\xff\xff\xff\x1f\xf0\x1f\xf0\xdf\xf7\x07\xf4\x07\xf4' \
               '\xf7\xf5\xf7\xf1\xf7\xfd\xf7\xfd\x07\xfc\xff\xff\xff\xff\xff\xff'
""" Restore/maximize button bitmap for a pane. """

minimize_bits = '\xff\xff\xff\xff\xff\xff\x07\xf0\xf7\xf7\x07\xf0\xff\xff\xff\xff' \
                '\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
""" Minimize button bitmap for a pane. """

restore_xpm = ["16 15 3 1",
               "       c None",
               ".      c #000000",
               "+      c #FFFFFF",
               "                ",
               "     .......... ",
               "     .++++++++. ",
               "     .......... ",
               "     .++++++++. ",
               " ..........+++. ",
               " .++++++++.+++. ",
               " ..........+++. ",
               " .++++++++..... ",
               " .++++++++.     ",
               " .++++++++.     ",
               " .++++++++.     ",
               " .++++++++.     ",
               " ..........     ",
               "                "]
""" Restore/minimize button bitmap for a pane. """

#----------------------------------------------------------------------

down_focus_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB0AAAAgCAIAAABhFeQrAAAAA3NCSVQICAjb4U/gAAACaUlE"
    "QVRIib2WvWsUQRjGn5mdnWxyTaqz9q+QlLnGToSgWAYDNjbpNCAGDGIvaRPbNJGQyiAEbK+w"
    "sAo2qexyEhbxsvt+jMXc3u3liPfhmWeXnWVm9vc+vO/M7prVzTb+gxyA7Ye/nXPWWmvtXKBb"
    "B9YBcM5lWZam6by4QNcBsNamaeq9d87NmWutdc59+NgGoKIizCwsxMTMFI8oZmZilzomZiFm"
    "FWERaXbv7eyueO+TJEHM79LSkvfeWnv2qftgex2ASGDmkrUkKUspiIuCy5IL4qKQgnghdQVx"
    "ScKsxCKiaH8lIu99NOwAEFGsG4Dv5xeiQYOKBBYVUWJlFhIVVmIlEZGQJKVIYBbWoKqqwQN5"
    "nqdpuri42OMys6rGOG/X78yW0bXWNyLqcyyAEEIIYcYK3aB5Lazb4o5fsPc3ToFaloxBwMle"
    "6+9Pjfd7stda6HR85+dCPC86Y6ETcQEcHz32eZ7meZrnx0ePJnlk0vwenm70r/PkTgWdjjuV"
    "rnPPfvxaa+3NcL3GMaub7XdPtNFoZFn24tmX1/trAOLuM6aaFQwQYExAMPWNaUw1FW+eHj5/"
    "dbfZbDYajY33F7e1L4gUA5uo3fd8AWbQH70bjGqEyxLq3LoMYhKCgakCIWZoLLdkMRE43Iy0"
    "tWi9QOP8xoIFAyBUjF7dgOizb9iMhLmByxIAHbAGKYigUPX3hqog47hSvfCHfYRaDcNg3IzO"
    "7GmydRaGi37zMujrut/9l58nijROQ9yd3ZXLy8urq6vZWFmW9f+Yhrje++XlZR2keDpZa4f+"
    "H/pKkiR+/f9dDsDWgQW6QHcuxKg/ZbVtCjjzINkAAAAASUVORK5CYII=")
""" VS2005 focused docking guide window down bitmap. """

#----------------------------------------------------------------------
down_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB0AAAAgCAIAAABhFeQrAAAAA3NCSVQICAjb4U/gAAACY0lE"
    "QVRIib2WwWrUUBSG/3tzc5s2m0JhXPsU0u1s3Lkpui4W3PgAuhAFi2/QbesTVEphwCIU3Hbh"
    "wk2LG1fujJQgtMk55x4Xd2aS6VAzM479JyQhufnOz3/uzcQMBgP8BzkAeZ4756y11tqlQIui"
    "cACcc1mWpWm6ZK61Nk1T771zbilcxBxiAs659x/OAAQJIswsLMTEzBR/UczMxC51TMxCzEGE"
    "RaR39WB3b9N7nyTJkLu2tua9t9ZefLx69GYbgIgyc82hJqlrqYiriuuaK+Kqkop4JXUVcU3C"
    "HIhFJODsCxF57xu/RBT7BuDb958SNGgQUZYgEogDs5AE4UAcSEREk6QWUWbhoCGEENQDZVmm"
    "abq6ujrkMnMIIdZ5t31vsUC3+l+JaMyxAFRVVRds0C1azsS6O273hH24cwq0UjIGipP9/t+f"
    "6vZ7st9fKQpf/FqJ28+iEzoTF8Dx0RNflmlZpmV5fPR4lkdmzffwdGe8XyZ3Luh83Ll0k3vx"
    "4/dWf3+B/Q2OGQwGGxsbeZ5nWfbi2efXB1sA4uozZjRKDaAwRqGmvTCNGQ3F26eHz1/d7/V6"
    "eZ6fn5/f1bogCmhsonU+9AWY5nr0bjCtKS6LtrltGcQQ1MCMCiEm1MmtWUwETh6mjq1qw0Jd"
    "fmPD1ADQEWPYNyD6HBs2U2Vu4bIoEBpWE0EE6ej68NaoSBdXRi/8SR/a6qE29830yKFmm2c6"
    "2fTbp8FYN/0evPw0U6UuTXB39zYvLy+vr68XY2VZNv5imuB679fX10MT8Xyy1k58P4yVJEn8"
    "9/93OQBFURRFsRTcWH8An5lwqISXsWUAAAAASUVORK5CYII=")
""" VS2005 unfocused docking guide window down bitmap. """

#----------------------------------------------------------------------
left_focus_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAdCAIAAABE/PnQAAAAA3NCSVQICAjb4U/gAAACVElE"
    "QVRIibWWvW8TQRDF3+7Ors8ShSsaSpo0dEgoFcINVChSBFRUkajpIKKgiPgP0pqGJiAhITqE"
    "FIk2BQUVHT2VK+y7ndmhWN/5Ixcbh8tYWvtO8vvdm5mdPXPv+RmuMgjA670/RGSttdZ2q354"
    "YgkAERVF4b3vHABMCIC11nsfQiCiqwJYa4noxbNvOw/6AJIk62ySJMLMwhI5MnPMnxzMzJHJ"
    "E0dmicxJhEXk+uTO0fFuCME5h1yDxbh5+zEz93q+LGOv50WUmStOVZSqkjJyWXJVcRm5LKWM"
    "3PNURq6iMKfIIpJw9n08Hg8Gg36/3wL4+eu3iHpykcWTS5pElCWJpMiJWaIk4RQ5RRERda4S"
    "UWbhpCmllDQA0+k0pZQFVwF3bzEAZ5N3jgje+0COnPVknbUAdm5cW5/1/eGPxcuL2saoAczC"
    "DQWAV0/fr1c/HxcBFNC8QGEMMu3NuyddAfIjG9QLjKJTB3NIHV050EZuoQI6+93q4P7B6TYA"
    "A2gW1xlC61K0OXi492HZ6EbAnGFqEmBmhlYc7A9HutRq/wgA5plSwDT9tORgfzgCNsmv2QfQ"
    "OvEwps7BooOPpwebxFsB83wazdWdl321BjOGWWejrciZ0+wBMwef76LPnx6trXFrivIfVOsl"
    "P2V7FwH4MhpuCTBLX7mjckU628naTImlrdDdLDJ59OT+XDDU8SwyTX+Y2bC7hIPVA+fty6/b"
    "SmwBODreHY/H0+n0P0WLomjegJYAIYTBYNAcp5cOa20IoQXgnMuvAh0GATg8scAEmHQrneMv"
    "3LAo6X/e0vAAAAAASUVORK5CYII=")
""" VS2005 focused docking guide window left bitmap. """

#----------------------------------------------------------------------
left_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAdCAIAAABE/PnQAAAAA3NCSVQICAjb4U/gAAACTklE"
    "QVRIibWWPWsUURSG3/u5u8RiKxtLmzR2gqQSttFKhKBWqQL+BQXL4D9IGxsrBUGEFCIEbC0s"
    "rOzshYHt3Jl7PizuzsduJhs3Ts7C3Z2BfZ95zzn33DGnp6e4zvAAdnZ2vPfWWmvtsOpFUXgA"
    "3vvxeBxCuC6AtTaEEGP03g8LQE5RTo73/sXzr7sPJwCExTorLMxExMSJEhGl/MlBRJTIB0+J"
    "iBORMBMz3/xz7+h4L8bonFsCunH77lMiGo1CWabRKDArEVUkVeKq4jJRWVJVUZmoLLlMNAq+"
    "TFQlJpJEzCz49n0+n0+n08lk0gP4+es3swbvEnHwTlSYlViYJZEQcWJhkkSSmJnVuYpZiZhE"
    "RUREI7BYLESkTVE37t8hAM5KcM57hBCid97Z4K2zFsDurRubk74/+9G9vKhtjBrAdG4oALw6"
    "eLdZ/XxcBFBA8wKFMci012+fDQXIj2xQLzCKQR20kDqGcqCNXKcCuvzd6+DB4dk2AANoFtcl"
    "QutS9Dl49Pj9qtFLAS3D1CTALA2tOdifnehKq/0jAGgzpYBp+mnFwf7sBLhMfsM+gNaJhzF1"
    "DroOPpwdXibeC2jzaTRXty37eg2WDLPJRl+RM6fZA6YFn++iTx+fbKxxb4ryH1TrJT9lfxcB"
    "+Hwy2xJgVr5yR+WKDLaTtZkSK1thuFlk8ujJ/dkxNPAsMk1/mOWwu4KD9QPnzcsv20psATg6"
    "3pvP54vF4j9Fx+Nx8wa0AogxTqfT5ji9clhrY4w9AOdcfhUYMDyAoiiKohhWt4m/9Qss43IB"
    "CBMAAAAASUVORK5CYII=")
""" VS2005 unfocused docking guide window left bitmap. """

#----------------------------------------------------------------------
right_focus_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAdCAIAAABE/PnQAAAAA3NCSVQICAjb4U/gAAACWElE"
    "QVRIibWWv2/TQBTHv3e+uyRbJwZWFv4AJNSRLjChSkhlYqrEzFZVDAwVC3PXsrAUISTExlKJ"
    "tQMSWzcmFqaqQqT2+8VwtuMkbiBp+mzF0pPz/dzX7z373IMXp7jJCABebf8JIXjvvffrVd8/"
    "9gFACGE4HMYY1w4AxgGA9z7GmFIKIdwUwHsfQth7/vXuoxEAFfWFV1ERZhYWYmJmykcOZmbi"
    "EAMTsxCzirCI3BrfPzjcTCkVRYFcg27cubfDzINBLEsaDKKIMXPFWpFUlZTEZclVxSVxWUpJ"
    "PIihJK5ImJVYRBSn387Pzzc2NkajUQ/g7McvEYuhIJYYCjUVMRYVUWJlFhIVVmIlERErikrE"
    "mIXVVFXVEnB5eamqWXAW8Gb39uKHevbzNwARZVFirUSIlFkqEVUD8Pb71P1Lt83LZ+8BAA7O"
    "AYABMAPcFfcvDXj97ikA5wxmHVVrf64LyA7Mau1so770uVjRQa1lzaKtSc2ZWAR4uHsyn2xq"
    "YBnjbFp4zsRCBw6Ptz/M5GoHgLla15AfUV8F/gEwA/Bk66jPgXNwMNhkyf199F816DIaB5bx"
    "yB2aO2qFLsp/+Xiy22YmczA1Cq4hLQlwsK56xwHgumLWln0pgPv8aWcmNdVF7TKujkWAL0db"
    "88nagXWb0xYgVn4XWf0CymdzWQNgapJzWC7HCnPQF5M5aBhXzthqgMkcoF57Zxx6YvaDMzO3"
    "148pwMHhJhFdXFwQ0XVEh8NhuwOaAqSUUkoxxvaLulp471NKPYC80ci7gXVFALB/7IExMF6j"
    "bht/AXIQRaTUgkiHAAAAAElFTkSuQmCC")
""" VS2005 focused docking guide window right bitmap. """

#----------------------------------------------------------------------
right_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAdCAIAAABE/PnQAAAAA3NCSVQICAjb4U/gAAACVElE"
    "QVRIibWWv2sUQRTHvzM7M3dHmlQWtjb+AYKkTaOVCEKsrAL+CxaWwcY6bWysRAQRUtgEbC0E"
    "u3RWNsJCCILZfb8sZvdu925zej/yveMWHnvvM9837+2OOz09xU0qANjZ2QkheO+999vNXpZl"
    "ABBCGI/HMcabAnjvY4wppRDCdgHIJcrFCSG8eP7l7sMJABX1hVdREWYWFmJiZsqfLGZm4hAD"
    "E7MQs4qwiNz6c//oeC+lVBRFA+jqzr0DZh6NYlXRaBRFjJlr1pqkrqUiriqua66Iq0oq4lEM"
    "FXFNwqzEIqL4+u3i4mJ3d3cymQwAzn/8ErEYCmKJoVBTEWNRESVWZiFRYSVWEhGxoqhFjFlY"
    "TVVVLQFXV1eqOitRV68Pby+v6fnP3wBElEWJtRYhUmapRVQNwJvvvftXbpuXz94BABycAwAD"
    "YAa4a+5fGfDq7VMAzhnMOllt+rMpIDswa3JnG81lyMWaDppc1i7a2tCCiWWAB4dni8F2Dyxj"
    "nPUTL5hY6sDh0eP3c7HGAWCuyWvIJRragX8AzAA82T8ZcuAcHAw2W/JwH/3XHnQZrQPLeOQO"
    "zR21Rhflv3w4O5xGZnPQGwXXklYEOFg3e8cB4LrJbLrtKwHcp48Hc6FeF02Xcb2WAT6f7C8G"
    "GwfWbU5bglj7WWTNAyh/28sWAL1JzrK8HWvMwZBmc9Ayrp2x9QCzOUCz9s44DGj+hTM3t5ur"
    "Bzg63iOiy8tLItok6Xg8np6AeoCUUkopxjh9o64n731KaQCQDxr5NLAtBQBlWZZlucWkXf0F"
    "imtJnvbT2psAAAAASUVORK5CYII=")
""" VS2005 unfocused docking guide window right bitmap. """

#----------------------------------------------------------------------
tab_focus_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB0AAAAdCAIAAADZ8fBYAAAAA3NCSVQICAjb4U/gAAAC10lE"
    "QVRIidWWT0gUcRTH387+ZveXZgzsbmvSsmqEfzBJSYz+gUsHCYJgISPytCQKFhJdpIN0qIUO"
    "ezIUaU/roQ5eEzp46ZT/DhG4haCXdSUPK+Wuzvze+02HgdFmFqMtD76Bx8yb3/v8vr/3Zn4z"
    "np6ReTgCYwAwdqfEGFMURVGU/wIdfaswAGCMcc5VVf1fXIBdBgCKoqiq+nxkobn3BABIkgBA"
    "hIiEJFAgorAOyxARBTKVoUAkgSiJkIhO73a/nLjGOd/nWkrPXbqLiH6/CgBEJiIaKA1BhkG6"
    "QF1Hw0BdoK6TLtCvMl2gIQhRCiQiCfPLm5ubtbW1YNXXtuzadyJTZV4AkKYkMpEkkRQoEUmQ"
    "JJQCpSAiMr1eg8hEJJSmlFJK0wdQLBYR0cl9laj7l6LGY5/tc2ejsrmdgeGJbG5nYHgym9uJ"
    "x9KHeGuMNd7B8fSMzCfvyerq6rHHn2bmEgPDE09G+/9WaSqZmRofisfSiadnotHoozclp94K"
    "oGWznNxn/e8q4LqznNwXmb4KuO6s4643lZyugOvOcj8PDyrgurOOe30r05tKZv7ALavXmszt"
    "rXZZL7EjhTmuU8lpRxNSyemZuUEAmJlLOPzU+CAAuKFluO7OWpF4LO1OPsTcejOOTcRepqXR"
    "tngs7Y6U4bbcqNrIF6bGh6yt0prAgm7kC6E2fSNfWF9b2d7e1jStvqGlbMSmeRsuP7zZZvp8"
    "PvCoW1s/a2qq7vddD57y3b7VZfmNfGFxadUQBgqztbWps7Pdy04uLq0WSyVJnoMRgUY45NM0"
    "bXZZ7OvtaA8vLOdeT85mP+4eXN35K/6W5nBjxFz5tv7+w8LWF3+oTW+IBpsavStf1+xIfTTY"
    "cNbknDPGfqsD5/xCa6AuDFe791xtEJyHIhHedTGw17tnj49EeFdH8GAkEAhwzgF+7HMZY5qm"
    "cc6tD6rDGGOMMUS075aN2Ho9R/R/9gsXZ7dKHM+ODQAAAABJRU5ErkJggg==")
""" VS2005 focused docking guide window center bitmap. """

#----------------------------------------------------------------------
tab_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB0AAAAdCAIAAADZ8fBYAAAAA3NCSVQICAjb4U/gAAAC1klE"
    "QVRIidWVTWgTQRTHJ5vZZEyMLKSx1RbSImJbioeiCKIechLBU8CCtadAaaGIFC/iQTxowENO"
    "hUohp/Tiocdce/Gk/TiIpTm0JCnmA+OaFJs2u/PerIcp27IbKsZ66Ft47P5n3m/evLc768lm"
    "s+Q/GCWEBINBSqmiKIqinApU13VKCKGUMsZUVT1lrqIoqqq+frYyeP8cIUSgIIQgAgACcuAA"
    "wOUlDQCAA1UpcADkAAIREPHiwa2383cYY0TWwa7AlRuPAMDvVwkhiBYAmCBMjqaJBgfDANME"
    "g4NhoMHBr1KDg8kRQHBAREE+r1er1Z6enkOubbn8d0RLpV5CiLAEogUoEAUHAYAcBYLgIDgi"
    "ouX1mogWAIKwhBBCWD5Cms0mADi57xKX/6Ws8dgX+97ZqFxpb3JmPlfam5x5nyvtxWPpE7yc"
    "I+c7OJ5sNhsOh4PB4Kunn5aWE5Mz87MvJv4201QyszA3HY+lE88vRaPRYrHozLcDaNsoJ/fl"
    "xIcOuO4oJ/dNZqwDrjvqrOebSi52wHVHud+HJx1w3VFnvb6d5ZtKZv7AbZuvXMztZbvkR+wI"
    "oY7nVHLR0YRUcnFpeYoQsrSccPiFuSlCiBvahuvurFTisbQ7+ARz55txHCL2NmWOtsVjabfS"
    "hjt0L1Cu1BfmpuVRKReQ0HKlHhkxypV6Ib/ZaDQ0TesfGGqr2DTv+Ph4IBDw+XzEo9Zqv0Kh"
    "wOOxu10XfA8f3JS+XKmvrm2Z3ARuDQ9fGx297qXnV9e2mvv7Aj3HFQ5md8Snadru7u7Rua6q"
    "6sp6aTNXzX08OL67q7f9Q4PdTP1ZKCn5Qq321R8ZMQaiXf19VuGbJ1/8IZX+aNdAnxWJRHp7"
    "e7e3t4+4oVCo0Wjout5qtdx9YIwxxlqtlj3aVgmHw5qmbWxsHNWXUqppGmNM/lCd/aWUUgoA"
    "9mhbhTFGKT3sm67ruq7v7Oy4cR3bb5uW079be13FAAAAAElFTkSuQmCC")
""" VS2005 unfocused docking guide window center bitmap. """

#----------------------------------------------------------------------
up_focus_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB0AAAAgCAIAAABhFeQrAAAAA3NCSVQICAjb4U/gAAACTUlE"
    "QVRIic2WP28TMRjGH/85c1miqEF8CnbUgSFdukVRmZGQ+gW6Vgwd+hW60Yq1gMQMQzpHGZAg"
    "C6JS+QIMmUju/L5+Ge6ulzoRTcMh5TmdZfv8/vT4Pcu26h2N8R9kAZwMfltrtdZa60agx5fa"
    "ArDWpmmaJElTXGBmAWitkyRxzllrG+Zqra21bz+OAQQOzETExJ48EfniKURE5MkmljwRe6LA"
    "TMz8ZPbs9GzXOWeMQZHfW33/NOufvALALESUU8g95zlnnrKM8pwyT1nGmadHic085Z6Jgidm"
    "Dhh/mU6nnU6n1WrFXAA/fv7iIEECsxAH5uApELHnwBQ8Bc/MLMbkzELEFCSEEII4YD6fhxAK"
    "Tsx9/tQDEIgqOzRggAQQQEEBguIFgKoNqDdfvy1yYq41emG4QKkSpDQAiNQfFQClpBoZcaK2"
    "s0awEHzXVVyri1gxN7FaFuILu6qwtAyokqWWwEvcxNTTKsIK95Cqs4JJzV02vMJvHS/1cFFQ"
    "UGV+K3tSzWlZq/5bOWGllIio0mzpX+pZSJXdVRmOuabcItRC+ZfKcn+pFRvN65fvNihj9Y7G"
    "o9FoMplcX18f9M5lUx30zofD4WQyubm56R2Nm9oYY20B98XeRfPcAro+ei1uf/DBt9u+3c7b"
    "7f7gfTPc/cOr7HE36+5k3Z28u5N1u/uHV/dG3X+gfb7YW8dgpC1YD1vBjfP7oEW6Lvf0bHc6"
    "nc7n881YaZre3pjucJ1znU7n9qx+qLTWzrkVXGNMcav4d1kAx5camAGzRoiF/gCKPmudbgYP"
    "HQAAAABJRU5ErkJggg==")
""" VS2005 focused docking guide window up bitmap. """

#----------------------------------------------------------------------
up_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB0AAAAgCAIAAABhFeQrAAAAA3NCSVQICAjb4U/gAAACTklE"
    "QVRIic2WP4vUQBjGn/mTMUtgWS6yvb29XGGzzXXHwdWCcGBzH8BCweK+wnWyWKtot6DNge0V"
    "gjYnNn4BA9vdJvO+81ok2ewmi7d3RtgnZEgm8/545skwiZrNZvgPsgCSJLHWaq211r1Asyyz"
    "AKy1cRxHUdQzV2sdRZFzzlrbCxdlDmUC1to3Hy8BBA7MRMTEnjwR+fIoRUTkyUaWPBF7osBM"
    "zDy+fnR2vu+cM8ZU3KV+fLo+fPUUALMQUUGh8FwUnHvKcyoKyj3lOeee7kU291R4JgqemDng"
    "8ut8Ph+NRoPBoM0F8PPXbw4SJDALcWAOngIRew5MwVPwzMxiTMEsRExBQgghiAMWi0UIoclh"
    "VY8fegACUVWHBgwQAQIoKEBQngBQ3wPq9bfv7XzX7o1eGS5QqgIpDQAizUMFQCmpR3bf26qc"
    "NYKV4nVX7aumaavNjayWlfrSriotdQF1WKoD7nAj00yrLCvdQ+rOGiYNt2t4g9+mXprhoqCg"
    "qnxre1LPqatN762asFJKRFRltvIvzSykTndTwm2uqbYItdL+5aLbX2nDRvPiyds7tC2p2WyW"
    "pmmSJHEcP3/25cPFSXfQNjqeTE9fPhiPx0mSXF1d9bMxdrUD3OPJtH9uCd0evRX38Oi9Hw79"
    "cFgMh4dH7/rhHpxc5PfTPN3L070i3cvT9ODk4saqmz9on6eTbQy2tAPrYSe47XxvtUi35Z6d"
    "78/n88VicTdWHMfLP6Y1rnNuNBotv9W3ldbaObeBa4wp/yr+XRZAlmVZlvWCW+oP2FUt8NYb"
    "g5wAAAAASUVORK5CYII=")
""" VS2005 unfocused docking guide window up bitmap. """

#----------------------------------------------------------------------
down = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB0AAAAgCAIAAABhFeQrAAAAA3NCSVQICAjb4U/gAAACFUlE"
    "QVRIidWVPWvbUBSG3+tv8KKpnYtH/4DuJtCti2nntBm7depQWih0zT9w/AtSQsDQ0JIfkKFD"
    "wRC61Iu3uBgRiKXz1eFalizb8QfxkFdCurofz3l1zhVyg8EAe1BhH9BHyC3NWkTU/XYFQEVF"
    "mFlYiImZyR9ezMzEpXKJiVmIWUVYRJ7cPT/uHizhFgqF6+93Lz8fAhAxZo5ZY5I4log4ijiO"
    "OSKOIomIq+VSRByTMCuxiCiufo3H4yAI8txisQjgz98bUVNTEWNRESVWZiFRYSVWEhGxYjEW"
    "MWZhNVVVtQoQhuESrtfXw6e7JbTd+k1E6dv3+/3dQPeo3+8/3n22Si+OLgFLn52D4aLTun/V"
    "er8XnVZ1NKqM/lX9eTNaC92IC+D87HUlDMthWA7D87NXmyzZNL+nl0ez60Nyt4Jux91Kee71"
    "8Lbd6uxwzXFcr9drNpv+4f2bn59O2gDMAMC5ZJY5wOCcwZxlV7tkKr68PX338Vmj0cBev7f8"
    "d0GkSG0i0576Alza7707LGqBy2JZblYOPgnm4JJA8Blay41ZnAfO3xbumWjTQOv8+oKZA2AJ"
    "Y1o3wPucGXYLYVZwWQzQlJWmwIMs6Z8OJUHWcUU1aWZ9WKaGlo67xZlTbbbPbL7oq7fBTHm/"
    "Jx9+bBRpnea4x92D4XA4mUx2Y9VqteVcAEEQ1Ov13bhZ5fP7IFB4v/v41f8HFQ1ap0nfm7YA"
    "AAAASUVORK5CYII=")
""" VS2005 unfocused docking guide window down bitmap. """

#----------------------------------------------------------------------
down_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB0AAAAgCAIAAABhFeQrAAAAA3NCSVQICAjb4U/gAAACaUlE"
    "QVRIib2WvWsUQRjGn5mdnWxyTaqz9q+QlLnGToSgWAYDNjbpNCAGDGIvaRPbNJGQyiAEbK+w"
    "sAo2qexyEhbxsvt+jMXc3u3liPfhmWeXnWVm9vc+vO/M7prVzTb+gxyA7Ye/nXPWWmvtXKBb"
    "B9YBcM5lWZam6by4QNcBsNamaeq9d87NmWutdc59+NgGoKIizCwsxMTMFI8oZmZilzomZiFm"
    "FWERaXbv7eyueO+TJEHM79LSkvfeWnv2qftgex2ASGDmkrUkKUspiIuCy5IL4qKQgnghdQVx"
    "ScKsxCKiaH8lIu99NOwAEFGsG4Dv5xeiQYOKBBYVUWJlFhIVVmIlEZGQJKVIYBbWoKqqwQN5"
    "nqdpuri42OMys6rGOG/X78yW0bXWNyLqcyyAEEIIYcYK3aB5Lazb4o5fsPc3ToFaloxBwMle"
    "6+9Pjfd7stda6HR85+dCPC86Y6ETcQEcHz32eZ7meZrnx0ePJnlk0vwenm70r/PkTgWdjjuV"
    "rnPPfvxaa+3NcL3GMaub7XdPtNFoZFn24tmX1/trAOLuM6aaFQwQYExAMPWNaUw1FW+eHj5/"
    "dbfZbDYajY33F7e1L4gUA5uo3fd8AWbQH70bjGqEyxLq3LoMYhKCgakCIWZoLLdkMRE43Iy0"
    "tWi9QOP8xoIFAyBUjF7dgOizb9iMhLmByxIAHbAGKYigUPX3hqog47hSvfCHfYRaDcNg3IzO"
    "7GmydRaGi37zMujrut/9l58nijROQ9yd3ZXLy8urq6vZWFmW9f+Yhrje++XlZR2keDpZa4f+"
    "H/pKkiR+/f9dDsDWgQW6QHcuxKg/ZbVtCjjzINkAAAAASUVORK5CYII=")
""" VS2005 focused docking guide window down bitmap. """

#----------------------------------------------------------------------
left = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAdCAIAAABE/PnQAAAAA3NCSVQICAjb4U/gAAACMElE"
    "QVRIib2WPYsTURSG3zv3ThKJRSqblDYLwU6wFMKCViIEtbKQ/QdWgrXt/oO4hZWCIEIKUfYH"
    "WFgIATuraewHM3PPh8XNZCaTSWI2oydwMx/kfeY959wzMbPZDC3FaDTavOi23Wgron8n/Z8A"
    "rnry/NmXk/vXAAhLZCNhYSYiJvbkiciHTwgiIk8uduSJ2BMJMzHzjd93zi9OmwEAbt5+TETd"
    "bpxlvtuNmZWIcpLcc55z5inLKM8p85RlnHnqxi7zlHsmEk/MLPj6LUmS4XDYDPjx8xezxs56"
    "4thZUWFWYmEWT0LEnoVJPIlnZlZrc2YlYhIVERHtAIvFYquDu7cIgI0kttY5xHHccdbZKHaR"
    "jSIAJ8Pru5M+GX+vnm4rslEDmMoFBYCXT9/uVt+MbQAFNCxQGINAe/XmSVuA8MgGxQKjaNVB"
    "CSmiLQe6kqtUQJfHjQ7unV0eAjCABnFdIrQoRZODBw/frRvdCygZpiABZmmo5mAynupaq/0l"
    "ACgzpYBZ9dOag8l4CuyT37EPoEXiYUyRg6qD95dn+8QbAWU+jYbqlmWv12DJMLtsNBU5cFZ7"
    "wJTgzS76+OHRzho3pij8QLVYwlM2dxGAT9PxgQCz9hU6KlSktZ2sqymxthXam0UmjJ7QnxVD"
    "Lc8is+oPsxx2V3BQf+G8fvH5UIkDAOcXp0mSVF94V4ter5emab/frwMADAaDcOOYSNO00+mE"
    "4zrgePWaiAMwn8+PF932//MPv0Uk8OspzrYAAAAASUVORK5CYII=")
""" VS2005 unfocused docking guide window left bitmap. """

#----------------------------------------------------------------------
left_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAdCAIAAABE/PnQAAAAA3NCSVQICAjb4U/gAAACVElE"
    "QVRIibWWvW8TQRDF3+7Ors8ShSsaSpo0dEgoFcINVChSBFRUkajpIKKgiPgP0pqGJiAhITqE"
    "FIk2BQUVHT2VK+y7ndmhWN/5Ixcbh8tYWvtO8vvdm5mdPXPv+RmuMgjA670/RGSttdZ2q354"
    "YgkAERVF4b3vHABMCIC11nsfQiCiqwJYa4noxbNvOw/6AJIk62ySJMLMwhI5MnPMnxzMzJHJ"
    "E0dmicxJhEXk+uTO0fFuCME5h1yDxbh5+zEz93q+LGOv50WUmStOVZSqkjJyWXJVcRm5LKWM"
    "3PNURq6iMKfIIpJw9n08Hg8Gg36/3wL4+eu3iHpykcWTS5pElCWJpMiJWaIk4RQ5RRERda4S"
    "UWbhpCmllDQA0+k0pZQFVwF3bzEAZ5N3jgje+0COnPVknbUAdm5cW5/1/eGPxcuL2saoAczC"
    "DQWAV0/fr1c/HxcBFNC8QGEMMu3NuyddAfIjG9QLjKJTB3NIHV050EZuoQI6+93q4P7B6TYA"
    "A2gW1xlC61K0OXi492HZ6EbAnGFqEmBmhlYc7A9HutRq/wgA5plSwDT9tORgfzgCNsmv2QfQ"
    "OvEwps7BooOPpwebxFsB83wazdWdl321BjOGWWejrciZ0+wBMwef76LPnx6trXFrivIfVOsl"
    "P2V7FwH4MhpuCTBLX7mjckU628naTImlrdDdLDJ59OT+XDDU8SwyTX+Y2bC7hIPVA+fty6/b"
    "SmwBODreHY/H0+n0P0WLomjegJYAIYTBYNAcp5cOa20IoQXgnMuvAh0GATg8scAEmHQrneMv"
    "3LAo6X/e0vAAAAAASUVORK5CYII=")
""" VS2005 focused docking guide window left bitmap. """

#----------------------------------------------------------------------
right = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAdCAIAAABE/PnQAAAAA3NCSVQICAjb4U/gAAACMklE"
    "QVRIibWWvY7TUBCFz83vojSuKNiShgdAol8hQYWQkJaKAuUNtqWmoeANFgoqhJAQHRLaB6BA"
    "orC0HWnSUEURK2LPmRmKayeO4wTysydWbI+c+e7xzDgOo9EIK0rTdDW4m0Ij4FBK07R1fdmj"
    "rh3QqZ6cPf965+ENAKbWardMTZWkUoVCUuIniiSFnW6HQqqQpkpVvfnn3uu395sBAG7fPSXZ"
    "73ezTPr9rqqTzGm5aJ5rJswy5jkzYZZpJux3O5kwFyVNqKqGb9/H4/Hx8XEz4PLnL1XvdtpC"
    "7Xba5qbqVFM1oZEqakoTmqiqerudqzqpNDczM+8Bs9lsrYNXw1ub7+nl+DcAVaOa0HJVESM1"
    "VzVzAG9+LF2/dZFfPHsPAAgIAQAcgDsQ1ly/NeDlu6cAQnC4V7L6/GtfQHTgXuSONopdk4sd"
    "HRS5vFy0l6EVE5sAD4YXq8GyBh4xwZcTr5jY6CDg0eMPtVjhAPBQ5HXEW9RUgX8A3AE8OTlv"
    "chACAhy+WHJzH/1XDaqM0oFHPGKHxo7aoYviTz5eDOeRxRwsjUIoSVsCAryaveIACNVkPi/7"
    "VoDw+dNpLbTURfNlrNcmwJfzk9Vg4cCrzekbEDs/i7x4AMWt3B0AsDTJUR7LscMcNGkxByVj"
    "7YztBljMAYq1V8ahQfU/nNrc7q/6e9FkMplOpyKyT9Kjo6MkSQaDQZqmdQdJkiRJsk92AFdX"
    "V71eLx7XAQfRYDCYH68FHOr19C8Ad0k9S0aHzwAAAABJRU5ErkJggg==")
""" VS2005 unfocused docking guide window right bitmap. """

#----------------------------------------------------------------------
right_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAdCAIAAABE/PnQAAAAA3NCSVQICAjb4U/gAAACWElE"
    "QVRIibWWv2/TQBTHv3e+uyRbJwZWFv4AJNSRLjChSkhlYqrEzFZVDAwVC3PXsrAUISTExlKJ"
    "tQMSWzcmFqaqQqT2+8VwtuMkbiBp+mzF0pPz/dzX7z373IMXp7jJCABebf8JIXjvvffrVd8/"
    "9gFACGE4HMYY1w4AxgGA9z7GmFIKIdwUwHsfQth7/vXuoxEAFfWFV1ERZhYWYmJmykcOZmbi"
    "EAMTsxCzirCI3BrfPzjcTCkVRYFcg27cubfDzINBLEsaDKKIMXPFWpFUlZTEZclVxSVxWUpJ"
    "PIihJK5ImJVYRBSn387Pzzc2NkajUQ/g7McvEYuhIJYYCjUVMRYVUWJlFhIVVmIlERErikrE"
    "mIXVVFXVEnB5eamqWXAW8Gb39uKHevbzNwARZVFirUSIlFkqEVUD8Pb71P1Lt83LZ+8BAA7O"
    "AYABMAPcFfcvDXj97ikA5wxmHVVrf64LyA7Mau1so770uVjRQa1lzaKtSc2ZWAR4uHsyn2xq"
    "YBnjbFp4zsRCBw6Ptz/M5GoHgLla15AfUV8F/gEwA/Bk66jPgXNwMNhkyf199F816DIaB5bx"
    "yB2aO2qFLsp/+Xiy22YmczA1Cq4hLQlwsK56xwHgumLWln0pgPv8aWcmNdVF7TKujkWAL0db"
    "88nagXWb0xYgVn4XWf0CymdzWQNgapJzWC7HCnPQF5M5aBhXzthqgMkcoF57Zxx6YvaDMzO3"
    "148pwMHhJhFdXFwQ0XVEh8NhuwOaAqSUUkoxxvaLulp471NKPYC80ci7gXVFALB/7IExMF6j"
    "bht/AXIQRaTUgkiHAAAAAElFTkSuQmCC")
""" VS2005 focused docking guide window right bitmap. """

#----------------------------------------------------------------------
tab = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB0AAAAdCAIAAADZ8fBYAAAAA3NCSVQICAjb4U/gAAACq0lE"
    "QVRIidWWTWgTQRTHJ+3ETEugERrtF6QhFhKsIlgQRITm5LkBvdiDhBZKc5DiRXryoAEPPRUi"
    "pTnVi4cee5NePAitFtFoVxSyheYDa02KCd3deW/Gw2Cy7MZaIz307TK7/Gfeb9587Nvx6LpO"
    "TsA6TgJ6glyqHgcHB4/ub0ZvdRFCBApCCCIAICAHDgBcXcoAADhQLwUOgBxAIAIinju89iRz"
    "gzHW5Pb09BBCImO3AcDn8xJCECUAWCAsjpaFJgfTBMsCk4NposnB56UmB4sjgOCAiIJsbJXL"
    "5b6+PsYYtQev5b8hSi/tJIQIKRAloEAUHAQAchQIgoPgiIiys9NClAAIQgohhJBnCKnX6wDQ"
    "jFfZ0+TA/8xpIv6+8e5cN61Qm05ltEJtOvVMK9QS8ewRpWqj2js4nsb+nbv3cnU9OZ3KzD2c"
    "/NdIF9IrS4sziXg2+aA/FAr5/X5nvG1AW3o5ufOTL9rgur2c3Mcrd9rgur1Oe7wL6edtcN1e"
    "7v1wtw2u2+u0z2978S6kV/7CbRmv6sxdquVSH7HTR/9tE+PLUsqp2cz27k/7PTWbkcezifHl"
    "tbW1XC6n6zp1dONeWaUk4tnjTwtx5F81KEcSaQxzdT1p1xPxrFtpwY3d7C6WKkuLMypVqg4U"
    "tFiqBEfNYqmi57er1WogEBgOx1oqDVoz/77e2Onu6hq7emGg/6w9imKp8ubt149a/mI0rGqV"
    "8u7DlyuXRuzKp8/5yzG/yr9NrmEYm1uFba2svTq0c0eu+2LR88z7Qy905PW9vZwvOGqGQ73D"
    "Q1Lf9eR3vitlONQbHpLBYHBwcJAx5rGfd6rV6v7+vmEY7nVgjDHGDMNo1LZUIpGIc34ppYFA"
    "gDGmfqgOo5RSSgGgUdtSUSUANLmqWp0q/mTK82hFcX4Bm24GMv+uL+EAAAAASUVORK5CYII=")
""" VS2005 unfocused docking guide window center bitmap. """

#----------------------------------------------------------------------
tab_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB0AAAAdCAIAAADZ8fBYAAAAA3NCSVQICAjb4U/gAAAC10lE"
    "QVRIidWWT0gUcRTH387+ZveXZgzsbmvSsmqEfzBJSYz+gUsHCYJgISPytCQKFhJdpIN0qIUO"
    "ezIUaU/roQ5eEzp46ZT/DhG4haCXdSUPK+Wuzvze+02HgdFmFqMtD76Bx8yb3/v8vr/3Zn4z"
    "np6ReTgCYwAwdqfEGFMURVGU/wIdfaswAGCMcc5VVf1fXIBdBgCKoqiq+nxkobn3BABIkgBA"
    "hIiEJFAgorAOyxARBTKVoUAkgSiJkIhO73a/nLjGOd/nWkrPXbqLiH6/CgBEJiIaKA1BhkG6"
    "QF1Hw0BdoK6TLtCvMl2gIQhRCiQiCfPLm5ubtbW1YNXXtuzadyJTZV4AkKYkMpEkkRQoEUmQ"
    "JJQCpSAiMr1eg8hEJJSmlFJK0wdQLBYR0cl9laj7l6LGY5/tc2ejsrmdgeGJbG5nYHgym9uJ"
    "x9KHeGuMNd7B8fSMzCfvyerq6rHHn2bmEgPDE09G+/9WaSqZmRofisfSiadnotHoozclp94K"
    "oGWznNxn/e8q4LqznNwXmb4KuO6s4643lZyugOvOcj8PDyrgurOOe30r05tKZv7ALavXmszt"
    "rXZZL7EjhTmuU8lpRxNSyemZuUEAmJlLOPzU+CAAuKFluO7OWpF4LO1OPsTcejOOTcRepqXR"
    "tngs7Y6U4bbcqNrIF6bGh6yt0prAgm7kC6E2fSNfWF9b2d7e1jStvqGlbMSmeRsuP7zZZvp8"
    "PvCoW1s/a2qq7vddD57y3b7VZfmNfGFxadUQBgqztbWps7Pdy04uLq0WSyVJnoMRgUY45NM0"
    "bXZZ7OvtaA8vLOdeT85mP+4eXN35K/6W5nBjxFz5tv7+w8LWF3+oTW+IBpsavStf1+xIfTTY"
    "cNbknDPGfqsD5/xCa6AuDFe791xtEJyHIhHedTGw17tnj49EeFdH8GAkEAhwzgF+7HMZY5qm"
    "cc6tD6rDGGOMMUS075aN2Ho9R/R/9gsXZ7dKHM+ODQAAAABJRU5ErkJggg==")
""" VS2005 focused docking guide window center bitmap. """

#----------------------------------------------------------------------
up = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB0AAAAgCAIAAABhFeQrAAAAA3NCSVQICAjb4U/gAAACHUlE"
    "QVRIidWWP4vUQBjGn8mfcyGQYiM2W8mWsRcLm22uWw6uFpQt7WwVLPwKVsp+gFMsAwqynY2F"
    "oLAgNu4HsEiz3E7mfee1yN9Lcueu7oo+IcPMZN4fz7yZzEQlSYIDyAMQx/F+ocvl0tkvsdKh"
    "uF6z8eLsAwDLlpmImNiQISKTX7mIiAx5vkeGiA2RZSZmvnF++9nzO0EQ9HC/vj2fPr0PgFmI"
    "KCObGc4y1oa0piwjbUhr1oau+Z42lBkmsoaY2eLjpzRN+7kAvn3/wVasWGYhtszWkCViw5bJ"
    "GrKGmVlcN2MWIiYr1lpr5QjYbDb9eQBw95YBIBBVdDiAC/iAAAoKEOQ3AJRtQL38/OXS/ALw"
    "XKcxXKBUAVIOAIjUDxUApaQcecV7A3DkuYJG8EVX7VpdtNXm+p4jjfjcrsotdQFlslQH3OH6"
    "bj2tPCx3Dyk7S5jU3K7hHr91vNTDRUFBFfkt7Uk5p6763lsxYaWUiKjCbOFf6llImd2+DLe5"
    "ruM0UlA5uazS7S/Usz88vnf2G2VLKkmSap989OD9m8WsO2gbnU7mD5/cHI/H+C/3yR24p5P5"
    "/rk5dHv0VtzpyWsThiYMszCcnrzaD/d4ttDXIx0NdTTMoqGOouPZ4pdR7e+iq3fzyTYGWzrY"
    "etj7zwOAOI7/yjmPHRfpFVKr1apqrNfrNE2bx+pOGgwGo9Eor1/wGwRB9QPwh/oH9oed9BPW"
    "YyQlBOJt4AAAAABJRU5ErkJggg==")
""" VS2005 unfocused docking guide window up bitmap. """
#----------------------------------------------------------------------
up_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB0AAAAgCAIAAABhFeQrAAAAA3NCSVQICAjb4U/gAAACTUlE"
    "QVRIic2WP28TMRjGH/85c1miqEF8CnbUgSFdukVRmZGQ+gW6Vgwd+hW60Yq1gMQMQzpHGZAg"
    "C6JS+QIMmUju/L5+Ge6ulzoRTcMh5TmdZfv8/vT4Pcu26h2N8R9kAZwMfltrtdZa60agx5fa"
    "ArDWpmmaJElTXGBmAWitkyRxzllrG+Zqra21bz+OAQQOzETExJ48EfniKURE5MkmljwRe6LA"
    "TMz8ZPbs9GzXOWeMQZHfW33/NOufvALALESUU8g95zlnnrKM8pwyT1nGmadHic085Z6Jgidm"
    "Dhh/mU6nnU6n1WrFXAA/fv7iIEECsxAH5uApELHnwBQ8Bc/MLMbkzELEFCSEEII4YD6fhxAK"
    "Tsx9/tQDEIgqOzRggAQQQEEBguIFgKoNqDdfvy1yYq41emG4QKkSpDQAiNQfFQClpBoZcaK2"
    "s0awEHzXVVyri1gxN7FaFuILu6qwtAyokqWWwEvcxNTTKsIK95Cqs4JJzV02vMJvHS/1cFFQ"
    "UGV+K3tSzWlZq/5bOWGllIio0mzpX+pZSJXdVRmOuabcItRC+ZfKcn+pFRvN65fvNihj9Y7G"
    "o9FoMplcX18f9M5lUx30zofD4WQyubm56R2Nm9oYY20B98XeRfPcAro+ei1uf/DBt9u+3c7b"
    "7f7gfTPc/cOr7HE36+5k3Z28u5N1u/uHV/dG3X+gfb7YW8dgpC1YD1vBjfP7oEW6Lvf0bHc6"
    "nc7n881YaZre3pjucJ1znU7n9qx+qLTWzrkVXGNMcav4d1kAx5camAGzRoiF/gCKPmudbgYP"
    "HQAAAABJRU5ErkJggg==")
""" VS2005 focused docking guide window up bitmap. """

#----------------------------------------------------------------------
aero_dock_pane = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAGcAAABlCAMAAABnVw3AAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAb3WKdHqPdnyRd3+deYGge4OifISifoallZaWgIy1g463hZC5hZG6iJO8o6Sk"
    "pKSkpKWlpaampqenqKmpqaqqqqurq6ysrKysra6urq+vr7CwsLGxsbKysrOzs7S0tLS0tLW1"
    "tba2tre3t7i4uLm5uru7vLy8vL29vr6+iZfLjJrNjpzPjpzQkZ7PkJ/SlKHSkaHek6Pgk6Th"
    "labjmKjlnqzhnqzjoa/npbPov8DAwMDAwMHBwsLCwsPDw8TExMXFxsbGycnJycrKysvLzMzM"
    "zM3Nzc7Ozs/Pz9DQ0NDQ0NHR0dLS0tPT09TU1NTU1tbW1tfX0tTY19jY1tjd2NjY2dnZ2dra"
    "2tvb29zc29zf3Nzc3N3d3d7e3t/fxs7szNPt0NXo0dfu1djk09js2tzk3d/k3d/m2Nzv3N/r"
    "1Nr02N713OH13OL23+X43+X54ODg4eHh4eLi4+Pj4uPm4uPn4+Tk5OTk5OXl5ubm5+fn4eLo"
    "4uTs5eXp5efv5+jo6Ojo6enp6urq6+vr6Onv7Ozs7O3t7e7u7u/v4Ob55Oj16Ov37e/26+/9"
    "7/D28PDw8fHx8vLy8/Pz8/P09PT09fX19vb29vf38vT89ff9+Pj4+Pj5+vr6+vr7+/v8/Pz8"
    "/f39/v7+////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAsPpcmgAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAf+SURBVGhD7Zr7d9tEGoa9UKBZCiapneBQN3bUQlgo13DbhXBrnRQnS0LJNhDbUO7s"
    "ErCtZWWZm2Rj2YHdbWQsCSQL2/yjZkaypBlpdKH14XA4vL8kkuebx3PR6HtnHBtHkfxyZoGs"
    "zMtR4sfjWJRi0mJRHZGlFhej1BCNs1XyocDbpa0ooEjtSfcDOHp6apyFAMzop4XfF2f0K7Vn"
    "qpyhr0bT5AwHvhpOkzPQ/TVNjt73lT6c4jzQNV/1p8npq77SBlNsj6b4Sr1Ozmtr99xpvQpm"
    "6LKvquyMVezOe9Ze81uDfNbRHLUtiPaqNi8KvrqqztvlRGGbypFJRM7mMoOtnPNHDV8JisOB"
    "QczyJolE4qy/MMAX6Pl23VdNGeeMBi+sE0AEzsYl92tgXqj5ipNcnNHo0oYX5OVsnfe8beab"
    "VVQcelHveTij895XrJdD6SGc4b+HCIglcHTK0yAPJ4dPAYOJtUd/78b3dAdE4owYz6zzcM4Q"
    "3tEop/v18ePHv+7aICJndMbdIDdn93Iwp/vh7VAf2iAy5/KuC+TmPN0J5HQ+eNDUB51Ji8ic"
    "zjMhnAwpIbT7TfjXU4+ZeuqfbRNE5miZEE5qGNSe2g//s/RDLYgzSoVwiKlammMmvVTjLE0w"
    "NH+UJmV37peFe3yInM3NZp0liz/azk+NM0ptt3tkdXZSxGQ1vD3EDOpiep6s9EVSeW+y5e03"
    "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"
    "XFekjMjBOHUAAAAASUVORK5CYII=")

#----------------------------------------------------------------------
aero_dock_pane_denied = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAGcAAABlCAYAAABQif3yAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "ABh0RVh0U29mdHdhcmUAUGFpbnQuTkVUIHYzLjM2qefiJQAAELhJREFUeF7tnfmPHNURx81v"
    "hEsCIQQCxCnu60d+AH5BCCGBAAkEP/AHABKKkEhCCErCHcA4WGAIYJwYbOMrGHzgE9vYxvje"
    "9drYa+99etd7zHov766PSn3avFFvu3v6dU9Pz2Q9K5Vmtt9VVd9X76zqOWtSCf4NDw9LfX39"
    "pMbGxklHR0YKxuHvzj570tVXXz3ptttuO6tgjUykioeGhuTbRYukrr5eRkZG5OTJkwUj6qcd"
    "2ptIOiyYLFV79khDQ0PBAPEDm/Zot2BCTZSKFy9eLGNjY6mCc+zYMaHdiaLDgsmxYOHCVIHB"
    "kvij3YIJNVEqLoNTwkgWAxysp2w5Fp2iDI6FkoqVxYBz4sQJSYPM6q1sORaIoyRAOX78eCpE"
    "W+VhzQIYsgAOwLC8TYvK4EQAB1DY66RBtIX1lIc1C4BQEgobHR1NhegAZXAsgDHDGgrj3CsN"
    "ohMwjJYtxwIglITCjh49mgrRAc5YcGpqa2Xjxo2yZs0aWaiKR/m5aNonn0hLS4vodUEq1NTc"
    "LO3t7UK7YbzBP3IgD3JZ9LXSzLJj505Zvny57Nu3T7p7eqS/v9/qzGzBggVO3h4tkwb19fU5"
    "wyft2lxPGN6QC/mQszQR8OGqqqpKlv3wg7S2tloJ61UISjpy5IgcPnw4FaIDMITaguPlFzmR"
    "F7lLGqRdu3bJ1q1bnTHcphf65UFJvb29cujQoVSoq6tLuHmNCw4yIC9yI39JAlRRUSH7q6tj"
    "g5I9RlFw6M1tbW2pUEdHh3D7mg84hnfkRw8lBRC3iNu3b88bGGenruDQm5t1oo5KKDpqGSx0"
    "cHAwEXDgHz2U1K0qEyMbx7hDmbtcXHDYSDa3tDsbyigAsVJLEhz0gD5KwnpYrcSd/IPmnKiW"
    "MzZ2TOoaWuWPr051PvnfFqCkwUEm9FESq7jlK1YkYjHuOScKOAMDA9LRmZG/vj07S/zPcxuA"
    "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"
    "VvwtWIicAAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
aero_dock_pane_bottom = PyEmbeddedImage(
    "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")

#----------------------------------------------------------------------
aero_dock_pane_center = PyEmbeddedImage(
    "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"
    "AAAASUVORK5CYII=")

#----------------------------------------------------------------------
aero_dock_pane_left = PyEmbeddedImage(
    "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")

#----------------------------------------------------------------------
aero_dock_pane_right = PyEmbeddedImage(
    "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"
    "SBhbDwAAAABJRU5ErkJggg==")

#----------------------------------------------------------------------
aero_dock_pane_top = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAGcAAABlCAYAAABQif3yAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzoyMTozMiArMDEwMExUiZ4AAAAHdElNRQfZAxkQKSNpU8hr"
    "AAAACXBIWXMAAAsSAAALEgHS3X78AAAABGdBTUEAALGPC/xhBQAAEFhJREFUeNrtXWlsXNUV"
    "Pm8W22OPYzu24yXxEscxzk621kDaAGVTFVqxCoRaxJ9WlSr6j1+V2p+tVLX9U1URUlNVXdgK"
    "EhRCgYKgSShtFpw4qx3I6iReEie2x57tvZ7vzVz7eebNzNtm3kPyJ109+828e885313Oucsb"
    "iTyISCSiHDzyOf3peIKGp+SildMa9tH31wVoZXvbi21tbT9wW+9MSG4LkInp6Wnl8Rc/J2XN"
    "nXRfn0JdIYlCfL+MJS3ja4Cvfr76DOancEoi8R8Jvkb5OsvXKU6DEYU+/UQi3xcHaPdj7S8x"
    "QU+7rb8WAbcFyMR/j/STtKqPfnFHkqoUifxszYAPZPDfTIwkpWqUmVoFgtD+ZAVJoQSI4hvb"
    "JYUeu8dHL8z00dD5z57ir3iKHKMVsGTYcyxOnTskKkumjFpMgKRgUqa6h1Lleg2eI2dsRqGm"
    "ComKN9IsBMppDfnUcr0Gz5GziHl4bswBUJuVHAmQyFyXp+j8LcYhtbxSNVOT8GTLwcCtKGlC"
    "cE0bT0kzZLYDUpSFSc7436vwZMsBGXE2mo+vki/loamMpK+Skr5nEHKa0KQiPLa0ay0vbEFe"
    "gyfJgbsrswVlad6YPjajTCk/WnWlTdR4lQC1lShz+Qm3mtSy/G6rrAtvkiOn3Fw/SEmnpOZz"
    "lRwTLUd0ZSIYVVtNOikWuslSwZPkoLtJLqjhCwNPswOldvyaD0ZTZah5erRf8yQ5MFxc03Ik"
    "zXiDq3oxUd0XEJJulXE5lZBfwqNNx5PkJLj/iSdT82gQEC1lriuT0/9bcQjSLVJc0ULVLBdb"
    "jnGkxgSFr0gStyApVfPF6CCZdwiSmi5NzRctSE5l4lFuik/O4NCQcvXKFYpGozQxMVFw8N1z"
    "gY2WSNVqdezxpQyrOgWa6NNnsuVox5lk2jlIpLtM0XJefe21vOKhyNraWiovL6fmlhZa3d1d"
    "1Fn9opCze/fu3q3btp0cHRmheCxGa9aupbJgkMLhcMFn9/z2E3rpukJN3K8tLZeoOihRlV+i"
    "CvZ2y5iooC81O+03YRYxCx1jdqJ8neYbk3GiGzE45xL9jfPayd977NFHC+Y1NTVF8Xicrl27"
    "Rnv37lUaly2jbVu3FoUkx8k5evSocvHSJWpuaqItmzdbymOWW84UG66cI1AI6FfdLYkUf8rj"
    "MkOO6NJATpTJmeUmM8PkRNjjmI4paqwTjyeNZcYQFayuro56e3tpeHiY3n7nHaVtxQrauHGj"
    "oyQ5Ss7hw4eVmZkZevCBB8jnsz4zFOVaHWGCKlm6crYbNx61GxNdGRwF2SQ58MxinFc0nWaS"
    "qTJAdtI4N1lobW2l5uZmOnTokKr/li1bHCPIsbm1I0eOKKHKStq+fbstYlRwTZ7mqj7Do/Ys"
    "J9T4WDKV4rL5pD7LA4varaXzRIpwGRG+2iFHNSLrC72hP+zglE0dIefosWNKgkfx23p6nJEq"
    "keraoolUbVdTOi5BDyScBW2C9xXmJpaU9T9X3XNZ04I0ZZBNcgSgP+wAeziRnyPkXOYx5vbb"
    "b3dGwzRkTVIKJHR33+0I0IrpS+rVJxV+Rpu/k4AdYA8nYJucg4cOKRs2bCC/353JQzgG326V"
    "6PjJL+gnL/xaveJ/M96co/KwHWAP2MVuXrbJGR0dVQdFt7BxaYCuXJugn//yVaqsblWv+B/3"
    "3QLsAbvYhS1yTp06pazq6nLNCJvrAzR58Sr97FfvU3Vdx1zC/7iPz90C7AL72MnDFjlXORBr"
    "bGx0RfmeGj9dPzdOL77yBYVr27MS7uPz2/h7bvRwDWyXEQ7C7cBW1ZqanKRQKFRyxTurfRQd"
    "jtAHn82oXVku4PNvJqepu62KTtxIlFTGKnarb926ZSsPW+REYzEKBoPOa+bL36SvRhS6t6uK"
    "WpsqC2YVrpToo7HcvnKxNlHALrCPHdgiR+bATjIzd28ADSGJqhCzlPlyGm6Wg8qP2eAhA9LP"
    "jKema/RQwc9XBX1UnS7Xacg21yI8t2Tw3PoAfXTMT8u/wcbnCh8OSBxcEoXYNy73pyY+A+m5"
    "tYQB3bGVd0mZxMGnRBWyQuX8fNCnUJlfoUbmrIpJqd8r0fc2e84U3iNn51130LqDn9Kfj/fR"
    "mq8ptLrSR7VMSpjJCLGhy/mKjjRgYm4NDSfGCZs6p5nQCU7XufWdmeUAeh9Rr/wf+taOO9xW"
    "PQuOkKM4vPnr7ef76P1/H6CX35NpKFK8NeTVPB79tNdH9z9zp6M6ONXVe67lCNz/jTs5uS2F"
    "u/Dkjs+vOpxqhbZbDgRxulv7qsOpbm0ul4GBAeXcuXM0Mztr+GFEwGZnCCoqKqijvZ3Wrl1b"
    "IlPZx4kTJ+j8hQs0a8I2MOzI6CgtW7bM8DMhtk1nZyetX79eEnnQ62+8oWzatImWt7ZSWVmZ"
    "4cwwuQdyzLScGAdml4eHqb+/n77z8MNZn2N9/tjAgJo3ZnidjqMyEQgEqH7pUhhE9/M333qL"
    "rNgGcgv7WLHNo488IklYGKoOh6mjo8O0Yii8oaHBklHOnz+vTm+sW7duwf19+/ereXavWmV/"
    "RdUAUBkGBwfVvzMJOn78OC1ZssSSbayQo7XN5NQU+c59+SUtX7686EbIxIoVK+gcC6EFdrZg"
    "D8I67vKw/QhTIMVOlZWVtHr1ahodG8uSEfK5ZhvmJYD5HzTtUgNlxqLRBV0i/nZjBhktNJlI"
    "ZHXPkM8t24CXRVfaw3AtCFVdcFoYE7jtkGe2HLfkEXLYIsdOjJPPC1O7tyJ7aWb1LOVzAnPk"
    "WMlIsfhcTgUyxp9SQ69MOxXQqn1ExQwIAaysPSj8jFHhMeiK76JwvZYh8rG7DmLWEPnk19pG"
    "yF1IPqEb7GNWF61tfFqjmIUZYs4Px2g25iv4nBOt0awOotx8nwOQH3oUir+EfpZ6I81zgcwb"
    "pjKiwoZElH/q7E06fDJOO7++lFrqKct9zmeQUiJft4bafP1WkvYfvsWxWJB6V9VQssA+XisV"
    "Tduj2BpzxBiR69lAIEgjY5P08WfD5AvWLVBWSZ9k1iOq1OTkLDerBUg0PZNgfUZpaY2fljVU"
    "UyIRz5exLV3mujUrSU73qXpJknx04dII/fGV/1FMs55spJXKmrGsVCkfcUIeAegDvaAf9MzU"
    "XWsfKzYVZfm0AphNuQRAhDs+PkF7/vLBgrPkipJRETIqhhg8rcpjJWUSkU++OX7SR7OhH/SE"
    "vnqV1ooe2hZsu+VkCoA+8/Tgl/Sb3/1V4xkrJIbdzNqhV3tL2XJyyaK9N/+deT1EZYOe0Fd4"
    "cYUqrhF5BNQxB1PVMQt7rDBJOT09ndUtdLa30vM/eop+/4d/ZtUMrImIgVT8P9ddsAyKetIs"
    "XjJ3Gp4XykO5mes1Qj7IheUCWQ5l6QM9a2uqFmwgBFHIF/ZBMgM4UMIbVMmBsQp5HnoQz2WS"
    "Mzk5SQ31dfTcM/fRy28ey1I4sw8XELUHeZaKHCPjXy55oF9DfQ3duHFjQT4idrJq1yxX2hI5"
    "eQTHWkb7imZ69snt9MZ7F+c8O3wfB4y0TV9rCK1BSoFcXYr4DPcg79zsNN8LBnz0JOsFb+3a"
    "tatZBIMctICkRT2EHAE7xpBztByB4eHLVFNTw/FNixrnKEp2y9F71mqNs4tcsszLjE2Iftqy"
    "pUXdjAj99CBiFdmCHtoA114QmqPGaYEm39q4hCLREIVZoWgkviDO0Ws5pdw0kumIZHw4px/G"
    "Jey77u0KsT6yqlcuiCkYK3pkudLFBlY4u1fITMyIqqQwSrar6u6igZ4rLQC5IT/0gD6lQEm2"
    "RqFp37x5M+fzc3+nbpRE8UKy6Olnxqt1oqK5vuMz15KB67J4AK6QM2eEHHNrrkCvB3CZLNdb"
    "TibMmEMMvHZWTeHyxtlVdus0eD4EhJKlhtYh0N4zChFLIHJHHGIVCR4PsU+sesmSrPLd7uQC"
    "5awcBmz416WIyucCNC5T78ii0WqCfLC3bf+BA7m7H82qpU+vAqbd3RompkvnVDjkc8s24CXQ"
    "uXKl+noqnJ0Xc0xGgagZyYzgKBxGxduWGnPsFjXSVcFgeIcbrvfec0/OKH1gYEDdONje3p71"
    "PGT/14cfUktLi24ZkO/ayAi18ucJnX1t+YwsbGNm3xvkxV7yy5cvq3umAxs3bJD+/vrrCjLB"
    "24/M7AeeZeNgu6qZKBhKDl+5QidPnqQdO3Ys+AyCof+H4QrJge9gRybkRg3XGzMg39jYGPX1"
    "9eme+kYgGYlE1MqiB+wE3bdvHwW5jKamJsO2EeTEuLLDPkaBxqHa5tQpvPttvnr29/crFy5e"
    "NLWTHqcM6mpr1Tkko8BL8eq5Rvaw4nq4dOmS+n6DgIEBuoINDkIxj6fXepFHkA2aa2Y4XFVF"
    "y9jojQX2e58ZHKRxJjkWN/ZLIYKc8fFxU6cMoEt7Wxs2zs+fMrAKvHbx7p07TU+LG4ERQ5QV"
    "45i9AwAxaDF7332Xnnj8ccs29pwrLeBVw5cSi3ulPQzPtpyhoSH1WIaVFVqjwACP8aa7u9tt"
    "dXXhSXJwgAovldu6dasp79EsxEkylLfjrrvcVjsLniMHp8zwVl0rp8nMAsSv5HgCASrKXZ3D"
    "g3QLnhtz0JWV+uV6OL2md7LNbdhqOWJzeqHN4GaAQMzIm6hmowpNzxYuM1QuUWVFfm8W5cUN"
    "xjBGIOxh90yrLXLKbU462sGFK7N06PgkTU7nnp2orvLTxtvCtK678Ku/nILYFgWyy22Ol7bI"
    "CVdXqwEoIttSb8jo6QwRyTF695OLOb9z97Y26ukqTIzT60ggB/v5zEzd6MEWOXgl/sTNm7S8"
    "qspxcowYrKerhgL+BL36j/6sz57YtYm6OmoM5ePkkomYUB2/ft3U1I1uXnYe7u3tlfALHxDG"
    "SQXNbF3t6qin557cSjOTV+cS/sd9I9thnYSYU4MXiJll2MdOfra9tabmZvU1IhDIKYLMGA0E"
    "LK0L07NP302zkXH1iv+NLmM4RZB28Q+xE2ax7cI2Ofh5krNnz6qCOfk6FDObv1FkZ3sL/fiH"
    "T6hX/O/UERAz5IilizOnTzvysy2OBKF40RDWZ9asWaNG3fl2gRolxuzKI77f3NRg2nu0GwaI"
    "hUEQA8fo8/7+rIU9q3AkCMXvxlSxU4C3TmHhyukxyCis7lq1CtGVQWestg4ODamvLHbqd3Qc"
    "myHA78bAU8Hrr1CDBEm4Z3aHTKkOTlnZFyB0EeMLVlihK1oM9jI4+fs5js6tQTD88hQmEnt6"
    "elRXG92MWH83Ygy7W52sQJRZaL1fBJggRiyPYy/EaR5j0JV5+penACHgwYMHlcEzZ9R9CXit"
    "FRQxEpSJ1lYqCIOL1ctC38XZIwTeWKK/wmEE9Nu1a1dRalPRq6j21w5vTEwU/D6UfujBBx2d"
    "68oHseMFL70zEjTWfdV/7VALswpgXwJehgcySwGQg8EcxNhZ7y8GPLeeA5TyZJuTM+pOw7Pk"
    "lOpMaK7zqV6AJ8mxEoRaxSI5JuHWsUOvwXPL1IuYxyI5HsYiOR7GIjkexiI5HoYnyfHSm3Hd"
    "hOfI0R6DLDYyj/l5DZ6Lc+wcgzSLzGN+XoMn+w8cg1zT22v6GKRZgHycosMCIY75ua13Jjwn"
    "kICVY5BmkXnMz2v4P+EM9joepX/9AAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
aero_down = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB8AAAAgCAYAAADqgqNBAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzo1MTo0MyArMDEwMMndnrAAAAAHdElNRQfZAxkQNALaVrQp"
    "AAAACXBIWXMAAAsSAAALEgHS3X78AAAABGdBTUEAALGPC/xhBQAAAhJJREFUeNrtl01v2kAQ"
    "ht81FgECjUQUyeKjBrfi0EOJktxzg0uOSaWe0lNP/Wk99lBy7A+oSiO1ORE+BChSTjGfBuPs"
    "rATCNSkmyHtpX2ll2czOs+OdWTMMXIPBwKnVaqjX6xiORghK0UgEuVwOhmEgFosx1u/3ncrV"
    "FYrFItKpFMLhcGBwy7LQ7nRQrVZRLpXAfl5fO4l4HLquBwb9U41GA2avB6V+e4t0Oi0NTMpk"
    "MiCuOuavQlXVtRNGFs+NobPWLhphiO783YZ4gut3tc3OEN9/9fDQt5+0ebEbwttCHG9eR335"
    "9A0v5LhDx0LlW+tJm6OTLAr59WDHcTaDiwXk96AqNj5/+eH57fzsEIa+t4k7KMur8TMMPYkP"
    "744xMO8Wg+7puZ/5HrhfMI3ZbIb9ZByX708xHt6LK93Tc78+XHu+alXrlNdT+PTxgpepBtu2"
    "fc8nu1AotB18Op1C0w7EdRMxxrwJtyn8uXOW9ezIt5ErctlwT+SUrTKkKIo7cvrUTSYTKXDK"
    "dEo6V+RULrLkipxeuSy4p9QILmvP5yfhAi4z4VbCZZXaMkfZws/W+g//R+Hz4046fIe3R3S6"
    "0YEftChIOtuJR1zRLvHiQ4r3afRxCbLeCRzhzWK73Rb/gNRXvGP8WqmILkLTtEAbRQqu0+3i"
    "980NyuUyxGabpila5GarhVGALTJF/TKbFS1yIpFgj6VqglrJraorAAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
aero_down_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB8AAAAgCAYAAADqgqNBAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzo1NDo1OSArMDEwMEcuCiQAAAAHdElNRQfZAxkQNxVyqGIt"
    "AAAACXBIWXMAAAsSAAALEgHS3X78AAAABGdBTUEAALGPC/xhBQAABENJREFUeNrtV02IHEUU"
    "/qq6529nFnezLtGNEEk0O5NZI4GoQaLxoCJ4NgjBgAe9K/5c9SToQbx6UaJgQLyJXgIRDUkk"
    "t+iS1YDgwSWHMGOyO7Mz0z/l97qqZ3vGnd0ZYb1oDY9XXT39vve+eu9VtwJHo9H4+ufrK8+f"
    "XQ6xuh5jt8ZCReNM3Ue9uvjl3NzcKUXgH06fvX7C1B7H08cNDpQUSvxjXlGofWqPWo8JYCiR"
    "CCchdZe6Q71OudE2uPy9gv7tEj57qXpeXbh4yXzQfAzvnTQoGwWPKL4WMM4JrERgZdwhDgh/"
    "sRExCMURLhjOu1rjrW8M3tz3I/QnPwW4/4RCPrIPjRpaWad2Er2Nl+JELoox+xwguP6tDYOD"
    "RYW4sz1wnjzeaW3nnh2VKQXDvYpH/FUYWShpLBPXH4fGPCNaudHC+ctNNG6HI/+35y4fTx6b"
    "wZGlaYTRznb91BszQmR0aKhWrUDo+fSrX0YaO/XsImqHp9HK+GcyOq2jOM6CG0kGByhabmo7"
    "ly1kHqJNg/X6PF7xAnz48Xd/A37t1adwqDqP9cAM2jIZ+0NbkVSQgAUuI6VE0lLpZ2xs9Z2e"
    "waHFBbz7xjNYa/7eF7mWdbkfG/QlGpqL/SDeZEDbyPlQbKx2YuhRMocTt367F+PevTN45+0X"
    "0F5bTbRcy3r/WQzZyNplJIO0x7YMPIp2ks2XpM4zJXQr9FCvHcBH77+OBw7ux83OYJmmVKcM"
    "JlE7SbekD57SnaUs21iGu5s8vLoBzNy3P9HDI7vnm83GYqg0w7MJF2QiV8YhO52oLep2Y0Q5"
    "DQA6VgO332IvNNnIaSSIbB/3XaR9qmN3PUF/jV3kUZrA8WbCqeFSs3tiqEXY043tUHG6O2rr"
    "yEcNM1QtiV1hwLW9NNtt5KH1Ktl7bR9Mki6lHtv37K0iz+5zWr6h29KByM81DPaS9z0Fhemc"
    "QpnHWdGzbTWn7enmTQCenmI9onepW1xYC4Amy1Hx9wVtnUzBO4x8nTcKSicLXpKuPCA8m7GT"
    "gKeUC3iX4NKaNwjeZka3erbWgyDajLwb2PY5xasC1xl8QnNKtSRiPCG4ZHaPtrpOpDIEQ4KJ"
    "ogztkuotulrmYpEbnReqo836NhOCJ22UKD2JPpLorbRDY50bAA8t9V1Kj1T3tHt9cm8xySEz"
    "IXjax/sMOIxkDIDDlQV2Pl7HAd9KUtvZMe574a6M/8H/q+D63/Mii+Pfzc+jMmugwka+2w4U"
    "WdjlnMY054Lrv/xQDheuedj3RIQSi7/C7lLJASV2mIJnDxb/H/T2gC2xyN5e4PM5HpF5vqnM"
    "c71M0LlvFc4c9eEfXTq8Ur96pfr58nHUHjV4cEpjhqAVgvHDgocNHYDteOOCy6nGMwT8KEGL"
    "rP5JadCjX/mu98dFoBpfwbGHl2z3bDab5uq1ZZxbiXGztXufyPeUNV6sajxypI7Z2Vn1F7X+"
    "m7ZM/KBNAAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
aero_down_focus_single = aero_down_focus

#----------------------------------------------------------------------
aero_down_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB8AAAAgCAYAAADqgqNBAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzo1MTo0MyArMDEwMMndnrAAAAAHdElNRQfZAxkQNALaVrQp"
    "AAAACXBIWXMAAAsSAAALEgHS3X78AAAABGdBTUEAALGPC/xhBQAAAhJJREFUeNrtl01v2kAQ"
    "ht81FgECjUQUyeKjBrfi0EOJktxzg0uOSaWe0lNP/Wk99lBy7A+oSiO1ORE+BChSTjGfBuPs"
    "rATCNSkmyHtpX2ll2czOs+OdWTMMXIPBwKnVaqjX6xiORghK0UgEuVwOhmEgFosx1u/3ncrV"
    "FYrFItKpFMLhcGBwy7LQ7nRQrVZRLpXAfl5fO4l4HLquBwb9U41GA2avB6V+e4t0Oi0NTMpk"
    "MiCuOuavQlXVtRNGFs+NobPWLhphiO783YZ4gut3tc3OEN9/9fDQt5+0ebEbwttCHG9eR335"
    "9A0v5LhDx0LlW+tJm6OTLAr59WDHcTaDiwXk96AqNj5/+eH57fzsEIa+t4k7KMur8TMMPYkP"
    "744xMO8Wg+7puZ/5HrhfMI3ZbIb9ZByX708xHt6LK93Tc78+XHu+alXrlNdT+PTxgpepBtu2"
    "fc8nu1AotB18Op1C0w7EdRMxxrwJtyn8uXOW9ezIt5ErctlwT+SUrTKkKIo7cvrUTSYTKXDK"
    "dEo6V+RULrLkipxeuSy4p9QILmvP5yfhAi4z4VbCZZXaMkfZws/W+g//R+Hz4046fIe3R3S6"
    "0YEftChIOtuJR1zRLvHiQ4r3afRxCbLeCRzhzWK73Rb/gNRXvGP8WqmILkLTtEAbRQqu0+3i"
    "980NyuUyxGabpila5GarhVGALTJF/TKbFS1yIpFgj6VqglrJraorAAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
aero_left = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAfCAYAAACGVs+MAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzo0NDo0MCArMDEwMN+SkKkAAAAHdElNRQfZAxkQMBKjjWFJ"
    "AAAACXBIWXMAAAsRAAALEQF/ZF+RAAAABGdBTUEAALGPC/xhBQAAAkJJREFUeNrtV02P0lAU"
    "PS2lUEIlkxlHpCAEZ89Ol27M8A/czMK4duO/caeJiSZu3SgTN25M3DhBFkMmDhQJEK0apnwM"
    "0wK1t+Z1YOqyr2w8yWuT95qcc9+97/UeAS6m06nTarWg6zrOZzPwhJJMolQqoVwuI5VKCcJk"
    "MnFqh4eoVCrQcjnIssxVgGVZ6PX7qNfrqO7vQ/jSaDhqOo1isciV+Co6nQ5G4zFEvd2GpmmR"
    "khPy+TyIW7xwt0SSpMgFECdxi5EzX8HGBDiOs1kBDH7ymaKoIAjCpQAiXy6XXAlFUfR4aBA5"
    "EyBGET2R630LM+tvxpmQNQG8BpE3T8/w8cjEb3MREMe1BuLxOL4bY3z4NIAob/nzq1x+DYQt"
    "IBaLodM18PpNA0p6Fwn5kpzqjXZmTUCYRUiR//x1hucv3yOl3lhbY4Gyd+g7QJGffG3jxau3"
    "SCg7gXUWLH3nC6BfJI2wUC4V8OTxAZ4+qwXWiGexWPgp8J40EeYwTRM72xk8OrgfEMC+CRzD"
    "sEUYhoFi4ToePriDuBTzBdD20zqrOWl1Mmz0ej1kMhncu3sTn5u2XwORHEOG4XAIbfcazm0F"
    "aUXExdQO3oS8MXZbrz1t6ZL/gG3ba2uRtEKUYirMf2Hj/cB/AZ4A1hxsREDCdUKrVyNvULD0"
    "HyBO4vackXsokXNtGR0R3t0RkSddf0iX1Hw+h3TbNYnvajXPKGSzWe7ekILsDwY4bjZRrVbh"
    "JX80Gnnu+Fu3ixlnd0zR3yoUPHesqqrwB18A5ik1mQXQAAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
aero_left_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAfCAYAAACGVs+MAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzo0NjozNCArMDEwMCXtbZ4AAAAHdElNRQfZAxkQLw561jOY"
    "AAAACXBIWXMAAAsSAAALEgHS3X78AAAABGdBTUEAALGPC/xhBQAABAVJREFUeNrtV02IHEUY"
    "fVXTO70/M+yOExNd4oIRkgljDIiyQZR4FD0rQlDw6E1RDwkoIqiHXLzlKK4Iwb2peFDJHgxx"
    "dS+LcTMhhw0qJkvG7Jjsz8xOd1f5qqu6p9eNMB2Y9WLBm6+nf+p79X2vvqoSYFtdXf3ql8bl"
    "52aWQlxbVxhkmyxJvFz3UK8dmq1Wqy8IOv/+xEzjSV17AsePaTw0KjDCF4cFUKQtEB6vJSz6"
    "bWYYERFqoEvbpW3TXm1rfDsvIBsX8OlLte/E3PkL+vTqNN4/rlGCQEE6hzGEta5TkYOATsCf"
    "SBtCGqGy9wJ2+vo5jbcqP0J+fDHA/qcEisqyHnSL6MTjz+GnBYxv+SdDso/x1rvgPBudvb6A"
    "8Z0nrQNpXsJIZaxBNt/a/c8bJf0vME3pDAGlbG4ieikYLUgrPOV6MULU4i5EqK0j7XyYazMz"
    "RGYkXsImhbAvpIJ0Q8/jHM55MmLl+out6hFMCUR8OyCksiMX0kYj7gV26CInCZVxbvofKQhs"
    "0Jq6IPkkISKTFKQRcB9plWGtt+ujH8QRcBgfEmj+3sYe1hVTY0J3PyUQhYiLhHkQa8Hlalvo"
    "7hITRYErl5r44txNrNyMUGQkEpKZFLBUKiu2AjKhduKLLXKKkB9MjQn89kcLs18vo+Dvi++b"
    "gQZRTwtecjMk3UiaHAlb97VlGidfO2306dy8N+krNK5cx+kz51GamMKo77TB3Ea6Nw1lEoFA"
    "WSKR00SaBpU/Dff4EteuN/HOB5/dUZyh01hKwITEpMDAEAl0EpV/6KIPDDPHlxYX8eprH94x"
    "Oqb/bmT7TFMw09SodgJUWJ/HKZoSpWqmjU9BDMWrI1dJt0L20w7Wj+LMRyfxxtuf7Hi2fFvh"
    "V+45vuTYp9IUBMAaZ8IGmW0SbaKjLLZULzr9ovFXhMn778V7p07sIGD62wyxvQ6ABDZC+6BD"
    "bEUWXYdA5UOH33yzAhw++ADeffMZ+MVCSsD013YzIUMgIgHFkWt+TFBJW6TYpQ2IxOaB6efz"
    "qwHWS+N4/tkDGC95lgCfbdJXkNUA1E7RKbcoZStknqacKBeaIer7q5jurGGC+8Em05Msfr0I"
    "DLgttRRGDpQxe4Ob3s3tQ/F2g4BJ7Q90fivYuaP4z3dE/xOQye9uMjG+ksrg7RkRGKMwy0W5"
    "KyR8yr7M+r5GPRrf3itHhjC3WODhJILPyVv27FowSgy7tWAo51pgmqkBoVtlhyPEffmePX+U"
    "6bg8h/iM6D16pH6jvjC/9+zSMTw4rXGIZ8MJeivTMS/hmzOiMCTypSl2Trulbeldp71NIsu8"
    "cfEnoHZrHo8dfdjuMVqtll74eQlnLyusbAz2gHbfmMSLNYnHH6mjUqmIvwGdqbciWIcx6wAA"
    "AABJRU5ErkJggg==")

#----------------------------------------------------------------------
aero_left_focus_single = aero_left_focus

#----------------------------------------------------------------------
aero_left_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAfCAYAAACGVs+MAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzo0NDo0MCArMDEwMN+SkKkAAAAHdElNRQfZAxkQMBKjjWFJ"
    "AAAACXBIWXMAAAsRAAALEQF/ZF+RAAAABGdBTUEAALGPC/xhBQAAAkJJREFUeNrtV02P0lAU"
    "PS2lUEIlkxlHpCAEZ89Ol27M8A/czMK4duO/caeJiSZu3SgTN25M3DhBFkMmDhQJEK0apnwM"
    "0wK1t+Z1YOqyr2w8yWuT95qcc9+97/UeAS6m06nTarWg6zrOZzPwhJJMolQqoVwuI5VKCcJk"
    "MnFqh4eoVCrQcjnIssxVgGVZ6PX7qNfrqO7vQ/jSaDhqOo1isciV+Co6nQ5G4zFEvd2GpmmR"
    "khPy+TyIW7xwt0SSpMgFECdxi5EzX8HGBDiOs1kBDH7ymaKoIAjCpQAiXy6XXAlFUfR4aBA5"
    "EyBGET2R630LM+tvxpmQNQG8BpE3T8/w8cjEb3MREMe1BuLxOL4bY3z4NIAob/nzq1x+DYQt"
    "IBaLodM18PpNA0p6Fwn5kpzqjXZmTUCYRUiR//x1hucv3yOl3lhbY4Gyd+g7QJGffG3jxau3"
    "SCg7gXUWLH3nC6BfJI2wUC4V8OTxAZ4+qwXWiGexWPgp8J40EeYwTRM72xk8OrgfEMC+CRzD"
    "sEUYhoFi4ToePriDuBTzBdD20zqrOWl1Mmz0ej1kMhncu3sTn5u2XwORHEOG4XAIbfcazm0F"
    "aUXExdQO3oS8MXZbrz1t6ZL/gG3ba2uRtEKUYirMf2Hj/cB/AZ4A1hxsREDCdUKrVyNvULD0"
    "HyBO4vackXsokXNtGR0R3t0RkSddf0iX1Hw+h3TbNYnvajXPKGSzWe7ekILsDwY4bjZRrVbh"
    "JX80Gnnu+Fu3ixlnd0zR3yoUPHesqqrwB18A5ik1mQXQAAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
aero_right = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAfCAYAAACGVs+MAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzo0NToxOCArMDEwMEtfu5QAAAAHdElNRQfZAxkQLyM/CW/t"
    "AAAACXBIWXMAAAsSAAALEgHS3X78AAAABGdBTUEAALGPC/xhBQAAAkpJREFUeNrtVz2PEkEY"
    "fnbZQ+AgGD+R5YSAxVkRLfwF5vgH11xlbeNPsbE2Ftf4C+RiY2OijeGu8GIusEjgiJ4Ej689"
    "YHedd8zgsBgt3Fkan+TNZnY3eZ55v2ZeDQzj8dir1+uwLAsT24ZKxGMxFAoFFItFJBIJTRuN"
    "Rl714ADlchlmNotoNKpUwHQ6RbvTQa1WQ2VnB9rh0ZGXSiaRz+eVEvvRbDYxGA6hW40GTNMM"
    "lZyQy+VA3PoFc4lhGKELIE7i1kNn9mFtAjzPW68AgUXwhaKwoGnaLwFE7rru4gOZWKsiFwJ0"
    "/+7tqQ6rw7JTVxcd4hOchvyCVPXOHbz9cA57soHtUhqO4yjxgMBSDogPo4mDN+/OcCVt4Ob1"
    "JGazmTJvLEIgu4Uwmzt48fI9mq2vPBzyP/9qlF9LZeh/KeP5/mucffuOSCTC/wnCBOcfPSDj"
    "6bN9fDppBOYJucJ4DtARSfbzKI6vCHjyeA+X05sYstMrCJA3RZVxAZTpwvx4tPcQ166m0ev1"
    "Ak2+lTIkctk1G0YEu7sPeBV0u91AyQmCyxALEiBUbcYjuH/vFnsC7XY7cHK5yS01Iqr3ZELH"
    "3VIc5g0X/X4/cHKx4d+ehiTgYvwFd0w3sIT7G1auQiRCZefzY+33gf8CuAD5eAxdwCXWfqkH"
    "qLyAyKDNUismTuLmkxErSmTZWEbZr/puSOQxNh9Sg5vP5zBKbEh8Va3yQSGTySifDWmTndNT"
    "fDw+RqVSAQ/+YDDg0/HnVgu24umYdn97a4tPx6lUSvsBjEDOU65zEi4AAAAASUVORK5CYII=")

#----------------------------------------------------------------------
aero_right_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAfCAYAAACGVs+MAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzo0ODo0NiArMDEwMKZ+RR0AAAAHdElNRQfZAxkQMQU5RdXP"
    "AAAACXBIWXMAAAsSAAALEgHS3X78AAAABGdBTUEAALGPC/xhBQAAA/ZJREFUeNrtV09oVEcY"
    "/83sy27UfZhtNLWxKNaDK6sNlJZIabHgpdB7KYiCR28W7EEPIkLbgz1481hIaSt4a6UUWkyh"
    "Imm9hKbpSg7pIUVjotnGJG5235uZft+bmc1bk8A+NTn1W743b98bvt9vvn9vRoBkbm7uxp/V"
    "ux8Mjce4t6ixkdJflDhZCVApH7je29v7oSDwX48PVd8x5bdx9IjB/q0CW2hitwDyNOZIA7qX"
    "sNqp8DIUaWyAJo1NGus0/l03+GlEQFZv46sT5Z/F8K3b5vLcID49alCEQE46wESFHZ1RkYGA"
    "8UoXZZiQQazts4iMfnzT4JPSb5BfjkV49V2BvLasN1oUgQR0OfieAGPLh+SSl8nfZhPA097p"
    "KwgwdpawbogEnpFOjazpeBv3P6uXzDrKok2KgNY2NopQcpwL0iaedlY4EY14hiQ0Fsg4DL7n"
    "yhCplQSeTUuFndBKSLf0LOBw4H7F2tlLRr1CsEVA0eyIVNLLAi19K9VhnWvHOOc7UllI6BQ4"
    "21euHLkvSHrjiUgfAp7I9d9H/n44VUdPl1hhbdrzoxM1ZrV6W7FfmyegYiRNIp8TmH6k8N3N"
    "R5j4axalvGwj8TyqUupJrhDgdqktCZb5xRjXf5jEvw9q2EutudXNMgKyPT96jVgV2kNgX1Jc"
    "Ur2w0VS4+MWPqE5Mob+giAS9z6j84/jrVC7ERrcW0+aByJXi03Lhs69x7/4sXqLszOIBg7Xd"
    "7xPRpD3ALklCsE6nOX3mc1RHR9FNOaLWMLiWxs5e7O4jF+amsuoXm5Th0KxB73JEEwMMyNXF"
    "dvXKOQQ7dmFiXnVchp5IRC5oENgy/VmiZJ+n7zKtA9/T2ve0+kAELNDLZY1VH/1L54+j/5Wd"
    "+GVGZWrF3tVRAm71CWEsKQuq0x4AEWB2DdV6gkI+h7Nn38ee3SXcmIptvDJ0Ik8gcbu2thMi"
    "ZIr3HJFqI6CIgKaJtvNtLwY4duw1LBS349vJJgJC9puULASiBNwkMW/QAw4Dd1jpynGFgKvR"
    "Wfo+9+yUGBwIgd0hRmbipDXL9LehQ1mvCTEOL0alq8DLP080vpmOkd8XYqzWecI9j6zakEzX"
    "NW49iFGPs6TcCySw2fI/Aemvm8mEsXLuPtixRWAblURI3/7NIFGgwg+7JBYoxxk7OHW4C8Oj"
    "OTqcKBSoaEMq0iIpb8u6iSbNRRedkHLP0Ih8K+6mimZbhcCeP0ICDoeRnBGDNw5XZip3Rvqu"
    "jR/BvkGDA7QB6SG0kPeGBFjgM6JgEtnClIDT2OAzIZFYpPExEZmkB2O/A+X5Ebw5cMjuM2u1"
    "mrnzxziu3dWYXtrYA9qubRIflSXeer2CUqkk/gNN/sDRnOMoBAAAAABJRU5ErkJggg==")

#----------------------------------------------------------------------
aero_right_focus_single = aero_right_focus

#----------------------------------------------------------------------
aero_right_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAfCAYAAACGVs+MAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzo0NToxOCArMDEwMEtfu5QAAAAHdElNRQfZAxkQLyM/CW/t"
    "AAAACXBIWXMAAAsSAAALEgHS3X78AAAABGdBTUEAALGPC/xhBQAAAkpJREFUeNrtVz2PEkEY"
    "fnbZQ+AgGD+R5YSAxVkRLfwF5vgH11xlbeNPsbE2Ftf4C+RiY2OijeGu8GIusEjgiJ4Ej689"
    "YHedd8zgsBgt3Fkan+TNZnY3eZ55v2ZeDQzj8dir1+uwLAsT24ZKxGMxFAoFFItFJBIJTRuN"
    "Rl714ADlchlmNotoNKpUwHQ6RbvTQa1WQ2VnB9rh0ZGXSiaRz+eVEvvRbDYxGA6hW40GTNMM"
    "lZyQy+VA3PoFc4lhGKELIE7i1kNn9mFtAjzPW68AgUXwhaKwoGnaLwFE7rru4gOZWKsiFwJ0"
    "/+7tqQ6rw7JTVxcd4hOchvyCVPXOHbz9cA57soHtUhqO4yjxgMBSDogPo4mDN+/OcCVt4Ob1"
    "JGazmTJvLEIgu4Uwmzt48fI9mq2vPBzyP/9qlF9LZeh/KeP5/mucffuOSCTC/wnCBOcfPSDj"
    "6bN9fDppBOYJucJ4DtARSfbzKI6vCHjyeA+X05sYstMrCJA3RZVxAZTpwvx4tPcQ166m0ev1"
    "Ak2+lTIkctk1G0YEu7sPeBV0u91AyQmCyxALEiBUbcYjuH/vFnsC7XY7cHK5yS01Iqr3ZELH"
    "3VIc5g0X/X4/cHKx4d+ehiTgYvwFd0w3sIT7G1auQiRCZefzY+33gf8CuAD5eAxdwCXWfqkH"
    "qLyAyKDNUismTuLmkxErSmTZWEbZr/puSOQxNh9Sg5vP5zBKbEh8Va3yQSGTySifDWmTndNT"
    "fDw+RqVSAQ/+YDDg0/HnVgu24umYdn97a4tPx6lUSvsBjEDOU65zEi4AAAAASUVORK5CYII=")

#----------------------------------------------------------------------
aero_tab = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAfCAMAAACxiD++AAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFq6ysra6urq+vr7CwsLGxsbKysrOzs7S0tLS0tLW1tba2tre3t7i4uLm5uru7vL29"
    "v8DAwMDAwsPDxMXFxsbGycrKzc3Nzc7Ozs/P09TU19fX2dnZ2tra29vb3Nzc3N3d3t7e4ODg"
    "4uLi4+Pj4+Tk5OTk5OXl5ubm5+fn6Ojo6enp6+vr7Ozs7e3t7e7u7+/v8fHx8vLy8/Pz9PT0"
    "9fX19vb29vf3+Pj4+fn5+vr6+/v7/Pz8/f39/v7+AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAA0PbvAwAAABh0RVh0U29mdHdhcmUAUGFpbnQuTkVUIHYzLjM2"
    "qefiJQAAAP9JREFUOE+t0tlSwjAUgOFjZQuxuOBSFURSSOyq0qYNEd//tUzCaIeLk9743Zyb"
    "f5LOSUH3AC3vyEWIuIw06KvHRmGqKIJ3+u1RzaD0BpL+S3DwfIO5oqAHDxPk1LOpr9oGe0/h"
    "ArXHmSCjLbpIpSobyBa3o5DSWqJaF+xqlPykEE/LHFWmU2AkS1GZCdYkFajkjZhAxCghCLwS"
    "zlCcE1j1BpOeE176gqU/mMBy3F0Rb/mpZENh0QWxyJNTfD6HRXfFthgNndFgcJzDa2aCv0WZ"
    "tUj7blLdP9nZ6PCZATtPft+qjELt/vAm/HCzCNYmuA2O5xqzmzOwAuIG0AfGfgDFvqY+8bKe"
    "lgAAAABJRU5ErkJggg==")

#----------------------------------------------------------------------
aero_tab_focus = PyEmbeddedImage(
    "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")

#----------------------------------------------------------------------
aero_tab_focus_single = aero_tab_focus

#----------------------------------------------------------------------
aero_tab_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACAAAAAfCAMAAACxiD++AAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFq6ysra6urq+vr7CwsLGxsbKysrOzs7S0tLS0tLW1tba2tre3t7i4uLm5uru7vL29"
    "v8DAwMDAwsPDxMXFxsbGycrKzc3Nzc7Ozs/P09TU19fX2dnZ2tra29vb3Nzc3N3d3t7e4ODg"
    "4uLi4+Pj4+Tk5OTk5OXl5ubm5+fn6Ojo6enp6+vr7Ozs7e3t7e7u7+/v8fHx8vLy8/Pz9PT0"
    "9fX19vb29vf3+Pj4+fn5+vr6+/v7/Pz8/f39/v7+AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAA0PbvAwAAABh0RVh0U29mdHdhcmUAUGFpbnQuTkVUIHYzLjM2"
    "qefiJQAAAP9JREFUOE+t0tlSwjAUgOFjZQuxuOBSFURSSOyq0qYNEd//tUzCaIeLk9743Zyb"
    "f5LOSUH3AC3vyEWIuIw06KvHRmGqKIJ3+u1RzaD0BpL+S3DwfIO5oqAHDxPk1LOpr9oGe0/h"
    "ArXHmSCjLbpIpSobyBa3o5DSWqJaF+xqlPykEE/LHFWmU2AkS1GZCdYkFajkjZhAxCghCLwS"
    "zlCcE1j1BpOeE176gqU/mMBy3F0Rb/mpZENh0QWxyJNTfD6HRXfFthgNndFgcJzDa2aCv0WZ"
    "tUj7blLdP9nZ6PCZATtPft+qjELt/vAm/HCzCNYmuA2O5xqzmzOwAuIG0AfGfgDFvqY+8bKe"
    "lgAAAABJRU5ErkJggg==")

#----------------------------------------------------------------------
aero_up = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB8AAAAgCAYAAADqgqNBAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzo1MToxNCArMDEwMESarloAAAAHdElNRQfZAxkQMyBAd2MK"
    "AAAACXBIWXMAAAsSAAALEgHS3X78AAAABGdBTUEAALGPC/xhBQAAAh9JREFUeNrFl0tv2kAU"
    "hY+NBXbAElS0QTwColKX4Q+gbvmDXXXVriq160olq7b7Rk2zICsejWyUqkmQgEIMxvWxZAti"
    "0eCoHo40ssZj+5vrO3fmXgmuut2u4zb0+33MZjPEJU3TUKvVUK/X2SRlOp067ZMTNBoNNJtN"
    "JJPJ2OCWZcEwTXw/OwO50o/zc0fPZFCtVmOD3tdgMMB4MoHc7/VQKpWEgalyuQxy5Tv3VyiK"
    "IhROHrmyUOo97QXuOM7+4L4CZ/uziVuSJG3CCRYFX5+AcDg5iURiP3Ba7bOE+3xde/H5huWi"
    "4SHLV6uVEKAsy5uW86hbLBZC4FzpoVCzbVsI3OcFcP5yUfBQqBEuyufkhFb7Y+BMubheHguX"
    "fXiURjH7sSw7yIKivh/Ao4g+KxQK6A1MvHr9wbuyv35a7arI8Gw2i+ubCd6++4KUlveu7PN+"
    "rPBcLoer33d48/4bDvTDoLHP+xyPop0zR13XYfxaov3VhJZ5Fhr/+NlE62UFh090jMfj/wdP"
    "p9O4uk3itDNB6iC/9bnTjoXjFxmUnu42AQ/+0GKZz+c4KuaQz6kPflBTJYxuRrtZnnJjlbsb"
    "N/xtsc7x2+thkIH8S6M/9tbdkkbyGxwn1yuX3OBDsVj0Dpc4j1aCVVWFYRhYLpdQnrsV46d2"
    "26siGK9xFoo0zhwO0bm4QKvVgudsd3F4JfLPy0vPv3GJVh9VKl6J7EaP9Be4+2JJRD7+lAAA"
    "AABJRU5ErkJggg==")

#----------------------------------------------------------------------
aero_up_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB8AAAAgCAYAAADqgqNBAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzo1NTo1MyArMDEwMAycPlQAAAAHdElNRQfZAxkQOBMcU9vX"
    "AAAACXBIWXMAAAsSAAALEgHS3X78AAAABGdBTUEAALGPC/xhBQAABFhJREFUeNq1V82LHEUU"
    "/1VNz863yWyUyCSaEPzYuGtkwY8QYwLiQfTgwYsgBPMXeDJXvYmo+Qc8KEFIULyJHhQWNcSE"
    "3KLrLgqLoMYVkplIdmZ2PrrL36uqnulOsu50YB68ed3V3fV771fvvZpSoDSbzS9/Xll9+czy"
    "EFc3IkxLGlWNE/MBjj17RBljPlQE/uH1MytHzcEjeOGwwYGSQokvzigqbUCbo9UTAhhqKMqL"
    "IW2PdpN2g/pbx+DH7xX02gWcPblwWi2dv2Debz2Dd48bVIxCjiiBFjBeE1iJwumkIg4If5ER"
    "NRiKIxxgtOhpjVNfGby15xL0xz8NsP+owkzoPpqmiBP5MEL9RUBw9bWuwe6iwvRWOi2C0yhp"
    "CO6kSzkVCWJvzBYqopBtScwdruM8sHj+wkYuiWGMBxTrHxrvQdZcMCat0S33qcgFbMBBTau0"
    "y3CL6K0yfmxCibzDoYkz3pdelGbA0U53Ir4RqfHLmq9FcHVmSy1D+GbEphnNF5cdLFYuAR65"
    "MsgJqNcwMZkFzxB5THXcbGzUXk1iGS240BGmPEw3lqwlkcyfcbNxGHbOKEW7W/M4cpVYb7Em"
    "I+0pQM/qIHIq8w1NMnLyMwhdHw98pCOqI39/NwnnGY2tMKwwLjUL7tbE0Iqyp1Ot5/HqqOwJ"
    "FyYot/MKA5EZMTOOfOi8smuv3Yc26RLdRWeMPLnOoU++oV/SVOTnmuzv5H22oFDLK1S4nRVZ"
    "DTN0JK/d7pbLAB7vYn2i92jbHLg5AFp9KV6Fs5zreAy+ycg3+KDADiMDOZuuCibnMjYLeEy5"
    "gPcIvsmQuwTvMKPbfWNrfTAIx5H36FWHDpR5V+A4g7c0x1RLIkYZwSWz+5yr57UbOgwJJgwT"
    "tEuqt+lqhYNFLvSMUB2O69tkBB/adm1IuwBL9E47ZMA6lwIfOup71D6p7mv/98m3VvHC3AIu"
    "tzuYI//2zG27WJy8cW1bBjyGFQ8+al5RQs02Ksvxyr4Ae9t/WqvV9t8k548l858JYeOlhsLy"
    "yhrePHXaWrnPUg13DX5oNsDf/9zAO+99jnKtYa3cy/hUwRd3Bbj5xzre/uAb1Or7Rir3Mi7P"
    "s8jEbz+yI4fm79fxyRdrqO588LbnH322hjdezePRvbP4pRVONOdE4PtrGr2rHXx7qWup3krk"
    "+bGwjYceqNCB4bbzOnD9//yv86Tx/IEKGrvL205YLSssXds68iROcC+PRxXmf5WNfCsHpEF8"
    "xwlLE/DUve7a6Z2kyO8reY0arwU3OPl4HktXctjzHCenw1V2l2oeKLF2Cjm3scQNZzjByUKO"
    "WvfwoDdgSyyytxf4fZ5b5Az/qdxHnyoE3fW1wonFAMHiwmOr85cvzn26fBgHnzZ4uKyxk6BV"
    "gvFgwc2GDsB1vEnEdjj+cA8BDyVo0+Eb1CbZ+5Unxr/OA3PRRTz5xILrnq1Wy1y+soxzqxHW"
    "29M7ON1f0XhtTuOpQ/Oo1+vqPxxtdiUOpmR7AAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
aero_up_focus_single = aero_up_focus

#----------------------------------------------------------------------
aero_up_single = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB8AAAAgCAYAAADqgqNBAAAALHRFWHRDcmVhdGlvbiBUaW1l"
    "AG1lciAyNSBtYXIgMjAwOSAxNzo1MToxNCArMDEwMESarloAAAAHdElNRQfZAxkQMyBAd2MK"
    "AAAACXBIWXMAAAsSAAALEgHS3X78AAAABGdBTUEAALGPC/xhBQAAAh9JREFUeNrFl0tv2kAU"
    "hY+NBXbAElS0QTwColKX4Q+gbvmDXXXVriq160olq7b7Rk2zICsejWyUqkmQgEIMxvWxZAti"
    "0eCoHo40ssZj+5vrO3fmXgmuut2u4zb0+33MZjPEJU3TUKvVUK/X2SRlOp067ZMTNBoNNJtN"
    "JJPJ2OCWZcEwTXw/OwO50o/zc0fPZFCtVmOD3tdgMMB4MoHc7/VQKpWEgalyuQxy5Tv3VyiK"
    "IhROHrmyUOo97QXuOM7+4L4CZ/uziVuSJG3CCRYFX5+AcDg5iURiP3Ba7bOE+3xde/H5huWi"
    "4SHLV6uVEKAsy5uW86hbLBZC4FzpoVCzbVsI3OcFcP5yUfBQqBEuyufkhFb7Y+BMubheHguX"
    "fXiURjH7sSw7yIKivh/Ao4g+KxQK6A1MvHr9wbuyv35a7arI8Gw2i+ubCd6++4KUlveu7PN+"
    "rPBcLoer33d48/4bDvTDoLHP+xyPop0zR13XYfxaov3VhJZ5Fhr/+NlE62UFh090jMfj/wdP"
    "p9O4uk3itDNB6iC/9bnTjoXjFxmUnu42AQ/+0GKZz+c4KuaQz6kPflBTJYxuRrtZnnJjlbsb"
    "N/xtsc7x2+thkIH8S6M/9tbdkkbyGxwn1yuX3OBDsVj0Dpc4j1aCVVWFYRhYLpdQnrsV46d2"
    "26siGK9xFoo0zhwO0bm4QKvVgudsd3F4JfLPy0vPv3GJVh9VKl6J7EaP9Be4+2JJRD7+lAAA"
    "AABJRU5ErkJggg==")

#----------------------------------------------------------------------
aero_denied = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABYAAAAWCAYAAADEtGw7AAADxklEQVQ4jbWUzWuUVxTGn3Pe"
    "+955k8nMJE5m0vqRZCaUYomG0uJGjDR0UXBRUj+6Kqg7kSy6SBf5A7ropiB07aK4qIsWIYsu"
    "ioXBQoRKqWhJ1eq0sWqSmWQyk8y8H/e+p4t8oJ2YuPHA2dzD+fHwnHMuiQheR/BroQJQOxWv"
    "Z7N9cRB8nAA+dIEBAIiAvwPgJ04kro1Vq/Mv66XtrCgVi9osLEx1K/V57+HD6a5Dh8B9fSDH"
    "ga1WsXr7Niq3btVrUfS1yue/HH34MNwV/HM+n1HN5vd7+/vHsmfPQu3bt60i8+wZli5fxr/l"
    "8nXT2fnJBwsLK8/XX/B4ZnhYqWbzyv5icaz34kXwnj2IrYUoBXge4HkQpRBbC85kkL1wAfuH"
    "hsZUs3llZnhYvRQclMvnsqnUidTp04DWoEwGnE6DEwmQ44AcB5xIgNNpUCYDaI3UqVPIplIn"
    "gnL53LbgmULB9YCp9LFjcHM5KM8D+z6o0QD+l9RogH0fyvPg5nLIHD8OD5iaKRTcTd6W/KhS"
    "OZJOJgc7R0ag4ngd8gqhAHSOjMArlQbrlcoRAL+8oJhFjiaLRWhmrN29ixYzfNdF6LqIXBdG"
    "a1itEWsNaA24LoQZ5s4duAC6hobAIkfbFDOQ78jlQPU6zNIS3jh/Hqz1jmolDFE7eRIqk4GX"
    "zYKBfDtYJGDfB9VqkNXVV7IBAOT+fYjrrs9DJGgDE/Ns+OABkExCBQHmL10CM4OMAVkLAkAb"
    "3hEAIgJZC7YW9OgRwnodzDy7xds8kF97et70iMrFfF6T1gAzEEWAMburBvBwcTH0RQbfX15+"
    "Cjw3vIOl0tMgiq7VVlbA9Tq4VgOvrYGDYNes1esIoujawVLpaZtiAPitu/sta8yt/o6OVI/a"
    "8X/aimVj8E+r1XCUeu/dWu3+tmAA+D2dHvetvdqtlDrgOEgQbQsMRDBnLWrGGM9xzozU6z88"
    "X28D29VV/LF370d+HH/bFOntIkKGCJuLFwJYEcGqCDqJKh7zZ+88efKj09WFHcFbtuTzOW61"
    "vghEPo1EDtiNdweASzSnib6Tjo6vum/eXCwUCm39dO/ePUxMTDizs7Oq0WjoMAy1tVbHceyK"
    "iJsFOs4wv32AqF8AzMXx3FWRP5cAn4hCZo6YOdRah8lkMhwYGDCTk5NGzc/PY21tDcYYiuOY"
    "RIQ30gGgKiL4xpi/AJSxvsIWAIjIAaBEJBYRx1rLxhhqtVqoVqu0rRXT09N048YNPH78mKrV"
    "KjWbTfi+TwDgeZ4kEgn09vZKX1+fjI6OYnx8vA3yHxWIwp50Lj49AAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
auinotebook_preview = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABGdBTUEAAK/INwWK6QAAABl0"
    "RVh0U29mdHdhcmUAQWRvYmUgSW1hZ2VSZWFkeXHJZTwAAAJXSURBVHjahFJRSBNxGP/d/+52"
    "W66ildPKJB9EMSwjFSNoUkS+GYg9pi9R9tJeopcgC3yKYvRUMHA+VBJUmpQPQSySKINqEGSB"
    "Np3aNjs3N8+7285d/7sxa9yw3/G777v/932/+76PP6PrOgw8D876dYImWJAEZPpW8l89nYea"
    "i8KGgMHRYPitvgkCw0G9qaNXD4x80Qs1BknRnzaBEfX1e+G758PQaEgvnP8VULA5lCS8/T7T"
    "NUQK4ApO4j/1l3s6N/zA8MiGTxiGgUGowGwkhqkfc5Bl1SKwlM6i90y75dzsoKX9fNPLF2Mt"
    "sZVGuMt3YPDpK7jsDnj7uiEIfH6ClAjF5rAKHD1xcW99a927C5e6hP3luwCGwONpRjwm4tqA"
    "H7du9pmJDG9HrsSeSE3dvmdnz512tFZVQaGjaCyBjedRubscxzva8PrNJzNR0xmsKsQq4KzY"
    "fqDW7UY8o4KjxRzLmpYlBLUNNZj48BVRMQlWA7I5yTqCvcyuq+s0qmkQiC1/QeiTo1ajNpWS"
    "oMoMVJVyXbZ2sDj9S5sMh2l7CjJUJGswq0HNZBCNLmEhPA0xsQpJUSHJJUZILCbuxuZF7fv8"
    "ApbTaaysrSEpSVgURdy//RDJxAp2RvzgMgkQzbpEziVEbgQfBeuXPY1dqcMS4W08cnIWY0Pj"
    "iIXjuNI2A0dsDmXyT3yUTtESZ5EAU3CuDvhD7ydDB1mWg6zQWelSj2ydwbE9v1Fd4TQZmHLj"
    "yTfBzP88PsgUCZTCne4qFycID7Zt4TuqK50TFaTmZMP1x5l/c/4IMABbKBvEcRELXgAAAABJ"
    "RU5ErkJggg==")

#----------------------------------------------------------------------
whidbey_down = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACsAAAAeCAMAAACCNBfsAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAaHWVZ3egZ3e1cYCncYa0dpC4cYXGcpDBc5fWd6Dld6Pwi5OzgJbUkJjQkqHC"
    "hafggqfwh7Dwk6Xgl7fwoKjWp7fXsbbWoLfnoLjwpMLwscHnsMfxtdD0wMHg5eTo5/D49/f3"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAverH2wAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAF4SURBVDhPbZTpYoIwEITRGqAQ1ChqOFrx/R8y3SObq8yvsHwZhglaOdH7HZbJIp1W"
    "YW7t7w68LOcwDaxd5+lhC3q18zwOMhTWIjtZ+3wGfFmWFYZjgJl9vWC/1x30AI2zzExzIcqz"
    "a2RxC2Ak2n4zJmEx1TgaY8R7AtHsdgMS1IivJRZHyN5lA6PMEgwZ2JVYs23bxwuWG3PGdF3X"
    "ImvtBO/KqAl38ytgO+WqMz1SDDz7KXaCbatU5YYARt+SbUHg61zT9/iMHsQZKHBI9A1S6njk"
    "fpuCRTjmJVQBymfxlfuCc5IX0cMhnhvCMYO8KCdCV0L9GbMz8Fhqous1ugbWw/32k2oDtFUn"
    "do2sz1ywqWvCElz6pq4pS3CRVxooMvjqsI2+v5LwXCVrljfrGclhqH2v5e+Nr6VnYltgBct7"
    "iDDExm+grvm0ouL/QwrjN1CHXvczYAz0xF5Pp9w168zfAhjeH9gSle8hnWutldL6H7rHOq2P"
    "agd1f/M7VhKuYPh3AAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
whidbey_down_single = whidbey_down

#----------------------------------------------------------------------
whidbey_down_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACsAAAAeCAMAAACCNBfsAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAECBXFC9vECiQBS2oIDKMIDigC0CVJUqQIkewJFKlJlayMEiwMlSiK2G0JFTR"
    "LmnSKGHmKHDoInDwMGjgNnDgNXLwSFeKQFizR2OkUGagcHiYZ3egcHilUHjRQXLwR4DwU4Pw"
    "YIfTcYXGcIjVc5fWZpHxcJj3d6PwgJbUkJjAkJjQkqHChafggqfwh7Dgh7Dwl7fwoKjAqLDI"
    "sLDIl8D4pMLw4Of35/D4AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAQS37pgAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAGeSURBVDhPfdOLUsIwEAVQCCixFSggVfBFQWpVjKJo+f8fi3ezSZq2jDsDhHB62Wyh"
    "o319Vctg9V2tO34p5H0bHw5D4Xe9FULK5bKhb6UcDs/cprNdMYiprhee30khaStx2NpudwCb"
    "JHG8WAhbEkU0cZgtUge0iYrjCEUuilheOWysS2U9GpluGJJ1mKyg1LHZn6NYNNYUCWuozSjr"
    "FVxnLGQUjZPJlLJKCk7piar0Nk2zS90RAhINVDZN5yUeNZuhbjq6BzmZomYoICr/Yr5mtcry"
    "fL2mfnvjSd2anvkSsqBZUazN2XS/YVNLrc2Qut3yHAj/10OWFwVRtrpPvZo5cL+28A7fnxfv"
    "G6Ps74GwsfXCsXKkMnWWko39DKukAbjUygKfskFqYLnnKvcHdQT9sA0EPdgD1ts9Vr2G/fpp"
    "pDMeAg3gWe2fgn9V9d+0yW5edCylQurnwJejZ/7VPJpZ7V+C1PBsFrPFXAul6rSRi2S2uFmq"
    "kdrK1frcWOq1kXrC6osHulnq963Wa3Nm9kOySrVpq1/yr5vNbtdK1foPGIxy6qmqIg0AAAAA"
    "SUVORK5CYII=")

#----------------------------------------------------------------------
whidbey_down_focus_single = whidbey_down_focus

#----------------------------------------------------------------------
whidbey_dock_pane = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAF0AAABdCAMAAADwr5rxAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAaHWVcHiYZ3egZ3e1cHilcYCncYa0dpC4ZobHcYXGcpDBc5fWcZjgd6Dld6Pw"
    "i5Ozl6e3q62xgIjQgpXEgJbUkJjAkJjQgJjggKDXkqHCk6TWhafggqfwh7Dwk6Xgk7Tkl7fw"
    "oKjAoKjWqLDIp7fXsLDIsbbWoLfnoLjwsLjjl8D4pMLwscHnsMfxtdD0wMHg6tTN4Njk5eTo"
    "4Of35/D48Ofh9/f3AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAA1jVbdgAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAdFSURBVGhDtZpxe6I4EMbtbQVXrHJuaY8KWq/XW3Sr196u8v0/GTfzThICRIh9nssf"
    "u4rw42UyM5kJHVVXjDQah/EV51ejK05+SKMwjK/BX0Gfzu9ms2kY3PgL8qd/nc/vCB+GN/54"
    "b/p0sVjMaRA+9Mb70hku9GvUe9IBV3TC+9rejy5wTfdX70VneJ7LHRaLu7uIPNPL9j50hp+a"
    "w9NzPOhQfvrHHqeZn/phOmzeofvZfpA+zTFa2rNsFk2GPWeIruB5y+5En00mg1E7QNfw/CSP"
    "oEaZ51kWDavvpxu4phOWB+hJMqi+l17DFb0sBa+eJB3KOX30xLIFeAQXvKYTvtf2PfTVbrtd"
    "6xsonrKMoafhJAwu5/vL9NWO6Rp/iZ5OJsFlfI/2onh7oxvQYKOsaeALjTV944fK2DFn489o"
    "ryqmA69NDrtjWPSwZyHs9RlWr+h5Qzt9UdqzPvhATTBMn/Uu4QOxut9r24vFdzSUvUh+lk37"
    "6wMXfVNf8sF0mdrt9nGF8YbB3ppNl/WpK8eNHPTNxuCXHx/a9q+vZz3exZvW6+m5NPhi58B3"
    "6Yfj2zYRHcuSgOI5Bi0fDgeiT/nT8oxTi2K3U1dZz9ChH5gu+GXJ9HNRvP/dgoOfyEGox/Tn"
    "nVlo04kNm34DHPRfv7psxuujywqm4rBo41t0Vs4uvl4/PXHUNLj7/f6VRvte5RJTjxBo4Zv0"
    "A3sDxw2d+EeTcnzd4qnIdR43zZ/EbSUjNfEN+oHppFsiEZOmxn5/ZJOBvt1uNivrNyvk2nib"
    "fmA6W0W0Z6FGkL0sNvLYt0L/aKeLtnqLTvCD2FzRNR53VelSZ8l1IfhfdqqDbRaWcWo6nWZs"
    "zrk1SZLVvww40oBeZTG9oDwbOm6pDy8svKHzA9Y2Z3oUgU5WOSIXdOjAv/Nv1hLJ5aZRr+lQ"
    "XhNYevSEZye6ZJo2PZNpd9ANXtFFuYtOM0p50UnP4Dm2VeQZavVCZ6cqudziCOL/szSKYkh7"
    "hReu62UJv/MyS8+HM2AZuUwNgwcdTlVSkasrrppOgSL0Rg2ME9l6uD8/cqmWd643T0Y90xEO"
    "azku48RzCu0vau1z0lNEhJu++Epkoks4dOhjPPaL+Hl+gf4upmlplzaIPGdUIbewXls7OTvo"
    "xktbdK6auNyAWz266fOHaoQ4ZKNpOl940vRXHQN99BU9XMkzqylo3+7u0nTEdkHkKzoKRUNH"
    "tccxeoEOwxs6XQqK0GdRNFoVKkqEDo+8lk4Syq2UsDWd2ubxKHtm5dzVid1Fe5bOjCx4kUt7"
    "mohf8QPCMg3ttGERjp4z1Yk27O5HV15r6MbuVF1S2xbHo+pRNblNf88SaN9sVIpyaSfDYhnU"
    "dEQK7A54ELO/f6VJqC2joknF4bmfPgadw4UtY+hmo4Vj9ZanWNtd07PsRfmDuYqTgc43nAei"
    "e8TEu4OuWh7kGcb70kViKfR7phdvVA808gw34thHkBzJ+EZHSquH5NfzN5p1ZU+S/kM3fZxE"
    "owBnCF2lWFAivcWi8jvUYydDBujAZ0JX7XD5Q0yTMT0EnVf6emHh9FC3anptgu3nc5ueppC2"
    "0nfT//OdqRcOA5HeoVsbFGZdFbxFJ3/7C3ShyTPoz/C4PwHnhau+rLmxVdcEwNunRdEYNcFK"
    "8Wv6jO4cBL8DfuC1xbqssbVi1TOMN6chHoSu1Ws6ByLZBfDzObEX8yRqNvd2LaZsLzt3k8lY"
    "1g8aDw+Ukc3gfatAzMJjapTnOdEb2yqNOtLgSdwkMHBWGEIxxmQS/iYTqvAyGSmN9rZEswZW"
    "eJkaC0Af4/ieBrPHY6kWWniCh+0NoVb9DjzbPLhZNhH07f4+CIK4heYbT0V7d0ui3XvcSvLk"
    "QEbXROPnz859zFGOLGptZhRDru2UTt9EeB3ISj31TQ784fBIRwVeVTQtrq2gbs93G0Vfvtg9"
    "3/Ft92KqdXWfI1cpVCspOOMnjg3QLr0aRwpOxuGOj+i7orD6JarnqVugEE2sfnXs2khx0CsD"
    "r6rv33U3w/3MZvNCQ/dP3EI+1c2pa5fGRbfa2e/SApohXbw6RmFk0a2rzMd+ujRclAI0XnYh"
    "pBRH1ujfhuil6yZX6LTEmlEnl158H91uFVVtr+t7ne0o9iKXSdSxHjpvLJieqLmXar5xaPfs"
    "Ll2mP8IMWqRzTy/PSTplzIvqe7Qndit3gc4581Paq4p23lHpYP+dH6K5lyoZSce1U3+vz0xb"
    "dMZLgcD/AB5aode9Qb+/S5VmtEsDUNMH334MxKqpcxw+4/HWbIAuNSbXgc3RLIo+4zNyjeBR"
    "ZdbjRKuFKec+F03qqrpCtuieb/uGLGNXyDbdR7mugXsezlUhn06D3iJED+2qvkcDpCpUvzdl"
    "nnTVnSg6vavxfT/spb1Wz9qpnPN7Q+mrXTsmF1I+78j0PHpq13jOLR7v966mV6jS0BX0vAJq"
    "OZ+3dlLP8UnDH+7nkUpRHMf/399CVFUcf7nu7zj+A8yummsi9EdGAAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
whidbey_dock_pane_bottom = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAF0AAABdCAMAAADwr5rxAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAECBXFC9vECiQBS2oIDKMIDigC0CVJUqQIkewJFKlJlayMEiwMlSiK2G0JFTR"
    "LmnSKGHmKHDoInDwMGjgNnDgNXLwSFeKQFizR2OkUGagaHWVcHiYZ3egZ3e1cHilUHjRQXLw"
    "cYCncYa0dpC4R4DwU4PwZobHYIfTcYXGcIjVcpDBc5fWZpHxcZjgcJj3d6Dld6Pwi5Ozl6e3"
    "q62xgIjQgpXEgJbUkJjAkJjQgJjggKDXkqHCk6TWhafggqfwh7Dgh7Dwk6Xgk7Tkl7fwoKjA"
    "oKjWqLDIp7fXsLDIsbbWoLfnoLjwsLjjl8D4pMLwscHnsMfxtdD0wMHg6tTN4Njk5eTo4Of3"
    "5/D48Ofh9/f3AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAaHenigAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAejSURBVGhDpZkLe9o2FIYpTVeWrG3Srmy9stk0LTPJMKMhmYM9KOuSERqydAX8//+H"
    "dy6SLF+QRXeepwtg+9XnT+fotlqyRQRey+1vcX9S2+Lm08Bz3f42+C3ond5xt9txnRf2guzp"
    "b3u9Y8C77gt7vDW94/t+DwLwrjXelo5wpm+j3pJOcEEHvK33dnSGS7q9eis6wqOIW/D942MP"
    "MtPKexs6wlfZsMwcCzopX/2jx6prp76aTp4X6HbeV9I7EUVOexh2vXZ15lTRBTzK+Q70brtd"
    "WbUVdAmPVvwKIuIoCkOvWr2ZruCSDlgMog+HleqN9BQu6HHMePEmQdWYY6IPNS+IB3DGSzrg"
    "jd4b6KOr6XQsGxA84YyiB27bdTaP95vpoyukS/wmetBuO5vxBu2z2fU1NACBpowh6AvEGL7h"
    "S4WYmN3W12hPEqQTXlpOvlNodNcwERpzBtULepTRDl+E9tAEr1gTVNO7xim8olbnc+k9O34F"
    "IfwC+WHYMa8PyuiT9JHPSOeunU7PRhTXFJitYWeQ3joqaaiEPpko/ODzZ+n9xcVaxi1n03jc"
    "WccKP7sqwRfpi5vr6ZB1DGIAcuYoNH9YLIDewU+DNd06m11diae0dyjQF0hn/CBG+no2u73M"
    "wYk/5B9JPXV/VOiFPB3Y5Ol7ghN9uSyyES9/HSRkFZZFHp+jo3JM8fH4/ByrJsOdz+cXEPm2"
    "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")

#----------------------------------------------------------------------
whidbey_dock_pane_center = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAF0AAABdCAMAAADwr5rxAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAGDeAJUqQIEOkIkewJFKlJlayMlSiMle1K2G0LmnSNnDgR2OkQ2O1UGagUHCo"
    "VXSzaHWVZ3egZ3e1cHilQ2TAQHDAQnLSVnfFQXfgcYCncYa0dpC4VYTTRIDjVYbgZobHYIfT"
    "ZpPXcYXGcpDBc5fWYIjgYZHgcZjgd6Dld6Pwi5Ozq62xgIjQgJbUkJjAkJjQgJjggKDXkqHC"
    "k6TWlrDXhafggqfwh7Dwk6Xgk7Tkl7fwoKjWqLDIp7fXsLDIsbbWoLfnoLjwsLjjpMLwscHn"
    "sMfxtdD0wMHg6tTN4Njk5eTo4Of35/D48Ofh9/f3AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAXehG6QAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAdqSURBVGhDtZoNW9pIEMc5emrB651VUDRUpEkqb5d6SBOUXq9Q8dprke//aXLzsrvZ"
    "hGWz+NzN87RCzP74M5mZzGyspDvYOGh4wx3OTys7nPx+HHjecBf8DnS/3wtD37t47S7InX7V"
    "7/cA73mv3fHOdH8wGPTBAO85413pCGf6Luod6QQXdMC7+t6NznBJd1fvREd4kvAnDAa9XgCR"
    "6eR7FzrCV3lzjBwHOilf/a3bKnRTX04nn2/Q3XxfSvcTsoL2OA6DdnnklNEFPCn4Hehhu12a"
    "tSV0CU9W/BWErZMkjoNy9Xa6gks6YNGIHkWl6q30DC7o6zXjxTcZl9UcGz3SfEE8gDNe0gFv"
    "9b2FPlnMZlP5AYInPKPoY6/tXWyv99vpkwXSJX4bfdxuX2zHW7TP5w8P8AFg6JQpGL0Bm8I7"
    "/FIxBmbYeI72NEU64aXLye9kGt2z3AitMYPqBT3JaYc3Qntsg5f0BOX00HoLL8nVL1+k79nj"
    "CzDhL5Afx769PzDR77Il35DOl3Y2u5mQPZBhtMb+KDt1YvggA/3uTuFH375J33/69CTtK0fT"
    "dOo/rRV+vjDgN+nLx4dZxDpGawBy5Cg0v1guge7jq9ETnTqfLxZilfYdNuhLpDN+tEb603z+"
    "9a8CnPgRHyT1dPmTjatQpAObfPo7wYn+48cmG/Hy6CglV2FaFPEFOirHEJ9Ob28xazRuFIYt"
    "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")

#----------------------------------------------------------------------
whidbey_dock_pane_left = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAF0AAABdCAMAAADwr5rxAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAECBXFC9vECiQBS2oIDKMIDigC0CVJUqQIkewJFKlJlayMEiwMlSiK2G0JFTR"
    "LmnSKGHmKHDoInDwMGjgNnDgNXLwSFeKQFizR2OkUGagaHWVcHiYZ3egZ3e1cHilUHjRQXLw"
    "cYCncYa0dpC4R4DwU4PwZobHYIfTcYXGcIjVcpDBc5fWZpHxcZjgcJj3d6Dld6Pwi5Ozl6e3"
    "q62xgIjQgpXEgJbUkJjAkJjQgKDXkqHCk6TWhafggqfwh7Dgh7Dwk6Xgk7Tkl7fwoKjAoKjW"
    "qLDIp7fXsLDIsbbWoLfnoLjwsLjjl8D4pMLwscHnsMfxtdD0wMHg6tTN4Njk5eTo4Of35/D4"
    "8Ofh9/f3AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAA+wCLtAAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAeDSURBVGhDtZr/f9JGGMdprZPJnHSKm9OKI4DG0K5kWAptGIw5FGqpdQL5//+Q7Ply"
    "d7kkR3L4ms8PWmjyzifP3T33PM+1FO1gQ6/u+DtcH5V2uPh86DmOvwt+B7p7etLpuE7jmb0g"
    "e/qr09MTwDvOM3u8Nd3tdrunYIB3rPG2dIQzfRf1lnSCCzrgbX1vR2e4pNurt6IjfDTiJ3S7"
    "JycezEwr39vQEb5OmuXMsaCT8vUn3dYdO/XFdPJ5hm7n+0K6OyJLaQ+CjtcsnjlFdAEfpfwO"
    "9E6zWbhqC+gSPlrzKwgLR6Mg8IrV59MVXNIBi0b0fr9QfS49hgt6GDJevMmwKObk0fuaL4gH"
    "cMZLOuBzfZ9Dv1xMp2P5AMETnlH0odN0Gtvj/Xb65QLpEr+NPmw2G9vxOdrn8+treAAYOmUM"
    "Rh/AxvAJXyrAidmpf432KEI64aXLye9kGt3J2Qhz5wyqF/RRQjt8ENqDPHhBTlBM7+Ru4QVr"
    "9epK+p49vgAT/gL5QeDm5wcm+n58y2ek89BOp4NLsmsynK2B24svvTQ8yEC/s6/wvc+fpe9n"
    "s420W55N47G7CRV+vjDgs/Q7D+/fE/heCECeOQrNPyyXQHfxp96GNM/ni0U/oz5DP3iIdML3"
    "QqRv5vPb9yk48fv8Jamn4R9lRiFNPzg8rCJ+j+BEX62ybMTLb3sRuQqXRRqfogP88BDo9/b2"
    "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"
    "RK5CYII=")

#----------------------------------------------------------------------
whidbey_dock_pane_right = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAF0AAABdCAMAAADwr5rxAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAECBXFC9vECiQBS2oIDKMIDigC0CVJUqQIkewJFKlJlayMEiwMlSiK2G0JFTR"
    "LmnSKGHmKHDoInDwMGjgNnDgNXLwSFeKQFizR2OkUGagaHWVcHiYZ3egZ3e1cHilUHjRQXLw"
    "cYCncYa0dpC4R4DwU4PwZobHYIfTcYXGcIjVcpDBc5fWZpHxcZjgcJj3d6Dld6Pwi5Ozl6e3"
    "q62xgIjQgpXEgJbUkJjAkJjQgJjggKDXkqHCk6TWhafggqfwh7Dgh7Dwk6Xgk7Tkl7fwoKjA"
    "oKjWqLDIp7fXsLDIsbbWoLfnoLjwsLjjl8D4pMLwscHnsMfxtdD0wMHg6tTN4Njk5eTo4Of3"
    "5/D48Ofh9/f3AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAaHenigAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAe+SURBVGhDtZr/W9pWFMYtW1en67RzunWdpUvQlgWdMCoyJFkotbaRCrMryP//f2Tn"
    "yz25N19ILn2enV8Qkn7y5s2555570414jRh4dbezxvnxxhonXww81+2sg1+D3jw7bbWarvPM"
    "XpA9/eXZ2SngXfeZPd6a3my322cQgHet8bZ0hDN9HfWWdIIrOuBtvbejM1zo9uqt6AgPAr5C"
    "u3166kFmWnlvQ0f4Ih2WmWNBJ+WLf8xYtOzUV9PJ8xzdzvtKejOgyGj3/ZbXqM6cKrqCBxnf"
    "gd5qNCpHbQVd4MGCb0HFMgh836tWX05P4EIHLAbRe71K9aV0DVf05ZLx6k4GVTWnjN4zvCAe"
    "wBkvdMCXel9CH96Mx6FcQPGUMwl94DZcZ3W9X00f3iBd8Kvog0bDWY0v0R5Ft7dwAQg0JYSg"
    "LxAhfMOb8jExW/Uv0R7HSCe8WE6+Uxh0t2QiLM0ZVK/oQUo7fFHa/TJ4RU9QTW+VTuEVY3Uy"
    "Ee/Z8RsI5RfI9/1meX9QRB/pf/IJ6fxox+P+kOKWArPVb3b1qT8XXKiAPhol+O6nT+L99fW9"
    "xB1nUxg275cJ/rvjV3l8nj6d3Y57fGJ3CUDOnATNf0ynQG/iX917OjU6Pn6Vx+foU6QzvrtE"
    "+n0U3b3PwInf4x9JfbR/cHzSzj3iLB3Y5OlrghN9Ps+zES+/duO7h/tMz+IzdFSOKR6Gl5c4"
    "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"
    "AABJRU5ErkJggg==")

#----------------------------------------------------------------------
whidbey_dock_pane_top = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAF0AAABdCAMAAADwr5rxAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAECBXFC9vECiQBS2oIDKMIDigC0CVJUqQIkewJFKlJlayMEiwMlSiK2G0JFTR"
    "LmnSKGHmKHDoInDwMGjgNnDgNXLwSFeKQFizR2OkUGagaHWVcHiYZ3egZ3e1cHilUHjRQXLw"
    "cYCncYa0dpC4R4DwU4PwZobHYIfTcYXGcIjVcpDBc5fWZpHxcZjgcJj3d6Dld6Pwi5Ozl6e3"
    "q62xgIjQgpXEgJbUkJjAkJjQgJjggKDXkqHCk6TWhafggqfwh7Dgh7Dwk6Xgk7Tkl7fwoKjA"
    "oKjWqLDIp7fXsLDIsbbWoLfnoLjwsLjjl8D4pMLwscHnsMfxtdD0wMHg6tTN4Njk5eTo4Of3"
    "5/D48Ofh9/f3AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAaHenigAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAe1SURBVGhDpZkNexJHEMcxrTVNao3G0PoSaXmJ4l1soBhCCUdBmkaPCDFWCN//e1z/"
    "M7O7t/fC3aHzPMqFu/3t/2ZnZmeTUrCBnZ83m2cbPB+UNni4NxweHx/9tsGIDei92bBF9B+L"
    "44vTu6T8AFb+oTC+ML07gfIjppe/L4ovSofy1nGjIfT9oviC9O6kBZ83flf0ovhi9NccLY2y"
    "sv39gvhC9NeT4R/HR5q+Xwa9GL4IvQmvLBcRK4gvQG/+DZcvP8JG2hYF1efTSflxY2HIdLHY"
    "gxVwTi696VGCptLz8Xl0Ud4oR90u2vOXNodOyjlYFjoa+VPT89Rn010ol+xXdP1Bq/oI9jA7"
    "azPprguX23T4R7QbejY+i/6q1YJXpLYwlZ0foz/4LqOkZdBfvaJgaZQzPbP3IAu/nv7rEYxi"
    "g+xgIfVrgX+44FV99HAP7J2dra216jO0/3RULkfpPAP9p+lg37u3Hp65r/oHByE9GvDy/e4u"
    "2Bnw7F3bD+kQbGbS1w/y4Dk9wd29vcePtc5dtp9/1rOQ8jt3MjfBnFwlPLtgd2dnG7YD291V"
    "88Et2fBU7eNQz2fgiU3kLWVPnhBflHfCRwcpb5GifTw2+M5n4Mkf21vbh7fanvzCPoLy25XB"
    "+9MUfJI+v55NuqKjswLw7v3729vPDVounj6ld6Grzi0/6vvTqRplvUOCPie64Dsrot9u3X//"
    "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"
    "gg==")

#----------------------------------------------------------------------
whidbey_left = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB4AAAArCAMAAABYWciOAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAaHWVZ3egZ3e1cHilcYCncYa0dpC4ZobHcYXGcpDBc5fWd6Dld6Pwi5OzgIjQ"
    "gJbUkJjQgJjggKDXkqHCk6TWhafggqfwh7Dwk6Xgk7Tkl7fwoKjWqLDIoLfnpMLwscHnsMfx"
    "tdD04Njk5/D49/f3AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAQ3JLMwAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAF4SURBVDhPfdTbdoIwEAVQL6htAi0tElBaoVb+/xPTM5M7wc6LLvc6GTIJbnRel/DT"
    "ZkUvwXOefsahdqGMJ2LvSwaO4P7d5BdMWejQ9+dzzhOiY4/quu4z44kYSebudbH4REwLm7Q6"
    "Jr2hk+lrGR4e7Rb17ZRSdV23v55vxL4vsRCBOesXRrquBbZm0yb7jIHD/EDNKPpUjRBvbmOU"
    "Heb7/f6g/aISJhz6R8J4MJvmbM6FGSrPmRaM09i0YZ4zDdHxTN8909I8RMtzyu3NztgwbyxK"
    "qytlq6pK0qqROC/0vipGzx0v7ll/MAY2Y1G1SWv9UpXlCvNlpiPZlyjX2w4VJ/rlZk7+D3Oe"
    "DsoXwqoNV5HzKHtidJlUC3eXifuXZcxNE91U4xFLIb6jm8oeWIKLcFPN/jxLKY/HhM3+TUl5"
    "OhU48eQN9Y6VTwe6D+kLbJ0W3m5X3m926ntgXb7+eg/z2ZzJhcuusN4Lsds9/WfSuhBes94U"
    "C6r/AM3yZVcU56/qAAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
whidbey_left_single = whidbey_left

#----------------------------------------------------------------------
whidbey_left_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB4AAAArCAMAAABYWciOAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAECBXFC9vECiQBS2oIDKMIDigC0CVJUqQIkewJFKlJlayMEiwMlSiK2G0JFTR"
    "LmnSKGHmKHDoInDwMGjgNnDgNXLwSFeKQFizR2OkUGagcHiYZ3egcHilUHjRQXLwR4DwU4Pw"
    "YIfTcYXGcIjVc5fWZpHxcJj3d6PwgJbUkJjAkJjQkqHChafggqfwh7Dgh7Dwl7fwoKjAqLDI"
    "sLDIl8D4pMLw4Of35/D4AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAQS37pgAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAGNSURBVDhPfdRrW8IgFAdwLyuXZLIUy67D6dK0pd3w+38xOuewAYOMN3ue/fbnwHZG"
    "R8ej5251Yu33nMfcHw8H1iNOxsiNh5xkGUfvmqIBg2YZ8KBrvM3J5BoGH7NBnW9xMjHMGTjl"
    "fQYlxvKYh/V5jErc5MEdJxMOAx/BKxZI0w/LkI340aZx5jZzdmdrU92TjKr8IbB2szHKqv1+"
    "r2Y4hFDINzWf082A2Xxu+II0TDNU4KcnAcNPz4TiV0P6pp1XKduM9R0XZdlipXBpnNXpgGl3"
    "Hu+qdSnzZ1sb0libpab2rqrKMs/d0tqstz+QhwfUt/dabFrrbYXzy1OsNwf0Fgs2TB+ad77B"
    "6SNOba9B/uV49D6ZqIvX3bJp1m++mEBO710rLg8wv8wNmVZj6ZdrxeWatmcZW9FjXbxj3ufR"
    "6NPr8wLyHnM28vtc6+INX08+w9LTKednwS9I+TwnJg3+UMoTGw3/7wKndxqyXq3gAdg9ZaO0"
    "1oab8yo+mRYLKe1p9se5tpCX/7G+dUfhL8vucupsrDz0AAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
whidbey_left_focus_single = whidbey_left_focus

#----------------------------------------------------------------------
whidbey_right = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB4AAAArCAMAAABYWciOAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAaHWVcHilcYCncYa0dpC4ZobHcYXGcpDBc5fWcZjgd6Dld6Pwi5OzgJbUkJjA"
    "kJjQgKDXk6TWhafggqfwh7Dwl7fwp7fXoLfnoLjwsLjjpMLwscHntdD0wMHg4Of35/D49/f3"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAWZqHkAAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAFySURBVDhPddPbeoIwDABglDqoFDcVGBVw8P4PyXJoQ0sxN+L3m0NDzVaJx/YoT5k8"
    "9fbAhfve2luS77kfhq5rir077pkTZ34Ng7Vt2yRO/ELuUPeOTIWxdOrA3Fc46p/9AVobsgnm"
    "Z0b1xRsTeLa+EV1f+jCBQ+8DlnzgsDBX2fLxYFR8WeYtxJF/u65tF95KM0/TNEv+ZzZfkElL"
    "TbKhuDEVnJ/4Z1+cufpmfsBwC47newNV1fV6v8cMTqMx67Jkhs0s3YIRsNbqHDCePczWhVIx"
    "S28NoVRdRyxrMaR5zZPjdcDJha+opxOf+33ACthtrR/glkY7LzmXs5npjbn3VqqcFHmE2i0E"
    "934+fd9PjKXdvylbR7yn/q7FuVB8HOF9uMJUOsjF3retb9PcysuFZ+aA0QrJJXYzC6/Fk+IO"
    "Eee628IOquJcx5wP6nYV9cYvGpYBKucNRqNHpfW+r9+580uS63vjD855vvXcF4fvB7r+A9+i"
    "Xf4K/oDaAAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
whidbey_right_single = whidbey_right

#----------------------------------------------------------------------
whidbey_right_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAAB4AAAArCAMAAABYWciOAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAECBXFC9vECiQBS2oIDKMIDigC0CVJUqQIkewJFKlJlayMEiwMlSiK2G0JFTR"
    "LmnSKGHmKHDoInDwMGjgNnDgNXLwSFeKQFizR2OkUGagcHiYZ3egcHilUHjRQXLwR4DwU4Pw"
    "YIfTcYXGcIjVc5fWZpHxcJj3d6PwgJbUkJjAkJjQkqHChafggqfwh7Dgh7Dwl7fwoKjAqLDI"
    "sLDIl8D4pMLw4Of35/D4AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAQS37pgAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAGXSURBVDhPdZTZVoNADEDpaC2CFSqC1hVaxWoVqdv0/39szDIraF7gcHuTTCankbJx"
    "6V7tW2TfTprVmDvcNKsxt7ismnbzOPQ1npaMh5zxNMdo4Afr0CfMNK8Bv4UcMdBzwshDHzBS"
    "wlWN6QM/UmKecu68hBj40ed8nmrOuN28u/qR+op9XNfANw+mf8asow31ge8Mh9au4zhlRIF+"
    "1z2zjwcTiKWL/f6p2zFHHMdJWkpty77/lpCffcQ3IwzHY5+GitkDG8fTddv/MB2v+9n6dlVJ"
    "aBxq9/Dk/l+95IDgu8b3eD0GJ1ibTmYwzqFt12wTLn07xKc51XW16XqaF20D1jPVtRHf3XHn"
    "Sxyqm1ovC5r+MZ97OcJEj/TULuA+B3ZRFIdm5njd/o1JaSgmvzK7Bh8LXAt8kku1/8KaAr61"
    "u+ZsQ1X0Aauks1tsKdhCzGb4gzMKr66uTTzLFwuNnctjmUycb3t2m6om7JPtu3qZyE+yBURI"
    "+UogvwAM5QfUYOw/ybIhtVghPuBUXrg/LiHG1NmwcSNXqV+4tHLqnJPo+QAAAABJRU5ErkJg"
    "gg==")

#----------------------------------------------------------------------
whidbey_right_focus_single = whidbey_right_focus

#----------------------------------------------------------------------
whidbey_up = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACsAAAAeCAMAAACCNBfsAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAaHWVcHilcYCncYa0dpC4ZobHcYXGcpDBc5fWcZjgd6Dld6Pwi5OzgJbUkJjA"
    "kJjQgKDXk6TWhafggqfwh7Dwl7fwp7fXoLfnoLjwsLjjpMLwscHntdD0wMHg4Of35/D49/f3"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAWZqHkAAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAFmSURBVDhPhdTtdoMgDAZgW3GKWjsL2i/X2vu/SJaEBBXcWf6Jz3l5i9bMpdNXR3Xa"
    "Wc/StXNfKXXawaktm1rrUuWHJCWxX01TA1bqkODYlm3bNjCAVYwji9TbneStJcoWcNR5Yz0V"
    "mySvLVJrvW/buq7g7NadVxbpvJ3taSyWUuef9cx6kxwsdU3sprPY0tJEucboqginwZapjfqC"
    "1UUhT9BboXb28Twfa42pQjLZQMUCwiHbdZKMdqFsPx+PeZee3w2w3WpXugvUY7GAsXPmLvdx"
    "HITzXe4QbK8Klbvsckcr+C/bF0WeZ+52ez6Bw+D2AwxdwAxwhRsaPDp9hA4OLWGpSn1pVlZh"
    "X8Cg2dpNLlxwrgFKFpP/sRqZf26Ph3T2Te8w3AyijSlJ8fuA1v/Acfy+0Dxp8DyZig2dr9fw"
    "WXj5ExoGnxpy5TSi78c0gRUacvE0XjvfsGnqwuryH3q/d6hz07L6CxOEXf5LAPv7AAAAAElF"
    "TkSuQmCC")

#----------------------------------------------------------------------
whidbey_up_single = whidbey_up

#----------------------------------------------------------------------
whidbey_up_focus = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAACsAAAAeCAMAAACCNBfsAAAAAXNSR0IArs4c6QAAAARnQU1B"
    "AACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAA"
    "AwBQTFRFAAAAECBXFC9vECiQBS2oIDKMIDigC0CVJUqQIkewJFKlJlayMEiwMlSiK2G0JFTR"
    "LmnSKGHmKHDoInDwMGjgNnDgNXLwSFeKQFizR2OkUGagcHiYZ3egcHilUHjRQXLwR4DwU4Pw"
    "YIfTcYXGcIjVc5fWZpHxcJj3d6PwgJbUkJjAkJjQkqHChafggqfwh7Dgh7Dwl7fwoKjAqLDI"
    "sLDIl8D4pMLw4Of35/D4AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAQS37pgAAAQB0Uk5T////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////////////////////////////////////////////////////////"
    "////////////////////AFP3ByUAAAAYdEVYdFNvZnR3YXJlAFBhaW50Lk5FVCB2My4zNqnn"
    "4iUAAAGTSURBVDhPfdQNV4IwFAZgpEyCVBRZaR8OUcI0yrLw//8xuvfuy22e7jkKZz68uxtg"
    "0Pm135fl24XxwB/bNU1VFS/+D77d/TY12lsPe3aLqTkUu3Gxa7cHSC3IsmsHOxZS64pzYTMH"
    "23Z7qKFXvpTWwZZd0w5wJivLbHxu14fmtSqUzRhYC5/ZEuY/tVbZ2NjyA1o9/UB9qmrtZG0x"
    "teKtdnjSplCmDWXLd7xZF63G0opUzux2Ra5eoLCYShvQqv2io7IymewGUsV9lVYdcG1TqAnd"
    "QbSbDbR6bqETkastYbCruob5xTNAhpp27PgK7WqFG8DZvz2kY8DBQwGF68XKW/HUtPCBE1rb"
    "dJKCjOMwDLq7gjHbkscvZUEOBiGtLc+NtTdYjCcJyFDsQ2cshOnr1PlYUmH7aTqbqYyEajRS"
    "12Bqr6f2V2CaLInjCCqGShJ5NTRAVOQSRokulDWfozap2gLGmaMwetIv7/yeulGpxnb94TCK"
    "Hp23fLHAedSgeS/C4fHo/y89R5qqfhF9+xJGvszoH5Xccuo6pVT3AAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
whidbey_up_focus_single = whidbey_up_focus

#----------------------------------------------------------------------

whidbey_denied = aero_denied

#----------------------------------------------------------------------

# ------------------------ #
# - AuiToolBar Constants - #
# ------------------------ #

ITEM_CONTROL = wx.ITEM_MAX
""" The item in the AuiToolBar is a control. """
ITEM_LABEL = ITEM_CONTROL + 1
""" The item in the AuiToolBar is a text label. """
ITEM_SPACER = ITEM_CONTROL + 2
""" The item in the AuiToolBar is a spacer. """
ITEM_SEPARATOR = wx.ITEM_SEPARATOR
""" The item in the AuiToolBar is a separator. """
ITEM_CHECK = wx.ITEM_CHECK
""" The item in the AuiToolBar is a toolbar check item. """
ITEM_NORMAL = wx.ITEM_NORMAL
""" The item in the AuiToolBar is a standard toolbar item. """
ITEM_RADIO = wx.ITEM_RADIO
""" The item in the AuiToolBar is a toolbar radio item. """
ID_RESTORE_FRAME = wx.ID_HIGHEST + 10000
""" Identifier for restoring a minimized pane. """

BUTTON_DROPDOWN_WIDTH = 10
""" Width of the drop-down button in AuiToolBar. """

DISABLED_TEXT_GREY_HUE = 153.0
""" Hue text colour for the disabled text in AuiToolBar. """
DISABLED_TEXT_COLOUR = wx.Colour(DISABLED_TEXT_GREY_HUE,
                                 DISABLED_TEXT_GREY_HUE,
                                 DISABLED_TEXT_GREY_HUE)
""" Text colour for the disabled text in AuiToolBar. """

AUI_TB_TEXT             = 1 << 0
""" Shows the text in the toolbar buttons; by default only icons are shown. """
AUI_TB_NO_TOOLTIPS      = 1 << 1
""" Don't show tooltips on `AuiToolBar` items. """
AUI_TB_NO_AUTORESIZE    = 1 << 2
""" Do not auto-resize the `AuiToolBar`. """
AUI_TB_GRIPPER          = 1 << 3
""" Shows a gripper on the `AuiToolBar`. """
AUI_TB_OVERFLOW         = 1 << 4
""" The `AuiToolBar` can contain overflow items. """
AUI_TB_VERTICAL         = 1 << 5
""" The `AuiToolBar` is vertical. """
AUI_TB_HORZ_LAYOUT      = 1 << 6
""" Shows the text and the icons alongside, not vertically stacked.
This style must be used with ``AUI_TB_TEXT``. """
AUI_TB_PLAIN_BACKGROUND = 1 << 7
""" Don't draw a gradient background on the toolbar. """
AUI_TB_CLOCKWISE        = 1 << 8
AUI_TB_COUNTERCLOCKWISE = 1 << 9

AUI_TB_HORZ_TEXT        = AUI_TB_HORZ_LAYOUT | AUI_TB_TEXT
""" Combination of ``AUI_TB_HORZ_LAYOUT`` and ``AUI_TB_TEXT``. """
AUI_TB_VERT_TEXT        = AUI_TB_VERTICAL | AUI_TB_CLOCKWISE | AUI_TB_TEXT

AUI_TB_DEFAULT_STYLE    = 0
""" `AuiToolBar` default style. """

# AuiToolBar settings
AUI_TBART_SEPARATOR_SIZE = 0
""" Separator size in AuiToolBar. """
AUI_TBART_GRIPPER_SIZE = 1
""" Gripper size in AuiToolBar. """
AUI_TBART_OVERFLOW_SIZE = 2
""" Overflow button size in AuiToolBar. """

# AuiToolBar text orientation
AUI_TBTOOL_TEXT_LEFT = 0     # unused/unimplemented
""" Text in AuiToolBar items is aligned left. """
AUI_TBTOOL_TEXT_RIGHT = 1
""" Text in AuiToolBar items is aligned right. """
AUI_TBTOOL_TEXT_TOP = 2      # unused/unimplemented
""" Text in AuiToolBar items is aligned top. """
AUI_TBTOOL_TEXT_BOTTOM = 3
""" Text in AuiToolBar items is aligned bottom. """

# AuiToolBar tool orientation
AUI_TBTOOL_HORIZONTAL = 0             # standard
AUI_TBTOOL_VERT_CLOCKWISE = 1         # rotation of 90 on the right
AUI_TBTOOL_VERT_COUNTERCLOCKWISE = 2  # rotation of 90 on the left


# --------------------- #
# - AuiMDI* Constants - #
# --------------------- #

wxWINDOWCLOSE = 4001
""" Identifier for the AuiMDI "close window" menu. """
wxWINDOWCLOSEALL = 4002
""" Identifier for the AuiMDI "close all windows" menu. """
wxWINDOWNEXT = 4003
""" Identifier for the AuiMDI "next window" menu. """
wxWINDOWPREV = 4004
""" Identifier for the AuiMDI "previous window" menu. """

# ----------------------------- #
# - AuiDockingGuide Constants - #
# ----------------------------- #

colourTargetBorder = wx.Colour(180, 180, 180)
colourTargetShade = wx.Colour(206, 206, 206)
colourTargetBackground = wx.Colour(224, 224, 224)
colourIconBorder = wx.Colour(82, 65, 156)
colourIconBackground = wx.Colour(255, 255, 255)
colourIconDockingPart1 = wx.Colour(215, 228, 243)
colourIconDockingPart2 = wx.Colour(180, 201, 225)
colourIconShadow = wx.Colour(198, 198, 198)
colourIconArrow = wx.Colour(77, 79, 170)
colourHintBackground = wx.Colour(0, 64, 255)
guideSizeX, guideSizeY = 29, 32
aeroguideSizeX, aeroguideSizeY = 31, 32
whidbeySizeX, whidbeySizeY = 43, 30

# ------------------------------- #
# - AuiSwitcherDialog Constants - #
# ------------------------------- #

SWITCHER_TEXT_MARGIN_X = 4
SWITCHER_TEXT_MARGIN_Y = 1
