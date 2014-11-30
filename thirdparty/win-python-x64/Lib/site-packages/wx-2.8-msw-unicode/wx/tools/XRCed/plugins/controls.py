# Name:         controls.py
# Purpose:      Control components
# Author:       Roman Rolinsky <rolinsky@femagsoft.com>
# Created:      31.05.2007
# RCS-ID:       $Id: core.py 47823 2007-07-29 19:24:35Z ROL $

from wx.tools.XRCed import component, images, attribute, params
from wx.tools.XRCed.globals import TRACE
import _bitmaps as bitmaps

TRACE('*** creating control components')

# Set panel images
component.Manager.panelImages['Controls'] = images.ToolPanel_Controls.GetImage()

### wxStaticText

c = component.Component('wxStaticText', ['control','tool'],
              ['pos', 'size', 'label', 'wrap'], defaults={'label': 'LABEL'},
              image=images.TreeStaticText.GetImage())
c.addStyles('wxALIGN_LEFT', 'wxALIGN_RIGHT', 'wxALIGN_CENTRE', 'wxST_NO_AUTORESIZE')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'label', 'wxStaticText', 10)
component.Manager.setTool(c, 'Controls', pos=(0,0))

### wxStaticLine

c = component.Component('wxStaticLine', ['control','tool'],
              ['pos', 'size'], image=images.TreeStaticLine.GetImage())
c.addStyles('wxLI_HORIZONTAL', 'wxLI_VERTICAL')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'line', 'wxStaticLine', 20)
component.Manager.setTool(c, 'Controls', pos=(0,3))

### wxStaticBitmap

c = component.Component('wxStaticBitmap', ['control','tool'],
              ['pos', 'size', 'bitmap'],
              image=images.TreeStaticBitmap.GetImage())
c.setSpecial('bitmap', attribute.BitmapAttribute)
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'bitmap', 'wxStaticLine', 30)
component.Manager.setTool(c, 'Controls', pos=(1,0))

### wxTextCtrl

c = component.Component('wxTextCtrl', ['control','tool'],
              ['pos', 'size', 'value'],
              image=images.TreeTextCtrl.GetImage())
c.addStyles('wxTE_NO_VSCROLL',
            'wxTE_AUTO_SCROLL',
            'wxTE_PROCESS_ENTER',
            'wxTE_PROCESS_TAB',
            'wxTE_MULTILINE',
            'wxTE_PASSWORD',
            'wxTE_READONLY',
            'wxHSCROLL',
            'wxTE_RICH',
            'wxTE_RICH2',
            'wxTE_AUTO_URL',
            'wxTE_NOHIDESEL',
            'wxTE_LEFT',
            'wxTE_CENTRE',
            'wxTE_RIGHT',
            'wxTE_DONTWRAP',
            'wxTE_LINEWRAP',
            'wxTE_CHARWRAP',
            'wxTE_WORDWRAP')
c.setParamClass('value', params.ParamMultilineText)
c.addEvents('EVT_TEXT', 'EVT_TEXT_ENTER', 'EVT_TEXT_URL', 'EVT_TEXT_MAXLEN')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'text ctrl', 'wxTextCtrl', 40)
component.Manager.setTool(c, 'Controls', pos=(0,2))

### wxChoice

c = component.Component('wxChoice', ['control','tool'],
              ['pos', 'size', 'content', 'selection'],
              image=images.TreeChoice.GetImage())
c.addStyles('wxCB_SORT')
c.setSpecial('content', attribute.ContentAttribute)
c.addEvents('EVT_CHOICE')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'choice', 'wxChoice', 50)
component.Manager.setTool(c, 'Controls', pos=(3,2))

### wxSlider

c = component.Component('wxSlider', ['control','tool'],
              ['pos', 'size', 'value', 'min', 'max', 
               'tickfreq', 'pagesize', 'linesize', 'thumb', 'tick',
               'selmin', 'selmax'],
              image=images.TreeSlider.GetImage())
c.addStyles('wxSL_HORIZONTAL', 'wxSL_VERTICAL', 'wxSL_AUTOTICKS', 'wxSL_LABELS',
            'wxSL_LEFT', 'wxSL_RIGHT', 'wxSL_TOP', 'wxSL_BOTTOM',
            'wxSL_BOTH', 'wxSL_SELRANGE', 'wxSL_INVERSE')
