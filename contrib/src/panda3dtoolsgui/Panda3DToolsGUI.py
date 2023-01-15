import wx
import wx.html
from wx.lib.filebrowsebutton import FileBrowseButton
import wx.lib.agw.pycollapsiblepane as PCP
import xml.dom.minidom
from xml.dom.minidom import Node, Document, Element
import time
import sys
import os
import subprocess

ID_CHOOSEPANDA = wx.NewId()
ID_CLEAROUTPUT = wx.NewId()
ID_LOADPREFS = wx.NewId()
ID_RUNBATCH = wx.NewId()
ID_SAVEPREFS = wx.NewId()
ID_ADDTOBATCH = wx.NewId()
ID_EDITSELBATCH = wx.NewId()
ID_REMOVESELBATCH = wx.NewId()
ID_REMOVEALLBATCH = wx.NewId()
ID_SAVEBATCH = wx.NewId()
ID_LOADBATCH = wx.NewId()
ID_SORTBATCH = wx.NewId()
ID_HELP = wx.NewId()
ID_ABOUT = wx.NewId()
ID_EXIT = wx.NewId()
ID_BAMADDTOBATCH = wx.NewId()
ID_OPTCHOOSEOUT = wx.NewId()
ID_OPTCHOOSEEGG = wx.NewId()
ID_SIMPLEEGGSAVE = wx.NewId()
ID_SIMPLEMBPICK = wx.NewId()
ID_RUNSIMPLE = wx.NewId()
ID_RUNSIMPLEEXPORT = wx.NewId()
ID_TXA = wx.NewId()
ID_INEGG = wx.NewId()
ID_OUTTEX = wx.NewId()
ID_OUTEGG = wx.NewId()
ID_PALETTIZEADDTOBATCH = wx.NewId()
ID_MAYAADDTOBATCH = wx.NewId()

#all the information used by selection dropbox
MAYA_VERSIONS = ['MAYA VERSION',
                 '6',
                 '65',
                 '7',
                 '8',
                 '85',
                 '2008',
                 '2009',
                 '2010',
                 '2012']

TOOLS = ['maya2egg',
         'egg2bam',
         'egg-rename',
         'egg-optchar',
         'egg-palettize']

DEFAULT_PANDA_DIR = 'D:\cvsroot\SourceCode\simBuilt\win'

UNIT_TYPES = ["mm",
              "cm",
              "m",
              "in",
              "ft",
              "yd"]


WELCOME_MSG ="""Panda3D Tools GUI version 0.1
April, 20th 2010
by Andrew Gartner,Shuying Feng and the PandaSE team (Spring '10)
Entertainment Technology Center
Carnegie Mellon University

September, 13th 2011
Przemyslaw Iwanowski
Walt Disney Imagineering - Creative Technology Group

Please send any feedback to:
przemyslaw.iwanowski@disney.com
andrewgartner@gmail.com
agartner@andrew.cmu.edu
shuyingf@andrew.cmu.edu


KNOWN ISSUES:
-Progress bar does not initialize/update well with maya2egg batch process
-maya2egg occasionally hangs during batch and therefore hangs the app due
using popen.poll() to retrieve output
-egg-palettize tool is in test
"""

#best layout size for Windows. Needs testing on Linux
WINDOW_WIDTH = 920
WINDOW_HEIGHT = 1000

#custom dialog that is shown for simple mode
class OutputDialog(wx.Dialog):
    def __init__(self, *args, **kwds):
        # begin wxGlade: OutputDialog.__init__
        kwds["style"] = wx.DEFAULT_DIALOG_STYLE
        wx.Dialog.__init__(self, *args, **kwds)
        self.dlg_main_panel = wx.Panel(self, -1)
        self.dlgOutText = wx.TextCtrl(self.dlg_main_panel, -1, "", style=wx.TE_MULTILINE|wx.NO_BORDER)
        self.dlgClearOutBTN = wx.Button(self.dlg_main_panel, -1, "Clear")
        self.dlgCloseBTN = wx.Button(self.dlg_main_panel, -1, "Close")
        self.dlg_static_sizer_staticbox = wx.StaticBox(self, -1, "Output")

        self.__set_properties()
        self.__do_layout()

        self.Bind(wx.EVT_BUTTON, self.OnClearDlgOut, self.dlgClearOutBTN)
        self.Bind(wx.EVT_BUTTON, self.OnOutDlgClose, self.dlgCloseBTN)
        # end wxGlade

    def __set_properties(self):
        # begin wxGlade: OutputDialog.__set_properties
        self.SetTitle("Exporter Output")
        self.SetSize((530, 405))
        self.dlgOutText.SetSize((500, 300))
        # end wxGlade

    def __do_layout(self):
        # begin wxGlade: OutputDialog.__do_layout
        self.dlg_static_sizer_staticbox.Lower()
        dlg_static_sizer = wx.StaticBoxSizer(self.dlg_static_sizer_staticbox, wx.HORIZONTAL)
        dlg_main_flex_sizer = wx.FlexGridSizer(3, 1, 0, 0)
        dlg_button_sizer = wx.GridSizer(1, 2, 0, 0)
        dlg_main_flex_sizer.Add(self.dlgOutText, 0, wx.ALL|wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL, 2)
        dlg_button_sizer.Add(self.dlgClearOutBTN, 0, wx.EXPAND|wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL, 0)
        dlg_button_sizer.Add(self.dlgCloseBTN, 0, wx.EXPAND|wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL, 0)
        dlg_main_flex_sizer.Add(dlg_button_sizer, 1, wx.EXPAND, 0)
        self.dlg_main_panel.SetSizer(dlg_main_flex_sizer)
        dlg_static_sizer.Add(self.dlg_main_panel, 1, wx.EXPAND, 0)
        self.SetSizer(dlg_static_sizer)
        self.Layout()
        self.Centre()
        # end wxGlade

    def OnClearDlgOut(self, event): # wxGlade: OutputDialog.<event_handler>,use for clear output
        self.dlgOutText.Clear()
        event.Skip()

    def OnOutDlgClose(self, event): # wxGlade: OutputDialog.<event_handler>,use for close dialog
        self.Close()
        event.Skip()
#End dialog output class

