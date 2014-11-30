"""
This module contains all the constants used by the persistent objects.
"""

import wx

# ----------------------------------------------------------------------------------- #
# PersistenceManager styles

PM_SAVE_RESTORE_AUI_PERSPECTIVES     = 1 << 0
PM_SAVE_RESTORE_TREE_LIST_SELECTIONS = 2 << 0
PM_DEFAULT_STYLE                     = PM_SAVE_RESTORE_AUI_PERSPECTIVES


# ----------------------------------------------------------------------------------- #

CONFIG_PATH_SEPARATOR = "/"

BAD_DEFAULT_NAMES = []
for name in dir(wx):
    if "NameStr" in name:
        BAD_DEFAULT_NAMES.append(eval("wx.%s"%name))

# ----------------------------------------------------------------------------------- #
# String constants used by BookHandler

PERSIST_BOOK_KIND = "Book"
PERSIST_BOOK_SELECTION = "Selection"

# To save and restore wx.lib.agw.aui.AuiNotebook perspectives
PERSIST_BOOK_AGW_AUI_PERSPECTIVE = "AGWAui_Notebook_Perspective"

# ----------------------------------------------------------------------------------- #
# String constants used by TreebookHandler

PERSIST_TREEBOOK_KIND = "TreeBook"

# this key contains the indices of all expanded nodes in the tree book
# separated by PERSIST_SEP
PERSIST_TREEBOOK_EXPANDED_BRANCHES = "Expanded"
PERSIST_SEP = ','

# ----------------------------------------------------------------------------------- #
# String constants used by TLWHandler

# we use just "Window" to keep configuration files and such short, there
# should be no confusion with wx.Window itself as we don't have persistent
# windows, just persistent controls which have their own specific kind strings

PERSIST_TLW_KIND = "Window"

# Names for various persistent options
PERSIST_TLW_X = "x"
PERSIST_TLW_Y = "y"
PERSIST_TLW_W = "w"
PERSIST_TLW_H = "h"

PERSIST_TLW_MAXIMIZED = "Maximized"
PERSIST_TLW_ICONIZED = "Iconized"

# To save and restore wx.aui and wx.lib.agw.aui perspectives
PERSIST_AGW_AUI_PERSPECTIVE = "AGWAui_Perspective"
PERSIST_AUI_PERSPECTIVE = "Aui_Perspective"

# ----------------------------------------------------------------------------------- #
# String constants used by CheckBoxHandler

PERSIST_CHECKBOX_KIND = "CheckBox"
PERSIST_CHECKBOX_3STATE = "3StateValue"
PERSIST_CHECKBOX = "Value"

# ----------------------------------------------------------------------------------- #
# String constants used by ListBoxHandler

PERSIST_LISTBOX_KIND = "ListBox"
PERSIST_LISTBOX_SELECTIONS = "Selections"

# ----------------------------------------------------------------------------------- #
# String constants used by ListCtrlHandler

PERSIST_LISTCTRL_KIND = "ListCtrl"
PERSIST_LISTCTRL_COLWIDTHS = "ColWidths"

# ----------------------------------------------------------------------------------- #
# String constants used by CheckListBoxHandler

PERSIST_CHECKLISTBOX_KIND = "CheckListBox"
PERSIST_CHECKLIST_CHECKED = "Checked"
PERSIST_CHECKLIST_SELECTIONS = "Selections"

# ----------------------------------------------------------------------------------- #
# String constants used by ChoiceComboHandler

PERSIST_CHOICECOMBO_KIND = "ComboBox"
PERSIST_CHOICECOMBO_SELECTION = "Selection"

# ----------------------------------------------------------------------------------- #
# String constants used by RadioBoxHandler

PERSIST_RADIOBOX_KIND = "RadioBox"
PERSIST_RADIOBOX_SELECTION = "Selection"

# ----------------------------------------------------------------------------------- #
# String constants used by RadioButtonHandler

PERSIST_RADIOBUTTON_KIND = "RadioButton"
PERSIST_RADIOBUTTON_VALUE = "Value"

# ----------------------------------------------------------------------------------- #
# String constants used by ScrolledWindowHandler

PERSIST_SCROLLEDWINDOW_KIND = "ScrolledWindow"
PERSIST_SCROLLEDWINDOW_POS_X = "Scroll_X"
PERSIST_SCROLLEDWINDOW_POS_Y = "Scroll_Y"

# ----------------------------------------------------------------------------------- #
# String constants used by SliderHandler

PERSIST_SLIDER_KIND = "Slider"
PERSIST_SLIDER_VALUE = "Value"

# ----------------------------------------------------------------------------------- #
# String constants used by SpinHandler

PERSIST_SPIN_KIND = "Spin"
PERSIST_SPIN_VALUE = "Value"

# ----------------------------------------------------------------------------------- #
# String constants used by SplitterHandler

PERSIST_SPLITTER_KIND = "Splitter"
PERSIST_SPLITTER_POSITION = "Position"

# ----------------------------------------------------------------------------------- #
# String constants used by TextCtrlHandler