component.Manager.register(c)
c.setParamClass('value', params.ParamInt)
c.setParamClass('tickfreq', params.ParamIntNN)
c.setParamClass('pagesize', params.ParamIntNN)
c.setParamClass('linesize', params.ParamIntNN)
c.setParamClass('thumb', params.ParamUnit)
c.setParamClass('tick', params.ParamInt)
c.setParamClass('selmin', params.ParamInt)
c.setParamClass('selmax', params.ParamInt)
c.addEvents('EVT_SCROLL', 'EVT_SCROLL_TOP', 'EVT_SCROLL_BOTTOM',
            'EVT_SCROLL_LINEUP', 'EVT_SCROLL_LINEDOWN', 'EVT_SCROLL_PAGEUP',
            'EVT_SCROLL_PAGEDOWN', 'EVT_SCROLL_THUMBTRACK', 'EVT_SCROLL_THUMBRELEASE',
            'EVT_SCROLL_CHANGED', 'EVT_SCROLL', 'EVT_SCROLL_TOP',
            'EVT_SCROLL_BOTTOM', 'EVT_SCROLL_LINEUP', 
            'EVT_SCROLL_LINEDOWN', 'EVT_SCROLL_PAGEUP',
            'EVT_SCROLL_PAGEDOWN', 'EVT_SCROLL_THUMBTRACK',
            'EVT_SCROLL_THUMBRELEASE', 'EVT_SCROLL_CHANGED')
component.Manager.setMenu(c, 'control', 'slider', 'wxSlider', 60)
component.Manager.setTool(c, 'Controls', pos=(2,3))

### wxGauge

c = component.Component('wxGauge', ['control','tool'],
              ['pos', 'size', 'range', 'value', 'shadow', 'bezel'],
              image=images.TreeGauge.GetImage())
c.addStyles('wxGA_HORIZONTAL', 'wxGA_VERTICAL', 'wxGA_PROGRESSBAR', 'wxGA_SMOOTH')
c.setParamClass('range', params.ParamIntNN)
c.setParamClass('value', params.ParamIntNN)
c.setParamClass('shadow', params.ParamUnit)
c.setParamClass('bezel', params.ParamUnit)
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'gauge', 'wxGauge', 70)
component.Manager.setTool(c, 'Controls', pos=(1,3))

### wxSpinCtrl

c = component.Component('wxSpinCtrl', ['control','tool'],
              ['pos', 'size', 'value', 'min', 'max'],
              image=images.TreeSpinCtrl.GetImage())
c.addStyles('wxSP_HORIZONTAL', 'wxSP_VERTICAL', 'wxSP_ARROW_KEYS', 'wxSP_WRAP')
c.setParamClass('value', params.ParamInt)
c.addEvents('EVT_SPINCTRL')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'spin ctrl', 'wxSpinCtrl', 80)
component.Manager.setTool(c, 'Controls', pos=(1,2))

### wxScrollBar

c = component.Component('wxScrollBar', ['control'],
              ['pos', 'size', 'value', 'thumbsize', 'range', 'pagesize'],
              image=images.TreeScrollBar.GetImage())
c.addStyles('wxSB_HORIZONTAL', 'wxSB_VERTICAL')
c.setParamClass('range', params.ParamIntNN)
c.setParamClass('value', params.ParamIntNN)
c.setParamClass('thumbsize', params.ParamUnit)
c.setParamClass('pagesize', params.ParamUnit)
c.addEvents('EVT_SCROLL', 'EVT_SCROLL_TOP', 'EVT_SCROLL_BOTTOM',
            'EVT_SCROLL_LINEUP', 'EVT_SCROLL_LINEDOWN', 'EVT_SCROLL_PAGEUP',
            'EVT_SCROLL_PAGEDOWN', 'EVT_SCROLL_THUMBTRACK', 'EVT_SCROLL_THUMBRELEASE',
            'EVT_SCROLL_CHANGED', 'EVT_SCROLL', 'EVT_SCROLL_TOP',
            'EVT_SCROLL_BOTTOM', 'EVT_SCROLL_LINEUP', 
            'EVT_SCROLL_LINEDOWN', 'EVT_SCROLL_PAGEUP',
            'EVT_SCROLL_PAGEDOWN', 'EVT_SCROLL_THUMBTRACK',
            'EVT_SCROLL_THUMBRELEASE', 'EVT_SCROLL_CHANGED')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'scroll bar', 'wxScrollBar', 90)