#custom dialog that is shown for pview
class OutputDialogpview(wx.Dialog):
    def __init__(self, main, *args, **kwds):
        # begin wxGlade: OutputDialog.__init__
        kwds["style"] = wx.DEFAULT_DIALOG_STYLE
        wx.Dialog.__init__(self, main, *args, **kwds)
        self.p = wx.Panel(self)

        #create the controls
        self.pview_modelFile = FileBrowseButton(self.p,labelText="Model File", buttonText="Choose..")
        self.pview_animFile = FileBrowseButton(self.p,labelText="Animation File", buttonText="Choose..")
        self.pview_run = wx.Button(self.p, -1, "Pview")

        self.__set_properties()
        self.__do_layout()

        self.Bind(wx.EVT_BUTTON, self.RunPview, self.pview_run)
        # end wxGlade

    def __set_properties(self):
        # begin wxGlade: OutputDialog.__set_properties
        self.SetTitle("Pview")
        self.SetSize((400, 100))
        self.pview_modelFile.SetMinSize((350, 23))
        self.pview_animFile.SetMinSize((400, 23))
        # end wxGlade

    def __do_layout(self):
        # begin wxGlade: OutputDialog.__do_layout
        pview_main_flex_sizer = wx.FlexGridSizer(4, 1, 0, 0)
        pview_main_flex_sizer.Add(self.pview_modelFile, 1, wx.EXPAND|wx.ALIGN_CENTER_VERTICAL)
        pview_main_flex_sizer.Add(self.pview_animFile, 1, wx.EXPAND|wx.ALIGN_CENTER_VERTICAL)
        pview_main_flex_sizer.Add(self.pview_run, 0, wx.ALIGN_RIGHT|wx.RIGHT, 3)
        self.SetSizer(pview_main_flex_sizer)
        self.Layout()
        self.Centre()
        # end wxGlade

    def RunPview(self,e):#pview function
        filename = self.pview_modelFile.GetValue()
        anim_filename = self.pview_animFile.GetValue()
        args = {}
        args['filename'] = str(filename)
        args['animfilename'] = str(anim_filename)

        if sys.platform == "win32":
            extension = ".exe"
        elif sys.platform == "darwin": #OSX
            extension = ""
        else: #Linux and UNIX
            extension = ""

        command = "pview" + extension +  ' ' +  '"' +args['filename'] + '"'+  ' ' +  '"' + args['animfilename'] + '"'

        try:
            p = subprocess.Popen(command, shell = True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines = True )
        except:
            dlg = wx.MessageDialog(self,"Failed To Find Or run the Exporter Application" ,"ERROR", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
#End dialog outputpview class

#custom dialog that is shown for pview
class OutputDialogPaths(wx.Dialog):
    def __init__(self, main, *args, **kwds):
        # begin wxGlade: OutputDialog.__init__
        kwds["style"] = wx.DEFAULT_DIALOG_STYLE
        wx.Dialog.__init__(self, main, *args, **kwds)
        self.p = wx.Panel(self)
        self.callback = None

        #create the controls
        self.paths_inputLbl = wx.StaticText(self.p, -1, "Input")
        self.paths_inputTxt = wx.TextCtrl(self.p,-1,'')
        self.paths_outputLbl = wx.StaticText(self.p, -1, "Output")
        self.paths_outputTxt = wx.TextCtrl(self.p, -1, '')
        self.paths_done = wx.Button(self.p, -1, "Done")

        self.__set_properties()
        self.__do_layout()

        self.Bind(wx.EVT_BUTTON, self.RunPaths, self.paths_done)
        # end wxGlade

    def setCallback(self, callback):
        self.callback = callback

    def __set_properties(self):
        # begin wxGlade: OutputDialog.__set_properties
        self.SetTitle("Change Paths")
        self.SetSize((400, 100))
        self.paths_inputTxt.SetMinSize((350, 21))
        self.paths_outputTxt.SetMinSize((350, 21))
        # end wxGlade

    def __do_layout(self):
        # begin wxGlade: OutputDialog.__do_layout
        paths_sizer = wx.BoxSizer(wx.VERTICAL)
        paths_main_flex_sizer = wx.FlexGridSizer(4, 2, 0, 0)
        paths_main_flex_sizer.Add(self.paths_inputLbl, 1, wx.TOP|wx.LEFT, 3)
        paths_main_flex_sizer.Add(self.paths_inputTxt, 1, wx.ALL, 1)
        paths_main_flex_sizer.Add(self.paths_outputLbl, 1, wx.TOP|wx.LEFT, 3)
        paths_main_flex_sizer.Add(self.paths_outputTxt, 1, wx.ALL, 1)
        paths_sizer.Add(paths_main_flex_sizer)
        paths_sizer.Add(self.paths_done, 0, wx.RIGHT|wx.ALIGN_RIGHT, 4)
        self.SetSizer(paths_sizer)
        self.Layout()
        self.Centre()
        # end wxGlade

    def RunPaths(self,e):#pview function
        self.callback(self.paths_inputTxt.GetValue(), self.paths_outputTxt.GetValue())
        self.Destroy()


class main(wx.Frame):
    def __init__(self, *args, **kwds):
        # begin wxGlade: main.__init__
        kwds["style"] = wx.DEFAULT_FRAME_STYLE
        wx.Frame.__init__(self, *args, **kwds)

        self.SetIcon(wx.Icon("pandaIcon.ico", wx.BITMAP_TYPE_ICO))

        self.pandaPathDir = ''

        #self.batchList is the global list of batch items waiting to be run
        self.batchList = []
        self.optchar_eggList = []
        self.palettize_eggList = []
        self.rename_eggList = []

        #store folders of the last import/export locationfor ease of navigation
        self.srcProjectFolder = ''
        self.destProjectFolder = ''

        self.txaExtraLines = [] #stores txa file

        self._setupUI()
        #Show the welcome message and the initial panda path
        #NOTE this shoudl eventually check for an install of Panda somewhere

        self.ShowInitialEnv()

    def _setupUI(self):
        #create all the elements

        # Menu Bar
        self.menuBar = wx.MenuBar()

        wxglade_tmp_menu = wx.Menu()
        self.prefsLoadButton = wx.MenuItem(wxglade_tmp_menu, ID_LOADPREFS, "Load Preferences", "", wx.ITEM_NORMAL)
        wxglade_tmp_menu.AppendItem(self.prefsLoadButton)
        self.savePrefsButton = wx.MenuItem(wxglade_tmp_menu, ID_SAVEPREFS, "Save Preferences", "", wx.ITEM_NORMAL)
        wxglade_tmp_menu.AppendItem(self.savePrefsButton)
        wxglade_tmp_menu.AppendSeparator()
        self.exitButton = wx.MenuItem(wxglade_tmp_menu, ID_EXIT, "Exit", "", wx.ITEM_NORMAL)
        wxglade_tmp_menu.AppendItem(self.exitButton)
        self.menuBar.Append(wxglade_tmp_menu, "File")

        wxglade_tmp_menu = wx.Menu()
        self.loadBatchMenuButton = wx.MenuItem(wxglade_tmp_menu, ID_LOADBATCH, "Load Batch ", "", wx.ITEM_NORMAL)
        wxglade_tmp_menu.AppendItem(self.loadBatchMenuButton)
        self.saveBatchMenuButton = wx.MenuItem(wxglade_tmp_menu, ID_SAVEBATCH, "Save Batch", "", wx.ITEM_NORMAL)
        wxglade_tmp_menu.AppendItem(self.saveBatchMenuButton)
        self.menuBar.Append(wxglade_tmp_menu, "Batch")
        self.SetMenuBar(self.menuBar)

        wxglade_tmp_menu = wx.Menu()
        self.HelpMenuButton = wx.MenuItem(wxglade_tmp_menu, ID_HELP, "Help ", "", wx.ITEM_NORMAL)
        wxglade_tmp_menu.AppendItem(self.HelpMenuButton)
        self.AboutMenuButton = wx.MenuItem(wxglade_tmp_menu, ID_ABOUT, "About", "", wx.ITEM_NORMAL)
        wxglade_tmp_menu.AppendItem(self.AboutMenuButton)
        self.menuBar.Append(wxglade_tmp_menu, "Help")
        self.SetMenuBar(self.menuBar)
        # Menu Bar end

        self.statusBar = self.CreateStatusBar(1, 0)

        self.tab_panel = wx.Notebook(self, 1)

        self.outdlg = OutputDialog(self)

        #Define simple interface elements
        self.simple_panel = wx.Panel(self.tab_panel, -1)
        self.simple_options_panel = wx.Panel(self.simple_panel, -1)
        self.simple_options_sizer_staticbox = wx.StaticBox(self.simple_options_panel, -1, "Maya2Egg")
        self.simple_mayaFileLbl = wx.StaticText(self.simple_options_panel, -1, "Maya Scene File")
        self.simple_mayaFileTxt = wx.TextCtrl(self.simple_options_panel,-1,'')
        self.simple_mayaFileBtn = wx.Button(self.simple_options_panel, ID_SIMPLEMBPICK, 'Choose..')
        self.simple_exportDestLbl = wx.StaticText(self.simple_options_panel, -1, "Export Destination")
        self.simple_exportDestTxt = wx.TextCtrl(self.simple_options_panel, -1, '')
        self.simple_exportDestBtn = wx.Button(self.simple_options_panel, ID_SIMPLEEGGSAVE, 'Choose..')
        self.simple_mayaVerLbl = wx.StaticText(self.simple_options_panel, -1, "Maya Version", style=wx.ALIGN_CENTRE)
        self.simple_mayaVerComboBox = wx.ComboBox(self.simple_options_panel, -1, choices=MAYA_VERSIONS, style=wx.CB_DROPDOWN|wx.CB_READONLY)
        self.simple_animOptChoice = wx.RadioBox(self.simple_options_panel, -1, "animation-mode", choices=["none", "model", "chan", "both"], majorDimension=4, style=wx.RA_SPECIFY_COLS)
        self.simple_runExportBtn = wx.Button(self.simple_options_panel, ID_RUNSIMPLEEXPORT, "Run Export")

        #define advanced interface elements
        self.main_panel = wx.Panel(self.tab_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)

        self.toolComboBox = wx.ComboBox(self.main_panel, -1, choices=TOOLS, style=wx.CB_DROPDOWN|wx.CB_READONLY)

        # tool options
        self.tool_options_panel = wx.Panel(self.main_panel, -1)
        self.tool_options_sizer_staticbox = wx.StaticBox(self.tool_options_panel, -1, "Tool Options")

        # batch list
        self.batch_panel = wx.Panel(self.main_panel, -1)
        self.batch_static_sizer_staticbox = wx.StaticBox(self.batch_panel, -1, "Batch List")

        self.loadBatchButton = wx.Button(self.batch_panel, ID_LOADBATCH, "Load Batch")
        self.saveBatchButton = wx.Button(self.batch_panel, ID_SAVEBATCH, "Save Batch")
        self.sortBatchButton = wx.Button(self.batch_panel, ID_SORTBATCH, "Sort Batch")
        self.changePathsButton = wx.Button(self.batch_panel, ID_SORTBATCH, "Change Paths")
        self.editSelBatchButton = wx.Button(self.batch_panel, ID_EDITSELBATCH, "Edit Selected")
        self.removeSelBatchButton = wx.Button(self.batch_panel, ID_REMOVESELBATCH, "Remove Selected")
        self.removeAllBatchButton = wx.Button(self.batch_panel, ID_REMOVEALLBATCH, "Remove All")

        self.batchTree = wx.TreeCtrl(self.batch_panel, -1, style=wx.TR_HAS_BUTTONS|wx.TR_LINES_AT_ROOT|wx.TR_DEFAULT_STYLE|wx.SUNKEN_BORDER|wx.TR_MULTIPLE)
        self.treeRoot = self.batchTree.AddRoot('Batch Files')

        # console
        self.console_panel = wx.Panel(self.main_panel, -1)
        self.consoleOutputTxt = wx.TextCtrl(self.console_panel, -1, "", style=wx.TE_MULTILINE|wx.TE_READONLY|wx.TE_WORDWRAP)
        self.console_static_sizer_staticbox = wx.StaticBox(self.console_panel, -1, "Console Output")
        self.runBatchButton = wx.Button(self.console_panel, ID_RUNBATCH, "Run Batch")
        self.clearConsoleButton = wx.Button(self.console_panel, ID_CLEAROUTPUT, "Clear Output")
        self.runPviewButton = wx.Button(self.console_panel,-1,"Load pview")

        self.pathLbl = wx.StaticText(self.console_panel, -1, "Panda Directory", style=wx.ALIGN_CENTRE)
        self.pandaPathTxt = wx.TextCtrl(self.console_panel, -1, "", style=wx.TE_READONLY)
        self.loadPandaPathBtn = wx.Button(self.console_panel, ID_CHOOSEPANDA, "Choose..")
        self.ignoreModDates = wx.CheckBox(self.console_panel, -1, "Override export changed maya scene files")


        # Maya2Egg Tool
        self.maya2egg_panel = wx.Panel(self.tool_options_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.m2e_mayaVerLbl = wx.StaticText(self.maya2egg_panel, -1, "Maya Version", style=wx.ALIGN_CENTRE)
        self.m2e_mayaVerComboBox = wx.ComboBox(self.maya2egg_panel, -1, choices=MAYA_VERSIONS, style=wx.CB_DROPDOWN|wx.CB_READONLY)
        self.m2e_mayaFileLbl = wx.StaticText(self.maya2egg_panel, -1, "Maya Scene File")
        self.m2e_mayaFileTxt = wx.TextCtrl(self.maya2egg_panel, -1, "")
        self.m2e_mayaFileBtn = wx.Button(self.maya2egg_panel, -1, "Choose..")
        self.m2e_exportDestLbl = wx.StaticText(self.maya2egg_panel, -1, "Export Destination")
        self.m2e_exportDestTxt = wx.TextCtrl(self.maya2egg_panel, -1, "")
        self.m2e_exportDestBtn = wx.Button(self.maya2egg_panel, -1, "Choose..")

        self.m2e_options_panel = wx.Panel(self.maya2egg_panel, -1)
        self.m2e_options_panel_sizer_staticbox = wx.StaticBox(self.m2e_options_panel, 1, "General Options")

        self.m2e_mayaUnitsLbl = wx.StaticText(self.m2e_options_panel, -1, "Maya Units (Input)")
        self.m2e_mayaUnitsComboBox = wx.ComboBox(self.m2e_options_panel, -1, choices=UNIT_TYPES, style=wx.CB_DROPDOWN|wx.CB_READONLY)
        self.m2e_pandaUnitsLbl = wx.StaticText(self.m2e_options_panel, -1, "--->   Panda Units (Output)")
        self.m2e_pandaUnitsComboBox = wx.ComboBox(self.m2e_options_panel, -1, choices=UNIT_TYPES, style=wx.CB_DROPDOWN|wx.CB_READONLY)
        self.m2e_backfaceChk = wx.CheckBox(self.m2e_options_panel , -1, "Back Face Rendering")
        self.m2e_polygonOutputChk = wx.CheckBox(self.m2e_options_panel , -1, "Polygon Output Only")
        self.m2e_tbnallChk = wx.CheckBox(self.m2e_options_panel, -1, "Calculate Tangent and Binormal")
        self.m2e_subrootsChk = wx.CheckBox(self.m2e_options_panel, -1, "Export Specified Subroots")
        self.m2e_subrootsTxt = wx.TextCtrl(self.m2e_options_panel, -1, "")
        self.m2e_subsetsChk = wx.CheckBox(self.m2e_options_panel, -1,  "Export Specified Subsets")
        self.m2e_subsetsTxt = wx.TextCtrl(self.m2e_options_panel, -1, "")
        self.m2e_excludesChk = wx.CheckBox(self.m2e_options_panel, -1, "Exclude Specified Subsets")
        self.m2e_excludesTxt = wx.TextCtrl(self.m2e_options_panel, -1, "")

        self.m2e_anim_options_panel = wx.Panel(self.maya2egg_panel, -1)
        self.m2e_anim_options_sizer_staticbox = wx.StaticBox(self.m2e_anim_options_panel, 1, "Animation Options")
        self.m2e_animOptChoice = wx.RadioBox(self.m2e_anim_options_panel, -1, "mode", choices=["none", "model", "chan", "both", "pose"], majorDimension=5, style=wx.RA_SPECIFY_COLS)
        self.m2e_startFrameChk = wx.CheckBox(self.m2e_anim_options_panel, -1, "Start Frame")
        self.m2e_startFrameSpin = wx.SpinCtrl(self.m2e_anim_options_panel, -1, "", min=-10000, max=10000)
        self.m2e_endFrameChk = wx.CheckBox(self.m2e_anim_options_panel, -1, "End Frame")
        self.m2e_endFrameSpin = wx.SpinCtrl(self.m2e_anim_options_panel, -1, "", min=-10000, max=10000)
        self.m2e_frameRateInChk = wx.CheckBox(self.m2e_anim_options_panel, -1, "Frame Rate Input (+/-)")
        self.m2e_frameRateInSpin = wx.SpinCtrl(self.m2e_anim_options_panel, -1, "", min=-10000, max=10000)
        self.m2e_frameRateOutChk = wx.CheckBox(self.m2e_anim_options_panel, -1, "Frame Rate Output (+/-)  ")
        self.m2e_frameRateOutSpin = wx.SpinCtrl(self.m2e_anim_options_panel, -1, "", min=-10000, max=10000)
        self.m2e_charNameChk = wx.CheckBox(self.m2e_anim_options_panel, -1, "Character Name")
        self.m2e_charNameTxt = wx.TextCtrl(self.m2e_anim_options_panel, -1, "")

        self.m2e_tex_options_panel = wx.Panel(self.maya2egg_panel, -1)
        self.m2e_tex_options_sizer_staticbox = wx.StaticBox(self.m2e_tex_options_panel, 1, "Texture/Shader Options")
        self.m2e_legacyShaderChk = wx.CheckBox(self.m2e_tex_options_panel, -1, "Use Legacy Shader Generation")
        self.m2e_copyTexChk = wx.CheckBox(self.m2e_tex_options_panel, -1, "Copy Textures")
        self.m2e_copyTexPathTxt = wx.TextCtrl(self.m2e_tex_options_panel, -1, "")
        self.m2e_copyTexPathBtn = wx.Button(self.m2e_tex_options_panel, -1, "Choose..")

        self.m2e_pathReplaceChk = wx.CheckBox(self.m2e_tex_options_panel, -1, "Path Replace")
        self.m2e_pathReplaceTxt = wx.TextCtrl(self.m2e_tex_options_panel, -1, "")
        self.m2e_pathReplaceBtn = wx.Button(self.m2e_tex_options_panel, -1, "Batch Files")

        self.m2e_addEgg2BamChk = wx.CheckBox(self.maya2egg_panel, -1, "Add with Egg2Bam")
        self.m2e_addEgg2BamChk.Hide()

        # batch item details
        self.batchItemNameLbl = wx.StaticText(self.main_panel, -1, "Batch Item Name")
        self.batchItemNameTxt = wx.TextCtrl(self.main_panel, -1, "")
        self.addToBatchBtn = wx.Button(self.main_panel, ID_MAYAADDTOBATCH, "Add To Batch >>")


        # Egg2Bam Tool
        self.egg2bam_panel = wx.Panel(self.tool_options_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.e2b_eggFileLbl = wx.StaticText(self.egg2bam_panel, -1, "Egg File")
        self.e2b_eggFileTxt = wx.TextCtrl(self.egg2bam_panel, -1, "")
        self.e2b_eggFileBtn = wx.Button(self.egg2bam_panel, -1, "Choose..")
        self.e2b_exportDestLbl = wx.StaticText(self.egg2bam_panel, -1, "Export Destination")
        self.e2b_exportDestTxt = wx.TextCtrl(self.egg2bam_panel, -1, "")
        self.e2b_exportDestBtn = wx.Button(self.egg2bam_panel, -1, "Choose..")

        self.e2b_bamBatchOutputLbl = wx.StaticText(self.egg2bam_panel, -1, "Bam Output Files from the Current Batch")
        self.e2b_bamBatchOutputBtn = wx.Button(self.egg2bam_panel, -1, "Bam Batch Output")

        self.e2b_options_panel = wx.Panel(self.egg2bam_panel, -1)
        self.e2b_options_sizer_staticbox = wx.StaticBox(self.e2b_options_panel, 1, "General Options")
        self.e2b_flattenChk = wx.CheckBox(self.e2b_options_panel, -1, "Flatten")
        self.e2b_embedTexChk = wx.CheckBox(self.e2b_options_panel, -1, "Embed Textures")
        self.e2b_useCurrEggChk = wx.CheckBox(self.e2b_options_panel, -1, "Use current egg file")


        # Egg-rename Tool
        self.eggRename_panel = wx.Panel(self.tool_options_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.rename_eggFilesLbl = wx.StaticText(self.eggRename_panel, -1, "Egg Files")
        self.rename_eggFilesTree = wx.TreeCtrl(self.eggRename_panel, -1, style=wx.TR_HAS_BUTTONS|wx.TR_LINES_AT_ROOT|wx.TR_DEFAULT_STYLE|wx.SUNKEN_BORDER)
        self.rename_eggFilesRoot = self.rename_eggFilesTree.AddRoot('Egg Files')
        self.rename_addEggBtn = wx.Button(self.eggRename_panel, -1, "Add")
        self.rename_addFromBatchBtn = wx.Button(self.eggRename_panel, -1, "Batch Files")
        self.rename_removeEggBtn = wx.Button(self.eggRename_panel, -1, "Remove")
        self.rename_removeAllEggsBtn = wx.Button(self.eggRename_panel, -1, "Remove All")
        self.rename_exportDestLbl = wx.StaticText(self.eggRename_panel, -1, "Export Destination")
        self.rename_exportInPlaceChk = wx.CheckBox(self.eggRename_panel, -1, "In Place")
        self.rename_exportDirLbl = wx.StaticText(self.eggRename_panel, -1, "Directory")
        self.rename_exportDirTxt = wx.TextCtrl(self.eggRename_panel, -1, "")
        self.rename_exportDirBtn = wx.Button(self.eggRename_panel, -1, "Choose..")
        self.rename_exportFileLbl = wx.StaticText(self.eggRename_panel, -1, "File")
        self.rename_exportFileTxt = wx.TextCtrl(self.eggRename_panel, -1, "")
        self.rename_exportFileBtn = wx.Button(self.eggRename_panel, -1, "Choose..")

        self.rename_options_panel = wx.Panel(self.eggRename_panel, -1)
        self.rename_options_sizer_staticbox = wx.StaticBox(self.rename_options_panel, 1, "General Options")
        self.rename_stripPrefixChk = wx.CheckBox(self.rename_options_panel, -1, "Strip Specified Prefix")
        self.rename_stripPrefixTxt = wx.TextCtrl(self.rename_options_panel, -1, "")

        # Egg-optchar Tool
        self.eggOptChar_panel = wx.Panel(self.tool_options_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.optchar_eggFilesLbl = wx.StaticText(self.eggOptChar_panel, -1, "Egg Files")
        self.optchar_eggFilesTree = wx.TreeCtrl(self.eggOptChar_panel, -1, style=wx.TR_HAS_BUTTONS|wx.TR_LINES_AT_ROOT|wx.TR_DEFAULT_STYLE|wx.SUNKEN_BORDER)
        self.optchar_eggFilesRoot = self.optchar_eggFilesTree.AddRoot('Egg Files')
        self.optchar_addEggBtn = wx.Button(self.eggOptChar_panel, -1, "Add")
        self.optchar_addFromBatchBtn = wx.Button(self.eggOptChar_panel, -1, "Batch Files")
        self.optchar_removeEggBtn = wx.Button(self.eggOptChar_panel, -1, "Remove")
        self.optchar_removeAllEggsBtn = wx.Button(self.eggOptChar_panel, -1, "Remove All")
        self.optchar_exportDestLbl = wx.StaticText(self.eggOptChar_panel, -1, "Export Destination")
        self.optchar_exportInPlaceChk = wx.CheckBox(self.eggOptChar_panel, -1, "In Place")
        self.optchar_exportDirLbl = wx.StaticText(self.eggOptChar_panel, -1, "Directory")
        self.optchar_exportDirTxt = wx.TextCtrl(self.eggOptChar_panel, -1, "")
        self.optchar_exportDirBtn = wx.Button(self.eggOptChar_panel, -1, "Choose..")
        self.optchar_exportFileLbl = wx.StaticText(self.eggOptChar_panel, -1, "File")
        self.optchar_exportFileTxt = wx.TextCtrl(self.eggOptChar_panel, -1, "")
        self.optchar_exportFileBtn = wx.Button(self.eggOptChar_panel, -1, "Choose..")

        self.optchar_options_panel = wx.Panel(self.eggOptChar_panel, -1)
        self.optchar_options_sizer_staticbox = wx.StaticBox(self.optchar_options_panel, 1, "General Options")
        self.optchar_keepAllJointsChk = wx.CheckBox(self.optchar_options_panel, -1, "Keep All Joints")
        self.optchar_keepJointsChk = wx.CheckBox(self.optchar_options_panel, -1, "Keep Specified Joints")
        self.optchar_keepJointsTxt = wx.TextCtrl(self.optchar_options_panel, -1, "")
        self.optchar_dropJointsChk = wx.CheckBox(self.optchar_options_panel, -1, "Drop Specified Joints")
        self.optchar_dropJointsTxt = wx.TextCtrl(self.optchar_options_panel, -1, "")
        self.optchar_exposeJointsChk = wx.CheckBox(self.optchar_options_panel, -1, "Expose Specified Joints")
        self.optchar_exposeJointsTxt = wx.TextCtrl(self.optchar_options_panel, -1, "")
        self.optchar_flagGeometryChk = wx.CheckBox(self.optchar_options_panel, -1, "Flag Specified Geometry")
        self.optchar_flagGeometryTxt = wx.TextCtrl(self.optchar_options_panel, -1, "")
        self.optchar_dartChoice = wx.RadioBox(self.optchar_options_panel, -1, "dart", choices=["default", "sync", "nosync", "structured"], majorDimension=4, style=wx.RA_SPECIFY_COLS)

        # Egg-palettize Tool
        self.eggPalettize_panel = wx.Panel(self.tool_options_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.palettize_eggFilesLbl = wx.StaticText(self.eggPalettize_panel, -1, "Egg Files")
        self.palettize_eggFilesTree = wx.TreeCtrl(self.eggPalettize_panel, -1, style=wx.TR_HAS_BUTTONS|wx.TR_LINES_AT_ROOT|wx.TR_DEFAULT_STYLE|wx.SUNKEN_BORDER)
        self.palettize_eggFilesRoot = self.palettize_eggFilesTree.AddRoot('Egg Files')
        self.palettize_addEggBtn = wx.Button(self.eggPalettize_panel, -1, "Add")
        self.palettize_addFromBatchBtn = wx.Button(self.eggPalettize_panel, -1, "Batch Files")
        self.palettize_removeEggBtn = wx.Button(self.eggPalettize_panel, -1, "Remove")
        self.palettize_removeAllEggsBtn = wx.Button(self.eggPalettize_panel, -1, "Remove All")
        self.palettize_exportDestLbl = wx.StaticText(self.eggPalettize_panel, -1, "Export Destination")
        self.palettize_exportInPlaceChk = wx.CheckBox(self.eggPalettize_panel, -1, "In Place")
        self.palettize_exportDirLbl = wx.StaticText(self.eggPalettize_panel, -1, "Directory")
        self.palettize_exportDirTxt = wx.TextCtrl(self.eggPalettize_panel, -1, "")
        self.palettize_exportDirBtn = wx.Button(self.eggPalettize_panel, -1, "Choose..")
        self.palettize_exportFileLbl = wx.StaticText(self.eggPalettize_panel, -1, "File")
        self.palettize_exportFileTxt = wx.TextCtrl(self.eggPalettize_panel, -1, "")
        self.palettize_exportFileBtn = wx.Button(self.eggPalettize_panel, -1, "Choose..")
        self.palettize_exportTexLbl = wx.StaticText(self.eggPalettize_panel, -1, "Texture")
        self.palettize_exportTexTxt = wx.TextCtrl(self.eggPalettize_panel, -1, "")
        self.palettize_exportTexBtn = wx.Button(self.eggPalettize_panel, ID_OUTTEX, "Choose..")

        self.palettize_options_panel = wx.Panel(self.eggPalettize_panel, -1)
        self.palettize_options_sizer_staticbox = wx.StaticBox(self.palettize_options_panel, 1, "Palettize Attributes")

        self.palettize_saveTxaLbl = wx.StaticText(self.palettize_options_panel, -1, "    Attrubutes File")
        self.palettize_saveTxaTxt = wx.TextCtrl(self.palettize_options_panel, -1, "")
        self.palettize_loadTxaBtn = wx.Button(self.palettize_options_panel, -1, "Choose..")
        self.palettize_saveTxaBtn = wx.Button(self.palettize_options_panel, -1, "Create")

        self.palettize_sizeLbl = wx.StaticText(self.palettize_options_panel, -1, "Palette Size")
        self.palettize_sizeWidthTxt = wx.TextCtrl(self.palettize_options_panel, -1, "4096",(30,20),(80,-1))
        self.palettize_sizeByLbl = wx.StaticText(self.palettize_options_panel, -1, " x ")
        self.palettize_sizeHeightTxt = wx.TextCtrl(self.palettize_options_panel, -1, "4096",(30,20),(80,-1))
        self.palettize_powerOf2Chk = wx.CheckBox(self.palettize_options_panel, -1, "Power of 2")

        self.palettize_imageTypeLbl = wx.StaticText(self.palettize_options_panel, -1, "Image Type")
        self.palettize_imageTypeChoice = wx.RadioBox(self.palettize_options_panel, -1, "", choices=["rgb", "jpg", "png"], majorDimension=3, style=wx.RA_SPECIFY_COLS)

        self.palettize_colorLbl = wx.StaticText(self.palettize_options_panel, -1, "Background Color")
        self.palettize_redLbl = wx.StaticText(self.palettize_options_panel, -1, "R")
        self.palettize_redTxt = wx.SpinCtrl(self.palettize_options_panel, -1, "",(30,20),(80,-1),min=0, max=255)
        self.palettize_greenLbl = wx.StaticText(self.palettize_options_panel, -1, "G")
        self.palettize_greenTxt = wx.SpinCtrl(self.palettize_options_panel, -1, "",(30,20),(80,-1),min=0, max=255)
        self.palettize_blueLbl = wx.StaticText(self.palettize_options_panel, -1, "B")
        self.palettize_blueTxt = wx.SpinCtrl(self.palettize_options_panel, -1, "",(30,20),(80,-1),min=0, max=255)
        self.palettize_alphaLbl = wx.StaticText(self.palettize_options_panel, -1, "A")
        self.palettize_alphaTxt = wx.SpinCtrl(self.palettize_options_panel, -1, "",(30,20),(80,-1),min=0, max=255)
        self.palettize_color_sizer_staticbox = wx.StaticBox(self.palettize_options_panel, -1, "")
        self.palettize_marginLbl = wx.StaticText(self.palettize_options_panel, -1, "Margin")
        self.palettize_marginTxt = wx.SpinCtrl(self.palettize_options_panel, -1, "",(30,20),(80,-1),min=0, max=10000)
        self.palettize_coverageLbl = wx.StaticText(self.palettize_options_panel, -1, "Coverage")
        self.palettize_coverageTxt = wx.TextCtrl(self.palettize_options_panel, -1, "1.0",(30,20),(80,-1))


        self.__set_properties()
        self.__do_layout()

        #bind events
        #simple mode
        self.Bind(wx.EVT_BUTTON, self.OnSimpleExport, self.simple_runExportBtn)
        self.Bind(wx.EVT_BUTTON, self.OnSimpleExportDest, self.simple_exportDestBtn)
        self.Bind(wx.EVT_BUTTON, self.OnSimpleMayaFile, self.simple_mayaFileBtn)

        #main menus
        self.Bind(wx.EVT_MENU, self.OnLoadPrefs, self.prefsLoadButton)
        self.Bind(wx.EVT_MENU, self.OnSavePrefs, self.savePrefsButton)
        self.Bind(wx.EVT_MENU, self.OnExit, self.exitButton)
        #self.Bind(wx.EVT_MENU, self.OnShowHelp,self.HelpMenuButton)

        #batch panel
        self.Bind(wx.EVT_COMBOBOX, self.OnTool, self.toolComboBox)
        self.Bind(wx.EVT_BUTTON, self.OnBatchItemEdit, id=ID_EDITSELBATCH)
        self.Bind(wx.EVT_BUTTON, self.OnRemoveBatch, id=ID_REMOVESELBATCH)
        self.Bind(wx.EVT_BUTTON, self.OnRemoveAllBatch, id=ID_REMOVEALLBATCH)
        self.Bind(wx.EVT_BUTTON, self.OnClearOutput, id=ID_CLEAROUTPUT)

        self.Bind(wx.EVT_BUTTON, self.OnAddToBatch, self.addToBatchBtn)

        self.Bind(wx.EVT_BUTTON, self.OnPandaPathChoose, id=ID_CHOOSEPANDA)

        #maya2egg panel
        self.Bind(wx.EVT_BUTTON, self.OnMaya2EggExportDest, self.m2e_exportDestBtn)
        self.Bind(wx.EVT_BUTTON, self.OnMaya2EggMayaFile, self.m2e_mayaFileBtn)
        self.Bind(wx.EVT_COMBOBOX, self.OnMayaVerChoose, self.m2e_mayaVerComboBox)
        self.Bind(wx.EVT_BUTTON, self.OnMaya2EggCopyTexPath, self.m2e_copyTexPathBtn)
        self.Bind(wx.EVT_RADIOBOX, self.OnMaya2EggAnimOpt, self.m2e_animOptChoice)
        self.Bind(wx.EVT_BUTTON, self.OnMaya2EggPathReplace, self.m2e_pathReplaceBtn)


        #egg2bam panel
        self.Bind(wx.EVT_BUTTON, self.OnEgg2BamEggFile, self.e2b_eggFileBtn)
        self.Bind(wx.EVT_BUTTON, self.OnEgg2BamExportDest, self.e2b_exportDestBtn)
        self.Bind(wx.EVT_BUTTON, self.OnEgg2BamBatchOutput, self.e2b_bamBatchOutputBtn)
        self.Bind(wx.EVT_CHECKBOX, self.OnEgg2BamUseCurrEgg, self.e2b_useCurrEggChk)

        #egg-rename panel
        self.Bind(wx.EVT_BUTTON, self.OnRenameAddEgg, self.rename_addEggBtn)
        self.Bind(wx.EVT_BUTTON, self.OnRenameAddFromBatch, self.rename_addFromBatchBtn)
        self.Bind(wx.EVT_BUTTON, self.OnRenameRemoveEgg, self.rename_removeEggBtn)
        self.Bind(wx.EVT_BUTTON, self.OnRenameRemoveAllEggs, self.rename_removeAllEggsBtn)
        self.Bind(wx.EVT_CHECKBOX, self.OnRenameInPlace, self.rename_exportInPlaceChk)
        self.Bind(wx.EVT_BUTTON, self.OnRenameExportDir, self.rename_exportDirBtn)
        self.Bind(wx.EVT_BUTTON, self.OnRenameExportFile, self.rename_exportFileBtn)

        #egg-optchar panel
        self.Bind(wx.EVT_BUTTON, self.OnOptcharAddEgg, self.optchar_addEggBtn)
        self.Bind(wx.EVT_BUTTON, self.OnOptcharAddFromBatch, self.optchar_addFromBatchBtn)
        self.Bind(wx.EVT_BUTTON, self.OnOptcharRemoveEgg, self.optchar_removeEggBtn)
        self.Bind(wx.EVT_BUTTON, self.OnOptcharRemoveAllEggs, self.optchar_removeAllEggsBtn)
        self.Bind(wx.EVT_CHECKBOX, self.OnOptcharInPlace, self.optchar_exportInPlaceChk)
        self.Bind(wx.EVT_BUTTON, self.OnOptcharExportDir, self.optchar_exportDirBtn)
        self.Bind(wx.EVT_BUTTON, self.OnOptcharExportFile, self.optchar_exportFileBtn)

        #egg-palettzie
        self.Bind(wx.EVT_BUTTON, self.OnPalettizeAddEgg, self.palettize_addEggBtn)
        self.Bind(wx.EVT_BUTTON, self.OnPalettizeAddFromBatch, self.palettize_addFromBatchBtn)
        self.Bind(wx.EVT_BUTTON, self.OnPalettizeRemoveEgg, self.palettize_removeEggBtn)
        self.Bind(wx.EVT_BUTTON, self.OnPalettizeRemoveAllEggs, self.palettize_removeAllEggsBtn)
        self.Bind(wx.EVT_CHECKBOX, self.OnPalettizeInPlace, self.palettize_exportInPlaceChk)
        self.Bind(wx.EVT_BUTTON, self.OnPalettizeExportDir, self.palettize_exportDirBtn)
        self.Bind(wx.EVT_BUTTON, self.OnPalettizeExportFile, self.palettize_exportFileBtn)
        self.Bind(wx.EVT_BUTTON, self.OnPalettizeExportTex, self.palettize_exportTexBtn)
        self.Bind(wx.EVT_BUTTON, self.OnPalettizeLoadTxa, self.palettize_loadTxaBtn)
        self.Bind(wx.EVT_BUTTON, self.OnPalettizeSaveTxa, self.palettize_saveTxaBtn)

        # batch panel
        self.Bind(wx.EVT_BUTTON, self.OnRunBatch, id=ID_RUNBATCH)
        self.Bind(wx.EVT_BUTTON, self.OnLoadBatch, id=ID_LOADBATCH)
        self.Bind(wx.EVT_BUTTON, self.OnSaveBatch, id=ID_SAVEBATCH)
        self.Bind(wx.EVT_BUTTON, self.OnSortBatch, id=ID_SORTBATCH)
        self.Bind(wx.EVT_BUTTON, self.OnChangePaths, self.changePathsButton)
        self.Bind(wx.EVT_TREE_SEL_CHANGED, self.OnBatchItemSelection, self.batchTree)

        # console panel
        self.Bind(wx.EVT_BUTTON, self.OnLoadPview, self.runPviewButton)

    def __set_properties(self):
        # begin wxGlade: main.__set_properties
        # This was mostly generated in wxGlade, however
        #I had to edit a lot of it by hand.
        #This function mainly takes care of setting labels and sizes
        # of most of the GUI elements.
        self.SetTitle("Panda3D Tools GUI")
        self.simple_exportDestTxt.SetMinSize((230,21))
        self.simple_mayaFileTxt.SetMinSize((230,21))
        self.simple_mayaVerComboBox.SetSelection(9)
        self.simple_animOptChoice.SetSelection(1)
        self.statusBar.SetStatusWidths([-1])
        #self.simple_panel.SetMaxSize((400,400))
        self.simple_mayaFileBtn.SetMinSize((-1, 23))
        self.simple_exportDestBtn.SetMinSize((-1, 23))
        self.simple_runExportBtn.SetMinSize((-1, 23))

        self.batchItemNameTxt.SetMinSize((210, 21))
        self.addToBatchBtn.SetMinSize((-1, 23))

        # Maya2Egg panel
        #self.maya2egg_panel.SetMinSize((420, 530))
        #self.m2e_options_panel.SetMinSize((420,166))
        #self.m2e_anim_options_panel.SetMinSize((400,175))
        #self.m2e_tex_options_panel.SetMinSize((400,86))
        self.m2e_mayaFileTxt.SetMinSize((230, 21))
        self.m2e_exportDestTxt.SetMinSize((230, 21))
        self.m2e_mayaVerComboBox.SetSelection(1)
        self.m2e_mayaUnitsComboBox.SetSelection(1)
        self.m2e_pandaUnitsComboBox.SetSelection(1)
        self.m2e_subrootsTxt.SetMinSize((235,21))
        self.m2e_subsetsTxt.SetMinSize((235, 21))
        self.m2e_excludesTxt.SetMinSize((235, 21))
        self.m2e_animOptChoice.SetSelection(1)
        self.m2e_charNameTxt.SetMinSize((283, 21))
        self.m2e_copyTexPathTxt.SetMinSize((218, 21))
        self.m2e_pathReplaceTxt.SetMinSize((218, 21))

        self.m2e_mayaFileBtn.SetMinSize((-1, 23))
        self.m2e_exportDestBtn.SetMinSize((-1, 23))
        self.m2e_copyTexPathBtn.SetMinSize((-1, 23))
        self.m2e_pathReplaceBtn.SetMinSize((-1, 23))

        self.m2e_startFrameSpin.SetMinSize((-1, 21))
        self.m2e_endFrameSpin.SetMinSize((-1, 21))
        self.m2e_frameRateInSpin.SetMinSize((-1, 21))
        self.m2e_frameRateOutSpin.SetMinSize((-1, 21))
        #self.m2e_addEgg2BamChk.SetValue(1)

        self.m2e_mayaFileLbl.SetToolTipString("Maya file to be exported")
        self.m2e_mayaFileTxt.SetToolTipString("Maya file to be exported")
        self.m2e_mayaFileBtn.SetToolTipString("Select a maya file to be exported")
        self.m2e_exportDestLbl.SetToolTipString("Destination of the exported file")
        self.m2e_exportDestTxt.SetToolTipString("Destination of the exported file")
        self.m2e_exportDestBtn.SetToolTipString("Select the destination of the exported file")
        self.m2e_mayaVerLbl.SetToolTipString("Version of the maya exporter to use, must match version of *.mb file")
        self.m2e_mayaVerComboBox.SetToolTipString("Version of the maya exporter to use, must match version of *.mb file")
        self.m2e_mayaUnitsLbl.SetToolTipString("The units of the input Maya file")
        self.m2e_mayaUnitsComboBox.SetToolTipString("defaults to centimeters")
        self.m2e_pandaUnitsLbl.SetToolTipString("The units of the output egg file")
        self.m2e_pandaUnitsComboBox.SetToolTipString("defaults to centimeters")
        self.m2e_backfaceChk.SetToolTipString("Enable/Disable backface rendering of polygons in the egg file (default is off)")
        self.m2e_polygonOutputChk.SetToolTipString("Generate polygon output only. Tesselate all NURBS surfaces to polygons via the built-in Maya tesselator")
        self.m2e_tbnallChk.SetToolTipString("Calculate the tangents and binormals for all texture coordinate sets (for normal maps, etc)")
        self.m2e_subrootsChk.SetToolTipString("Export specified subroots of the geometry in the Maya file to be converted")
        self.m2e_subrootsTxt.SetToolTipString("Export specified subroots of the geometry in the Maya file to be converted")
        self.m2e_subsetsChk.SetToolTipString("Export specified subsets of the geometry in the Maya file to be converted")
        self.m2e_subsetsTxt.SetToolTipString("Export specified subsets of the geometry in the Maya file to be converted")
        self.m2e_excludesChk.SetToolTipString("Exclude specified subsets of the geometry in the Maya file to be converted")
        self.m2e_excludesTxt.SetToolTipString("Exclude specified subsets of the geometry in the Maya file to be converted")
        self.m2e_animOptChoice.SetToolTipString("Specifies how animation from the Maya file is converted to egg, if at all")
        self.m2e_startFrameChk.SetToolTipString("Starting frame of animation to exctract. For pose, this is the one frame of animation to extract")
        self.m2e_endFrameChk.SetToolTipString("Ending frame of animation to exctract")
        self.m2e_frameRateInChk.SetToolTipString("Frame rate (frames per second) of the input Maya file")
        self.m2e_frameRateOutChk.SetToolTipString("Frame rate (frames per second) of the generated animation file. If this is specified, the animation speed is scaled by the appropriate factor based on the frame rate of the input file")
        self.m2e_charNameChk.SetToolTipString("Name of the animation character. This should match between all of the model files and all of the channel files for a particular model and its associated channels")
        self.m2e_legacyShaderChk.SetToolTipString("Turn off modern (Phong) shader generation and treat all shaders as if they were Lamberts (legacy)")
        self.m2e_copyTexChk.SetToolTipString("Copy the textures to a textures sub directory relative to the written out egg file")
        self.m2e_copyTexPathTxt.SetToolTipString("Copy the textures to a textures sub directory relative to the written out egg file")
        self.m2e_copyTexPathBtn.SetToolTipString("Specify texture directory")
        self.m2e_pathReplaceChk.SetToolTipString("Remap prefixes for texture and external reference paths")
        self.m2e_pathReplaceTxt.SetToolTipString("Remap prefixes for texture and external reference paths. Ex: orig_prefix=replacement_prefix")
        self.m2e_pathReplaceBtn.SetToolTipString("Replace path prefixes for all Maya2Egg commands for selected batch commands")


        # Egg2Bam panel
        #self.egg2bam_panel.SetMinSize((400,166))
        #self.e2b_options_panel.SetMinSize((400, 80))
        self.e2b_useCurrEggChk.SetValue(0)
        self.e2b_useCurrEggChk.Enable(True)
        self.e2b_eggFileTxt.SetMinSize((230, 21))
        self.e2b_exportDestTxt.SetMinSize((230, 21))
        self.egg2bam_panel.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))

        self.e2b_eggFileBtn.SetMinSize((-1, 23))
        self.e2b_exportDestBtn.SetMinSize((-1, 23))
        self.e2b_bamBatchOutputBtn.SetMinSize((-1, 23))

        self.e2b_bamBatchOutputLbl.SetToolTipString("Generate and add to batch bam2egg commands from currently selected batch items' outputs")
        self.e2b_bamBatchOutputBtn.SetToolTipString("Generate and add to batch bam2egg commands from currently selected batch items' outputs")
        self.e2b_eggFileLbl.SetToolTipString("Input egg file to be converted")
        self.e2b_eggFileTxt.SetToolTipString("Input egg file to be converted")
        self.e2b_eggFileBtn.SetToolTipString("Select an input egg file to be converted")
        self.e2b_exportDestLbl.SetToolTipString("Destination of the exported file")
        self.e2b_exportDestTxt.SetToolTipString("Destination of the exported file")
        self.e2b_exportDestBtn.SetToolTipString("Select the destination of the exported file")
        self.e2b_useCurrEggChk.SetToolTipString("Use output from Maya2Egg panel as the input file")
        self.e2b_flattenChk.SetToolTipString("Flatten the egg hierarchy after it is loaded (unnecessary nodes are eliminated)")
        self.e2b_embedTexChk.SetToolTipString("Record texture data directly in the bam file, instead of storing a reference to the texture elsewhere on disk")


        # Egg-Rename panel
        self.eggRename_panel.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))
        #self.eggRename_panel.SetMinSize((400,240))
        #self.rename_options_panel.SetMinSize((400, 45))
        self.rename_eggFilesTree.SetMinSize((230,120))
        self.rename_exportDirTxt.SetMinSize((230, 21))
        self.rename_exportFileTxt.SetMinSize((230, 21))
        self.rename_stripPrefixTxt.SetMinSize((260, 21))

        self.rename_addEggBtn.SetMinSize((-1, 23))
        self.rename_addFromBatchBtn.SetMinSize((-1, 23))
        self.rename_removeEggBtn.SetMinSize((-1, 23))
        self.rename_removeAllEggsBtn.SetMinSize((-1, 23))
        self.rename_exportDirBtn.SetMinSize((-1, 23))
        self.rename_exportFileBtn.SetMinSize((-1, 23))


        # Egg-Optchar panel
        self.eggOptChar_panel.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))
        #self.eggOptChar_panel.SetMinSize((400,366))
        #self.optchar_options_panel.SetMinSize((400, 171))
        self.optchar_eggFilesTree.SetMinSize((230,120))
        self.optchar_exportDirTxt.SetMinSize((230, 21))
        self.optchar_exportFileTxt.SetMinSize((230, 21))
        self.optchar_keepJointsTxt.SetMinSize((245, 21))
        self.optchar_dropJointsTxt.SetMinSize((245, 21))
        self.optchar_exposeJointsTxt.SetMinSize((245, 21))
        self.optchar_flagGeometryTxt.SetMinSize((245, 21))

        self.optchar_addEggBtn.SetMinSize((-1, 23))
        self.optchar_addFromBatchBtn.SetMinSize((-1, 23))
        self.optchar_removeEggBtn.SetMinSize((-1, 23))
        self.optchar_removeAllEggsBtn.SetMinSize((-1, 23))
        self.optchar_exportDirBtn.SetMinSize((-1, 23))
        self.optchar_exportFileBtn.SetMinSize((-1, 23))

        #self.palettize_.SetToolTipString("
        self.optchar_exportInPlaceChk.SetToolTipString("Input egg files will be rewritten in place with the results (original input files are lost)")
        self.optchar_keepAllJointsChk.SetToolTipString("Keep all joints in the character, except those named explicitily by drop")
        self.optchar_keepJointsChk.SetToolTipString("Keep the specified joints in the character, even if they don't appear to be needed by the animation")
        self.optchar_dropJointsChk.SetToolTipString("Remove the specified joints in the character, even if they appear to be needed by the animation")
        self.optchar_exposeJointsChk.SetToolTipString("Expose the specified joints in the character by flagging them with a DCS attribute, so each one can be found in the scene graph when the character is loaded and object can be parented to it")
        self.optchar_flagGeometryChk.SetToolTipString("Assign the indicated name to the geometry within the given nodes. This will make the geometry visible as a node in the resulting character model when loaded in the scene graph")
        self.optchar_dartChoice.SetToolTipString("Change the dart value in the input eggs")


        # Egg-Palettize panel
        self.eggPalettize_panel.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))
        #self.eggPalettize_panel.SetMinSize((400,443))
        #self.palettize_options_panel.SetMinSize((400, 225))
        self.palettize_eggFilesTree.SetMinSize((230,120))
        self.palettize_exportDirTxt.SetMinSize((230, 21))
        self.palettize_exportFileTxt.SetMinSize((230, 21))
        self.palettize_exportTexTxt.SetMinSize((230, 21))
        self.palettize_saveTxaTxt.SetMinSize((230, 21))
        self.palettize_imageTypeChoice.SetSelection(2)
        self.palettize_marginTxt.SetValue(2)

        self.palettize_sizeWidthTxt.SetMinSize((60,21))
        self.palettize_sizeHeightTxt.SetMinSize((60,21))
        self.palettize_redTxt.SetMinSize((60,21))
        self.palettize_greenTxt.SetMinSize((60,21))
        self.palettize_blueTxt.SetMinSize((60,21))
        self.palettize_alphaTxt.SetMinSize((60,21))
        self.palettize_marginTxt.SetMinSize((-1, 21))
        self.palettize_coverageTxt.SetMinSize((-1, 21))

        self.palettize_addEggBtn.SetMinSize((-1, 23))
        self.palettize_addFromBatchBtn.SetMinSize((-1, 23))
        self.palettize_removeEggBtn.SetMinSize((-1, 23))
        self.palettize_removeAllEggsBtn.SetMinSize((-1, 23))
        self.palettize_exportDirBtn.SetMinSize((-1, 23))
        self.palettize_exportFileBtn.SetMinSize((-1, 23))
        self.palettize_exportTexBtn.SetMinSize((-1, 23))
        self.palettize_saveTxaBtn.SetMinSize((-1, 23))
        self.palettize_loadTxaBtn.SetMinSize((-1, 23))

        self.palettize_exportInPlaceChk.SetToolTipString("Input egg files will be rewritten in place with the results (original input files are lost)")
        self.palettize_exportDirLbl.SetToolTipString("Destination directory for the exported files")
        self.palettize_exportDirTxt.SetToolTipString("Destination directory for the exported files")
        self.palettize_exportDirBtn.SetToolTipString("Select the destination directory for the exported files")
        self.palettize_exportFileLbl.SetToolTipString("Destination of the exported file")
        self.palettize_exportFileTxt.SetToolTipString("Destination of the exported file")
        self.palettize_exportFileBtn.SetToolTipString("Select the destination of the exported file")
        self.palettize_exportTexLbl.SetToolTipString("Destination folder of the exported textures")
        self.palettize_exportTexTxt.SetToolTipString("Destination folder of the exported textures")
        self.palettize_exportTexBtn.SetToolTipString("Select the destination folder of the exported textures")

        self.palettize_addEggBtn.SetToolTipString("Add an egg file/s to be exported")
        self.palettize_addFromBatchBtn.SetToolTipString("Add egg file/s to be exported from the batch list")
        self.palettize_removeEggBtn.SetToolTipString("Remove the selected egg file")
        self.palettize_removeAllEggsBtn.SetToolTipString("Remove all egg files")

        self.palettize_sizeLbl.SetToolTipString("Specifies the size of the palette images to be created")
        self.palettize_sizeWidthTxt.SetToolTipString("Width of the palette images to be created")
        self.palettize_sizeHeightTxt.SetToolTipString("Height of the palette images to be created")
        self.palettize_powerOf2Chk.SetToolTipString("Specifies whether texures should be forced to a power of two size when they are placed within a palette")
        self.palettize_imageTypeLbl.SetToolTipString("Image type of each generated texture palette")
        self.palettize_imageTypeChoice.SetToolTipString("Image type of each generated texture palette")

        self.palettize_colorLbl.SetToolTipString("Color of the palette's background")
        self.palettize_redLbl.SetToolTipString("Red value of the palette's background color")
        self.palettize_redTxt.SetToolTipString("Red value of the palette's background color")
        self.palettize_greenLbl.SetToolTipString("Green value of the palette's background color")
        self.palettize_greenTxt.SetToolTipString("Green value of the palette's background color")
        self.palettize_blueLbl.SetToolTipString("Blue value of the palette's background color")
        self.palettize_blueTxt.SetToolTipString("Blue value of the palette's background color")
        self.palettize_alphaLbl.SetToolTipString("Alpha value of the palette's background color")
        self.palettize_alphaTxt.SetToolTipString("Alpha value of the palette's background color")
        self.palettize_color_sizer_staticbox.SetToolTipString("Palette's background color")
        self.palettize_marginLbl.SetToolTipString("Specifies the amount of margin to apply to all textures that are placed within a palette image")
        self.palettize_marginTxt.SetToolTipString("Specifies the amount of margin to apply to all textures that are placed within a palette image")
        self.palettize_coverageLbl.SetToolTipString("Fraction of the area in the texture image that is actually used")
        self.palettize_coverageTxt.SetToolTipString("Fraction of the area in the texture image that is actually used")
        self.palettize_saveTxaLbl.SetToolTipString("Attributes (.txa) file")
        self.palettize_saveTxaTxt.SetToolTipString("Attributes (.txa) file")
        self.palettize_loadTxaBtn.SetToolTipString("Choose attributes file (.txa)")
        self.palettize_saveTxaBtn.SetToolTipString("Save current attributes to the .txa file")

        self.toolComboBox.SetSelection(0)

        # batch panel
        self.batch_panel.SetMinSize((2000, 608))
        self.batchTree.SetMinSize((2000,524))
        self.loadBatchButton.SetMinSize((-1, 23))
        self.saveBatchButton.SetMinSize((-1, 23))
        self.sortBatchButton.SetMinSize((-1, 23))
        self.changePathsButton.SetMinSize((-1, 23))
        self.editSelBatchButton.SetMinSize((-1, 23))
        self.removeSelBatchButton.SetMinSize((-1, 23))
        self.removeAllBatchButton.SetMinSize((-1, 23))

        # console panel
        self.console_panel.SetMinSize((2000, 1000))
        self.consoleOutputTxt.SetMinSize((2000, 1000))
        self.consoleOutputTxt.SetBackgroundColour(wx.Colour(192, 192, 192))
        self.consoleOutputTxt.SetToolTipString("maya2egg console output appears here when batch process is running")
        self.consoleOutputTxt.Enable(True)

        self.runBatchButton.SetMinSize((-1, 23))
        self.clearConsoleButton.SetMinSize((-1, 23))
        self.runPviewButton.SetMinSize((-1, 23))

        self.ignoreModDates.SetValue(True)
        self.pandaPathTxt.SetMinSize((200, 21))
        self.loadPandaPathBtn.SetMinSize((-1, 23))
        self.pandaPathTxt.SetBackgroundColour(wx.Colour(192, 192, 192))
        self.ignoreModDates.SetToolTipString("Use this check box to export all the mb files regardless if they have been modified since the last export")
        self.pandaPathTxt.SetToolTipString("Select the particular installed version of Panda3D, if not chosen the first entry in the system path is used")

        self.main_panel.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))
        self.simple_panel.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))

        self.SetStatusText('Welcome to Panda3D Tools GUI')
        # end wxGlade

    def __do_layout(self):
        # This was mostly generated in wxGlade, but since it didn't allow for pyCollapsible Pane
        #I had to edit a lot of it by hand.
        #This function takes care of adding all the GUI elements to sizers
        #A few are not named properly as of now due to their late addition
        # begin wxGlade: main.__do_layout
        tab_panel_sizer = wx.BoxSizer(wx.HORIZONTAL)

        #simple panel
        self.simple_options_sizer_staticbox.Lower()
        simple_sizer = wx.FlexGridSizer(1,1,0,0)
        simple_options_static_sizer = wx.StaticBoxSizer(self.simple_options_sizer_staticbox, wx.VERTICAL)

        file_flex_grid_sizer = wx.FlexGridSizer(3,3,0,0)
        file_flex_grid_sizer.Add(self.simple_mayaFileLbl, -1, wx.TOP, 5)
        file_flex_grid_sizer.Add(self.simple_mayaFileTxt, -1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.simple_mayaFileBtn, -1, 0, 0)
        file_flex_grid_sizer.Add(self.simple_exportDestLbl, -1, wx.TOP, 5)
        file_flex_grid_sizer.Add(self.simple_exportDestTxt, -1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.simple_exportDestBtn, -1, 0, 0)
        file_flex_grid_sizer.Add(self.simple_mayaVerLbl, 1, wx.TOP, 5)
        file_flex_grid_sizer.Add(self.simple_mayaVerComboBox, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.simple_runExportBtn, -1, 0, 0)
        simple_options_static_sizer.Add(file_flex_grid_sizer)
        simple_options_static_sizer.Add(self.simple_animOptChoice)

        self.simple_options_panel.SetSizer(simple_options_static_sizer)
        simple_sizer.Add(self.simple_options_panel)
        self.simple_panel.SetSizer(simple_sizer)

        # main panel
        main_sizer = wx.FlexGridSizer(4,1,0,0)
        top_sizer = wx.FlexGridSizer(1,2,0,0)
        main_sizer.Add(top_sizer, 0, wx.EXPAND, 0)
        top_left_sizer = wx.FlexGridSizer(3,1,0,0)
        top_right_sizer = wx.FlexGridSizer(3,1,0,0)
        top_sizer.Add(top_left_sizer, 0, wx.ALL|wx.EXPAND, 5)
        top_sizer.Add(top_right_sizer, 0, wx.ALL|wx.EXPAND, 5)

        top_left_sizer.Add(self.toolComboBox, 0, wx.ALL, 5)

        # Maya2Egg
        self.tool_options_sizer_staticbox.Lower()
        self.tool_options_static_sizer = wx.StaticBoxSizer(self.tool_options_sizer_staticbox, wx.VERTICAL)

        maya2egg_grid_sizer = wx.FlexGridSizer(8, 1, 0, 0)

        file_flex_grid_sizer = wx.FlexGridSizer(2, 3, 0, 0)
        file_flex_grid_sizer.Add(self.m2e_mayaFileLbl, 1, wx.TOP, 5)
        file_flex_grid_sizer.Add(self.m2e_mayaFileTxt, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.m2e_mayaFileBtn, 1, 0, 0)
        file_flex_grid_sizer.Add(self.m2e_exportDestLbl, 1, wx.TOP, 5)
        file_flex_grid_sizer.Add(self.m2e_exportDestTxt, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.m2e_exportDestBtn, 1, 0, 0)
        maya2egg_grid_sizer.Add(file_flex_grid_sizer, 1, wx.ALL, 0)
        maya_ver_sizer = wx.FlexGridSizer(1,2,0,0)
        maya_ver_sizer.Add(self.m2e_mayaVerLbl, 1, wx.TOP|wx.RIGHT, 3)
        maya_ver_sizer.Add(self.m2e_mayaVerComboBox, 1, wx.LEFT, 3)
        maya2egg_grid_sizer.Add(maya_ver_sizer, 1, wx.TOP, 3)
        self.maya2egg_panel.SetSizer(maya2egg_grid_sizer)
        self.tool_options_static_sizer.Add(self.maya2egg_panel, 1, wx.ALL|wx.EXPAND, 0)

        self.m2e_options_panel_sizer_staticbox.Lower()
        general_options_static_sizer = wx.StaticBoxSizer(self.m2e_options_panel_sizer_staticbox, wx.VERTICAL)
        general_options_grid_sizer = wx.FlexGridSizer(6,1,0,0)
        units_sizer = wx.FlexGridSizer(1,4,0,0)
        units_sizer.Add(self.m2e_mayaUnitsLbl, 1, wx.ALL, 3)
        units_sizer.Add(self.m2e_mayaUnitsComboBox, 1, wx.RIGHT, 6)
        units_sizer.Add(self.m2e_pandaUnitsLbl, 1, wx.ALL, 3)
        units_sizer.Add(self.m2e_pandaUnitsComboBox, 1, wx.ALL, 0)
        general_options_grid_sizer.Add(units_sizer)
        general_options_grid_sizer.Add(self.m2e_backfaceChk, 1, wx.ALL, 3)
        general_options_grid_sizer.Add(self.m2e_polygonOutputChk, 1, wx.ALL, 3)
        general_options_grid_sizer.Add(self.m2e_tbnallChk, 1, wx.ALL, 3)

        subroots_sizer = wx.FlexGridSizer(3,2,0,0)
        subroots_sizer.Add(self.m2e_subrootsChk, 1, wx.ALL, 3)
        subroots_sizer.Add(self.m2e_subrootsTxt, 1, wx.LEFT, 1)
        subroots_sizer.Add(self.m2e_subsetsChk, 1, wx.ALL, 3)
        subroots_sizer.Add(self.m2e_subsetsTxt, 1, wx.LEFT, 1)
        subroots_sizer.Add(self.m2e_excludesChk, 1, wx.ALL, 3)
        subroots_sizer.Add(self.m2e_excludesTxt, 1, wx.LEFT, 1)
        general_options_grid_sizer.Add(subroots_sizer, 1, wx.ALL, 0)
        general_options_static_sizer.Add(general_options_grid_sizer)
        self.m2e_options_panel.SetSizer(general_options_static_sizer)
        maya2egg_grid_sizer.Add(self.m2e_options_panel, 1, wx.TOP|wx.EXPAND, 10)

        self.m2e_anim_options_sizer_staticbox.Lower()
        animation_options_static_sizer = wx.StaticBoxSizer(self.m2e_anim_options_sizer_staticbox, wx.VERTICAL)
        animation_options_grid_sizer = wx.FlexGridSizer(6,1,0,0)
        animation_options_grid_sizer.Add(self.m2e_animOptChoice, 1, wx.BOTTOM, 3)
        frames_grid_sizer = wx.FlexGridSizer(4,2,0,0)
        frames_grid_sizer.Add(self.m2e_startFrameChk, 0, wx.ALL, 3)
        frames_grid_sizer.Add(self.m2e_startFrameSpin, 0, 0, 0)
        frames_grid_sizer.Add(self.m2e_endFrameChk, 0, wx.ALL, 3)
        frames_grid_sizer.Add(self.m2e_endFrameSpin, 0, 0, 0)
        frames_grid_sizer.Add(self.m2e_frameRateInChk, 0, wx.ALL, 3)
        frames_grid_sizer.Add(self.m2e_frameRateInSpin, 0, 0, 0)
        frames_grid_sizer.Add(self.m2e_frameRateOutChk, 0, wx.ALL, 3)
        frames_grid_sizer.Add(self.m2e_frameRateOutSpin, 0, 0, 0)
        animation_options_grid_sizer.Add(frames_grid_sizer, 1, wx.ALL, 0)
        names_grid_sizer = wx.FlexGridSizer(1,2,0,0)
        names_grid_sizer.Add(self.m2e_charNameChk, 1, wx.ALL, 3)
        names_grid_sizer.Add(self.m2e_charNameTxt, 1, wx.ALL, 0)
        animation_options_grid_sizer.Add(names_grid_sizer, 1, wx.ALL, 0)
        animation_options_static_sizer.Add(animation_options_grid_sizer)
        self.m2e_anim_options_panel.SetSizer(animation_options_static_sizer)
        maya2egg_grid_sizer.Add(self.m2e_anim_options_panel, 1, wx.TOP|wx.EXPAND, 10)

        self.m2e_tex_options_sizer_staticbox.Lower()
        tex_options_static_sizer = wx.StaticBoxSizer(self.m2e_tex_options_sizer_staticbox, wx.VERTICAL)
        tex_options_grid_sizer = wx.FlexGridSizer(2,1,0,0)
        tex_options_grid_sizer.Add(self.m2e_legacyShaderChk, 1, wx.ALL, 3)

        copytex_sizer = wx.FlexGridSizer(2,3,0,0)
        copytex_sizer.Add(self.m2e_copyTexChk, 1, wx.ALL, 3)
        copytex_sizer.Add(self.m2e_copyTexPathTxt, 1, wx.TOP, 1)
        copytex_sizer.Add(self.m2e_copyTexPathBtn, 1, wx.ALL, 0)
        copytex_sizer.Add(self.m2e_pathReplaceChk, 1, wx.ALL, 3)
        copytex_sizer.Add(self.m2e_pathReplaceTxt, 1, wx.TOP, 1)
        copytex_sizer.Add(self.m2e_pathReplaceBtn, 1, wx.ALL, 0)
        tex_options_grid_sizer.Add(copytex_sizer, 1, wx.ALL, 0)
        tex_options_static_sizer.Add(tex_options_grid_sizer)
        self.m2e_tex_options_panel.SetSizer(tex_options_static_sizer)
        maya2egg_grid_sizer.Add(self.m2e_tex_options_panel, 1, wx.TOP|wx.EXPAND, 10)

        #maya2egg_grid_sizer.Add(self.m2e_addEgg2BamChk, 1, wx.TOP|wx.ALIGN_RIGHT, 3)

        self.tool_options_panel.SetSizer(self.tool_options_static_sizer)
        top_left_sizer.Add(self.tool_options_panel, 1, wx.ALL|wx.EXPAND, 0)

        # batch item details
        batch_item_grid_sizer = wx.FlexGridSizer(1,3,0,0)
        batch_item_grid_sizer.Add(self.batchItemNameLbl, -1, wx.ALL, 4)
        batch_item_grid_sizer.Add(self.batchItemNameTxt, -1, wx.ALL, 1)
        batch_item_grid_sizer.Add(self.addToBatchBtn, -1, wx.LEFT, 5)
        top_left_sizer.Add(batch_item_grid_sizer, 1, wx.TOP, 6)

        # Egg2Bam
        egg2bam_grid_sizer = wx.FlexGridSizer(8, 1, 0, 0)
        file_flex_grid_sizer = wx.FlexGridSizer(2, 3, 0, 0)
        file_flex_grid_sizer.Add(self.e2b_eggFileLbl, 1, wx.TOP, 5)
        file_flex_grid_sizer.Add(self.e2b_eggFileTxt, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.e2b_eggFileBtn, 1, 0, 0)
        file_flex_grid_sizer.Add(self.e2b_exportDestLbl, 1, wx.TOP, 5)
        file_flex_grid_sizer.Add(self.e2b_exportDestTxt, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.e2b_exportDestBtn, 1, 0, 0)
        egg2bam_grid_sizer.Add(file_flex_grid_sizer, 1, wx.ALL, 0)
        self.egg2bam_panel.SetSizer(egg2bam_grid_sizer)
        self.tool_options_static_sizer.Add(self.egg2bam_panel, 1, wx.ALL, 0)

        bam_batch_sizer = wx.FlexGridSizer(1,2,0,0)
        bam_batch_sizer.Add(self.e2b_bamBatchOutputLbl, 1, wx.TOP, 4)
        bam_batch_sizer.Add(self.e2b_bamBatchOutputBtn, 1, wx.LEFT, 3)
        egg2bam_grid_sizer.Add(bam_batch_sizer, 1, wx.TOP|wx.ALIGN_RIGHT, 5)

        self.e2b_options_sizer_staticbox.Lower()
        e2b_options_static_sizer = wx.StaticBoxSizer(self.e2b_options_sizer_staticbox, wx.VERTICAL)
        e2b_options_grid_sizer = wx.FlexGridSizer(6,1,0,0)
        e2b_options_grid_sizer.Add(self.e2b_useCurrEggChk, 1, wx.ALL, 3)
        e2b_options_grid_sizer.Add(self.e2b_flattenChk, 1, wx.ALL, 3)
        e2b_options_grid_sizer.Add(self.e2b_embedTexChk, 1, wx.ALL, 3)
        e2b_options_static_sizer.Add(e2b_options_grid_sizer)
        self.e2b_options_panel.SetSizer(e2b_options_static_sizer)
        egg2bam_grid_sizer.Add(self.e2b_options_panel, 1, wx.TOP|wx.EXPAND, 10)

        # Egg-Rename
        eggRename_grid_sizer = wx.FlexGridSizer(8, 1, 0, 0)
        file_flex_grid_sizer = wx.FlexGridSizer(4, 3, 0, 0)
        file_flex_grid_sizer.Add(self.rename_eggFilesLbl, 1, wx.TOP, 5)
        file_flex_grid_sizer.Add(self.rename_eggFilesTree, 1, wx.ALL, 1)
        eggfiles_grid_sizer = wx.FlexGridSizer(4, 1, 0, 0)
        eggfiles_grid_sizer.Add(self.rename_addEggBtn, 1, 0, 0)
        eggfiles_grid_sizer.Add(self.rename_addFromBatchBtn, 1, 0, 0)
        eggfiles_grid_sizer.Add(self.rename_removeEggBtn, 1, 0, 0)
        eggfiles_grid_sizer.Add(self.rename_removeAllEggsBtn, 1, 0, 0)
        file_flex_grid_sizer.Add(eggfiles_grid_sizer, 1, 0, 0)

        file_flex_grid_sizer.Add(self.rename_exportDestLbl, 1, wx.TOP, 0)
        file_flex_grid_sizer.Add(self.rename_exportInPlaceChk, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add((10,2),0,0)
        file_flex_grid_sizer.Add(self.rename_exportDirLbl, 1, wx.TOP|wx.ALIGN_RIGHT, 3)
        file_flex_grid_sizer.Add(self.rename_exportDirTxt, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.rename_exportDirBtn, 1, 0, 0)
        file_flex_grid_sizer.Add(self.rename_exportFileLbl, 1, wx.TOP|wx.ALIGN_RIGHT, 3)
        file_flex_grid_sizer.Add(self.rename_exportFileTxt, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.rename_exportFileBtn, 1, 0, 0)

        eggRename_grid_sizer.Add(file_flex_grid_sizer, 1, wx.ALL, 0)
        self.eggRename_panel.SetSizer(eggRename_grid_sizer)
        self.tool_options_static_sizer.Add(self.eggRename_panel, 1, wx.ALL|wx.EXPAND, 0)

        self.rename_options_sizer_staticbox.Lower()
        rename_options_static_sizer = wx.StaticBoxSizer(self.rename_options_sizer_staticbox, wx.VERTICAL)
        rename_options_grid_sizer = wx.FlexGridSizer(6,1,0,0)

        prefix_sizer = wx.FlexGridSizer(2,2,0,0)
        prefix_sizer.Add(self.rename_stripPrefixChk, 1, wx.ALL, 3)
        prefix_sizer.Add(self.rename_stripPrefixTxt, 1, wx.LEFT, 2)
        rename_options_grid_sizer.Add(prefix_sizer, 1, wx.ALL, 0)

        rename_options_static_sizer.Add(rename_options_grid_sizer)
        self.rename_options_panel.SetSizer(rename_options_static_sizer)
        eggRename_grid_sizer.Add(self.rename_options_panel, 1, wx.TOP|wx.EXPAND, 10)

        # Egg-Optchar
        eggOptChar_grid_sizer = wx.FlexGridSizer(8, 1, 0, 0)
        file_flex_grid_sizer = wx.FlexGridSizer(4, 3, 0, 0)
        file_flex_grid_sizer.Add(self.optchar_eggFilesLbl, 1, wx.TOP, 5)
        file_flex_grid_sizer.Add(self.optchar_eggFilesTree, 1, wx.ALL, 1)
        eggfiles_grid_sizer = wx.FlexGridSizer(4, 1, 0, 0)
        eggfiles_grid_sizer.Add(self.optchar_addEggBtn, 1, 0, 0)
        eggfiles_grid_sizer.Add(self.optchar_addFromBatchBtn, 1, 0, 0)
        eggfiles_grid_sizer.Add(self.optchar_removeEggBtn, 1, 0, 0)
        eggfiles_grid_sizer.Add(self.optchar_removeAllEggsBtn, 1, 0, 0)
        file_flex_grid_sizer.Add(eggfiles_grid_sizer, 1, 0, 0)

        file_flex_grid_sizer.Add(self.optchar_exportDestLbl, 1, wx.TOP, 0)
        file_flex_grid_sizer.Add(self.optchar_exportInPlaceChk, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add((10,2),0,0)
        file_flex_grid_sizer.Add(self.optchar_exportDirLbl, 1, wx.TOP|wx.ALIGN_RIGHT, 3)
        file_flex_grid_sizer.Add(self.optchar_exportDirTxt, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.optchar_exportDirBtn, 1, 0, 0)
        file_flex_grid_sizer.Add(self.optchar_exportFileLbl, 1, wx.TOP|wx.ALIGN_RIGHT, 3)
        file_flex_grid_sizer.Add(self.optchar_exportFileTxt, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.optchar_exportFileBtn, 1, 0, 0)

        eggOptChar_grid_sizer.Add(file_flex_grid_sizer, 1, wx.ALL, 0)
        self.eggOptChar_panel.SetSizer(eggOptChar_grid_sizer)
        self.tool_options_static_sizer.Add(self.eggOptChar_panel, 1, wx.ALL|wx.EXPAND, 0)

        self.optchar_options_sizer_staticbox.Lower()
        optchar_options_static_sizer = wx.StaticBoxSizer(self.optchar_options_sizer_staticbox, wx.VERTICAL)
        optchar_options_grid_sizer = wx.FlexGridSizer(6,1,0,0)

        joint_options_sizer = wx.FlexGridSizer(4,2,0,0)
        joint_options_sizer.Add(self.optchar_keepAllJointsChk, 1, wx.ALL, 3)
        joint_options_sizer.Add((10,0),0,0)
        joint_options_sizer.Add(self.optchar_keepJointsChk, 1, wx.ALL, 3)
        joint_options_sizer.Add(self.optchar_keepJointsTxt, 1, wx.LEFT, 2)
        joint_options_sizer.Add(self.optchar_dropJointsChk, 1, wx.ALL, 3)
        joint_options_sizer.Add(self.optchar_dropJointsTxt, 1, wx.LEFT, 2)
        joint_options_sizer.Add(self.optchar_exposeJointsChk, 1, wx.ALL, 3)
        joint_options_sizer.Add(self.optchar_exposeJointsTxt, 1, wx.LEFT, 2)
        joint_options_sizer.Add(self.optchar_flagGeometryChk, 1, wx.ALL, 3)
        joint_options_sizer.Add(self.optchar_flagGeometryTxt, 1, wx.LEFT, 2)
        optchar_options_grid_sizer.Add(joint_options_sizer, 1, wx.ALL, 0)
        optchar_options_grid_sizer.Add(self.optchar_dartChoice, 1, wx.ALL, 0)
        optchar_options_static_sizer.Add(optchar_options_grid_sizer)
        self.optchar_options_panel.SetSizer(optchar_options_static_sizer)
        eggOptChar_grid_sizer.Add(self.optchar_options_panel, 1, wx.TOP|wx.EXPAND, 10)

        # Egg-palettize
        eggPalettize_grid_sizer = wx.FlexGridSizer(8, 1, 0, 0)
        file_flex_grid_sizer = wx.FlexGridSizer(5, 3, 0, 0)
        file_flex_grid_sizer.Add(self.palettize_eggFilesLbl, 1, wx.TOP, 5)
        file_flex_grid_sizer.Add(self.palettize_eggFilesTree, 1, wx.ALL, 1)
        eggfiles_grid_sizer = wx.FlexGridSizer(4, 1, 0, 0)
        eggfiles_grid_sizer.Add(self.palettize_addEggBtn, 1, 0, 0)
        eggfiles_grid_sizer.Add(self.palettize_addFromBatchBtn, 1, 0, 0)
        eggfiles_grid_sizer.Add(self.palettize_removeEggBtn, 1, 0, 0)
        eggfiles_grid_sizer.Add(self.palettize_removeAllEggsBtn, 1, 0, 0)
        file_flex_grid_sizer.Add(eggfiles_grid_sizer, 1, 0, 0)

        file_flex_grid_sizer.Add(self.palettize_exportDestLbl, 1, wx.TOP, 0)
        file_flex_grid_sizer.Add(self.palettize_exportInPlaceChk, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add((10,2),0,0)
        file_flex_grid_sizer.Add(self.palettize_exportDirLbl, 1, wx.TOP|wx.ALIGN_RIGHT, 3)
        file_flex_grid_sizer.Add(self.palettize_exportDirTxt, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.palettize_exportDirBtn, 1, 0, 0)
        file_flex_grid_sizer.Add(self.palettize_exportFileLbl, 1, wx.TOP|wx.ALIGN_RIGHT, 3)
        file_flex_grid_sizer.Add(self.palettize_exportFileTxt, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.palettize_exportFileBtn, 1, 0, 0)
        file_flex_grid_sizer.Add(self.palettize_exportTexLbl, 1, wx.TOP|wx.ALIGN_RIGHT, 3)
        file_flex_grid_sizer.Add(self.palettize_exportTexTxt, 1, wx.ALL, 1)
        file_flex_grid_sizer.Add(self.palettize_exportTexBtn, 1, 0, 0)

        eggPalettize_grid_sizer.Add(file_flex_grid_sizer, 1, wx.ALL, 0)
        self.eggPalettize_panel.SetSizer(eggPalettize_grid_sizer)
        self.tool_options_static_sizer.Add(self.eggPalettize_panel, 1, wx.ALL|wx.EXPAND, 0)

        self.palettize_options_sizer_staticbox.Lower()
        palettize_options_static_sizer = wx.StaticBoxSizer(self.palettize_options_sizer_staticbox, wx.VERTICAL)
        palettize_options_grid_sizer = wx.FlexGridSizer(6,1,0,0)

        save_flex_grid_sizer = wx.FlexGridSizer(2, 3, 0, 0)
        save_flex_grid_sizer.Add(self.palettize_saveTxaLbl, 0, wx.TOP, 3)
        save_flex_grid_sizer.Add(self.palettize_saveTxaTxt, 0, wx.ALL, 1)
        save_flex_grid_sizer.Add(self.palettize_loadTxaBtn, 0, 0, 0)
        save_flex_grid_sizer.Add((10,2),0,0)
        save_flex_grid_sizer.Add((10,2),0,0)
        save_flex_grid_sizer.Add(self.palettize_saveTxaBtn, 0, 0, 0)
        palettize_options_grid_sizer.Add(save_flex_grid_sizer, 1, wx.TOP, 6)

        txa_grid_sizer = wx.FlexGridSizer(4,2,0,0)

        palettize_size_sizer = wx.FlexGridSizer(1, 5, 0, 0)
        palettize_size_sizer.Add(self.palettize_sizeWidthTxt, 0, 0, 0)
        palettize_size_sizer.Add(self.palettize_sizeByLbl, 0, wx.TOP, 3)
        palettize_size_sizer.Add(self.palettize_sizeHeightTxt, 0, wx.RIGHT, 10)
        palettize_size_sizer.Add(self.palettize_powerOf2Chk, 1, wx.ALL, 4)
        #palettize_options_grid_sizer.Add(palettize_size_sizer, 1, wx.ALL, 3)
        txa_grid_sizer.Add(self.palettize_sizeLbl, 0, wx.TOP|wx.ALIGN_RIGHT, 3)
        txa_grid_sizer.Add(palettize_size_sizer, 0, wx.ALL, 1)

        #palettize_options_grid_sizer.Add(self.palettize_imageTypeChoice, 1, wx.ALL, 0)
        txa_grid_sizer.Add(self.palettize_imageTypeLbl, 1, wx.TOP|wx.ALIGN_RIGHT, 16)
        txa_grid_sizer.Add(self.palettize_imageTypeChoice, 1, wx.ALL, 0)

        self.palettize_color_sizer_staticbox.Lower()
        color_static_sizer = wx.StaticBoxSizer(self.palettize_color_sizer_staticbox, wx.HORIZONTAL)
        color_flex_grid_sizer = wx.FlexGridSizer(1, 8, 0, 0)
        color_flex_grid_sizer.Add(self.palettize_redLbl, 0, wx.TOP, 3)
        color_flex_grid_sizer.Add(self.palettize_redTxt, 0, wx.RIGHT, 6)
        color_flex_grid_sizer.Add(self.palettize_greenLbl, 0, wx.TOP, 3)
        color_flex_grid_sizer.Add(self.palettize_greenTxt, 0, wx.RIGHT, 6)
        color_flex_grid_sizer.Add(self.palettize_blueLbl, 0, wx.TOP, 3)
        color_flex_grid_sizer.Add(self.palettize_blueTxt, 0, wx.RIGHT, 6)
        color_flex_grid_sizer.Add(self.palettize_alphaLbl, 0, wx.TOP, 3)
        color_flex_grid_sizer.Add(self.palettize_alphaTxt, 0,wx.RIGHT, 6)
        color_static_sizer.Add(color_flex_grid_sizer, 1, 0, 0)
        txa_grid_sizer.Add(self.palettize_colorLbl, 1, wx.TOP|wx.ALIGN_RIGHT, 13)
        txa_grid_sizer.Add(color_static_sizer, 1, wx.ALL, 0)

        txa_grid_sizer.Add(self.palettize_marginLbl, 1, wx.TOP|wx.ALIGN_RIGHT, 8)
        txa_grid_sizer.Add(self.palettize_marginTxt, 1, wx.TOP, 5)
        txa_grid_sizer.Add(self.palettize_coverageLbl, 1, wx.TOP|wx.ALIGN_RIGHT, 8)
        txa_grid_sizer.Add(self.palettize_coverageTxt, 1, wx.TOP, 5)

        palettize_options_grid_sizer.Add(txa_grid_sizer, 1, wx.ALL, 0)

        palettize_options_static_sizer.Add(palettize_options_grid_sizer)
        self.palettize_options_panel.SetSizer(palettize_options_static_sizer)
        eggPalettize_grid_sizer.Add(self.palettize_options_panel, 1, wx.TOP|wx.EXPAND, 10)

        # Batch List
        self.batch_static_sizer_staticbox.Lower()
        batch_static_sizer = wx.StaticBoxSizer(self.batch_static_sizer_staticbox, wx.VERTICAL)
        batch_grid_sizer = wx.FlexGridSizer(3,1,0,0)
        batch_buttons_sizer = wx.FlexGridSizer(1,6,0,0)
        batch_buttons_sizer.Add(self.loadBatchButton, 1, wx.ALL, 2)
        batch_buttons_sizer.Add(self.saveBatchButton, 1, wx.ALL, 2)
        batch_buttons_sizer.Add(self.sortBatchButton, 1, wx.ALL, 2)
        batch_buttons_sizer.Add(self.changePathsButton, 1, wx.ALL, 2)
        batch_buttons_sizer.Add((25,0),0,0)
        batch_grid_sizer.Add(batch_buttons_sizer, 1, wx.ALIGN_LEFT, 0)
        batch_grid_sizer.Add(self.batchTree, 1, wx.ALL|wx.ALIGN_LEFT|wx.EXPAND, 2)
        batch_controls_sizer = wx.FlexGridSizer(1,3,0,0)
        batch_controls_sizer.Add(self.editSelBatchButton, 1, wx.ALL, 2)
        batch_controls_sizer.Add(self.removeSelBatchButton, 1, wx.ALL, 2)
        batch_controls_sizer.Add(self.removeAllBatchButton, 1, wx.ALL, 2)
        batch_grid_sizer.Add(batch_controls_sizer, 1, wx.ALIGN_LEFT, 0)
        batch_static_sizer.Add(batch_grid_sizer,1 , wx.EXPAND)
        self.batch_panel.SetSizer(batch_static_sizer)
        top_right_sizer.Add(self.batch_panel, 1, wx.ALIGN_TOP|wx.EXPAND, 0)

        # Console Output
        self.console_static_sizer_staticbox.Lower()
        console_static_sizer = wx.StaticBoxSizer(self.console_static_sizer_staticbox, wx.VERTICAL)
        console_grid_sizer = wx.FlexGridSizer(2,1,0,0)

        console_controls_sizer = wx.BoxSizer(wx.HORIZONTAL)

        console_buttons_sizer = wx.FlexGridSizer(1,6,0,0)
        console_buttons_sizer.Add(self.runBatchButton, 1, wx.ALL, 2)
        console_buttons_sizer.Add(self.clearConsoleButton, 1, wx.ALL, 2)
        console_buttons_sizer.Add(self.runPviewButton, 1, wx.ALL, 2)
        console_controls_sizer.Add(console_buttons_sizer)

        console_options_sizer = wx.FlexGridSizer(2,1,0,0)
        console_options_sizer.Add(self.ignoreModDates, 0, wx.ALL, 0)
        panda_dir_sizer = wx.BoxSizer(wx.HORIZONTAL)
        panda_dir_sizer.Add(self.pathLbl, 0, wx.TOP, 4)
        panda_dir_sizer.Add(self.pandaPathTxt, 0, wx.TOP, 1)
        panda_dir_sizer.Add(self.loadPandaPathBtn, 0, wx.ALL, 0)
        console_options_sizer.Add(panda_dir_sizer, 0, 0, 0)
        console_controls_sizer.Add(console_options_sizer, 0, wx.LEFT|wx.ALIGN_RIGHT, 25)

        main_sizer.Add(self.console_panel, -1, wx.ALIGN_TOP|wx.EXPAND|wx.RIGHT|wx.BOTTOM, 5)
        self.console_panel.SetSizer(console_static_sizer)
        console_grid_sizer.Add(console_controls_sizer, 1, wx.ALIGN_LEFT, 0)
        console_grid_sizer.Add(self.consoleOutputTxt, 1, wx.ALIGN_LEFT|wx.EXPAND|wx.TOP|wx.BOTTOM, 3)
        console_static_sizer.Add(console_grid_sizer, 1, wx.EXPAND)

        self.main_panel.SetSizer(main_sizer)

        #These are the notebook pages (or where they are added)
        #add more for more tools (ie dae2egg etc).
        #NOTE you must create the panel in setup UI before
        #adding to the sizer
        self.tab_panel.AddPage(self.simple_panel, "Simple Mode")
        self.tab_panel.AddPage(self.main_panel, "Advanced Mode")
        tab_panel_sizer.Add(self.tab_panel,1,wx.EXPAND,0)
        self.SetSizer(tab_panel_sizer)
        tab_panel_sizer.Fit(self)

        self.SetSize((WINDOW_WIDTH, WINDOW_HEIGHT))
        self.Layout()

    def ShowInitialEnv(self):#show the initial environment of the GUI
        self.consoleOutputTxt.AppendText(WELCOME_MSG)
        self.pandaPathDir = DEFAULT_PANDA_DIR
        self.consoleOutputTxt.AppendText( "\nPanda initial path: " + self.pandaPathDir)
        self.consoleOutputTxt.AppendText("\nIf this is not correct please use the Path options to change.")
        self.m2e_mayaVerComboBox.SetSelection(len(MAYA_VERSIONS)-1)
        self.consoleOutputTxt.AppendText("\nUsing Maya Version " + MAYA_VERSIONS[len(MAYA_VERSIONS)-1])
        self.consoleOutputTxt.AppendText("\nIf this is not correct please use the Path options to change.")
        self.pandaPathTxt.SetValue(self.pandaPathDir)

        self.egg2bam_panel.Show(False)
        self.eggRename_panel.Show(False)
        self.eggOptChar_panel.Show(False)
        self.eggPalettize_panel.Show(False)
        self.former = 'maya2egg'
        self.current = 'maya2egg'
        self.main_panel.Layout()


    def OnTool(self,e):
        #change panels of different tools
        self.former = self.current
        self.current = self.toolComboBox.GetValue()

        if self.former == 'maya2egg':
            self.maya2egg_panel.Show(False)
        if self.former == 'egg2bam':
            self.egg2bam_panel.Show(False)
        if self.former == 'egg-rename':
            self.eggRename_panel.Show(False)
        if self.former == 'egg-optchar':
            self.eggOptChar_panel.Show(False)
        if self.former == 'egg-palettize':
            self.eggPalettize_panel.Show(False)

        if self.current == 'maya2egg':
            self.maya2egg_panel.Show(True)
        if self.current == 'egg2bam':
            self.egg2bam_panel.Show(True)
        if self.current == 'egg-rename':
            self.eggRename_panel.Show(True)
        if self.current == 'egg-optchar':
            self.eggOptChar_panel.Show(True)
        if self.current == 'egg-palettize':
            self.eggPalettize_panel.Show(True)

        self.batchItemNameTxt.SetValue('')

        self.main_panel.Layout()

    def OnShowHelp(self,event):#show help files
        def _addBook(filename):
            if not self.help.AddBook(filename):
                wx.MessageBox("Unable to open: " + filename,
                              "Error", wx.OK|wx.ICON_EXCLAMATION)

        self.help = wx.html.HtmlHelpController()

        _addBook("helpfiles/help.hhp")

        self.help.DisplayContents()

    def OnPandaPathChoose(self, event):
        #Choose Panda directory
        dlg = wx.DirDialog(self, "Choose your Panda directory:")
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.pandaPathTxt.SetValue(dlg.GetPath())
        dlg.Destroy() #otherwise just kill the file dialog

    def OnMayaVerChoose(self, event):
        #choose maya version
        event.Skip()


    # Simple Export Panel Functions
    ################################

    def OnSimpleExport(self,e):
        #This is essentially the same behavior as a batch export
        #build up the dictionary that gets passed to Run Export
        #with all relevant items to a given command and
        #pass it to RunCommand along with the batchmode boolean
        finput = self.simple_mayaFileTxt.GetValue()
        foutput = self.simple_exportDestTxt.GetValue()
        if ((finput == "") or (foutput == "")):
            dlg = wx.MessageDialog(self,"Both an input and output file must be present to perform the export" ,"ERROR", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
            return

        item = {}
        item['cmd'] = 'maya2egg'+self.simple_mayaVerComboBox.GetStringSelection()
        item['finput'] = str(finput)
        item['foutput'] = str(foutput)
        item['args'] = { 'a' : self.simple_animOptChoice.GetStringSelection() }

        self.outdlg.Show()
        self.RunCommand(self.BuildCommand(item), False)


    def OnSimpleMayaFile(self,e):
        #choose the input maya scene for simple mode
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a location and filename", dirname, "", "*.mb", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            self.simple_mayaFileTxt.SetValue(os.path.join(dirname + os.sep, filename))
        dlg.Destroy() #otherwise just kill the file dialog

    def OnSimpleExportDest(self,e):
        #choose the output egg file for simple mode
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a location and filename", dirname, "", "*.egg", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            self.simple_exportDestTxt.SetValue(os.path.join(dirname + os.sep, filename))
        dlg.Destroy() #otherwise just kill the file dialog


    # Maya2Egg Panel Functions
    ###########################

    def OnMaya2EggMayaFile(self, event):
        #choose input maya scene for maya2egg
        filename = ''
        if (self.m2e_mayaFileTxt.GetValue()!= ''):
            self.srcProjectFolder = ''
            for item in self.m2e_mayaFileTxt.GetValue().split('\\'):
                self.srcProjectFolder += item + "\\"
            self.srcProjectFolder = self.srcProjectFolder.split(item+'\\')[0]

        dlg = wx.FileDialog(self, "Choose an Egg file to load", self.srcProjectFolder, "", "*.mb", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            self.srcProjectFolder = dlg.GetDirectory()
            self.m2e_mayaFileTxt.SetValue(os.path.join(self.srcProjectFolder + os.sep , filename)) #this is for a text control
        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("Current Scene File is: " + self.srcProjectFolder + os.sep + filename)

    def OnMaya2EggExportDest(self, event):
        #choose output egg for maya2egg
        filename = ''
        if (self.m2e_exportDestTxt.GetValue()!= ''):
            self.destProjectFolder = ''
            for item in self.m2e_exportDestTxt.GetValue().split('\\'):
                self.destProjectFolder += item + "\\"
            self.destProjectFolder = self.destProjectFolder.split(item+'\\')[0]

        dlg = wx.FileDialog(self, "Choose an Egg file to load", self.destProjectFolder, "", "*.egg", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            self.destProjectFolder = dlg.GetDirectory()
            self.m2e_exportDestTxt.SetValue(os.path.join(self.destProjectFolder + os.sep , filename)) #this is for a text control
        dlg.Destroy() #otherwise just kill the file dialog
        if (self.batchItemNameTxt.GetValue() == '' and len(filename) > 4):
            self.batchItemNameTxt.SetValue(filename[:-4])
        self.statusBar.SetStatusText("Current Egg File is: " + self.destProjectFolder + os.sep + filename)

    def OnMaya2EggAnimOpt(self,event):
        #get value from copytexture check box
        if (self.m2e_animOptChoice.GetStringSelection() == 'none'):
            self.m2e_startFrameChk.SetValue(0)
            self.m2e_endFrameChk.SetValue(0)
            self.m2e_frameRateInChk.SetValue(0)
            self.m2e_frameRateOutChk.SetValue(0)
            self.m2e_charNameChk.SetValue(0)

    def OnMaya2EggCopyTexPath(self,event):
        #choose texture copy output directory
        dirname = self.destProjectFolder
        if (self.m2e_copyTexPathTxt.GetValue()!= ''):
            dirname = ''
            for item in self.m2e_copyTexPathTxt.GetValue().split('\\'):
                dirname += item + "\\"
            dirname = dirname.split(item+'\\')[0]

        dlg = wx.DirDialog(self, "Choose your output directory:", dirname)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.m2e_copyTexPathTxt.SetValue(dlg.GetPath())
        dlg.Destroy() #otherwise just kill the file dialog
        #self.statusBar.SetStatusText("Current Egg File is: " + dirname + '\\' + filename)

    def OnMaya2EggPathReplace(self,event):
        batchList = self.GetSelectedBatchList()
        for batchItem in batchList:
            if batchItem['cmd'].count('maya2egg'):
                if self.m2e_pathReplaceChk.GetValue():
                    batchItem['args']['pr'] = self.m2e_pathReplaceTxt.GetValue()
                elif batchItem['args'].has_key('pr'):
                    del batchItem['args']['pr']
        self.UpdateBatchDisplay()


    # Egg2Bam Panel Functions
    ##########################

    def OnEgg2BamEggFile(self, event):
        #choose the input egg file for egg2bam
        filename = ''
        dirname = ''
        dlg = wx.FileDialog(self, "Choose an Egg file to BAM", dirname, "", "*.egg", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            self.e2b_eggFileTxt.SetValue(os.path.join(dirname + os.sep , self.filename)) #this is for a text control
        dlg.Destroy() #otherwise just kill the file dialog

    def OnEgg2BamExportDest(self, event):
        #choose the output bam file for egg2bam
        dirname = ''
        dlg = wx.FileDialog(self, "Choose an Egg file to load", dirname, "", "*.bam", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            self.e2b_exportDestTxt.SetValue(os.path.join(dirname + os.sep , self.filename)) #this is for a text control duh
        dlg.Destroy() #otherwise just kill the file dialog

    def OnEgg2BamUseCurrEgg(self, event):
        #check if we want to use a custom egg file or not for egg2bam panel
        if (not self.e2b_useCurrEggChk.GetValue()):
            self.e2b_eggFileTxt.Enable()
            self.e2b_eggFileBtn.Enable()
        else:
            self.e2b_eggFileTxt.Disable()
            self.e2b_eggFileBtn.Disable()

    def OnEgg2BamBatchOutput(self, event):
        for eggInfo in self.GetOutputFromBatch():
            batchItemInfo = {}
            batchItemInfo['cmd'] = 'egg2bam'
            batchItemInfo['args'] = self.BuildEgg2BamArgs()
            batchItemInfo['finput'] = str(eggInfo)

            dirname = ''
            for item in eggInfo.split('\\'):
                dirname += item + "\\"
            dirname = dirname.split(item+'\\')[0]
            batchItemInfo['foutput'] = str(dirname+item[:-4]+'.bam')
            batchItemInfo['label'] = item[:-4]

            self.batchList.append(batchItemInfo)
            self.AddToBatchDisplay(batchItemInfo)


    # Egg-Rename Panel Functions
    ##############################

    def OnRenameAddEgg(self, event):
        filename = ''
        dirname = self.destProjectFolder
        dlg = wx.FileDialog(self, "Choose your input egg files", dirname, "", "*.egg", wx.MULTIPLE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filenames = dlg.GetFilenames()
            dirname = dlg.GetDirectory()

            for filename in filenames:
                eggInfo = os.path.join(dirname + os.sep , filename)
                self.rename_eggList.append(eggInfo)
                self.rename_eggFilesTree.AppendItem(self.rename_eggFilesRoot,str(len(self.rename_eggList)) + ' ' + eggInfo)
                self.rename_eggFilesTree.ExpandAll()

        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("The input egg File is: " + dirname + os.sep + filename)
        self.OnRenameInPlace(None)

    def OnRenameAddFromBatch(self, event):
        for eggInfo in self.GetOutputFromBatch():
            self.rename_eggList.append(eggInfo)
            self.rename_eggFilesTree.AppendItem(self.rename_eggFilesRoot, str(len(self.rename_eggList)) + ' ' + eggInfo)
            self.rename_eggFilesTree.ExpandAll()
        self.OnRenameInPlace(None)

    def OnRenameRemoveEgg(self, event):
        #remove selected input egg file from eggs list in egg-rename panel
        item = self.rename_eggFilesTree.GetSelection()
        if (item != self.rename_eggFilesRoot):
            index  = self.rename_eggFilesTree.GetItemText(item).split()[0]
            index = int(index) - 1
            self.rename_eggList.pop(index)
            self.rename_eggFilesTree.Delete(item)
            self.UpdateEggRenameDisplay()
        self.OnRenameInPlace(None)

    def OnRenameRemoveAllEggs(self, event):
        #remove all input egg files from eggs list in egg-rename panel
        self.rename_eggFilesTree.DeleteAllItems()
        if self.rename_eggList != []:
            self.rename_eggList = []
        self.rename_eggFilesRoot = self.rename_eggFilesTree.AddRoot('Egg Files')
        self.OnRenameInPlace(None)

    def UpdateEggRenameDisplay(self):
        #update the tree display of egg file list in egg-rename panel
        self.rename_eggFilesTree.DeleteAllItems()
        self.rename_eggFilesRoot = self.rename_eggFilesTree.AddRoot('Egg Files')
        index = 0
        for item in self.rename_eggList:
            index += 1
            treeitem = item
            self.rename_eggFilesTree.AppendItem(self.rename_eggFilesRoot, str(index) + ' ' + str(treeitem))
        self.rename_eggFilesTree.ExpandAll()
        self.OnRenameInPlace(None)

    def OnRenameInPlace(self, event):
        #check if we want to use a custom egg file or not for egg-rename panel
        if (self.rename_exportInPlaceChk.GetValue()):
            self.rename_exportDirTxt.SetValue("")
            self.rename_exportDirTxt.Disable()
            self.rename_exportDirBtn.Disable()
            self.rename_exportFileTxt.SetValue("")
            self.rename_exportFileTxt.Disable()
            self.rename_exportFileBtn.Disable()
        else:
            if (len(self.rename_eggList) > 1):
                self.rename_exportDirTxt.Enable()
                self.rename_exportDirBtn.Enable()
                self.rename_exportFileTxt.Disable()
                self.rename_exportFileBtn.Disable()
            else:
                self.rename_exportDirTxt.Enable()
                self.rename_exportDirBtn.Enable()
                self.rename_exportFileTxt.Enable()
                self.rename_exportFileBtn.Enable()

    def OnRenameExportFile(self, event):
        #choose output egg for egg-rename panel
        if (self.rename_exportFileTxt.GetValue()!= ''):
            self.destProjectFolder = ''
            for item in self.rename_exportFileTxt.GetValue().split('\\'):
                self.destProjectFolder += item + "\\"
            self.destProjectFolder = self.destProjectFolder.split(item+'\\')[0]

        dlg = wx.FileDialog(self, "Choose a location and filename", self.destProjectFolder, "", "*.egg", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            self.destProjectFolder = dlg.GetDirectory()
            self.rename_exportFileTxt.SetValue(os.path.join(self.destProjectFolder + os.sep, filename))
        dlg.Destroy() #otherwise just kill the file dialog

    def OnRenameExportDir(self, event):
        if (self.rename_exportDirTxt.GetValue()!= ''):
            self.destProjectFolder = self.rename_exportDirTxt.GetValue()

        #choose the output directory in egg-rename panel
        dlg = wx.DirDialog(self, "Choose the output directory:", self.destProjectFolder)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.destProjectFolder = dlg.GetPath()
            self.rename_exportDirTxt.SetValue(dlg.GetPath())
        dlg.Destroy()



    # Egg-Optchar Panel Functions
    ##############################

    def OnOptcharAddEgg(self, event):
        filename = ''
        dirname = ''
        dlg = wx.FileDialog(self, "Choose your input egg files", dirname, "", "*.egg", wx.MULTIPLE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filenames = dlg.GetFilenames()
            dirname = dlg.GetDirectory()

            for filename in filenames:
                eggInfo = os.path.join(dirname + os.sep , filename)
                self.optchar_eggList.append(eggInfo)
                self.optchar_eggFilesTree.AppendItem(self.optchar_eggFilesRoot,str(len(self.optchar_eggList)) + ' ' + eggInfo)
                self.optchar_eggFilesTree.ExpandAll()

        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("The input egg File is: " + dirname + os.sep + filename)
        self.OnOptcharInPlace(None)

    def OnOptcharAddFromBatch(self, event):
        for eggInfo in self.GetOutputFromBatch():
            self.optchar_eggList.append(eggInfo)
            self.optchar_eggFilesTree.AppendItem(self.optchar_eggFilesRoot, str(len(self.optchar_eggList)) + ' ' + eggInfo)
            self.optchar_eggFilesTree.ExpandAll()
        self.OnOptcharInPlace(None)

    def OnOptcharRemoveEgg(self, event):
        #remove selected input egg file from eggs list in egg-optchar panel
        item = self.optchar_eggFilesTree.GetSelection()
        if (item != self.optchar_eggFilesRoot):
            index  = self.optchar_eggFilesTree.GetItemText(item).split()[0]
            index = int(index) - 1
            self.optchar_eggList.pop(index)
            self.optchar_eggFilesTree.Delete(item)
            self.UpdateEggOptcharDisplay()
        self.OnOptcharInPlace(None)

    def OnOptcharRemoveAllEggs(self, event):
        #remove all input egg files from eggs list in egg-optchar panel
        self.optchar_eggFilesTree.DeleteAllItems()
        if self.optchar_eggList != []:
            self.optchar_eggList = []
        self.optchar_eggFilesRoot = self.optchar_eggFilesTree.AddRoot('Egg Files')
        self.OnOptcharInPlace(None)

    def UpdateEggOptcharDisplay(self):
        #update the tree display of egg file list in egg-optchar panel
        self.optchar_eggFilesTree.DeleteAllItems()
        self.optchar_eggFilesRoot = self.optchar_eggFilesTree.AddRoot('Egg Files')
        index = 0
        for item in self.optchar_eggList:
            index += 1
            treeitem = item
            self.optchar_eggFilesTree.AppendItem(self.optchar_eggFilesRoot, str(index) + ' ' + str(treeitem))
        self.optchar_eggFilesTree.ExpandAll()
        self.OnOptcharInPlace(None)

    def OnOptcharInPlace(self, event):
        #check if we want to use a custom egg file or not for egg-optchar panel
        if (self.optchar_exportInPlaceChk.GetValue()):
            self.optchar_exportDirTxt.SetValue("")
            self.optchar_exportDirTxt.Disable()
            self.optchar_exportDirBtn.Disable()
            self.optchar_exportFileTxt.SetValue("")
            self.optchar_exportFileTxt.Disable()
            self.optchar_exportFileBtn.Disable()
        else:
            if (len(self.optchar_eggList) > 1):
                self.optchar_exportDirTxt.Enable()
                self.optchar_exportDirBtn.Enable()
                self.optchar_exportFileTxt.Disable()
                self.optchar_exportFileBtn.Disable()
            else:
                self.optchar_exportDirTxt.Enable()
                self.optchar_exportDirBtn.Enable()
                self.optchar_exportFileTxt.Enable()
                self.optchar_exportFileBtn.Enable()

    def OnOptcharExportFile(self, event):
        #choose output egg for egg-optchar panel
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a location and filename", dirname, "", "*.egg", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            self.optchar_exportFileTxt.SetValue(os.path.join(dirname + os.sep, filename))
        dlg.Destroy() #otherwise just kill the file dialog

    def OnOptcharExportDir(self, event):
        #choose the output directory in egg-optchar panel
        dlg = wx.DirDialog(self, "Choose the output directory:")
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.optchar_exportDirTxt.SetValue(dlg.GetPath())
        dlg.Destroy()


    # Egg-Palettize Panel Functions
    ################################
    def OnPalettizeAddEgg(self,e):
        #add input egg file in egg-palettize panel
        filename = ''
        dirname = ''
        dlg = wx.FileDialog(self, "Choose your input egg files", dirname, "", "*.egg", wx.MULTIPLE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filenames = dlg.GetFilenames()
            dirname = dlg.GetDirectory()

            for filename in filenames:
                eggInfo = os.path.join(dirname + os.sep , filename)
                self.palettize_eggList.append(eggInfo)
                self.palettize_eggFilesTree.AppendItem(self.palettize_eggFilesRoot,str(len(self.palettize_eggList)) + ' ' + eggInfo)
                self.palettize_eggFilesTree.ExpandAll()

        dlg.Destroy() #otherwise just kill the file dialog
        self.OnPalettizeInPlace(None)

    def OnPalettizeAddFromBatch(self, event):
        for eggInfo in self.GetOutputFromBatch():
            self.palettize_eggList.append(eggInfo)
            self.palettize_eggFilesTree.AppendItem(self.palettize_eggFilesRoot, str(len(self.palettize_eggList)) + ' ' + eggInfo)
            self.palettize_eggFilesTree.ExpandAll()
        self.OnPalettizeInPlace(None)

    def OnPalettizeRemoveEgg(self,e):
        #remove selected input egg file from multiple eggs panel in egg-palettize panel
        item = self.palettize_eggFilesTree.GetSelection()
        if (item != self.palettize_eggFilesRoot):
            index  = self.palettize_eggFilesTree.GetItemText(item)[0]
            index = int(index) - 1
            self.palettize_eggList.pop(index)
            self.palettize_eggFilesTree.Delete(item)
            self.UpdateEggPalettizeDisplay()
        self.OnPalettizeInPlace(None)

    def OnPalettizeRemoveAllEggs(self,event):
        #remove all input egg file from multiple eggs panel in egg-palettize panel
        self.palettize_eggFilesTree.DeleteAllItems()
        if self.palettize_eggList != []:
            self.palettize_eggList = []
        self.palettize_eggFilesRoot = self.palettize_eggFilesTree.AddRoot('Egg Files')
        self.OnPalettizeInPlace(None)

    def UpdateEggPalettizeDisplay(self):
        #update the output of the selected egg files of multiple eggs panel in egg-palettize panel
        self.palettize_eggFilesTree.DeleteAllItems()
        self.palettize_eggFilesRoot = self.palettize_eggFilesTree.AddRoot('Egg Files')
        index = 0
        for item in self.palettize_eggList:
            index += 1
            treeitem = item
            self.palettize_eggFilesTree.AppendItem(self.palettize_eggFilesRoot, str(index) + ' ' + str(treeitem))
        self.palettize_eggFilesTree.ExpandAll()
        self.OnPalettizeInPlace(None)

    def OnPalettizeInPlace(self, event):
        #check if we want to use a custom egg file or not for egg-palettize panel
        if (self.palettize_exportInPlaceChk.GetValue()):
            self.palettize_exportDirTxt.SetValue("")
            self.palettize_exportDirTxt.Disable()
            self.palettize_exportDirBtn.Disable()
            self.palettize_exportFileTxt.SetValue("")
            self.palettize_exportFileTxt.Disable()
            self.palettize_exportFileBtn.Disable()
        else:
            if (len(self.palettize_eggList) > 1):
                self.palettize_exportDirTxt.Enable()
                self.palettize_exportDirBtn.Enable()
                self.palettize_exportFileTxt.Disable()
                self.palettize_exportFileBtn.Disable()
            else:
                self.palettize_exportDirTxt.Enable()
                self.palettize_exportDirBtn.Enable()
                self.palettize_exportFileTxt.Enable()
                self.palettize_exportFileBtn.Enable()

    def OnPalettizeExportFile(self, event):
        #choose output egg for egg-palettize panel
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a location and filename", dirname, "", "*.egg", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            self.palettize_exportFileTxt.SetValue(os.path.join(dirname + os.sep, filename))
        dlg.Destroy() #otherwise just kill the file dialog

    def OnPalettizeExportDir(self, event):
        #choose the output directory in egg-palettize panel
        dlg = wx.DirDialog(self, "Choose the output directory:")
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.palettize_exportDirTxt.SetValue(dlg.GetPath())
        dlg.Destroy()

    def OnPalettizeExportTex(self, event):
        #choose the output texture directory of single egg panel in egg-palettize panel
        dlg = wx.DirDialog(self, "Choose your Output Texture directory:")
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.palettize_exportTexTxt.SetValue(dlg.GetPath())
        dlg.Destroy()

    def OnPalettizeLoadTxa(self, event):
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a .txa file to use", dirname, "", "*.txa", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            try:
                self.txaExtraLines = []
                txafile = open((dirname + os.sep + filename), 'r')
                for line in txafile:
                    words = line.split()
                    if len(words):
                        if words[0] == ':palette':
                            self.palettize_sizeWidthTxt.SetValue(words[1])
                            self.palettize_sizeHeightTxt.SetValue(words[2])
                        elif words[0] == ':imagetype':
                            self.palettize_imageTypeChoice.SetStringSelection(words[1])
                        elif words[0] == ':powertwo':
                            self.palettize_powerOf2Chk.SetValue(int(words[1]))
                        elif words[0] == ':background':
                            self.palettize_redTxt.SetValue(int(words[1]))
                            self.palettize_greenTxt.SetValue(int(words[2]))
                            self.palettize_blueTxt.SetValue(int(words[3]))
                            self.palettize_alphaTxt.SetValue(int(words[4]))
                        elif words[0] == ':margin':
                            self.palettize_marginTxt.SetValue(int(words[1]))
                        elif words[0] == ':coverage':
                            self.palettize_coverageTxt.SetValue(words[1])
                        else:
                            self.txaExtraLines.append(line)
                txafile.close()
            except:
                print("Error opening .txa file!")
            self.palettize_saveTxaTxt.SetValue(os.path.join(dirname + os.sep , filename))
        dlg.Destroy()


    def OnPalettizeSaveTxa(self,event):
        #save all the attributes to the .txa file for egg-palettize panel
        filename = ''
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a location and filename", dirname, "", "*.txa", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            file = dirname + os.sep + filename
            myfile = open(file, 'w')
            mydata = {}
            mydata =[':palette ']
            mydata+=str(str(self.palettize_sizeWidthTxt.GetValue()))
            mydata+=[' ']
            mydata+=str(str(self.palettize_sizeHeightTxt.GetValue()))
            mydata+=['\n:imagetype ']
            mydata+=[self.palettize_imageTypeChoice.GetStringSelection()]
            mydata+=['\n:powertwo ']
            mydata+=str(int(self.palettize_powerOf2Chk.GetValue()))
            mydata+=['\n:background ']
            mydata+=str(int(self.palettize_redTxt.GetValue()))
            mydata+=[' ']
            mydata+=str(int(self.palettize_greenTxt.GetValue()))
            mydata+=[' ']
            mydata+=str(int(self.palettize_blueTxt.GetValue()))
            mydata+=[' ']
            mydata+=str(int(self.palettize_alphaTxt.GetValue()))
            mydata+=['\n:margin ']
            mydata+=str(str(int(self.palettize_marginTxt.GetValue())))
            mydata+=['\n:coverage ']
            mydata+=str(str(self.palettize_coverageTxt.GetValue()))
            mydata+=['\n\n']
            for line in mydata:
                myfile.writelines(line)
            for line in self.txaExtraLines:
                myfile.writelines(line)
            myfile.close()
            self.palettize_saveTxaTxt.SetValue(os.path.join(dirname + os.sep , filename)) #this is for a text control
        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("The output Txa File is: " + dirname + os.sep + filename)


    # Batch Related Functions
    ##########################

    def GetSelectedBatchList(self):
        batchList = []
        selectedItems = self.batchTree.GetSelections()
        for item in selectedItems:
            selectedItemIndex = int(self.batchTree.GetItemText(item).split()[0])-1
            batchList.append(self.batchList[selectedItemIndex])
        if not len(batchList):
            batchList = self.batchList

        return batchList

    def GetOutputFromBatch(self, index=-1):
        #Extract all output files from selected items in the batch
        if (index != -1):
            batchList = [self.batchList[index]]
        else:
            batchList = self.GetSelectedBatchList()
        outputFiles = []
        for batchItem in batchList:
            if batchItem['cmd'].count('maya2egg'):
                outputFiles.append(batchItem['foutput'])
            elif batchItem['cmd'] in ['egg-rename', 'egg-optchar', 'egg-palettize']:
                if batchItem['args'].has_key('inplace'):
                    for filename in batchItem['finput'].split('|'):
                        if filename:
                            outputFiles.append(filename)
                elif batchItem['args'].has_key('d'):
                    dirname = batchItem['args']['d']
                    if dirname[-1] != '\\':
                        dirname += '\\'
                    for filename in batchItem['finput'].split('|'):
                        if filename:
                            #seperate name from the whole path
                            for item in filename.split('\\'):
                                pass
                            filename = item
                            outputFiles.append(dirname+filename)
                else:
                    outputFiles.append(batchItem['foutput'])

        return outputFiles


    def OnAddToBatch(self, event):
        currTool = self.toolComboBox.GetValue()
        if (currTool == 'maya2egg'):
            self.addMaya2EggToBatch()
            if (self.m2e_addEgg2BamChk.GetValue()):
                self.e2b_eggFileTxt.SetValue(self.m2e_exportDestTxt.GetValue())
                self.e2b_exportDestTxt.SetValue(self.m2e_exportDestTxt.GetValue()[:-3]+"bam")
                self.addEgg2BamToBatch()
        elif (currTool == 'egg2bam'):
            self.addEgg2BamToBatch()
        elif (currTool == 'egg-rename'):
            self.addEggRenameToBatch()
        elif (currTool == 'egg-optchar'):
            self.addEggOptcharToBatch()
        elif (currTool == 'egg-palettize'):
            self.addEggPalettizeToBatch()

    def addMaya2EggToBatch(self, editItemIndex=-1):
        #add command line to batch list for maya2egg panel
        finput = self.m2e_mayaFileTxt.GetValue()
        foutput = self.m2e_exportDestTxt.GetValue()
        if ((finput == "") or (foutput == "")):
            dlg = wx.MessageDialog(self,"Both an input and output file must be present to add an item to the batch queue" ,"ERROR", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
            return

        batchItemInfo = {}
        batchItemInfo['label'] = self.batchItemNameTxt.GetValue()
        batchItemInfo['cmd'] = 'maya2egg' + self.m2e_mayaVerComboBox.GetStringSelection()
        batchItemInfo['args'] = self.BuildMaya2EggArgs()
        batchItemInfo['finput'] = str(finput)
        batchItemInfo['foutput'] = str(foutput)

        if (editItemIndex>=0):
            self.batchList[editItemIndex] = batchItemInfo
            self.UpdateBatchDisplay()
        else:
            self.batchList.append(batchItemInfo)
            self.AddToBatchDisplay(batchItemInfo)

    def addEgg2BamToBatch(self, editItemIndex=-1):
        #add command line to batch list for egg2bam panel
        finput = ''
        if (self.e2b_useCurrEggChk.GetValue()):
            finput = self.m2e_exportDestTxt.GetValue()
        else:
            finput = self.e2b_eggFileTxt.GetValue()
        foutput = self.e2b_exportDestTxt.GetValue()
        if ((finput == "") or (foutput == "")):
            dlg = wx.MessageDialog(self,"Both an input and output file must be present to add an item to the batch queue" ,"ERROR", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
            return

        batchItemInfo = {}
        batchItemInfo['label'] = self.batchItemNameTxt.GetValue()
        batchItemInfo['cmd'] = 'egg2bam'
        batchItemInfo['args'] = self.BuildEgg2BamArgs()
        batchItemInfo['finput'] = str(finput)
        batchItemInfo['foutput'] = str(foutput)

        if (editItemIndex>=0):
            self.batchList[editItemIndex] = batchItemInfo
            self.UpdateBatchDisplay()
        else:
            self.batchList.append(batchItemInfo)
            self.AddToBatchDisplay(batchItemInfo)

    def addEggRenameToBatch(self, editItemIndex=-1):
        finput = ''
        if (len(self.rename_eggList) == 0):
            dlg = wx.MessageDialog(self, "At least one input egg file must be present to add an item to the batch queue" ,"ERROR", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
            return
        for file in self.rename_eggList:
            finput += str(file) + '|'

        foutput = self.rename_exportFileTxt.GetValue()
        if not self.rename_exportInPlaceChk.GetValue():
            if ((not self.rename_exportFileTxt.GetValue()) and (not self.rename_exportDirTxt.GetValue())):
                dlg = wx.MessageDialog(self, "Export destination must be specified to add an item to the batch queue" ,"ERROR", wx.OK )
                dlg.ShowModal()
                dlg.Destroy()
                return

        batchItemInfo = {}
        batchItemInfo['label'] = self.batchItemNameTxt.GetValue()
        batchItemInfo['cmd'] = 'egg-rename'
        batchItemInfo['args'] = self.BuildEggRenameArgs()
        batchItemInfo['finput'] = str(finput)
        batchItemInfo['foutput'] = str(foutput)

        if (editItemIndex>=0):
            self.batchList[editItemIndex] = batchItemInfo
            self.UpdateBatchDisplay()
        else:
            self.batchList.append(batchItemInfo)
            self.AddToBatchDisplay(batchItemInfo)

    def addEggOptcharToBatch(self, editItemIndex=-1):
        finput = ''
        if (len(self.optchar_eggList) == 0):
            dlg = wx.MessageDialog(self, "At least one input egg file must be present to add an item to the batch queue" ,"ERROR", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
            return
        for file in self.optchar_eggList:
            finput += str(file) + '|'

        foutput = self.optchar_exportFileTxt.GetValue()
        if not self.optchar_exportInPlaceChk.GetValue():
            if ((not self.optchar_exportFileTxt.GetValue()) and (not self.optchar_exportDirTxt.GetValue())):
                dlg = wx.MessageDialog(self, "Export destination must be specified to add an item to the batch queue" ,"ERROR", wx.OK )
                dlg.ShowModal()
                dlg.Destroy()
                return

        batchItemInfo = {}
        batchItemInfo['label'] = self.batchItemNameTxt.GetValue()
        batchItemInfo['cmd'] = 'egg-optchar'
        batchItemInfo['args'] = self.BuildEggOptcharArgs()
        batchItemInfo['finput'] = str(finput)
        batchItemInfo['foutput'] = str(foutput)

        if (editItemIndex>=0):
            self.batchList[editItemIndex] = batchItemInfo
            self.UpdateBatchDisplay()
        else:
            self.batchList.append(batchItemInfo)
            self.AddToBatchDisplay(batchItemInfo)

    def addEggPalettizeToBatch(self, editItemIndex=-1):
        finput = ''
        if (len(self.palettize_eggList) == 0):
            dlg = wx.MessageDialog(self, "At least one input egg file must be present to add an item to the batch queue" ,"ERROR", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
            return
        for file in self.palettize_eggList:
            finput += str(file) + '|'

        foutput = self.palettize_exportFileTxt.GetValue()
        if not self.palettize_exportInPlaceChk.GetValue():
            if ((not self.palettize_exportFileTxt.GetValue()) and (not self.palettize_exportDirTxt.GetValue())):
                dlg = wx.MessageDialog(self, "Export destination must be specified to add an item to the batch queue" ,"ERROR", wx.OK )
                dlg.ShowModal()
                dlg.Destroy()
                return

        if (not self.palettize_exportTexTxt.GetValue()):
            dlg = wx.MessageDialog(self, "Export texture destination folder must be specified to add an item to the batch queue" ,"ERROR", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
            return

        batchItemInfo = {}
        batchItemInfo['label'] = self.batchItemNameTxt.GetValue()
        batchItemInfo['cmd'] = 'egg-palettize'
        batchItemInfo['args'] = self.BuildEggPalettizeArgs()
        batchItemInfo['finput'] = str(finput)
        batchItemInfo['foutput'] = str(foutput)

        if (editItemIndex>=0):
            self.batchList[editItemIndex] = batchItemInfo
            self.UpdateBatchDisplay()
        else:
            self.batchList.append(batchItemInfo)
            self.AddToBatchDisplay(batchItemInfo)


    def AddToBatchDisplay(self, batchItemInfo):
        #add items to the display for batch list
        label = self.GetBatchItemLabel(batchItemInfo)
        self.SetStatusText("Batch item added: " + label)

        self.batchTree.AppendItem(self.treeRoot,str(len(self.batchList)) + ' ' + label)
        self.batchTree.ExpandAll()

    def GetBatchItemLabel(self, batchItemInfo):
        label = ''
        if (batchItemInfo['label'] == ''):
            label = self.BuildCommand(batchItemInfo)
        else:
            label = batchItemInfo['cmd'] + ' | ' + batchItemInfo['label']
        return label


    def BuildMaya2EggArgs(self):
        #Build up all the command line arguments present in the maya2egg panel
        args = {}
        args['a'] = self.m2e_animOptChoice.GetStringSelection()
        args['ui'] = UNIT_TYPES[self.m2e_mayaUnitsComboBox.GetSelection()]
        args['uo'] = UNIT_TYPES[self.m2e_pandaUnitsComboBox.GetSelection()]

        if (self.m2e_backfaceChk.GetValue()):
           args['bface'] = None
        if (self.m2e_polygonOutputChk.GetValue()):
           args['p'] = None
        if (self.m2e_tbnallChk.GetValue()):
            args['tbnall'] = None
        if (self.m2e_charNameChk.GetValue()):
            args['cn'] = self.m2e_charNameTxt.GetValue()
        if (self.m2e_subsetsChk.GetValue()):
            args['subset'] = self.m2e_subsetsTxt.GetValue()
        if (self.m2e_subrootsChk.GetValue()):
            args['subroot'] = self.m2e_subrootsTxt.GetValue()
        if (self.m2e_excludesChk.GetValue()):
            args['exclude'] = self.m2e_excludesTxt.GetValue()
        if (self.m2e_startFrameChk.GetValue()):
            args['sf'] = self.m2e_startFrameSpin.GetValue()
        if (self.m2e_endFrameChk.GetValue()):
            args['ef'] = self.m2e_endFrameSpin.GetValue()
        if (self.m2e_frameRateInChk.GetValue()):
            args['fri'] = self.m2e_frameRateInSpin.GetValue()
        if (self.m2e_frameRateOutChk.GetValue()):
            args['fro'] = self.m2e_frameRateOutSpin.GetValue()
        if (self.m2e_copyTexChk.GetValue()):
            args['copytex'] = self.m2e_copyTexPathTxt.GetValue()
        if (self.m2e_legacyShaderChk.GetValue()):
            args['legacy-shader'] = None
        if (self.m2e_pathReplaceChk.GetValue()):
            args['pr'] = self.m2e_pathReplaceTxt.GetValue()

        return args

    def BuildEgg2BamArgs(self):
        #Build up all the command line arguments present in the egg2bam panel
        args = {}

        if (self.e2b_flattenChk.GetValue()):
            args['flatten'] = 3
        if (self.e2b_embedTexChk.GetValue()):
            args['rawtex'] = None

        return args

    def BuildEggRenameArgs(self):
        #Build up all the command line arguments present in the egg-rename panel
        args = {}

        if (self.rename_exportInPlaceChk.GetValue()):
           args['inplace'] = None
        if (self.rename_exportDirTxt.GetValue()):
           args['d'] = self.rename_exportDirTxt.GetValue()
        if (self.rename_stripPrefixChk.GetValue()):
            args['strip_prefix'] = self.rename_stripPrefixTxt.GetValue()

        return args


    def BuildEggOptcharArgs(self):
        #Build up all the command line arguments present in the egg-optchar panel
        args = {}

        if (self.optchar_exportInPlaceChk.GetValue()):
           args['inplace'] = None
        if (self.optchar_exportDirTxt.GetValue()):
           args['d'] = self.optchar_exportDirTxt.GetValue()
        if (self.optchar_keepAllJointsChk.GetValue()):
            args['keepall'] = None
        if (self.optchar_keepJointsChk.GetValue()):
            args['keep'] = self.optchar_keepJointsTxt.GetValue()
        if (self.optchar_dropJointsChk.GetValue()):
            args['drop'] = self.optchar_dropJointsTxt.GetValue()
        if (self.optchar_exposeJointsChk.GetValue()):
            args['expose'] = self.optchar_exposeJointsTxt.GetValue()
        if (self.optchar_flagGeometryChk.GetValue()):
            args['flag'] = self.optchar_flagGeometryTxt.GetValue()
        args['dart'] = self.optchar_dartChoice.GetStringSelection()

        return args

    def BuildEggPalettizeArgs(self):
        args = {}

        if (self.palettize_exportInPlaceChk.GetValue()):
           args['inplace'] = None
        if (self.palettize_exportDirTxt.GetValue()):
           args['d'] = self.palettize_exportDirTxt.GetValue()
        if (self.palettize_exportTexTxt.GetValue()):
           args['dm'] = self.palettize_exportTexTxt.GetValue()
        if (self.palettize_saveTxaTxt.GetValue()):
           args['af'] = self.palettize_saveTxaTxt.GetValue()

        return args


    def BuildCommand(self, item):
        command = item['cmd']
        for param in item['args'].keys():
            if param in ['subroot','exclude','subset','keep', 'drop', 'expose', 'flag', 'strip_prefix']:
                for arg in item['args'][param].split(' '):
                    command += ' -' + param + ' ' + arg
                    #command += ' "' + arg + '"'
            else:
                command += ' -' + param
                if item['args'][param] is not None:
                    if param in ['d', 'dm', 'copytex', 'af']:
                        command += ' "' + str(item['args'][param]) + '"'
                    else:
                        command += ' ' + str(item['args'][param])
        if (item['foutput'] != ""):
            command += ' -o '
            command += '"' + item['foutput'] + '"'
        for filename in item['finput'].split('|'):
            if filename:
                command += ' "' + filename + '"'
        return command


    def RunCommand(self, command, batchmode):
        #run command lines
        if batchmode and self.pandaPathTxt.GetValue()!='':
            PATH = self.pandaPathTxt.GetValue() + os.sep + 'bin' + os.sep
        else:
            PATH = ''

        if batchmode:
            self.consoleOutputTxt.AppendText(command+'\n')

        try:
            p = subprocess.Popen(PATH+command, shell = True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines = True )
        except:
            dlg = wx.MessageDialog(self,"Failed To Find Or run the Exporter Application" ,"ERROR", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
        else:
            if batchmode:
                while p.poll() is None:
                    out = p.stdout.readline()
                    if (out != '\n'): self.consoleOutputTxt.AppendText(out)
            else:
                while p.poll() is None:
                    out = p.stdout.readline()
                    self.outdlg.dlgOutText.AppendText(out)
        if batchmode:
            self.consoleOutputTxt.AppendText('\n')

    def OnRemoveBatch(self,event):
        #remove selected item from the batch list
        items = self.batchTree.GetSelections()
        toRemove = []
        for item in items:
            if (item != self.treeRoot):
                index  = int(self.batchTree.GetItemText(item).split()[0])-1
                toRemove.append(index)
                self.batchTree.Delete(item)
        toRemove.sort()
        if len(toRemove):
            for i in range(0, len(toRemove)):
                self.batchList.pop(toRemove[len(toRemove)-i-1])
        self.UpdateBatchDisplay()

    def UpdateBatchDisplay(self):
        #update the display of the batch list

        #For now just re calculate the entire list order
        self.batchTree.DeleteAllItems()
        self.treeRoot = self.batchTree.AddRoot('Batch Files')
        index = 0
        for batchItemInfo in self.batchList:
            index += 1
            label = self.GetBatchItemLabel(batchItemInfo)
            self.batchTree.AppendItem(self.treeRoot,str(index) + ' ' + label)
        self.batchTree.ExpandAll()

    def OnRemoveAllBatch(self,event):
        #remove all the items from the batch list
        self.batchTree.DeleteAllItems()
        if self.batchList != []:
            self.batchList = []
        self.treeRoot = self.batchTree.AddRoot('Batch Files') #rather than loop through it re-add the root

    def OnClearOutput(self, event):
        #clear the console output content
        self.consoleOutputTxt.Clear()

    def OnExit(self, e):
        #exit the GUI
        self.Close(True)

    def OnLoadPview(self, event):
        #load pview
        path = self.pandaPathTxt.GetValue() + os.sep + 'bin' + os.sep
        self.outpview = OutputDialogpview(self)
        self.outpview.Show()

    def OnSortBatch(self, event):
        maya2eggCommands = []
        egg2bamCommands = []
        eggRenameCommands = []
        eggOptcharCommands = []
        eggPalettizeCommands = []
        for item in self.batchList:
            if item['cmd'].count('maya2egg'):
                maya2eggCommands.append(item)
            elif item['cmd'] == 'egg2bam':
                egg2bamCommands.append(item)
            elif item['cmd'] == 'egg-rename':
                eggRenameCommands.append(item)
            elif item['cmd'] == 'egg-optchar':
                eggOptcharCommands.append(item)
            elif item['cmd'] == 'egg-palettize':
                eggPalettizeCommands.append(item)
        self.batchList = maya2eggCommands + eggRenameCommands + eggOptcharCommands + eggPalettizeCommands + egg2bamCommands
        self.UpdateBatchDisplay()

    def OnChangePaths(self, event):
        self.paths = OutputDialogPaths(self)
        self.paths.setCallback(self.ChangeBatchPaths)
        self.paths.Show()

    def ChangeBatchPaths(self, fromStr, toStr):
        for item in self.batchList:
            # check input file path
            item['finput'] = item['finput'].replace(fromStr, toStr)
            # check output file path
            item['foutput'] = item['foutput'].replace(fromStr, toStr)
            # paths inside of arguments
            if item['args'].has_key('d'):
                item['args']['d'] = item['args']['d'].replace(fromStr, toStr)
            if item['args'].has_key('copytex'):
                item['args']['copytex'] = item['args']['copytex'].replace(fromStr, toStr)
            if item['args'].has_key('dm'):
                item['args']['dm'] = item['args']['dm'].replace(fromStr, toStr)
            if item['args'].has_key('af'):
                item['args']['af'] = item['args']['af'].replace(fromStr, toStr)
        self.UpdateBatchDisplay()


    def PrefixReplaceInBatch(self, inPrefix, outPrefix):
        for batchItem in self.batchList:
            if batchItem['finput'].count(inPrefix):
                pass


    def OnSaveBatch(self,event):
        #save current batch list
        newdoc = Document()
        top_element = newdoc.createElement('batch')
        newdoc.appendChild(top_element)
        for item in self.batchList:

            batchitem = newdoc.createElement("batchitem")
            top_element.appendChild(batchitem)

            label = newdoc.createTextNode(item['label'])
            cmd = newdoc.createTextNode(item['cmd'])
            finput = newdoc.createTextNode(item['finput'])
            foutput = newdoc.createTextNode(item['foutput'])

            labelNode = newdoc.createElement('label')
            cmdNode = newdoc.createElement('cmd')
            argsNode = newdoc.createElement('args')
            finputNode = newdoc.createElement('finput')
            foutputNode = newdoc.createElement('foutput')

            # process arguments
            for param in item['args']:
                paramItem = newdoc.createElement(param)
                if item['args'][param] is not None:
                    paramText = newdoc.createTextNode(str(item['args'][param]))
                    paramItem.appendChild(paramText)
                argsNode.appendChild(paramItem)

            batchitem.appendChild(labelNode)
            batchitem.appendChild(cmdNode)
            batchitem.appendChild(argsNode)
            batchitem.appendChild(finputNode)
            batchitem.appendChild(foutputNode)

            labelNode.appendChild(label)
            cmdNode.appendChild(cmd)
            finputNode.appendChild(finput)
            foutputNode.appendChild(foutput)

        dirname = self.destProjectFolder
        filename = ''
        dlg = wx.FileDialog(self, "Choose a location and filename", dirname, "", "*.xml", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            file = dirname + os.sep + filename
            f = open(file,'w')
            #it's somehow easier to store it in a string and then write it.
            out = newdoc.toprettyxml()

            for line in out:
                f.writelines(line)
            f.close()

        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("Batch list exported to " + dirname + os.sep + filename)

    def OnLoadBatch(self,event):
        #load saved bacth list
        dirname = self.destProjectFolder
        filename = ''
        dlg = wx.FileDialog(self, "Load Preferences file...", dirname, "", "*.xml", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            file = dirname + os.sep + filename
            #sanity check to see if there's anything hanging out in the batch tree
            #May want to prompt the user if we should overwrite the list or not
            if self.batchList != []:
                #if there is delete it from the internal list
                self.batchList = []
            #Always delete it visually
            self.batchTree.DeleteAllItems()
            #Re-add the root item
            self.treeRoot = self.batchTree.AddRoot('Batch Files')
            doc = xml.dom.minidom.parse(str(file))
            for node in doc.getElementsByTagName('batchitem'):
                batchItemInfo = {}
                label = ''
                cmd = ''
                args = {}
                finput = ''
                foutput = ''

                elemlabel = node.getElementsByTagName('label')
                for node2 in elemlabel:
                    for node3 in node2.childNodes:
                        label = node3.data

                elemcmd = node.getElementsByTagName('cmd')
                for node2 in elemcmd:
                    for node3 in node2.childNodes:
                        cmd = node3.data

                elemargs = node.getElementsByTagName('args')
                for param in elemargs.item(0).childNodes:
                    if isinstance(param, Element):
                        if param.childNodes:
                            args[param.tagName] = str(param.childNodes[0].data).strip()
                        else:
                            args[param.tagName] = None

                elemfinput = node.getElementsByTagName('finput')
                for node2 in elemfinput:
                    for node3 in node2.childNodes:
                        finput = node3.data
                elemoutput = node.getElementsByTagName('foutput')
                for node2 in elemoutput:
                    for node3 in node2.childNodes:
                        foutput = node3.data

                batchItemInfo['label'] = str(label).strip()
                batchItemInfo['cmd'] = str(cmd).strip()
                batchItemInfo['args'] = args
                batchItemInfo['finput'] = str(finput).strip()
                batchItemInfo['foutput'] = str(foutput).strip()

                self.batchList.append(batchItemInfo)
                self.AddToBatchDisplay(batchItemInfo)

        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("Batch imported succesfully from:" + dirname + os.sep + filename)

    def OnRunBatch(self, event):
        #run current batch list
        self.batchProgress = wx.ProgressDialog('Progress', "Running Batch export please be patient...", maximum = len(self.batchList),  style = wx.PD_REMAINING_TIME)
        self.batchProgress.SetSizeWH(300,150)
        self.batchProgress.Show()
        index = 0

        # reset batch tree style
        if self.batchTree.ItemHasChildren(self.treeRoot):
            child, cookie = self.batchTree.GetFirstChild(self.treeRoot)
            while child.IsOk():
                self.batchTree.SetItemBold(child, False)
                self.batchTree.SetItemTextColour(child, wx.Colour(0,0,0))
                #self.batchTree.SetItemBackgroundColour(child, wx.Colour(0,0,0))
                child = self.batchTree.GetNextSibling(child)

        currNode = None

        if (self.batchList):
            self.SetStatusText("Running Batch export please be patient...")
            for item in self.batchList:
                if currNode:
                    currNode = self.batchTree.GetNextSibling(currNode)
                else:
                    currNode, cookie = self.batchTree.GetFirstChild(self.treeRoot)
                self.batchTree.SetItemBold(currNode, True)

                self.batchProgress.Update(index ,'')
                index += 1
                if (self.ignoreModDates.GetValue()): #do we want to just egg them regardless if they're newer?
                    self.RunCommand(self.BuildCommand(item),True)
                    self.batchTree.SetItemTextColour(currNode, wx.Colour(0,175,0))
                else:
                    if self.CheckModDates(item):
                        self.RunCommand(self.BuildCommand(item),True)
                        self.batchTree.SetItemTextColour(currNode, wx.Colour(0,175,0))
                    else:
                        self.batchTree.SetItemTextColour(currNode, wx.Colour(0,0,175))

            self.batchProgress.Destroy()
            dlg = wx.MessageDialog(self,"Export Complete","Complete", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()

            self.SetStatusText("Batch process complete, see Console Output for details")
        else:
            self.batchProgress.Destroy()
            dlg = wx.MessageDialog(self,"No items in the batch queue to export. Please add items" ,"ERROR", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()

    def CheckModDates(self, item):
        finput = item['finput']
        foutput = item['foutput']
        if 1:
            if (item['cmd'].count('maya2egg') or item['cmd'].count('egg2bam')):
                inputTime = os.path.getmtime(finput)
                outputTime = os.path.getmtime(foutput)
                item['changed'] = (inputTime > outputTime)
                return (inputTime > outputTime) # True if file was modified
            elif (item['cmd'] in ['egg-rename', 'egg-optchar', 'egg-palettize']):
                if item['args'].has_key('inplace'): # hard to check these
                    index = self.batchList.index(item)
                    # look for the input of the file
                    inputFiles = item['finput'].split('|')
                    for inFile in inputFiles:
                        if (inFile != ''):
                            inputFound = False
                            inputChanged = False
                            filename = inFile.split('\\')[-1]
                            for i in range(0, index):
                                idx = index - (i+1)
                                outputFiles = self.GetOutputFromBatch(idx)
                                if inFile in outputFiles:
                                    inputFound = True
                                    if self.batchList[idx]['cmd'].count('maya2egg'):
                                        inputFile = self.batchList[idx]['finput']
                                        inputTime = os.path.getmtime(inputFile)
                                        outputTime = os.path.getmtime(inFile)
                                        inputChanged = (inputTime > outputTime)
                                    else:
                                        if self.batchList[idx]['args'].has_key('inplace'):
                                            inputChanged = self.batchList[idx]['changed']
                                        elif self.batchList[idx]['args'].has_key('d'):
                                            inputChanged = self.batchList[idx]['changed']
                                            '''
                                            inputs = self.batchList[idx]['finput'].split('|')
                                            for inputFile in inputs:
                                                if (inputFile != ''):
                                                    inputFilename = inputFile.split('\\')[-1]
                                                    print("Compare: ", inFile, filename, inputFile, inputFilename)
                                                    if inputFilename == filename:
                                                        inputTime = os.path.getmtime(inputFile)
                                                        outputTime = os.path.getmtime(inFile)
                                                        print("Matched: ", (inputTime > outputTime))
                                                        inputChanged = (inputTime > outputTime)
                                                        break
                                            '''
                                        else:
                                            inputFile = self.batchList[idx]['finput']
                                            inputTime = os.path.getmtime(finput)
                                            outputTime = os.path.getmtime(inFile)
                                            inputChanged = (inputTime > outputTime)
                                    break
                            if not inputFound:
                                item['changed'] = True
                                return True
                            if inputChanged:
                                item['changed'] = True
                                return True
                    item['changed'] = False
                    return False
                if item['args'].has_key('d'): # compare files in the directory
                    inputFiles = item['finput'].split('|')
                    filesChanged = False
                    for inFile in inputFiles:
                        if (inFile != ''):
                            filename = inFile.split('\\')[-1]
                            directory = item['args']['d']
                            inputTime = os.path.getmtime(inFile)
                            outputTime = os.path.getmtime(directory+'\\'+filename)
                            filesChanged = filesChanged or (inputTime > outputTime)
                    item['changed'] = filesChanged
                    return filesChanged
                else:
                    inputTime = os.path.getmtime(finput)
                    outputTime = os.path.getmtime(foutput)
                    item['changed'] = (inputTime > outputTime)
                    return (inputTime > outputTime) # True if file was modified
        else:
            item['changed'] = True
            return True
        item['changed'] = True
        return True


    def OnBatchItemSelection(self, event):
        #edit batch item clicked
        if not len(self.batchTree.GetSelections()):
            return
        try:
            selectedItemId = self.batchTree.GetSelections()[0]
            selectedItemIndex = int(self.batchTree.GetItemText(selectedItemId).split()[0])-1
            batchItem = self.batchList[selectedItemIndex]

            print('\n'+self.BuildCommand(batchItem))

            if (batchItem['cmd'].count('maya2egg')):
                # Display Maya2Egg Tool Panel
                self.toolComboBox.SetStringSelection('maya2egg')
                self.OnTool(None)
                self.m2e_mayaVerComboBox.SetValue(batchItem['cmd'].split('maya2egg')[1])
                self.m2e_mayaFileTxt.SetValue(batchItem['finput'])
                self.m2e_exportDestTxt.SetValue(batchItem['foutput'])
                self.m2e_mayaUnitsComboBox.SetValue(batchItem['args']['ui'])
                self.m2e_pandaUnitsComboBox.SetValue(batchItem['args']['uo'])
                self.m2e_backfaceChk.SetValue(batchItem['args'].has_key('bface'))
                self.m2e_polygonOutputChk.SetValue(batchItem['args'].has_key('p'))
                self.m2e_tbnallChk.SetValue(batchItem['args'].has_key('tbnall'))
                self.m2e_subsetsChk.SetValue(batchItem['args'].has_key('subset'))
                self.m2e_subsetsTxt.SetValue('' if not batchItem['args'].has_key('subset') else batchItem['args']['subset'])
                self.m2e_excludesChk.SetValue(batchItem['args'].has_key('exclude'))
                self.m2e_excludesTxt.SetValue('' if not batchItem['args'].has_key('exclude') else batchItem['args']['exclude'])
                self.m2e_animOptChoice.SetStringSelection(batchItem['args']['a'])
                self.m2e_charNameChk.SetValue(batchItem['args'].has_key('cn'))
                self.m2e_charNameTxt.SetValue('' if not batchItem['args'].has_key('cn') else batchItem['args']['cn'])
                self.m2e_startFrameChk.SetValue(batchItem['args'].has_key('sf'))
                self.m2e_startFrameSpin.SetValue(0 if not batchItem['args'].has_key('sf') else int(batchItem['args']['sf']))
                self.m2e_endFrameChk.SetValue(batchItem['args'].has_key('ef'))
                self.m2e_endFrameSpin.SetValue(0 if not batchItem['args'].has_key('ef') else int(batchItem['args']['ef']))
                self.m2e_frameRateInChk.SetValue(batchItem['args'].has_key('fri'))
                self.m2e_frameRateInSpin.SetValue(0 if not batchItem['args'].has_key('fri') else int(batchItem['args']['fri']))
                self.m2e_frameRateOutChk.SetValue(batchItem['args'].has_key('fro'))
                self.m2e_frameRateOutSpin.SetValue(0 if not batchItem['args'].has_key('fro') else int(batchItem['args']['fro']))
                self.m2e_subrootsChk.SetValue(batchItem['args'].has_key('subroot'))
                self.m2e_subrootsTxt.SetValue('' if not batchItem['args'].has_key('subroot') else batchItem['args']['subroot'])
                self.m2e_legacyShaderChk.SetValue(batchItem['args'].has_key('legacy-shader'))
                self.m2e_copyTexChk.SetValue(batchItem['args'].has_key('copytex'))
                self.m2e_copyTexPathTxt.SetValue('' if not batchItem['args'].has_key('copytex') else batchItem['args']['copytex'])
                self.m2e_pathReplaceChk.SetValue(batchItem['args'].has_key('pr'))
                self.m2e_pathReplaceTxt.SetValue('' if not batchItem['args'].has_key('pr') else batchItem['args']['pr'])

            elif (batchItem['cmd'].count('egg2bam')):
                self.toolComboBox.SetStringSelection('egg2bam')
                self.OnTool(None)
                self.e2b_eggFileTxt.SetValue(batchItem['finput'])
                self.e2b_exportDestTxt.SetValue(batchItem['foutput'])
                self.e2b_useCurrEggChk.SetValue(0)
                self.e2b_flattenChk.SetValue(batchItem['args'].has_key('flatten'))
                self.e2b_embedTexChk.SetValue(batchItem['args'].has_key('rawtex'))


            elif (batchItem['cmd'].count('egg-rename')):
                self.toolComboBox.SetStringSelection('egg-rename')
                self.OnTool(None)
                self.rename_eggList = []
                for filename in batchItem['finput'].split('|'):
                    if filename:
                        self.rename_eggList.append(filename)
                self.UpdateEggRenameDisplay()
                self.rename_exportInPlaceChk.SetValue(batchItem['args'].has_key('inplace'))
                self.rename_exportFileTxt.SetValue(batchItem['foutput'])
                if (batchItem['args'].has_key('d')):
                    self.rename_exportDirTxt.SetValue(batchItem['args']['d'])
                else:
                    self.rename_exportDirTxt.SetValue("")
                self.OnRenameInPlace(None)
                self.rename_stripPrefixChk.SetValue(batchItem['args'].has_key('strip_prefix'))
                self.rename_stripPrefixTxt.SetValue('' if not batchItem['args'].has_key('strip_prefix') else batchItem['args']['strip_prefix'])

            elif (batchItem['cmd'].count('egg-optchar')):
                self.toolComboBox.SetStringSelection('egg-optchar')
                self.OnTool(None)
                self.optchar_eggList = []
                for filename in batchItem['finput'].split('|'):
                    if filename:
                        self.optchar_eggList.append(filename)
                self.UpdateEggOptcharDisplay()
                self.optchar_exportInPlaceChk.SetValue(batchItem['args'].has_key('inplace'))
                self.optchar_exportFileTxt.SetValue(batchItem['foutput'])
                if (batchItem['args'].has_key('d')):
                    self.optchar_exportDirTxt.SetValue(batchItem['args']['d'])
                else:
                    self.optchar_exportDirTxt.SetValue("")
                self.OnOptcharInPlace(None)
                self.optchar_keepAllJointsChk.SetValue(batchItem['args'].has_key('keepall'))
                self.optchar_keepJointsChk.SetValue(batchItem['args'].has_key('keep'))
                self.optchar_keepJointsTxt.SetValue('' if not batchItem['args'].has_key('keep') else batchItem['args']['keep'])
                self.optchar_dropJointsChk.SetValue(batchItem['args'].has_key('drop'))
                self.optchar_dropJointsTxt.SetValue('' if not batchItem['args'].has_key('drop') else batchItem['args']['drop'])
                self.optchar_exposeJointsChk.SetValue(batchItem['args'].has_key('expose'))
                self.optchar_exposeJointsTxt.SetValue('' if not batchItem['args'].has_key('expose') else batchItem['args']['expose'])
                self.optchar_flagGeometryChk.SetValue(batchItem['args'].has_key('flag'))
                self.optchar_flagGeometryTxt.SetValue('' if not batchItem['args'].has_key('flag') else batchItem['args']['flag'])
                self.optchar_dartChoice.SetStringSelection(batchItem['args']['dart'])

            elif (batchItem['cmd'].count('egg-palettize')):
                self.toolComboBox.SetStringSelection('egg-palettize')
                self.OnTool(None)
                self.palettize_eggList = []
                for filename in batchItem['finput'].split('|'):
                    if filename:
                        self.palettize_eggList.append(filename)
                self.UpdateEggPalettizeDisplay()
                self.palettize_exportInPlaceChk.SetValue(batchItem['args'].has_key('inplace'))
                self.palettize_exportFileTxt.SetValue(batchItem['foutput'])
                if (batchItem['args'].has_key('d')):
                    self.palettize_exportDirTxt.SetValue(batchItem['args']['d'])
                else:
                    self.palettize_exportDirTxt.SetValue("")
                self.OnPalettizeInPlace(None)
                self.palettize_exportTexTxt.SetValue(batchItem['args']['dm'])
                self.palettize_saveTxaTxt.SetValue(batchItem['args']['af'])

                try:
                    self.txaExtraLines = []
                    txafile = open(batchItem['args']['af'], 'r')
                    for line in txafile:
                        words = line.split()
                        if len(words):
                            if words[0] == ':palette':
                                self.palettize_sizeWidthTxt.SetValue(words[1])
                                self.palettize_sizeHeightTxt.SetValue(words[2])
                            elif words[0] == ':imagetype':
                                self.palettize_imageTypeChoice.SetStringSelection(words[1])
                            elif words[0] == ':powertwo':
                                self.palettize_powerOf2Chk.SetValue(int(words[1]))
                            elif words[0] == ':background':
                                self.palettize_redTxt.SetValue(int(words[1]))
                                self.palettize_greenTxt.SetValue(int(words[2]))
                                self.palettize_blueTxt.SetValue(int(words[3]))
                                self.palettize_alphaTxt.SetValue(int(words[4]))
                            elif words[0] == ':margin':
                                self.palettize_marginTxt.SetValue(int(words[1]))
                            elif words[0] == ':coverage':
                                self.palettize_coverageTxt.SetValue(words[1])
                            else:
                                self.txaExtraLines.append(line)
                    txafile.close()
                except:
                    print("Error opening .txa file!")

            self.batchItemNameTxt.SetValue(batchItem['label'])



        except ValueError:
            return
        #print(self.batchList)

    def OnBatchItemEdit(self, event):
        selectedItemId = self.batchTree.GetSelections()
        if len(selectedItemId):
            selectedItemId = selectedItemId[0]
        else:
            return
        if (selectedItemId):
            selectedItemIndex = int(self.batchTree.GetItemText(selectedItemId).split()[0])-1
            batchItem = self.batchList[selectedItemIndex]

            if (batchItem['cmd'].count('maya2egg')):
                self.addMaya2EggToBatch(selectedItemIndex)
            if (batchItem['cmd'].count('egg2bam')):
                self.addEgg2BamToBatch(selectedItemIndex)
            if (batchItem['cmd'].count('egg-rename')):
                self.addEggRenameToBatch(selectedItemIndex)
            if (batchItem['cmd'].count('egg-optchar')):
                self.addEggOptcharToBatch(selectedItemIndex)
            if (batchItem['cmd'].count('egg-palettize')):
                self.addEggPalettizeToBatch(selectedItemIndex)


    def OnSavePrefs(self, event): # wxGlade: main.<event_handler>
        #save preferences
        newdoc = Document()

        top_element = newdoc.createElement('preferences')
        newdoc.appendChild(top_element)

        envitem = newdoc.createElement("environment")
        genitem = newdoc.createElement("genoptions")
        animitem = newdoc.createElement("animoptions")
        texitem = newdoc.createElement("textureoptions")
        overitem = newdoc.createElement("overridemod")
        attributeitem = newdoc.createElement("attribute")
        top_element.appendChild(envitem)
        top_element.appendChild(genitem)
        top_element.appendChild(animitem)
        top_element.appendChild(texitem)
        top_element.appendChild(overitem)
        top_element.appendChild(attributeitem)

        pandadir = newdoc.createTextNode(str(self.pandaPathTxt.GetValue()))
        mayaver = newdoc.createTextNode(str(self.m2e_mayaVerComboBox.GetSelection()))
        pandadirElem = newdoc.createElement('pandadir')
        mayaverElem = newdoc.createElement('mayaver')
        pandadirElem.appendChild(pandadir)
        mayaverElem.appendChild(mayaver)
        envitem.appendChild(pandadirElem)
        envitem.appendChild(mayaverElem)

        inunits = newdoc.createTextNode(str(self.m2e_mayaUnitsComboBox.GetValue()))
        outunits = newdoc.createTextNode(str(self.m2e_pandaUnitsComboBox.GetValue()))
        bface = newdoc.createTextNode(str(int(self.m2e_backfaceChk.GetValue())))
        tbnall = newdoc.createTextNode(str(int(self.m2e_tbnallChk.GetValue())))
        subsets = newdoc.createTextNode(str(int(self.m2e_subsetsChk.GetValue())))
        subsetsval = newdoc.createTextNode(str(self.m2e_subsetsTxt.GetValue()))
        excludes = newdoc.createTextNode(str(int(self.m2e_excludesChk.GetValue())))
        excludesval = newdoc.createTextNode(str(self.m2e_excludesTxt.GetValue()))

        inunitsElem = newdoc.createElement('inunits')
        outunitsElem = newdoc.createElement('outunits')
        bfaceElem = newdoc.createElement('bface')
        tbnallElem = newdoc.createElement('tbnall')
        subsetsElem = newdoc.createElement('subsets')
        subnamesElem = newdoc.createElement('subnames')
        excludesElem = newdoc.createElement('excludes')
        exnamesElem = newdoc.createElement('excludesval')

        inunitsElem.appendChild(inunits)
        outunitsElem.appendChild(outunits)
        bfaceElem.appendChild(bface)
        tbnallElem.appendChild(tbnall)
        subsetsElem.appendChild(subsets)
        subnamesElem.appendChild(subsetsval)
        excludesElem.appendChild(excludes)
        exnamesElem.appendChild(exnamesElem)

        genitem.appendChild(inunitsElem)
        genitem.appendChild(outunitsElem)
        genitem.appendChild(bfaceElem)
        genitem.appendChild(tbnallElem)
        genitem.appendChild(subsetsElem)
        genitem.appendChild(subnamesElem)
        genitem.appendChild(excludesElem)
        genitem.appendChild(exnamesElem)


        modeloptsElem = newdoc.createElement('modelopts')
        cnElem = newdoc.createElement('cn')
        charnameElem = newdoc.createElement('charname')
        framerangeElem = newdoc.createElement('framerange')
        subrootsElem = newdoc.createElement('subroots')
        subrnamesElem = newdoc.createElement('subrnames')
        sfElem = newdoc.createElement('sf')
        sfvalElem = newdoc.createElement('sfval')
        efElem = newdoc.createElement('ef')
        efvalElem = newdoc.createElement('efval')
        friElem = newdoc.createElement('fri')
        frivalElem = newdoc.createElement('frival')
        froElem = newdoc.createElement('fro')
        frovalElem = newdoc.createElement('froval')
        framerangeElem.appendChild(sfElem)
        framerangeElem.appendChild(sfvalElem)
        framerangeElem.appendChild(efElem)
        framerangeElem.appendChild(efvalElem)
        framerangeElem.appendChild(friElem)
        framerangeElem.appendChild(frivalElem)
        framerangeElem.appendChild(froElem)
        framerangeElem.appendChild(frovalElem)

        modelopts = newdoc.createTextNode(str(self.m2e_animOptChoice.GetSelection()))
        cn = newdoc.createTextNode(str(int(self.m2e_charNameChk.GetValue())))
        charname = newdoc.createTextNode(str(self.m2e_charNameTxt.GetValue()))

        modeloptsElem.appendChild(modelopts)
        cnElem.appendChild(cn)
        charnameElem.appendChild(charname)

        sf = newdoc.createTextNode(str(int(self.m2e_startFrameChk.GetValue())))
        sfval = newdoc.createTextNode(str(self.m2e_startFrameSpin.GetValue()))
        ef = newdoc.createTextNode(str(int(self.m2e_endFrameChk.GetValue())))
        efval = newdoc.createTextNode(str(self.m2e_endFrameSpin.GetValue()))
        fri = newdoc.createTextNode(str(int(self.m2e_frameRateInChk.GetValue())))
        frival = newdoc.createTextNode(str(self.m2e_frameRateInSpin.GetValue()))
        fro = newdoc.createTextNode(str(int(self.m2e_frameRateOutChk.GetValue())))
        froval = newdoc.createTextNode(str(self.m2e_frameRateOutSpin.GetValue()))

        sfElem.appendChild(sf)
        sfvalElem.appendChild(sfval)
        efElem.appendChild(ef)
        efvalElem.appendChild(efval)
        friElem.appendChild(fri)
        frivalElem.appendChild(frival)
        froElem.appendChild(fro)
        frovalElem.appendChild(froval)

        subroots = newdoc.createTextNode(str(int(self.m2e_subrootsChk.GetValue())))
        subrnames = newdoc.createTextNode(str(self.m2e_subrootsTxt.GetValue()))
        subrootsElem.appendChild(subroots)
        subrnamesElem.appendChild(subrnames)

        animitem.appendChild(modeloptsElem)
        animitem.appendChild(cnElem)
        animitem.appendChild(charnameElem)
        animitem.appendChild(framerangeElem)
        animitem.appendChild(subrootsElem)
        animitem.appendChild(subrnamesElem)

        legacy_shaderElem = newdoc.createElement('legacy-shader')
        copytexElem = newdoc.createElement('copytex')
        destpathElem = newdoc.createElement('path')
        legacy_shader = newdoc.createTextNode(str(int(self.m2e_legacyShaderChk.GetValue())))
        copytex = newdoc.createTextNode(str(int(self.m2e_copyTexChk.GetValue())))
        destpath = newdoc.createTextNode(str(self.m2e_copyTexPathTxt.GetValue()))

        legacy_shaderElem.appendChild(legacy_shader)
        copytexElem.appendChild(copytex)
        destpathElem.appendChild(destpath)

        texitem.appendChild(legacy_shaderElem)
        texitem.appendChild(copytexElem)
        texitem.appendChild(destpathElem)

        override = newdoc.createTextNode(str(int(self.ignoreModDates.GetValue())))
        overitem.appendChild(override)

        imagetype = newdoc.createTextNode(str(self.palettize_imageTypeChoice.GetSelection()))
        powertwo = newdoc.createTextNode(str(self.powerflag.GetSelection()))
        imagetypeElem = newdoc.createElement('imagetype')
        powertwoElem = newdoc.createElement('powertwo')
        R = newdoc.createTextNode(str(int(self.palettize_redTxt.GetValue())))
        G = newdoc.createTextNode(str(int(self.palettize_greenTxt.GetValue())))
        B = newdoc.createTextNode(str(int(self.palettize_blueTxt.GetValue())))
        A = newdoc.createTextNode(str(int(self.palettize_alphaTxt.GetValue())))
        margin = newdoc.createTextNode(str(int(self.palettize_marginTxt.GetValue())))
        coverage = newdoc.createTextNode(str(self.palettize_coverageTxt.GetValue()))
        RElem = newdoc.createElement('R')
        GElem = newdoc.createElement('G')
        BElem = newdoc.createElement('B')
        AElem = newdoc.createElement('A')
        marginElem = newdoc.createElement('margin')
        coverageElem = newdoc.createElement('coverage')

        imagetypeElem.appendChild(imagetype)
        powertwoElem.appendChild(powertwo)
        RElem.appendChild(R)
        GElem.appendChild(G)
        BElem.appendChild(B)
        AElem.appendChild(A)
        marginElem.appendChild(margin)
        coverageElem.appendChild(coverage)

        attributeitem.appendChild(imagetypeElem)
        attributeitem.appendChild(powertwoElem)
        attributeitem.appendChild(RElem)
        attributeitem.appendChild(GElem)
        attributeitem.appendChild(BElem)
        attributeitem.appendChild(AElem)
        attributeitem.appendChild(marginElem)
        attributeitem.appendChild(coverageElem)

        filename = ''
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a location and filename", dirname, "", "*.xml", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            file = dirname + os.sep + filename
            f = open(file,'w')
            #it's somehow easier to store it in a string and then write it.
            out = newdoc.toprettyxml()
            for line in out:
                f.writelines(line)
            f.close()

    def OnLoadPrefs(self, event): # wxGlade: main.<event_handler>
        #load preferences
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a location and filename", dirname, "", "*.xml", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            file = dirname + os.sep + filename
            prefs = {}
            doc = xml.dom.minidom.parse(str(file))
            prefs = self.parseXML(doc)
            self.updateOptions(prefs)

    def parseXML(self , doc):
        #parse the file of preferences
        prefsDict = {}
        for list in doc.getElementsByTagName('preferences'):

            envlist = list.getElementsByTagName('environment')
            genlist = list.getElementsByTagName('genoptions')
            animlist = list.getElementsByTagName('animoptions')
            texlist = list.getElementsByTagName('textureoptions')
            overlist = list.getElementsByTagName('overridemod')
            attributelist = list.getElementsByTagName('attribute')

            for elem in envlist:
                for nodes in elem.childNodes:
                    for val in nodes.childNodes:
                        key = val.parentNode.nodeName
                        data =  str(val.data)
                        prefsDict[str(key)] = data.strip()
            for elem in genlist:
                for nodes in elem.childNodes:
                    for val in nodes.childNodes:
                        key = val.parentNode.nodeName
                        data =  str(val.data)
                        prefsDict[str(key)] = data.strip()
            for elem in overlist:
                for node in elem.childNodes:
                    key = node.parentNode.nodeName
                    data =  str(node.data)
                    prefsDict[str(key)] = data.strip()

            for elem in animlist:
                for elem2 in elem.childNodes:
                    for elem3 in elem2.childNodes:
                        data = elem3
                        parent =  str(elem3.parentNode.nodeName)
                        if parent == 'framerange':
                            list = data.childNodes
                            for items in list:
                                key = str(items.parentNode.nodeName)
                                keydata = str(items.data)
                                prefsDict[key] = keydata.strip()
                        else:
                            prefsDict[parent] =  str(data.data).strip()
            for elem in texlist:
                for nodes in elem.childNodes:
                    for val in nodes.childNodes:
                        key = val.parentNode.nodeName
                        data =  str(val.data)
                        prefsDict[str(key)] = data.strip()

            for elem in attributelist:
                for nodes in elem.childNodes:
                    for val in nodes.childNodes:
                        key = val.parentNode.nodeName
                        data =  str(val.data)
                        prefsDict[str(key)] = data.strip()

        return prefsDict

    def updateOptions(self,prefs):
        #lots of type conversions since I think XML only stores strings
        #I'm not sure I have to cast the ints to bools
        #but I don't want to chance it with wxPython
        #other than that all we're doing here is
        #setting all the options to the saved ones in the dictionary
        self.pandaPathTxt.SetValue(prefs['pandadir'])
        self.m2e_mayaVerComboBox.SetSelection(int(prefs['mayaver']))
        self.m2e_mayaUnitsComboBox.SetValue(prefs['inunits'])
        self.m2e_pandaUnitsComboBox.SetValue(prefs['outunits'])
        self.m2e_backfaceChk.SetValue(bool(prefs['bface']))
        self.m2e_tbnallChk.SetValue(bool(int(prefs['tbnall'])))
        self.m2e_subsetsChk.SetValue(bool(int(prefs['subsets'])))
        self.m2e_subsetsTxt.SetValue(prefs['subnames'])
        self.m2e_excludesChk.SetValue(bool(int(prefs['excludes'])))
        self.m2e_excludesTxt.SetValue(prefs['exnames'])
        self.m2e_animOptChoice.SetSelection(int(prefs['modelopts']))
        self.m2e_charNameChk.SetValue(bool(int(prefs['cn'])))
        self.m2e_charNameTxt.SetValue(prefs['charname'])
        self.m2e_startFrameChk.SetValue(bool(int(prefs['sf'])))
        self.m2e_startFrameSpin.SetValue(int(prefs['sfval']))
        self.m2e_endFrameChk.SetValue(bool(int(prefs['ef'])))
        self.m2e_endFrameSpin.SetValue(int(prefs['efval']))
        self.m2e_frameRateInChk.SetValue(bool(int(prefs['fri'])))
        self.m2e_frameRateInSpin.SetValue(int(prefs['frival']))
        self.m2e_frameRateOutChk.SetValue(bool(int(prefs['fro'])))
        self.m2e_frameRateOutSpin.SetValue(int(prefs['froval']))
        self.m2e_subrootsChk.SetValue(bool(int(prefs['subroots'])))
        self.m2e_subrootsTxt.SetValue(prefs['subrnames'])
        self.ignoreModDates.SetValue(bool(int(prefs['overridemod'])))
        self.m2e_legacyShaderChk.SetValue(bool(int(prefs['legacy-shader'])))
        self.m2e_copyTexChk.SetValue(bool(int(prefs['copytex'])))
        self.m2e_copyTexPathTxt.SetValue(prefs['path'])
        self.palettize_imageTypeChoice.SetSelection(int(prefs['imagetype']))
        self.powerflag.SetSelection(int(prefs['powertwo']))
        self.palettize_redTxt.SetValue(int(prefs['R']))
        self.palettize_greenTxt.SetValue(int(prefs['G']))
        self.palettize_blueTxt.SetValue(int(prefs['B']))
        self.palettize_alphaTxt.SetValue(int(prefs['A']))
        self.palettize_marginTxt.SetValue(int(prefs['margin']))
        self.palettize_coverageTxt.SetValue(prefs['coverage'])


if __name__ == "__main__":
    app = wx.App(0)
    wx.InitAllImageHandlers()
    new_frame = main(None, -1, "")
    app.SetTopWindow(new_frame)
    new_frame.Show()
    app.MainLoop()
