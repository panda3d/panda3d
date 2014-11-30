# Name:         wxlib.py
# Purpose:      Component plugins for wx.lib classes
# Author:       Roman Rolinsky <rolinsky@femagsoft.com>
# Created:      05.09.2007
# RCS-ID:       $Id$

import xh_wxlib 
from wx.tools.XRCed import component, params
from wx.tools.XRCed.globals import TRACE
from wx.lib.ticker_xrc import wxTickerXmlHandler

TRACE('*** creating wx.lib components')

# wx.lib.foldpanelbar.FoldPanelBar

c = component.SmartContainer('wx.lib.foldpanelbar.FoldPanelBar', ['book', 'window', 'control'], 
                   ['pos', 'size'],
                   implicit_klass='foldpanel', 
                   implicit_page='FoldPanel', 
                   implicit_attributes=['label', 'collapsed'],
                   implicit_params={'collapsed': params.ParamBool})
c.addStyles('FPB_DEFAULT_STYLE', 'FPB_SINGLE_FOLD', 'FPB_COLLAPSE_TO_BOTTOM',
            'FPB_EXCLUSIVE_FOLD', 'FPB_HORIZONTAL', 'FPB_VERTICAL')
component.Manager.register(c)
component.Manager.addXmlHandler(xh_wxlib.FoldPanelBarXmlHandler)
component.Manager.setMenu(c, 'bar', 'fold panel bar', 'FoldPanelBar', 1000)

# wx.lib.ticker.Ticker

class ParamDirection(params.RadioBox):
    choices = {'right to left': 'rtl', 'left to right': 'ltr'}
    default = 'rtl'
c = component.Component('wxTicker', ['control'], 
              ['pos', 'size', 'start', 'text', 'ppf', 'fps', 'direction'],
              params={'ppf': params.ParamInt, 'fps': params.ParamInt,
                      'direction': ParamDirection})
component.Manager.register(c)
component.Manager.addXmlHandler(wxTickerXmlHandler)
component.Manager.setMenu(c, 'control', 'ticker', 'Ticker', 1000)