component.Manager.setTool(c, 'Controls', pos=(3,3))

### wxListCtrl

c = component.Component('wxListCtrl', ['control','tool'], ['pos', 'size'],
                        image=images.TreeListCtrl.GetImage())
c.addStyles('wxLC_LIST', 'wxLC_REPORT', 'wxLC_ICON', 'wxLC_SMALL_ICON',
            'wxLC_ALIGN_TOP', 'wxLC_ALIGN_LEFT', 'wxLC_AUTOARRANGE',
            'wxLC_USER_TEXT', 'wxLC_EDIT_LABELS', 'wxLC_NO_HEADER',
            'wxLC_SINGLE_SEL', 'wxLC_SORT_ASCENDING', 'wxLC_SORT_DESCENDING',
            'wxLC_VIRTUAL', 'wxLC_HRULES', 'wxLC_VRULES', 'wxLC_NO_SORT_HEADER')
c.addEvents('EVT_LIST_BEGIN_DRAG',
            'EVT_LIST_BEGIN_RDRAG', 
            'EVT_LIST_BEGIN_LABEL_EDIT', 
            'EVT_LIST_END_LABEL_EDIT', 
            'EVT_LIST_DELETE_ITEM', 
            'EVT_LIST_DELETE_ALL_ITEMS', 
            'EVT_LIST_ITEM_SELECTED', 
            'EVT_LIST_ITEM_DESELECTED', 
            'EVT_LIST_KEY_DOWN', 
            'EVT_LIST_INSERT_ITEM', 
            'EVT_LIST_COL_CLICK', 
            'EVT_LIST_ITEM_RIGHT_CLICK', 
            'EVT_LIST_ITEM_MIDDLE_CLICK', 
            'EVT_LIST_ITEM_ACTIVATED', 
            'EVT_LIST_CACHE_HINT', 
            'EVT_LIST_COL_RIGHT_CLICK', 
            'EVT_LIST_COL_BEGIN_DRAG', 
            'EVT_LIST_COL_DRAGGING', 
            'EVT_LIST_COL_END_DRAG', 
            'EVT_LIST_ITEM_FOCUSED')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'list ctrl', 'wxListCtrl', 100)
component.Manager.setTool(c, 'Panels', pos=(0,1))

### wxTreeCtrl

c = component.Component('wxTreeCtrl', ['control','tool'], ['pos', 'size'],
                        image=images.TreeTreeCtrl.GetImage())
c.addStyles('wxTR_EDIT_LABELS',
            'wxTR_NO_BUTTONS',
            'wxTR_HAS_BUTTONS',
            'wxTR_TWIST_BUTTONS',
            'wxTR_NO_LINES',
            'wxTR_FULL_ROW_HIGHLIGHT',
            'wxTR_LINES_AT_ROOT',
            'wxTR_HIDE_ROOT',
            'wxTR_ROW_LINES',
            'wxTR_HAS_VARIABLE_ROW_HEIGHT',
            'wxTR_SINGLE',
            'wxTR_MULTIPLE',
            'wxTR_EXTENDED',
            'wxTR_DEFAULT_STYLE')
c.addEvents('EVT_TREE_BEGIN_DRAG', 
            'EVT_TREE_BEGIN_RDRAG', 
            'EVT_TREE_BEGIN_LABEL_EDIT', 
            'EVT_TREE_END_LABEL_EDIT', 
            'EVT_TREE_DELETE_ITEM', 
            'EVT_TREE_GET_INFO', 
            'EVT_TREE_SET_INFO', 
            'EVT_TREE_ITEM_EXPANDED', 
            'EVT_TREE_ITEM_EXPANDING', 
            'EVT_TREE_ITEM_COLLAPSED', 
            'EVT_TREE_ITEM_COLLAPSING', 
            'EVT_TREE_SEL_CHANGED', 
            'EVT_TREE_SEL_CHANGING', 
            'EVT_TREE_KEY_DOWN', 
            'EVT_TREE_ITEM_ACTIVATED', 
            'EVT_TREE_ITEM_RIGHT_CLICK', 
            'EVT_TREE_ITEM_MIDDLE_CLICK', 
            'EVT_TREE_END_DRAG', 
            'EVT_TREE_STATE_IMAGE_CLICK', 
            'EVT_TREE_ITEM_GETTOOLTIP', 
            'EVT_TREE_ITEM_MENU')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'tree ctrl', 'wxTreeCtrl', 110)