PERSIST_TEXTCTRL_KIND = "TextCtrl"
PERSIST_TEXTCTRL_VALUE = "Value"

# ----------------------------------------------------------------------------------- #
# String constants used by ToggleButtonHandler

PERSIST_TOGGLEBUTTON_KIND = "ToggleButton"
PERSIST_TOGGLEBUTTON_TOGGLED = "Toggled"

# ----------------------------------------------------------------------------------- #
# String constants used by TreeCtrlHandler

PERSIST_TREECTRL_KIND = "TreeCtrl"
PERSIST_TREECTRL_CHECKED_ITEMS = "Checked"
PERSIST_TREECTRL_EXPANSION = "Expansion"
PERSIST_TREECTRL_SELECTIONS = "Selections"

# ----------------------------------------------------------------------------------- #
# String constants used by TreeListCtrlHandler

PERSIST_TREELISTCTRL_KIND = "TreeListCtrl"
PERSIST_TREELISTCTRL_COLWIDTHS = "ColWidths"

# ----------------------------------------------------------------------------------- #
# String constants used by CalendarCtrlHandler

PERSIST_CALENDAR_KIND = "Calendar"
PERSIST_CALENDAR_DATE = "Date"

# ----------------------------------------------------------------------------------- #
# String constants used by CollapsiblePaneHandler

PERSIST_COLLAPSIBLE_KIND = "Collapsible"
PERSIST_COLLAPSIBLE_STATE = "Collapse"

# ----------------------------------------------------------------------------------- #
# String constants used by DatePickerHandler

PERSIST_DATEPICKER_KIND = "DatePicker"
PERSIST_DATEPICKER_DATE = "Date"

# ----------------------------------------------------------------------------------- #
# String constants used by MediaCtrlHandler

PERSIST_MEDIA_KIND = "MediaCtrl"

PERSIST_MEDIA_POS = "Seek"
PERSIST_MEDIA_VOLUME = "Volume"
PERSIST_MEDIA_RATE = "Rate"

# ----------------------------------------------------------------------------------- #
# String constants used by ColourPickerHandler

PERSIST_COLOURPICKER_KIND = "ColourPicker"
PERSIST_COLOURPICKER_COLOUR = "Colour"

# ----------------------------------------------------------------------------------- #
# String constants used by FileDirPickerHandler

PERSIST_FILEDIRPICKER_KIND = "FileDirPicker"
PERSIST_FILEDIRPICKER_PATH = "Path"

# ----------------------------------------------------------------------------------- #
# String constants used by FontPickerHandler

PERSIST_FONTPICKER_KIND = "FontPicker"
PERSIST_FONTPICKER_FONT = "Font"

# ----------------------------------------------------------------------------------- #
# String constants used by FileHistoryHandler

PERSIST_FILEHISTORY_KIND = "FileHistory"
PERSIST_FILEHISTORY_PATHS = "Paths"

# ----------------------------------------------------------------------------------- #
# String constants used by FindReplaceHandler

PERSIST_FINDREPLACE_KIND = "FindReplace"
PERSIST_FINDREPLACE_FLAGS = "Flags"
PERSIST_FINDREPLACE_SEARCH = "Search"
PERSIST_FINDREPLACE_REPLACE = "Replace"

# ----------------------------------------------------------------------------------- #
# String constants used by FontDialogHandler

PERSIST_FONTDIALOG_KIND = "FontDialog"
PERSIST_FONTDIALOG_EFFECTS = "Effects"
PERSIST_FONTDIALOG_SYMBOLS = "Symbols"
PERSIST_FONTDIALOG_COLOUR = "Colour"
PERSIST_FONTDIALOG_FONT = "Font"
PERSIST_FONTDIALOG_HELP = "Help"

# ----------------------------------------------------------------------------------- #
# String constants used by ColourDialogHandler

PERSIST_COLOURDIALOG_KIND = "ColourDialog"
PERSIST_COLOURDIALOG_COLOUR = "Colour"
PERSIST_COLOURDIALOG_CHOOSEFULL = "ChooseFull"
PERSIST_COLOURDIALOG_CUSTOMCOLOURS = "CustomColours"

# ----------------------------------------------------------------------------------- #
# String constants used by ChoiceDialogHandler

PERSIST_CHOICEDIALOG_KIND = "ChoiceDialog"
PERSIST_CHOICEDIALOG_SELECTIONS = "Selections"

# ----------------------------------------------------------------------------------- #
# String constants used by MenuBarHandler

PERSIST_MENUBAR_KIND = "MenuBar"
PERSIST_MENUBAR_CHECKRADIO_ITEMS = "Checked"

# ----------------------------------------------------------------------------------- #
# String constants used by ToolBarHandler

PERSIST_TOOLBAR_KIND = "ToolBar"
PERSIST_TOOLBAR_CHECKRADIO_ITEMS = "Checked"

# ----------------------------------------------------------------------------------- #
# String constants used by FoldPanelBarHandler

PERSIST_FOLDPANELBAR_KIND = "FoldPanelBar"
PERSIST_FOLDPANELBAR_EXPANDED = "FoldpPanelExpanded"
