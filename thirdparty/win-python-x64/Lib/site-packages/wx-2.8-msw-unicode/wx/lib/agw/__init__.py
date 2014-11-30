
"""
This is the Advanced Generic Widgets package (AGW). It provides many 
custom-drawn wxPython controls: some of them can be used as a replacement 
of the platform native controls, others are simply an addition to the 
already rich wxPython widgets set.


Description:

AGW contains many different modules, listed below. Items labelled with 
an asterisk were already present in `wx.lib` before:

- AdvancedSplash: reproduces the behaviour of `wx.SplashScreen`, with more
  advanced features like custom shapes and text animations;
- AquaButton: this is another custom-drawn button class which
  *approximatively* mimics the behaviour of Aqua buttons on the Mac;
- AUI: a pure-Python implementation of `wx.aui`, with many bug fixes and
  new features like HUD docking and L{AuiNotebook} tab arts;
- BalloonTip: allows you to display tooltips in a balloon style window
  (actually a frame), similarly to the Windows XP balloon help;
- ButtonPanel (*): a panel with gradient background shading with the
  possibility to add buttons and controls still respecting the gradient
  background;
- CubeColourDialog: an alternative implementation of `wx.ColourDialog`, it
  offers different functionalities like colour wheel and RGB cube;
- CustomTreeCtrl (*): mimics the behaviour of `wx.TreeCtrl`, with almost the
  same base functionalities plus a bunch of enhancements and goodies;
- FlatMenu: as the name implies, it is a generic menu implementation,
  offering the same `wx.MenuBar`/`wx.Menu`/`wx.ToolBar` capabilities and much more;
- FlatNotebook (*): a full implementation of the `wx.Notebook`, and designed
  to be a drop-in replacement for `wx.Notebook` with enhanced capabilities;
- FloatSpin: this class implements a floating point spinctrl, cabable (in
  theory) of handling infinite-precision floating point numbers;
- FoldPanelBar (*): a control that contains multiple panels that can be
  expanded or collapsed a la Windows Explorer/Outlook command bars;
- FourWaySplitter: this is a layout manager which manages four children like
  four panes in a window, similar to many CAD software interfaces;
- GenericMessageDialog: it is a possible replacement for the standard
  `wx.MessageDialog`, with a fancier look and extended functionalities;
- GradientButton: another custom-drawn button class which mimics Windows CE
  mobile gradient buttons, using a tri-vertex blended gradient background;
- HyperLinkCtrl (*): this widget acts line an hyper link in a typical browser;
- HyperTreeList: a class that mimics the behaviour of `wx.gizmos.TreeListCtrl`,
  with almost the same base functionalities plus some more enhancements;
- KnobCtrl: a widget which lets the user select a numerical value by
  rotating it, like a slider with a wheel shape;
- LabelBook and FlatImageBook: these are a quasi-full implementations of
  `wx.ListBook`, with additional features;
- MultiDirDialog: it represents a possible replacement for `wx.DirDialog`,
  with the additional ability of selecting multiple folders at once and a
  fancier look;
- PeakMeter: this widget mimics the behaviour of LED equalizers that are
  usually found in stereos and MP3 players;
- PersistentControls: widgets which automatically save their state
  when they are destroyed and restore it when they are recreated, even during
  another program invocation;
- PieCtrl and ProgressPie: these are simple classes that reproduce the
  behavior of a pie chart, in a static or progress-gauge-like way;
- PyBusyInfo: constructs a busy info window and displays a message in it:
  it is similar to `wx.BusyInfo`;
- PyCollapsiblePane: a pure Python implementation of the original wxWidgets
  C++ code of `wx.CollapsiblePane`, with customizable buttons;
- PyGauge: a generic `wx.Gauge` implementation, it supports the determinate
  mode functions as `wx.Gauge`;
- PyProgress: it is similar to `wx.ProgressDialog` in indeterminated mode, but
  with a different gauge appearance and a different spinning behavior;
- RibbonBar: the RibbonBar library is a set of classes for writing a ribbon
  user interface, similar to the user interface present in recent versions
  of Microsoft Office;
- RulerCtrl: it implements a ruler window that can be placed on top, bottom,
  left or right to any wxPython widget. It is somewhat similar to the rulers
  you can find in text editors software;
- ShapedButton: this class tries to fill the lack of "custom shaped" controls
  in wxPython. It can be used to build round buttons or elliptic buttons;
- SpeedMeter: this widget tries to reproduce the behavior of some car
  controls (but not only), by creating an "angular" control;
- SuperToolTip: a class that mimics the behaviour of `wx.TipWindow` and
  generic tooltips, with many features and highly customizable;
- ThumbnailCtrl: a widget that can be used to display a series of images
  in a "thumbnail" format; it mimics, for example, the Windows Explorer
  behavior when you select the "view thumbnails" option;
- ToasterBox: a cross-platform widget to make the creation of MSN-style
  "toaster" popups easier;
- UltimateListCtrl: mimics the behaviour of `wx.ListCtrl`, with almost the same
  base functionalities plus some more enhancements;
- ZoomBar: a class that *appoximatively* mimics the behaviour of the Mac Dock,
  inside a `wx.Panel`.


Bugs and Limitations: many, patches and fixes welcome :-D

See the demos for an example of what AGW can do, and on how to use it.

Copyright: Andrea Gavana

License: Same as the version of wxPython you are using it with.

SVN for latest code:
http://svn.wxwidgets.org/viewvc/wx/wxPython/3rdParty/AGW/

Mailing List:
wxpython-users@lists.wxwidgets.org

My personal web page:
http://xoomer.alice.it/infinity77

Please let me know if you are using AGW!

You can contact me at:

andrea.gavana@gmail.com
gavana@kpo.kz

AGW version: 0.9.1

Last updated: 21 Jun 2011, 22.00 GMT

"""

__version__ = "0.9.1"
__author__ = "Andrea Gavana <andrea.gavana@gmail.com>"