component.Manager.setTool(c, 'Panels', pos=(0,2))

### wxHtmlWindow

c = component.Component('wxHtmlWindow', ['control'],
              ['pos', 'size', 'borders', 'url', 'htmlcode'])
c.addStyles('wxHW_SCROLLBAR_NEVER', 'wxHW_SCROLLBAR_AUTO', 'wxHW_NO_SELECTION')
c.setParamClass('url', params.ParamLongText)
c.setParamClass('htmlcode', params.ParamMultilineText)
c.addEvents('EVT_HTML_CELL_CLICKED', 'EVT_HTML_CELL_HOVER',
            'EVT_HTML_LINK_CLICKED')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'HTML window', 'wxHtmlWindow', 120)

### wxCalendarCtrl

c = component.Component('wxCalendarCtrl', ['control', 'tool'], ['pos', 'size'])
c.addStyles('wxCAL_SUNDAY_FIRST', 'wxCAL_MONDAY_FIRST', 'wxCAL_SHOW_HOLIDAYS',
            'wxCAL_NO_YEAR_CHANGE', 'wxCAL_NO_MONTH_CHANGE',
            'wxCAL_SEQUENTIAL_MONTH_SELECTION', 'wxCAL_SHOW_SURROUNDING_WEEKS')
c.addEvents('EVT_CALENDAR_SEL_CHANGED', 'EVT_CALENDAR_DAY_CHANGED',
            'EVT_CALENDAR_MONTH_CHANGED', 'EVT_CALENDAR_YEAR_CHANGED',
            'EVT_CALENDAR_DOUBLECLICKED', 'EVT_CALENDAR_WEEKDAY_CLICKED')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'calendar ctrl', 'wxCalendarCtrl', 130)

### wxGenericDirCtrl

c = component.Component('wxGenericDirCtrl', ['control'],
              ['pos', 'size', 'defaultfolder', 'filter', 'defaultfilter'])
c.addStyles('wxDIRCTRL_DIR_ONLY', 'wxDIRCTRL_3D_INTERNAL', 'wxDIRCTRL_SELECT_FIRST',
            'wxDIRCTRL_SHOW_FILTERS', 'wxDIRCTRL_EDIT_LABELS')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'generic dir ctrl', 'wxGenericDirCtrl', 160)

### wxFilePickerCtrl

c = component.Component('wxFilePickerCtrl', ['control'],
              ['pos', 'size', 'value', 'message', 'wildcard'])
c.addStyles('wxFLP_OPEN', 'wxFLP_SAVE', 'wxFLP_OVERWRITE_PROMPT',
            'wxFLP_FILE_MUST_EXIST', 'wxFLP_CHANGE_DIR',
            'wxFLP_DEFAULT_STYLE')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'file picker ctrl', 'wxFilePickerCtrl', 170)
component.Manager.setTool(c, 'Controls', pos=(4,2))

### wxDatePickerCtrl

c = component.Component('wxDatePickerCtrl', ['control'], ['pos', 'size', 'borders'])
c.addStyles('wxDP_DEFAULT', 'wxDP_SPIN', 'wxDP_DROPDOWN',
            'wxDP_ALLOWNONE', 'wxDP_SHOWCENTURY')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'date picker ctrl', 'wxDateCtrl', 180)

### wxGrid

c = component.Component('wxGrid', ['control'], ['pos', 'size'])
c.addEvents('EVT_GRID_CELL_LEFT_CLICK', 
            'EVT_GRID_CELL_RIGHT_CLICK', 
            'EVT_GRID_CELL_LEFT_DCLICK', 
            'EVT_GRID_CELL_RIGHT_DCLICK', 
            'EVT_GRID_LABEL_LEFT_CLICK', 
            'EVT_GRID_LABEL_RIGHT_CLICK', 
            'EVT_GRID_LABEL_LEFT_DCLICK', 
            'EVT_GRID_LABEL_RIGHT_DCLICK', 
            'EVT_GRID_ROW_SIZE', 
            'EVT_GRID_COL_SIZE', 
            'EVT_GRID_RANGE_SELECT', 
            'EVT_GRID_CELL_CHANGE', 
            'EVT_GRID_SELECT_CELL', 
            'EVT_GRID_EDITOR_SHOWN', 
            'EVT_GRID_EDITOR_HIDDEN', 
            'EVT_GRID_EDITOR_CREATED', 
            'EVT_GRID_CELL_BEGIN_DRAG')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'grid', 'wxGrid', 190)
component.Manager.setTool(c, 'Panels', pos=(2,1), span=(1,2))

### wxHyperlinkCtrl

c = component.Component('wxHyperlinkCtrl', ['control','tool'],
              ['pos', 'size', 'label', 'url'],
              params={'url': params.ParamText},
              defaults={'url': 'http://'})
c.addStyles('wxHL_CONTEXTMENU', 'wxHL_ALIGN_LEFT', 'wxHL_ALIGN_RIGHT',
            'wxHL_ALIGN_CENTRE', 'wxHL_DEFAULT_STYLE')
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'hyperlink', 'wxHyperlinkCtrl', 200)
component.Manager.setTool(c, 'Controls', pos=(3,0))

################################################################################
# Buttons

### wxButton

c = component.Component('wxButton', ['control', 'tool', 'stdbtn'],
                        ['pos', 'size', 'label', 'default'],
                        image=images.TreeButton.GetImage())
c.addStyles('wxBU_LEFT', 'wxBU_TOP', 'wxBU_RIGHT', 'wxBU_BOTTOM', 'wxBU_EXACTFIT',
            'wxNO_BORDER')
c.setParamClass('default', params.ParamBool)
c.addEvents('EVT_BUTTON')
component.Manager.register(c)
component.Manager.setMenu(c, 'button', 'button', 'wxButton', 10)
component.Manager.setTool(c, 'Controls', pos=(0,1))

### wxBitmapButton

c = component.Component('wxBitmapButton', ['control', 'tool'],
              ['pos', 'size', 'default', 
               'bitmap', 'selected', 'focus', 'disabled', 'hover'],
              image=images.TreeBitmapButton.GetImage())
c.addStyles('wxBU_AUTODRAW', 'wxBU_LEFT', 'wxBU_RIGHT', 'wxBU_TOP', 'wxBU_BOTTOM',
            'wxBU_EXACTFIT')
c.setParamClass('default', params.ParamBool)
c.setSpecial('bitmap',  attribute.BitmapAttribute)
c.setSpecial('selected',  attribute.BitmapAttribute)
c.setParamClass('selected', params.ParamBitmap)
c.setSpecial('focus',  attribute.BitmapAttribute)
c.setParamClass('focus', params.ParamBitmap)
c.setSpecial('disabled',  attribute.BitmapAttribute)
c.setParamClass('disabled', params.ParamBitmap)
c.setSpecial('hover',  attribute.BitmapAttribute)
c.setParamClass('hover', params.ParamBitmap)
c.addEvents('EVT_BUTTON')
component.Manager.register(c)
component.Manager.setMenu(c, 'button', 'bitmap button', 'wxBitmapButton', 20)
component.Manager.setTool(c, 'Controls', pos=(1,1))

### wxRadioButton

c = component.Component('wxRadioButton', ['control', 'tool'], 
                        ['pos', 'size', 'label', 'value'],
                        image=images.TreeRadioButton.GetImage())
c.addStyles('wxRB_GROUP', 'wxRB_SINGLE')
c.setParamClass('value', params.ParamBool)
c.addEvents('EVT_RADIOBUTTON')
component.Manager.register(c)
component.Manager.setMenu(c, 'button', 'radio button', 'wxRadioButton', 30)
component.Manager.setTool(c, 'Controls', pos=(3,1))

### wxSpinButton

c = component.Component('wxSpinButton', ['control', 'tool'],
              ['pos', 'size', 'value', 'min', 'max'],
              image=images.TreeSpinButton.GetImage())
c.addStyles('wxSP_HORIZONTAL', 'wxSP_VERTICAL', 'wxSP_ARROW_KEYS', 'wxSP_WRAP')
c.addEvents('EVT_SPIN', 'EVT_SPIN_UP', 'EVT_SPIN_DOWN')
component.Manager.register(c)
component.Manager.setMenu(c, 'button', 'spin button', 'wxSpinButton', 40)
component.Manager.setTool(c, 'Controls', pos=(2,0))

### wxToggleButton

c = component.Component('wxToggleButton', ['control', 'tool'],
              ['pos', 'size', 'label', 'checked'],
              image=images.TreeToggleButton.GetImage())
c.addEvents('EVT_TOGGLEBUTTON')
component.Manager.register(c)
component.Manager.setMenu(c, 'button', 'toggle button', 'wxToggleButton', 50)
component.Manager.setTool(c, 'Controls', pos=(2,1))

################################################################################
# Boxes

### wxCheckBox

c = component.Component('wxCheckBox', ['control','tool'],
                        ['pos', 'size', 'label', 'checked'],
                        image=images.TreeCheckBox.GetImage())
c.addEvents('EVT_CHECKBOX')
component.Manager.register(c)
component.Manager.setMenu(c, 'box', 'check box', 'wxCheckBox', 10)
component.Manager.setTool(c, 'Controls', pos=(4,1))

### wxComboBox

c = component.Component('wxComboBox', ['control','tool'],
              ['pos', 'size', 'content', 'selection', 'value'],
              image=images.TreeComboBox.GetImage())
c.addStyles('wxCB_SINGLE', 'wxCB_DROPDOWN', 'wxCB_READONLY',
            'wxCB_SORT', 'wxTE_PROCESS_ENTER')
c.setSpecial('content',  attribute.ContentAttribute)
c.addEvents('EVT_COMBOBOX', 'EVT_TEXT', 'EVT_TEXT_ENTER')
component.Manager.register(c)
component.Manager.setMenu(c, 'box', 'combo box', 'wxComboBox', 20)
component.Manager.setTool(c, 'Controls', pos=(2,2))

### wxRadioBox

c = component.Component('wxRadioBox', ['control','tool'],
              ['pos', 'size', 'label', 'dimension', 
               'content', 'selection', 'dimension'])
c.addStyles('wxRA_SPECIFY_ROWS', 'wxRA_SPECIFY_COLS')
c.setSpecial('content',  attribute.ContentAttribute)
c.setParamClass('dimension', params.ParamInt)
c.addEvents('EVT_RADIOBOX')
component.Manager.register(c)
component.Manager.setMenu(c, 'box', 'radio box', 'wxRadioBox', 30)
#component.Manager.setTool(c, 'Panels')

### wxListBox

c = component.Component('wxListBox', ['control','tool'],
                        ['pos', 'size', 'content', 'selection'],
                        image=images.TreeListBox.GetImage())
c.addStyles('wxLB_SINGLE', 'wxLB_MULTIPLE', 'wxLB_EXTENDED', 'wxLB_HSCROLL',
            'wxLB_ALWAYS_SB', 'wxLB_NEEDED_SB', 'wxLB_SORT')
c.setSpecial('content',  attribute.ContentAttribute)
c.addEvents('EVT_LISTBOX', 'EVT_LISTBOX_DCLICK')
component.Manager.register(c)
component.Manager.setMenu(c, 'box', 'list box', 'wxListBox', 40)
component.Manager.setTool(c, 'Panels', pos=(0,0))

### wxCheckListBox

c = component.Component('wxCheckListBox', ['control','tool'],
              ['pos', 'size', 'content', 'selection'])
c.addStyles('wxLB_SINGLE', 'wxLB_MULTIPLE', 'wxLB_EXTENDED', 'wxLB_HSCROLL',
            'wxLB_ALWAYS_SB', 'wxLB_NEEDED_SB', 'wxLB_SORT')
c.setSpecial('content',  attribute.CheckContentAttribute)
c.setParamClass('content', params.ParamContentCheckList)
c.addEvents('EVT_CHECKLISTBOX')
component.Manager.register(c)
component.Manager.setMenu(c, 'box', 'check list box', 'wxCheckListBox', 50)
#component.Manager.setTool(c, 'Panels', pos=(0,0))

### wxStaticBox

c = component.Component('wxStaticBox', ['control','tool'],
              ['pos', 'size', 'label'],
              image=images.TreeStaticBox.GetImage())
component.Manager.register(c)
component.Manager.setMenu(c, 'box', 'static box', 'wxStaticBox', 60)
component.Manager.setTool(c, 'Panels', pos=(2,0))

### unknown

c = component.Component('unknown', ['control'], ['pos', 'size'])
component.Manager.register(c)
component.Manager.setMenu(c, 'control', 'unknown', 'unknown control')

### wxXXX

#c = component.Component('wxXXX', ['control','tool'],
#              ['pos', 'size', ...])
#c.addStyles(...)
#component.Manager.register(c)
#component.Manager.setMenu(c, 'control', 'XXX', 'wxXXX', NN)
