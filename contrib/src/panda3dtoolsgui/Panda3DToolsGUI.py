import wx
import wx.html
from wx.lib.filebrowsebutton import FileBrowseButton
import wx.lib.agw.pycollapsiblepane as PCP
import xml.dom.minidom
from xml.dom.minidom import Node, Document
import time
import sys
import os
import subprocess

ID_PICKEGGPATH = wx.NewId()
ID_PICKMAYAFILE = wx.NewId()
ID_CHOOSEPANDA = wx.NewId()
ID_CHOOSEPANDA = wx.NewId()
ID_CLEAROUTPUT = wx.NewId()
ID_LOADPREFS = wx.NewId()
ID_RUNBATCH = wx.NewId()
ID_SAVEPREFS = wx.NewId()
ID_ADDTOBATCH = wx.NewId()
ID_REMOVESELBATCH = wx.NewId()
ID_REMOVEALLBATCH = wx.NewId()
ID_SAVEBATCH = wx.NewId()
ID_LOADBATCH = wx.NewId()
ID_HELP = wx.NewId()
ID_ABOUT = wx.NewId()
ID_EXIT = wx.NewId()
ID_BAMLOADEGG = wx.NewId()
ID_BAMADDTOBATCH = wx.NewId()
ID_BAMCHOOSEDEST = wx.NewId()
ID_RUNOPTCHAR = wx.NewId()
ID_OPTCHOOSEOUT = wx.NewId()
ID_OPTCHOOSEEGG = wx.NewId()
ID_SIMPLEEGGSAVE = wx.NewId()
ID_SIMPLEMBPICK = wx.NewId()
ID_RUNSIMPLE = wx.NewId()
ID_RUNSIMPLEEXPORT = wx.NewId()
ID_TEXCHOOSEPATH = wx.NewId()
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
                 '2010'] 

TOOLS = ['Maya2Egg',
         'Egg2Bam',
         'egg-opt-char',
         'egg-palettize'] 
         
EGGPALETTIZE = ['single_egg',
                'multiple_egg'] 
               
EGG_TYPES = ['actor',
             'model',
             'both',
             'animation']


DEFAULT_PANDA_DIR = ''

UNIT_TYPES = ["mm", 
              "cm", 
              "m", 
              "in", 
              "ft", 
              "yd"]

ANIMATION_OPTIONS = ["none", 
                     "model", 
                     "chan", 
                     "both"]
                     
WELCOME_MSG ="""Panda3D Tools GUI version 0.1
April, 20th 2010
by Andrew Gartner,Shuying Feng and the PandaSE team (Spring '10)
Entertainment Technology Center
Carnegie Mellon University

Please send any feedback to:
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
WINDOW_HEIGHT = 920 
WINDOW_WIDTH = 900

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
        self.dlgOutText.SetMinSize((500, 300))
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
        self.pview_static_sizer_staticbox = wx.StaticBox(self, -1, "Pview")
        self.egg = FileBrowseButton(self.p,labelText="Select egg file:")#,fileMask="*.egg"
        self.egganim = FileBrowseButton(self.p,labelText="Select animation egg file:")
        self.btn = wx.Button(self.p, -1, "Pview")

        self.__set_properties()
        self.__do_layout()

        self.Bind(wx.EVT_BUTTON, self.RunPview, self.btn)
        # end wxGlade

    def __set_properties(self):
        # begin wxGlade: OutputDialog.__set_properties
        self.SetTitle("Pview")
        self.SetSize((400, 133))
        self.egg.SetMinSize((350, 30))
        self.egganim.SetMinSize((400, 30))
        # end wxGlade

    def __do_layout(self):
        # begin wxGlade: OutputDialog.__do_layout
        self.pview_static_sizer_staticbox.Lower()
        pview_static_sizer = wx.StaticBoxSizer(self.pview_static_sizer_staticbox, wx.HORIZONTAL)
        pview_main_flex_sizer = wx.FlexGridSizer(4, 1, 0, 0)
        pview_main_flex_sizer.Add(self.egg, 1, wx.EXPAND|wx.ALIGN_CENTER_VERTICAL)
        pview_main_flex_sizer.Add(self.egganim, 1, wx.EXPAND|wx.ALIGN_CENTER_VERTICAL)
        pview_main_flex_sizer.Add(self.btn, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_CENTER_HORIZONTAL|wx.LEFT,20)
        self.p.SetSizer(pview_main_flex_sizer)
        pview_static_sizer.Add(self.p, 1, wx.EXPAND, 0)
        self.SetSizer(pview_static_sizer)
        self.Layout()
        self.Centre()
        # end wxGlade

    def RunPview(self,e):#pview function
        filename = self.egg.GetValue()
        anim_filename = self.egganim.GetValue()
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


class main(wx.Frame):
    def __init__(self, *args, **kwds):
        # begin wxGlade: main.__init__
        kwds["style"] = wx.DEFAULT_FRAME_STYLE
        wx.Frame.__init__(self, *args, **kwds)
        
        self.pandaPathDir = ''
        #self.numFiles is the global list of batch items waiting to be run
        #it is used by the update display function but really has no effect 
        #on what is shown. It is more for internal data handling
        self.numFiles = []
        self.eggnumFiles = []
        self._setupUI()
        #Show the welcome message and the initial panda path
        #NOTE this shoudl eventually check for an install of Panda somewhere
        self.ShowInitialEnv()
        self.ExpandPanes()
        self.OnPaneChanged(None)
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
        self.simple_panel = wx.Panel(self.tab_panel, -1)
        #Define simple interface elements
        #initialize the output dialog for simple mode
        #This needs to be shown later so it's better to do it here
        #rather than having to check for it's existence 
        #and then deleting the entire dialog to get a new one created
        #Rather the output continually is re-shown rather than created
        self.outdlg = OutputDialog(self)
        self.simpleEggTxt = wx.TextCtrl(self.simple_panel, -1, '')
        self.simpleMBtxt = wx.TextCtrl(self.simple_panel,-1,'')
        self.simpleGetEggBTN = wx.Button(self.simple_panel, ID_SIMPLEEGGSAVE, 'Save')
        self.simpleGetMBBTN = wx.Button(self.simple_panel, ID_SIMPLEMBPICK, 'Choose..')
        self.simpleEggLBL = wx.StaticText(self.simple_panel, -1, "Egg file to be written")
        self.simpleMBLBL = wx.StaticText(self.simple_panel, -1, "Maya file to be exported")
        self.simpleAnimOptChoice = wx.RadioBox(self.simple_panel, -1, "Animation", choices=["none", "model", "chan", "both"], majorDimension=4, style=wx.RA_SPECIFY_COLS)
        self.simpleEggItBTN = wx.Button(self.simple_panel, ID_RUNSIMPLEEXPORT, "Run Export")
        self.simplemayaVerComboBox = wx.ComboBox(self.simple_panel, -1, choices=MAYA_VERSIONS, style=wx.CB_DROPDOWN|wx.CB_READONLY)
        
        #define advanced interface elements
        #self.main_panel = wx.ScrolledWindow(self.tab_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.main_panel = wx.Panel(self.tab_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.setup_panel = wx.Panel(self.main_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        
        #bacth panel
        self.panelBatch = wx.Panel(self.setup_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.AddToolComboBox = wx.ComboBox(self.panelBatch, -1, choices=TOOLS, style=wx.CB_DROPDOWN|wx.CB_READONLY)
        self.pathLbl = wx.StaticText(self.panelBatch, -1, "Panda Directory", style=wx.ALIGN_CENTRE)
        self.pandaPathTxt = wx.TextCtrl(self.panelBatch, -1, "", style=wx.TE_READONLY)
        self.loadPandaPathBtn = wx.Button(self.panelBatch, ID_CHOOSEPANDA, "Choose...")
        self.console_panel = wx.Panel(self.panelBatch, -1)
        self.consoleOutputTxt = wx.TextCtrl(self.console_panel, -1, "", style=wx.TE_MULTILINE|wx.TE_READONLY|wx.TE_WORDWRAP)
        self.console_static_sizer_staticbox = wx.StaticBox(self.console_panel, -1, "Console Output")
        self.batch_panel = wx.Panel(self.panelBatch, -1)
        self.batchTree = wx.TreeCtrl(self.batch_panel, -1, style=wx.TR_HAS_BUTTONS|wx.TR_LINES_AT_ROOT|wx.TR_DEFAULT_STYLE|wx.SUNKEN_BORDER)
        self.batch_static_sizer_staticbox = wx.StaticBox(self.batch_panel, -1, "Batch List")
        self.treeRoot = self.batchTree.AddRoot('Batch Files')
        self.removeSelBatchButton = wx.Button(self.panelBatch, ID_REMOVESELBATCH, "Remove Selected")
        self.removeAllBatchButton = wx.Button(self.panelBatch, ID_REMOVEALLBATCH, "Remove All")
        self.clearConsoleButton = wx.Button(self.panelBatch, ID_CLEAROUTPUT, "Clear Output")
        self.ignoreModDates = wx.CheckBox(self.panelBatch, -1, "Override export changed maya scene files")
        
        #maya2egg panel
        self.maya2eggPanel = wx.Panel(self.setup_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)

        self.maya2egg = wx.StaticText(self.maya2eggPanel, -1, "Maya2Egg Tool")
        self.filepane = PCP.PyCollapsiblePane(self.maya2eggPanel, -1, style =  PCP.CP_GTK_EXPANDER )
        self.file_child_panel = wx.Panel(self.filepane.GetPane(), -1)
        self.savePathFileLBL = wx.StaticText(self.file_child_panel, -1, "Export Filename\nand Location")
        self.eggFileTxt = wx.TextCtrl(self.file_child_panel, -1, "")
        self.storeOutputButton = wx.Button(self.file_child_panel, ID_PICKEGGPATH, "Choose..")
        self.mayaFileLBL = wx.StaticText(self.file_child_panel, -1, "Maya Scene File\n")
        self.mayaFileTxt = wx.TextCtrl(self.file_child_panel, -1, "")
        self.loadMayaScene = wx.Button(self.file_child_panel, ID_PICKMAYAFILE, "Choose..")
        
        self.envpane = PCP.PyCollapsiblePane(self.maya2eggPanel,-1, style = PCP.CP_GTK_EXPANDER )
        self.paths_child_panel = wx.Panel(self.envpane.GetPane(), -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.mayaVerLbl = wx.StaticText(self.paths_child_panel, -1, "Choose Maya Version", style=wx.ALIGN_CENTRE)
        self.mayaVerComboBox = wx.ComboBox(self.paths_child_panel, -1, choices=MAYA_VERSIONS, style=wx.CB_DROPDOWN|wx.CB_READONLY)
        
        self.genpane = PCP.PyCollapsiblePane(self.maya2eggPanel,-1, style = PCP.CP_GTK_EXPANDER | wx.CP_NO_TLW_RESIZE)
        self.panelGenOpts = wx.Panel(self.genpane.GetPane(), -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.units_panel = wx.Panel(self.panelGenOpts, -1)
        self.mayaUnitsLBL = wx.StaticText(self.units_panel, -1, "Maya Units (Input)")
        self.mayaUnitsCombo = wx.ComboBox(self.units_panel, -1, choices=UNIT_TYPES, style=wx.CB_DROPDOWN|wx.CB_READONLY)
        self.pandaUnitsLBL = wx.StaticText(self.units_panel, -1, "Panda Units (Output)")
        self.pandaUnitsCombo = wx.ComboBox(self.units_panel, -1, choices=UNIT_TYPES, style=wx.CB_DROPDOWN|wx.CB_READONLY)
        self.units_child_sizer_staticbox = wx.StaticBox(self.units_panel, -1, "Units Setup")
        self.backfaceChk = wx.CheckBox(self.panelGenOpts , -1, "Back Face Rendering (-bface)")
        self.tbnallChk = wx.CheckBox(self.panelGenOpts, -1, "Calculate Tangent and Binormal (-tbnall)")
        self.useSubsetChk = wx.CheckBox(self.panelGenOpts, -1, "Export Specified Subsets")
        self.subsetsTxt = wx.TextCtrl(self.panelGenOpts, -1, "")
        
        self.animpane = PCP.PyCollapsiblePane(self.maya2eggPanel,-1, style = PCP.CP_GTK_EXPANDER )
        self.animopts_main_panel = wx.Panel(self.animpane.GetPane(), -1)
        self.animOptChoice = wx.RadioBox(self.animopts_main_panel, -1, "Animation", choices=["none", "model", "chan", "both"], majorDimension=4, style=wx.RA_SPECIFY_COLS)
        self.charChk = wx.CheckBox(self.animopts_main_panel, -1, "Character Name (-cn)")
        self.charTxt = wx.TextCtrl(self.animopts_main_panel, -1, "")
        self.sfChk = wx.CheckBox(self.animopts_main_panel, -1, "Start Frame")
        self.sfSpin = wx.SpinCtrl(self.animopts_main_panel, -1, "", min=-10000, max=10000)
        self.efChk = wx.CheckBox(self.animopts_main_panel, -1, "End Frame")
        self.efSpin = wx.SpinCtrl(self.animopts_main_panel, -1, "", min=-10000, max=10000)
        self.friChk = wx.CheckBox(self.animopts_main_panel, -1, "Frame Rate Input (+/-) (-fri)")
        self.friSpin = wx.SpinCtrl(self.animopts_main_panel, -1, "", min=-10000, max=10000)
        self.froChk = wx.CheckBox(self.animopts_main_panel, -1, "Frame Rate Output (+/-) (-fro)")
        self.froSpin = wx.SpinCtrl(self.animopts_main_panel, -1, "", min=-10000, max=10000)
        self.keyframe_static_sizer_staticbox = wx.StaticBox(self.animopts_main_panel, -1, "Frame Range ")
        self.useSubrootsChk = wx.CheckBox(self.animopts_main_panel, -1, "Export Named:")
        self.subrootsTxt = wx.TextCtrl(self.animopts_main_panel, -1, "")
        self.subs_static_sizer_staticbox = wx.StaticBox(self.animopts_main_panel, -1, "Export Named Subroots")
        
        self.texpane = PCP.PyCollapsiblePane(self.maya2eggPanel, -1, style = PCP.CP_GTK_EXPANDER )
        self.tex_panel = wx.Panel(self.texpane.GetPane())
        self.copyTexCHK = wx.CheckBox(self.tex_panel, -1, "Copy Textures To Specific Path")
        self.texDestPathTxt = wx.TextCtrl(self.tex_panel, -1, "")
        self.texDestPathBTN = wx.Button(self.tex_panel, ID_TEXCHOOSEPATH, "Choose Directory")
        self.useLegacyShaderCHK = wx.CheckBox(self.tex_panel, -1, "Use Legacy Shader Generation")
        
        self.mayaAddToBatch = wx.Button(self.maya2eggPanel, ID_MAYAADDTOBATCH, "Add To Batch")
       
        #egg2bam panel
        self.egg2bamPanel = wx.Panel(self.setup_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        
        self.egg2bam = wx.StaticText(self.egg2bamPanel, -1, "Egg2Bam Tool")
        self.bamFlattenChk = wx.CheckBox(self.egg2bamPanel, -1, "Flatten")
        self.eggEmbedChk = wx.CheckBox(self.egg2bamPanel, -1, "Embed Textures")
        self.bamUseCurrentChk = wx.CheckBox(self.egg2bamPanel, -1, "Use current egg file")
        self.bamLoadText = wx.TextCtrl(self.egg2bamPanel, -1, "")
        self.bamLoadEgg = wx.Button(self.egg2bamPanel, ID_BAMLOADEGG, "Load Egg")
        self.bamOutputText = wx.TextCtrl(self.egg2bamPanel, -1, "")
        self.bamChooseOutputDir = wx.Button(self.egg2bamPanel, ID_BAMCHOOSEDEST, "Output Directory")
        self.bamAddToBatch = wx.Button(self.egg2bamPanel, ID_BAMADDTOBATCH, "Add To Batch")
        
        #egg-opt-char panel
        self.eggOptCharPanel = wx.Panel(self.setup_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        
        self.eggOptChar = wx.StaticText(self.eggOptCharPanel, -1, "Egg-Opt-Char Tool")
        self.optEggLBL = wx.StaticText(self.eggOptCharPanel, -1, "Egg File")
        self.optUseCurrentChk = wx.CheckBox(self.eggOptCharPanel, -1, "Use current egg file")
        self.optEggTxt = wx.TextCtrl(self.eggOptCharPanel, -1, "")
        self.loadOptEggButton = wx.Button(self.eggOptCharPanel, ID_OPTCHOOSEEGG, "Browse")
        self.optOutputLBL = wx.StaticText(self.eggOptCharPanel, -1, "Output Name")
        self.optOutputTxt = wx.TextCtrl(self.eggOptCharPanel, -1, "")
        self.optChooseOutputButton = wx.Button(self.eggOptCharPanel, ID_OPTCHOOSEOUT, "Choose...")
        self.optRunOptCharButton = wx.Button(self.eggOptCharPanel, ID_RUNOPTCHAR, "Add To Batch")
        
        #egg-palettize panel
        self.eggPalettizePanel = wx.Panel(self.setup_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        
        self.eggPalettize = wx.StaticText(self.eggPalettizePanel, -1, "Egg-Palettize Tool")
        self.multieggComboBox = wx.ComboBox(self.eggPalettizePanel, -1, choices=EGGPALETTIZE, style=wx.CB_DROPDOWN|wx.CB_READONLY)
        
        self.eggpfile_child_panel = wx.Panel(self.eggPalettizePanel, -1)
        
        self.single_egg_panel = wx.Panel(self.eggpfile_child_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.ineggTxt = wx.StaticText(self.single_egg_panel, -1, "Input Egg File\n")
        self.ineggFileTxt = wx.TextCtrl(self.single_egg_panel, -1, "")
        self.inegg = wx.Button(self.single_egg_panel, ID_INEGG, "Choose..")
        self.outtexTxt = wx.StaticText(self.single_egg_panel, -1, "Output Texture File\n")
        self.outtexFileTxt = wx.TextCtrl(self.single_egg_panel, -1, "")
        self.outtex = wx.Button(self.single_egg_panel, ID_OUTTEX, "Choose..")
        self.outeggTxt = wx.StaticText(self.single_egg_panel, -1, "Output Egg File\n")
        self.outeggFileTxt = wx.TextCtrl(self.single_egg_panel, -1, "")
        self.outegg = wx.Button(self.single_egg_panel, ID_OUTEGG, "Choose..")
        
        self.multiple_egg_panel = wx.Panel(self.eggpfile_child_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.mult_ineggTxt = wx.StaticText(self.multiple_egg_panel, -1, "Input Egg File\n")
        self.mult_batchTree = wx.TreeCtrl(self.multiple_egg_panel, -1, style=wx.TR_HAS_BUTTONS|wx.TR_LINES_AT_ROOT|wx.TR_DEFAULT_STYLE|wx.SUNKEN_BORDER)
        self.mult_treeRoot = self.mult_batchTree.AddRoot('Egg Files')
        self.mult_inegg_panel = wx.Panel(self.multiple_egg_panel, -1 )
        self.mult_inegg = wx.Button(self.mult_inegg_panel, -1, "Add")
        self.mult_inegg_remove = wx.Button(self.mult_inegg_panel, -1, "Remove")
        self.mult_inegg_remove_all = wx.Button(self.mult_inegg_panel, -1, "Remove All")
        self.mult_outtexTxt = wx.StaticText(self.multiple_egg_panel, -1, "Output Texture File\n")
        self.mult_outtexFileTxt = wx.TextCtrl(self.multiple_egg_panel, -1, "")
        self.mult_outtex = wx.Button(self.multiple_egg_panel, -1, "Choose..")
        self.mult_outeggTxt = wx.StaticText(self.multiple_egg_panel, -1, "Output Egg File\n")
        self.mult_outeggFileTxt = wx.TextCtrl(self.multiple_egg_panel, -1, "")
        self.mult_outegg = wx.Button(self.multiple_egg_panel, -1, "Choose..")
        
        #attribute open
        self.attripane = PCP.PyCollapsiblePane(self.eggPalettizePanel,-1, style = PCP.CP_GTK_EXPANDER | wx.CP_NO_TLW_RESIZE)
        self.attri_child_panel = wx.Panel(self.attripane.GetPane(), -1)
        
        self.sizeTxt = wx.StaticText(self.attri_child_panel, -1, "Set Palettize size:")
        self.sizeSpin1 = wx.TextCtrl(self.attri_child_panel, -1, "5102",(30,20),(80,-1))
        self.sizeSpin2 = wx.TextCtrl(self.attri_child_panel, -1, "5102",(30,20),(80,-1))
        
        self.imagetype = wx.RadioBox(self.attri_child_panel, -1, "Imagetype", choices=["rgb", "jpg", "png"], majorDimension=3, style=wx.RA_SPECIFY_COLS)

        self.RTxt = wx.StaticText(self.attri_child_panel, -1, "r:")
        self.RSpin = wx.SpinCtrl(self.attri_child_panel, -1, "",(30,20),(80,-1),min=0, max=255)
        self.GTxt = wx.StaticText(self.attri_child_panel, -1, "g:")
        self.GSpin = wx.SpinCtrl(self.attri_child_panel, -1, "",(30,20),(80,-1),min=0, max=255)
        self.BTxt = wx.StaticText(self.attri_child_panel, -1, "b:")
        self.BSpin = wx.SpinCtrl(self.attri_child_panel, -1, "",(30,20),(80,-1),min=0, max=255)
        self.ATxt = wx.StaticText(self.attri_child_panel, -1, "a:")
        self.ASpin = wx.SpinCtrl(self.attri_child_panel, -1, "",(30,20),(80,-1),min=0, max=255)
        self.color_static_sizer_staticbox = wx.StaticBox(self.attri_child_panel, -1, "Set background color")
        
        self.marginTxt = wx.StaticText(self.attri_child_panel, -1, "Margin:")
        self.marginSpin = wx.SpinCtrl(self.attri_child_panel, -1, "",(30,20),(80,-1),min=0, max=10000)
        
        self.coverTxt = wx.StaticText(self.attri_child_panel, -1, "Coverage:")
        self.coverSpin = wx.TextCtrl(self.attri_child_panel, -1, "1.0",(30,20),(80,-1))
        
        self.powerflag = wx.RadioBox(self.attri_child_panel, -1, "Powertwo flag", choices=["off", "on"], majorDimension=2, style=wx.RA_SPECIFY_COLS)
        
        self.savetxaFileTxt = wx.TextCtrl(self.attri_child_panel, -1, "")
        self.savetxa = wx.Button(self.attri_child_panel, -1, "Save Attributes")
        self.palettizeAddToBatch = wx.Button(self.eggPalettizePanel, ID_PALETTIZEADDTOBATCH, "Add To Batch")
        
        #run panel
        self.runPanel = wx.Panel(self.main_panel, -1, style=wx.NO_BORDER|wx.TAB_TRAVERSAL)
        self.loadBatchButton = wx.Button(self.runPanel, ID_LOADBATCH, "Load Batch")
        self.saveBatchButton = wx.Button(self.runPanel, ID_SAVEBATCH, "Save Batch")
        self.runBatchButton = wx.Button(self.runPanel, ID_RUNBATCH, "Run Batch")
        self.runPviewButton = wx.Button(self.runPanel,-1,"Load pview")
        
        self.__set_properties()
        self.__do_layout()
        
        #bind events
        #simple mode
        self.Bind(wx.EVT_BUTTON, self.SimpleExport, id = ID_RUNSIMPLEEXPORT)
        self.Bind(wx.EVT_BUTTON, self.LoadSimpleEgg, id = ID_SIMPLEEGGSAVE)
        self.Bind(wx.EVT_BUTTON, self.LoadSimpleMB, id = ID_SIMPLEMBPICK)
        
        #main menus
        self.Bind(wx.EVT_MENU, self.OnLoadPrefs, self.prefsLoadButton)
        self.Bind(wx.EVT_MENU, self.OnSavePrefs, self.savePrefsButton)
        self.Bind(wx.EVT_MENU, self.OnExit, self.exitButton)
        self.Bind(wx.EVT_MENU, self.OnShowHelp,self.HelpMenuButton)
        
        #batch panel
        self.Bind(wx.EVT_COMBOBOX, self.OnTool, self.AddToolComboBox)
        self.Bind(wx.EVT_BUTTON, self.OnRemoveBatch, id=ID_REMOVESELBATCH)
        self.Bind(wx.EVT_BUTTON, self.OnRemoveAllBatch, id=ID_REMOVEALLBATCH)
        self.Bind(wx.EVT_BUTTON, self.OnClearOutput, id=ID_CLEAROUTPUT)
        
        #maya2egg panel
        self.Bind(wx.EVT_BUTTON, self.OnChooseEggOutput, id=ID_PICKEGGPATH)
        self.Bind(wx.EVT_BUTTON, self.OnPickMayaFile, id=ID_PICKMAYAFILE)
        self.Bind(wx.EVT_BUTTON, self.OnPandaPathChoose, id=ID_CHOOSEPANDA)
        self.Bind(wx.EVT_COMBOBOX, self.OnMayaVerChoose, self.mayaVerComboBox)
        self.Bind(wx.EVT_BUTTON, self.ChooseTexCopyDest, id = ID_TEXCHOOSEPATH)
        self.Bind(wx.EVT_CHECKBOX, self.OnCopyTexToggle, self.copyTexCHK)
        self.Bind(wx.EVT_BUTTON, self.OnAddEggToBatch, self.mayaAddToBatch)
        
        #egg2bam panel
        self.Bind(wx.EVT_BUTTON, self.OnBamLoadEgg, id=ID_BAMLOADEGG)
        self.Bind(wx.EVT_BUTTON, self.OnBamChooseOutput, id=ID_BAMCHOOSEDEST)
        self.Bind(wx.EVT_BUTTON, self.BamAddToBatch, id=ID_BAMADDTOBATCH)
        self.Bind(wx.EVT_CHECKBOX, self.EnableBamChoose, self.bamUseCurrentChk)
        
        #egg-opt-char panel
        self.Bind(wx.EVT_BUTTON, self.OnOptChooseEgg, id=ID_OPTCHOOSEEGG)
        self.Bind(wx.EVT_BUTTON, self.OnOptChooseOutput, id=ID_OPTCHOOSEOUT)
        self.Bind(wx.EVT_BUTTON, self.OnRunOptChar, id=ID_RUNOPTCHAR)
        self.Bind(wx.EVT_CHECKBOX, self.EnableOptChoose, self.optUseCurrentChk)
        
        #egg-palettzie 
        self.Bind(wx.EVT_COMBOBOX, self.OnEggpTool, self.multieggComboBox)
        self.Bind(wx.EVT_BUTTON, self.Onmultinegg, self.mult_inegg)
        self.Bind(wx.EVT_BUTTON, self.Onmultineggremove, self.mult_inegg_remove)
        self.Bind(wx.EVT_BUTTON, self.Onmultineggremoveall, self.mult_inegg_remove_all)
        self.Bind(wx.EVT_BUTTON, self.Onmultouttex, self.mult_outtex)
        self.Bind(wx.EVT_BUTTON, self.Onmultoutegg, self.mult_outegg)
        self.Bind(wx.EVT_BUTTON, self.Oninegg, self.inegg)
        self.Bind(wx.EVT_BUTTON, self.Onouttex, self.outtex)
        self.Bind(wx.EVT_BUTTON, self.Onoutegg, self.outegg)
        self.Bind(wx.EVT_BUTTON, self.OnSavetxa, self.savetxa)
        self.Bind(wx.EVT_BUTTON, self.Onpalettizeadd, self.palettizeAddToBatch)
    
        #run panel
        self.Bind(wx.EVT_BUTTON, self.OnRunBatch, id=ID_RUNBATCH)
        self.Bind(wx.EVT_BUTTON, self.OnLoadBatch, id=ID_LOADBATCH)
        self.Bind(wx.EVT_BUTTON, self.OnSaveBatch, id=ID_SAVEBATCH)
        self.Bind(wx.EVT_BUTTON, self.OnLoadPview, self.runPviewButton)
        
        #the layout bind
        self.filepane.Bind(wx.EVT_COLLAPSIBLEPANE_CHANGED, self.OnPaneChanged)
        self.envpane.Bind(wx.EVT_COLLAPSIBLEPANE_CHANGED, self.OnPaneChanged)
        self.genpane.Bind(wx.EVT_COLLAPSIBLEPANE_CHANGED, self.OnPaneChanged)
        self.animpane.Bind(wx.EVT_COLLAPSIBLEPANE_CHANGED, self.OnPaneChanged)
        self.texpane.Bind(wx.EVT_COLLAPSIBLEPANE_CHANGED,self.OnPaneChanged)
        self.attripane.Bind(wx.EVT_COLLAPSIBLEPANE_CHANGED,self.OnPaneChanged)  
        

    def __set_properties(self):
        # begin wxGlade: main.__set_properties
        # This was mostly generated in wxGlade, however
        #I had to edit a lot of it by hand. 
        #This function mainly takes care of setting labels and sizes
        # of most of the GUI elements.
        self.SetTitle("Panda3D Tools GUI")
        self.simplemayaVerComboBox.SetValue(MAYA_VERSIONS[0])
        self.simpleEggTxt.SetMinSize((200,21))
        self.simpleMBtxt.SetMinSize((200,21))
        self.statusBar.SetStatusWidths([-1])
        self.simple_panel.SetMaxSize((400,400))
        
        #batch panel
        self.AddToolComboBox.SetSelection(0)
        self.consoleOutputTxt.SetMinSize((400, 375))
        self.consoleOutputTxt.SetBackgroundColour(wx.Colour(192, 192, 192))
        self.consoleOutputTxt.SetToolTipString("maya2egg console output appears here when batch process is running")
        self.consoleOutputTxt.Enable(True)
        self.batchTree.SetMinSize((350,350))
        self.ignoreModDates.SetToolTipString("Use this check box to export all the mb files regardless if they have been modified since the last export")
        self.ignoreModDates.SetValue(True)
        
        #maya2egg panel
        self.maya2eggPanel.SetMinSize((400, 750))
        self.filepane.SetLabel("File Options")
        self.envpane.SetLabel("Environment Options")
        self.genpane.SetLabel("General Options")
        self.animpane.SetLabel("Animation Options")
        self.texpane.SetLabel("Texture/Shader Options")
        self.texDestPathTxt.Enable(False)
        self.filepane.SetExpanderDimensions(5,8)
        self.envpane.SetExpanderDimensions(5,8)
        self.genpane.SetExpanderDimensions(5,8)
        self.animpane.SetExpanderDimensions(5,8)
        self.texpane.SetExpanderDimensions(5,8)
        self.texDestPathTxt.SetMinSize((200,21))
        self.eggFileTxt.SetMinSize((200, 21))
        self.storeOutputButton.SetToolTipString("select the destination and filename of the exported file")
        self.mayaFileTxt.SetMinSize((200, 21))
        self.loadMayaScene.SetToolTipString("select an MB file to be exported")
        self.pandaPathTxt.SetMinSize((200,21))
        self.pandaPathTxt.SetBackgroundColour(wx.Colour(192, 192, 192))
        self.pandaPathTxt.SetToolTipString("Select the particular installed version of Panda3D, if not chosen the first entry in the system path is used")
        self.mayaVerLbl.SetToolTipString("*required* which version of the maya exporter to use, must match version of *.mb file")
        self.mayaVerComboBox.SetSelection(1)
        self.mayaUnitsCombo.SetToolTipString("defaults to centimeters")
        self.mayaUnitsCombo.SetSelection(1)
        self.pandaUnitsLBL.SetToolTipString("defaults to cm")
        self.pandaUnitsCombo.SetSelection(1)
        self.backfaceChk.SetToolTipString("enable/disable backface rendering of polygons in the egg file (default is off)")
        self.tbnallChk.SetToolTipString("calculate the tangets and normals for every polygon to be exported (for normal maps, etc)")
        self.useSubsetChk.SetToolTipString("Export susets of a hierarchy contained in the maya scene file")
        self.subsetsTxt.SetMinSize((200, 21))
        self.animOptChoice.SetToolTipString("Select the particular animation to written to egg file (if any)")
        self.animOptChoice.SetSelection(1)
        self.charTxt.SetMinSize((200, 21))
        self.friChk.SetToolTipString("increase or decrease the frame rate of the maya scene file (default is none, or 24fps)")
        self.froChk.SetToolTipString("increase/decrease the egg file's animation frame rate (defaults to input file's frame rate)")
        self.useSubrootsChk.SetToolTipString("Export subroots of a hierarchy contained in the maya scene file")
        self.subrootsTxt.SetMinSize((200,21))

        #maya2bam panel
        self.bamUseCurrentChk.SetValue(0)
        self.bamUseCurrentChk.Enable(True)
        self.bamLoadText.SetMinSize((200, 21))
        self.bamLoadText.Enable(True)
        self.bamLoadEgg.Enable(True)
        self.egg2bamPanel.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))
        self.egg2bamPanel.SetToolTipString("Compress the egg file to a binary format (*.Bam)")
        
        #egg-opt-char panel
        self.optEggTxt.SetMinSize((200, 21))
        self.optEggTxt.Enable(True)
        self.optOutputTxt.SetMinSize((200, 21))
        self.eggOptCharPanel.SetBackgroundColour(wx.SystemSettings_GetColour(wx.SYS_COLOUR_3DFACE))
        self.eggOptCharPanel.SetToolTipString("egg-opt-char tool")
        
        #egg-palettize panel
        self.attripane.SetLabel("Attribute Options")
        
        self.attripane.SetExpanderDimensions(5,8)
        
        self.multieggComboBox.SetSelection(0)
        self.mult_batchTree.SetMinSize((200,100))
        self.mult_inegg.SetToolTipString("select an egg file to be exported")
        self.mult_inegg_remove.SetToolTipString("delete the selected egg file")
        self.mult_inegg_remove_all.SetToolTipString("delete all the egg file")
        self.mult_outtexFileTxt.SetMinSize((200, 21))
        self.mult_outtex.SetToolTipString("select the destination and filename of the exported textures")
        self.mult_outeggFileTxt.SetMinSize((200, 21))
        self.mult_outegg.SetToolTipString("select an egg file to be exported")
        self.savetxaFileTxt.SetMinSize((200, 21))
        self.savetxa.SetToolTipString("save current attributes to the .txa file")
        self.ineggFileTxt.SetMinSize((200, 21))
        self.inegg.SetToolTipString("select an egg file to be exported")
        self.outtexFileTxt.SetMinSize((200, 21))
        self.outtex.SetToolTipString("select the destination and filename of the exported textures")
        self.outeggFileTxt.SetMinSize((200, 21))
        self.outegg.SetToolTipString("select an egg file to be exported")
        self.imagetype.SetToolTipString("Select the image type for the textures in the egg-palete")
        self.imagetype.SetSelection(2)
        self.powerflag.SetToolTipString("Specify whether the texure should be the power of two size")
        self.powerflag.SetSelection(0)
        self.marginSpin.SetValue(2)
        self.coverSpin.SetToolTipString("Please enter the number of the coverage")
        self.sizeSpin1.SetToolTipString("Please enter the n2mber of the width")
        self.sizeSpin2.SetToolTipString("Please enter the number of the height")
        
        #run panel
        self.runPanel.SetMinSize((800, 50))

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
        simple_sizer = wx.BoxSizer(wx.VERTICAL)
        simple_flex_sizer = wx.FlexGridSizer(5,3,0,0)
    
        simple_flex_sizer.Add(self.simpleEggLBL, -1, wx.ALL, 5)
        simple_flex_sizer.Add(self.simpleEggTxt, -1, wx.ALL, 4)
        simple_flex_sizer.Add(self.simpleGetEggBTN, -1, wx.ALL, 4)
        simple_flex_sizer.Add(self.simpleMBLBL, -1, wx.ALL, 5)
        simple_flex_sizer.Add(self.simpleMBtxt, -1, wx.ALL, 4)
        simple_flex_sizer.Add(self.simpleGetMBBTN, -1, wx.ALL, 4)
        simple_flex_sizer.Add(self.simplemayaVerComboBox, -1, wx.ALL, 4) 
        simple_flex_sizer.Add(self.simpleAnimOptChoice)
        simple_flex_sizer.Add(self.simpleEggItBTN, -1, wx.ALL , 4)
        self.simple_panel.SetSizerAndFit(simple_flex_sizer)
        simple_sizer.Add(self.simple_panel,-1,wx.EXPAND, 2)
        
        #main panel
        main_sizer = wx.BoxSizer(wx.HORIZONTAL)
        main_grid_sizer = wx.FlexGridSizer(2, 1, 0, 0)
        self.setup_grid_sizer = wx.FlexGridSizer(1, 2, 0, 0)
        main_left_grid_sizer = wx.FlexGridSizer(5, 1, 0, 0)
        
        #Left part of main panel
        main_left_grid_sizer_firstline = wx.FlexGridSizer(3, 2, 0, 0)
        main_left_grid_sizer_firstline.Add(self.AddToolComboBox,1,wx.RIGHT,8)
        main_left_grid_sizer_firstline.Add((20, 20), 0, 0, 0)
        main_left_grid_sizer_firstline.Add(self.pathLbl, 0, wx.ALIGN_CENTER_VERTICAL, 0)
        main_left_grid_sizer_firstline.Add((20, 20), 0, 0, 0)
        main_left_grid_sizer_firstline.Add(self.pandaPathTxt, 0, wx.EXPAND, 0)
        main_left_grid_sizer_firstline.Add(self.loadPandaPathBtn, 0, wx.ALL, 0)
        main_left_grid_sizer.Add(main_left_grid_sizer_firstline,1,wx.ALIGN_LEFT|wx.ALL,8)
    
        
        self.console_static_sizer_staticbox.Lower()
        console_static_sizer = wx.StaticBoxSizer(self.console_static_sizer_staticbox, wx.HORIZONTAL)
        console_static_sizer.Add(self.consoleOutputTxt, 0, wx.EXPAND, 0)
        self.console_panel.SetSizer(console_static_sizer)
        main_left_grid_sizer.Add(self.console_panel, 1, wx.EXPAND, 0)

        grid_sizer_1_copy = wx.GridSizer(2, 3, 0, 0)
        grid_sizer_1_copy.Add(self.clearConsoleButton, 0, wx.EXPAND|wx.ALIGN_LEFT, 0)
        grid_sizer_1_copy.Add(self.removeSelBatchButton, 0, wx.EXPAND|wx.ALIGN_LEFT, 0)
        grid_sizer_1_copy.Add(self.removeAllBatchButton, 0, wx.EXPAND|wx.ALIGN_LEFT, 0)
        main_left_grid_sizer.Add(grid_sizer_1_copy, 0, wx.EXPAND|wx.ALIGN_CENTER_HORIZONTAL, 0)
        
        self.batch_static_sizer_staticbox.Lower()
        batch_static_sizer = wx.StaticBoxSizer(self.batch_static_sizer_staticbox, wx.HORIZONTAL)
        batch_static_sizer.Add(self.batchTree, 1, wx.ALIGN_TOP|wx.EXPAND, 0)
        self.batch_panel.SetSizer(batch_static_sizer)
        main_left_grid_sizer.Add(self.batch_panel, 1, wx.ALIGN_TOP|wx.EXPAND, 0)
        
        main_left_grid_sizer.Add(self.ignoreModDates, 0, wx.LEFT, 2)
        self.panelBatch.SetSizer(main_left_grid_sizer)
       
        #maya2egg panel
        maya2egg_sizer = wx.FlexGridSizer(8, 1, 0, 0)
        
        maya2egg_sizer.Add(self.maya2egg, 0, wx.TOP|wx.BOTTOM, 10)
        
        file_flex_grid_sizer = wx.FlexGridSizer(2, 3, 0, 0)
        file_flex_grid_sizer.Add(self.savePathFileLBL, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2)
        file_flex_grid_sizer.Add(self.eggFileTxt, 0, wx.ALL, 2)
        file_flex_grid_sizer.Add(self.storeOutputButton, 0, 0, 0)
        file_flex_grid_sizer.Add(self.mayaFileLBL, 0, wx.ALL, 2)
        file_flex_grid_sizer.Add(self.mayaFileTxt, 0, wx.ALL, 2)
        file_flex_grid_sizer.Add(self.loadMayaScene, 0, 0, 0)
        self.file_child_panel.SetSizerAndFit(file_flex_grid_sizer)
        maya2egg_sizer.Add(self.filepane, 1, wx.EXPAND, 0)
        
        pathGridSizer = wx.GridSizer(1, 2, 0, 0)
        pathGridSizer.Add(self.mayaVerLbl, 0, wx.ALIGN_CENTER_VERTICAL, 0)
        pathGridSizer.Add(self.mayaVerComboBox, 0, wx.LEFT, 0)
        self.paths_child_panel.SetSizerAndFit(pathGridSizer)
        maya2egg_sizer.Add(self.envpane, 1, wx.EXPAND, 0)
        
        genopts_static_sizer = wx.BoxSizer(wx.HORIZONTAL)
        units_sizer = wx.FlexGridSizer(5, 1, 0, 0)
        subset_sizer = wx.FlexGridSizer(1, 3, 0, 0)
        self.units_child_sizer_staticbox.Lower()
        units_child_sizer = wx.StaticBoxSizer(self.units_child_sizer_staticbox, wx.HORIZONTAL)
        units_child_flex_sizer = wx.FlexGridSizer(2, 2, 0, 0)
        units_child_flex_sizer.Add(self.mayaUnitsLBL, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2)
        units_child_flex_sizer.Add(self.mayaUnitsCombo, 0, wx.ALL, 2)
        units_child_flex_sizer.Add(self.pandaUnitsLBL, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2)
        units_child_flex_sizer.Add(self.pandaUnitsCombo, 0, wx.ALL, 2)
        units_child_sizer.Add(units_child_flex_sizer, 1, wx.EXPAND|wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL, 0)
        self.units_panel.SetSizerAndFit(units_child_sizer)
        units_sizer.Add(self.units_panel, 1, wx.EXPAND, 0)
        units_sizer.Add(self.backfaceChk, 0,wx.ALIGN_CENTER_VERTICAL | wx.TOP, 3)
        units_sizer.Add(self.tbnallChk, 0, wx.ALIGN_CENTER_VERTICAL | wx.TOP, 3)
        subset_sizer.Add(self.useSubsetChk, 0, wx.ALIGN_CENTER | wx.TOP, 3)
        subset_sizer.Add(self.subsetsTxt, 0, wx.TOP|wx.ALIGN_CENTER_HORIZONTAL, 2)
        units_sizer.Add(subset_sizer, 1, wx.EXPAND, 0)
        genopts_static_sizer.Add(units_sizer, 1, wx.EXPAND|wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL, 0)
        self.panelGenOpts.SetSizerAndFit(genopts_static_sizer)
        maya2egg_sizer.Add(self.genpane, 1, wx.EXPAND|wx.ALIGN_CENTER_HORIZONTAL|wx.ALIGN_CENTER_VERTICAL, 0)
        
        anim_choice_sizer = wx.FlexGridSizer(5, 1, 0, 0)
        self.subs_static_sizer_staticbox.Lower()
        subs_static_sizer = wx.StaticBoxSizer(self.subs_static_sizer_staticbox, wx.HORIZONTAL)
        subroots_grid_sizer = wx.FlexGridSizer(1, 2, 2, 0)
        self.keyframe_static_sizer_staticbox.Lower()
        keyframe_static_sizer = wx.StaticBoxSizer(self.keyframe_static_sizer_staticbox, wx.HORIZONTAL)
        keyframe_grid_sizer = wx.GridSizer(4, 2, 0, 0)
        grid_sizer_2 = wx.FlexGridSizer(1, 2, 0, 0)
        anim_choice_sizer.Add(self.animOptChoice, 0, 0, 0)
        grid_sizer_2.Add(self.charChk, 0, wx.ALIGN_CENTER, 0)
        grid_sizer_2.Add(self.charTxt, 0, wx.ALIGN_CENTER, 0)
        anim_choice_sizer.Add(grid_sizer_2, 1, 0, 0)
        keyframe_grid_sizer.Add(self.sfChk, 0, 0, 0)
        keyframe_grid_sizer.Add(self.sfSpin, 0, 0, 0)
        keyframe_grid_sizer.Add(self.efChk, 0, 0, 0)
        keyframe_grid_sizer.Add(self.efSpin, 0, 0, 0)
        keyframe_grid_sizer.Add(self.friChk, 0, 0, 0)
        keyframe_grid_sizer.Add(self.friSpin, 0, 0, 0)
        keyframe_grid_sizer.Add(self.froChk, 0, 0, 0)
        keyframe_grid_sizer.Add(self.froSpin, 0, 0, 0)
        keyframe_static_sizer.Add(keyframe_grid_sizer, 1, 0, 0)
        anim_choice_sizer.Add(keyframe_static_sizer, 1, 0, 0)
        subroots_grid_sizer.Add(self.useSubrootsChk, 0, wx.ALIGN_CENTER, 0)
        subroots_grid_sizer.Add(self.subrootsTxt, 0, wx.EXPAND | wx.ALIGN_LEFT, 2)
        subs_static_sizer.Add(subroots_grid_sizer, 1, wx.EXPAND | wx.ALIGN_LEFT, 0)
        anim_choice_sizer.Add(subs_static_sizer, 1, wx.EXPAND, 0)
        self.animopts_main_panel.SetSizerAndFit(anim_choice_sizer)
        maya2egg_sizer.Add(self.animpane, 1, wx.EXPAND, 0)
        
        textures_grid_sizer = wx.FlexGridSizer(3,2,0,0)
        textures_grid_sizer.Add(self.useLegacyShaderCHK, 1, wx.ALIGN_LEFT,0)
        textures_grid_sizer.Add((20,20),0,0)
        textures_grid_sizer.Add(self.copyTexCHK, 1, wx.ALIGN_LEFT, 0)
        textures_grid_sizer.Add((20,20),0,0)
        textures_grid_sizer.Add(self.texDestPathTxt, 1,wx.EXPAND | wx.ALIGN_LEFT | wx.BOTTOM, 5)
        textures_grid_sizer.Add(self.texDestPathBTN, 1,wx.ALIGN_LEFT | wx.BOTTOM ,5)
        self.tex_panel.SetSizerAndFit(textures_grid_sizer)
        maya2egg_sizer.Add(self.texpane,1,wx.EXPAND, 0)
        maya2egg_sizer.Add(self.mayaAddToBatch, 0, wx.ALIGN_CENTER_VERTICAL|wx.TOP|wx.BOTTOM, 5)
        
        self.maya2eggPanel.SetSizer(maya2egg_sizer)
        
        #egg 2 bam panel
        egg2bam_sizer = wx.FlexGridSizer(6, 2, 0, 0)
        
        egg2bam_sizer.Add(self.egg2bam, 0, wx.TOP|wx.BOTTOM, 10)
        egg2bam_sizer.Add((20, 20), 0, wx.TOP|wx.BOTTOM, 10)
        egg2bam_sizer.Add(self.bamFlattenChk, 0, 0, 0)
        egg2bam_sizer.Add((20, 20), 0, 0, 0)
        egg2bam_sizer.Add(self.eggEmbedChk, 0, wx.TOP|wx.BOTTOM, 5)
        egg2bam_sizer.Add(self.bamUseCurrentChk, 0, wx.TOP|wx.BOTTOM, 5)
        egg2bam_sizer.Add(self.bamLoadText, 0, wx.EXPAND|wx.TOP|wx.BOTTOM|wx.RIGHT, 5)
        egg2bam_sizer.Add(self.bamLoadEgg, 0, wx.TOP|wx.BOTTOM, 5)
        egg2bam_sizer.Add(self.bamOutputText, 0, wx.EXPAND|wx.TOP|wx.BOTTOM|wx.RIGHT, 5)
        egg2bam_sizer.Add(self.bamChooseOutputDir, 0, wx.TOP|wx.BOTTOM, 5)
        egg2bam_sizer.Add(self.bamAddToBatch, 0, wx.ALIGN_CENTER_VERTICAL|wx.TOP|wx.BOTTOM, 5)
        self.egg2bamPanel.SetSizer(egg2bam_sizer)
        
        #egg-opt-char panel
        eggOptChar_grid_sizer = wx.FlexGridSizer(5, 2, 0, 0)
        
        eggOptChar_grid_sizer.Add(self.eggOptChar, 0, wx.TOP|wx.BOTTOM, 10)
        eggOptChar_grid_sizer.Add((20, 20), 0, wx.TOP|wx.BOTTOM, 10)
        eggOptChar_grid_sizer.Add(self.optEggLBL, 0,  wx.TOP|wx.BOTTOM, 10)
        eggOptChar_grid_sizer.Add(self.optUseCurrentChk, 0,wx.TOP|wx.BOTTOM, 10)
        eggOptChar_grid_sizer.Add(self.optEggTxt, 0, wx.EXPAND|wx.ALIGN_CENTER_VERTICAL, 0)
        eggOptChar_grid_sizer.Add(self.loadOptEggButton, 0, wx.LEFT, 5)
        eggOptChar_grid_sizer.Add(self.optOutputLBL, 0, wx.TOP|wx.BOTTOM|wx.ALIGN_CENTER_VERTICAL, 5)
        eggOptChar_grid_sizer.Add((20, 20), 0, wx.TOP|wx.BOTTOM, 5)
        eggOptChar_grid_sizer.Add(self.optOutputTxt, 0, wx.EXPAND, 0)
        eggOptChar_grid_sizer.Add(self.optChooseOutputButton, 0, wx.LEFT, 5)
        eggOptChar_grid_sizer.Add(self.optRunOptCharButton, 0, wx.TOP|wx.BOTTOM|wx.ALIGN_CENTER_VERTICAL, 5)
        self.eggOptCharPanel.SetSizer(eggOptChar_grid_sizer)
        
        #egg-palettize panel
        eggpalettize_sizer = wx.FlexGridSizer(5, 1, 0, 0)
        
        #file open panel

        self.single_egg_flex_grid_sizer = wx.FlexGridSizer(5, 3, 0, 0)
        self.single_egg_flex_grid_sizer.Add(self.ineggTxt, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2)
        self.single_egg_flex_grid_sizer.Add(self.ineggFileTxt, 0, wx.ALL, 2)
        self.single_egg_flex_grid_sizer.Add(self.inegg, 0, 0, 0)
        self.single_egg_flex_grid_sizer.Add(self.outtexTxt, 0, wx.ALL, 2)
        self.single_egg_flex_grid_sizer.Add(self.outtexFileTxt, 0, wx.ALL, 2)
        self.single_egg_flex_grid_sizer.Add(self.outtex, 0, 0, 0)
        self.single_egg_flex_grid_sizer.Add(self.outeggTxt, 0, wx.ALL, 2)
        self.single_egg_flex_grid_sizer.Add(self.outeggFileTxt, 0, wx.ALL, 2)
        self.single_egg_flex_grid_sizer.Add(self.outegg, 0, 0, 0)
        self.single_egg_panel.SetSizerAndFit(self.single_egg_flex_grid_sizer)
        
        self.mult_inegg_flex_grid_sizer = wx.FlexGridSizer(3, 1, 0, 0)
        self.mult_inegg_flex_grid_sizer.Add(self.mult_inegg, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2)
        self.mult_inegg_flex_grid_sizer.Add(self.mult_inegg_remove, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2)
        self.mult_inegg_flex_grid_sizer.Add(self.mult_inegg_remove_all, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2)
        self.mult_inegg_panel.SetSizerAndFit(self.mult_inegg_flex_grid_sizer)
        
        self.mult_egg_flex_grid_sizer = wx.FlexGridSizer(5, 3, 0, 0)
        self.mult_egg_flex_grid_sizer.Add(self.mult_ineggTxt, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2)
        self.mult_egg_flex_grid_sizer.Add(self.mult_batchTree, 0, wx.ALL, 2)
        self.mult_egg_flex_grid_sizer.Add(self.mult_inegg_panel, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2)
        self.mult_egg_flex_grid_sizer.Add(self.mult_outtexTxt, 0, wx.ALL, 2)
        self.mult_egg_flex_grid_sizer.Add(self.mult_outtexFileTxt, 0, wx.ALL, 2)
        self.mult_egg_flex_grid_sizer.Add(self.mult_outtex, 0, wx.ALL, 2)
        self.mult_egg_flex_grid_sizer.Add(self.mult_outeggTxt, 0, wx.ALL, 2)
        self.mult_egg_flex_grid_sizer.Add(self.mult_outeggFileTxt, 0, wx.ALL, 2)
        self.mult_egg_flex_grid_sizer.Add(self.mult_outegg, 0, wx.ALL, 2)
        self.multiple_egg_panel.SetSizerAndFit(self.mult_egg_flex_grid_sizer)
        
        self.eggpfile_flex_grid_sizer = wx.FlexGridSizer(2, 1, 0, 0)
        self.eggpfile_flex_grid_sizer.Add(self.single_egg_panel, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2 )
        self.multiple_egg_panel.Show(False)
        self.eggpformer = 'single_egg'
        self.eggpcurrent = 'single_egg'
        self.eggpfile_child_panel.SetSizerAndFit(self.eggpfile_flex_grid_sizer)
        
        attri_flex_grid_sizer = wx.FlexGridSizer(6, 1, 0, 0)
        size_sizer = wx.wx.FlexGridSizer(1, 3, 0, 0)
        size_sizer.Add(self.sizeTxt, 0, 0, 0)
        size_sizer.Add(self.sizeSpin1, 0, 0, 0)
        size_sizer.Add(self.sizeSpin2, 0, 0, 0)
        attri_flex_grid_sizer.Add(size_sizer, 0, 0, 0)
        attri_flex_grid_sizer.Add(self.imagetype, 0, 0, 0)
        attri_flex_grid_sizer.Add(self.powerflag, 0, 0, 0)
        
        self.color_static_sizer_staticbox.Lower()
        color_static_sizer = wx.StaticBoxSizer(self.color_static_sizer_staticbox, wx.HORIZONTAL)
        color_flex_grid_sizer = wx.FlexGridSizer(1, 8, 0, 0)
        color_flex_grid_sizer.Add(self.RTxt, 0, 0, 0)
        color_flex_grid_sizer.Add(self.RSpin, 0, 0, 0)
        color_flex_grid_sizer.Add(self.GTxt, 0, 0, 0)
        color_flex_grid_sizer.Add(self.GSpin, 0, 0, 0)
        color_flex_grid_sizer.Add(self.BTxt, 0, 0, 0)
        color_flex_grid_sizer.Add(self.BSpin, 0, 0, 0)
        color_flex_grid_sizer.Add(self.ATxt, 0, 0, 0)
        color_flex_grid_sizer.Add(self.ASpin, 0, 0, 0)
        color_static_sizer.Add(color_flex_grid_sizer, 1, 0, 0)
        
        attri_flex_grid_sizer.Add(color_static_sizer, 1, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 0)
        
        margin_flex_grid_sizer = wx.FlexGridSizer(1, 4, 0, 0)
        margin_flex_grid_sizer.Add(self.marginTxt, 0, wx.ALL|wx.ALIGN_CENTER, 2)
        margin_flex_grid_sizer.Add(self.marginSpin, 0, wx.ALL|wx.ALIGN_CENTER, 2)
        margin_flex_grid_sizer.Add(self.coverTxt, 0, wx.ALL|wx.ALIGN_CENTER, 2)
        margin_flex_grid_sizer.Add(self.coverSpin, 0, wx.ALL|wx.ALIGN_CENTER, 2)
        attri_flex_grid_sizer.Add(margin_flex_grid_sizer, 0, 0, 0)
        
        save_flex_grid_sizer = wx.FlexGridSizer(1, 2, 0, 0)
        save_flex_grid_sizer.Add(self.savetxa, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2)
        save_flex_grid_sizer.Add(self.savetxaFileTxt, 0, wx.ALL, 2)
        attri_flex_grid_sizer.Add(save_flex_grid_sizer, 0, 0, 0)
        self.attri_child_panel.SetSizerAndFit(attri_flex_grid_sizer)
        
        eggpalettize_sizer.Add(self.eggPalettize, 1, wx.TOP|wx.BOTTOM, 10)
        eggpalettize_sizer.Add(self.multieggComboBox, 1,  wx.TOP|wx.BOTTOM, 10 )
        eggpalettize_sizer.Add(self.eggpfile_child_panel, 1, wx.EXPAND, 0)
        eggpalettize_sizer.Add(self.attripane, 1, wx.EXPAND, 0)
        eggpalettize_sizer.Add(self.palettizeAddToBatch, 0, wx.ALIGN_CENTER_VERTICAL|wx.TOP|wx.BOTTOM, 5)
        self.eggPalettizePanel.SetSizer(eggpalettize_sizer)
        
        #whole up part console panel and maya2egg panel
        self.setup_grid_sizer.Add(self.panelBatch, 1, wx.EXPAND, 0)
        self.setup_panel.SetSizer(self.setup_grid_sizer)
        
        main_bottom_sizer = wx.FlexGridSizer(1, 4, 0, 0)
        main_bottom_sizer.Add(self.loadBatchButton, 1, wx.EXPAND|wx.LEFT|wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_CENTER_HORIZONTAL, 110)
        main_bottom_sizer.Add(self.saveBatchButton, 1, wx.EXPAND|wx.LEFT|wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_CENTER_HORIZONTAL, 100)
        main_bottom_sizer.Add(self.runBatchButton, 1, wx.EXPAND|wx.LEFT|wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_CENTER_HORIZONTAL, 100)
        main_bottom_sizer.Add(self.runPviewButton, 1, wx.EXPAND|wx.LEFT|wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_CENTER_HORIZONTAL, 100)
        self.runPanel.SetSizerAndFit(main_bottom_sizer)

        main_grid_sizer.Add(self.setup_panel, 1, wx.EXPAND, 0)
        main_grid_sizer.Add(self.runPanel, 1, wx.EXPAND|wx.ALL|wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_CENTER_HORIZONTAL, 10)
        self.main_panel.SetSizer(main_grid_sizer)
        
        main_sizer.Add(self.main_panel, 1, wx.EXPAND, 2) 
        
        #These are the notebook pages (or where they are added)
        #add more for more tools (ie dae2egg etc). 
        #NOTE you must create the panel in setup UI before 
        #adding to the sizer      
        self.tab_panel.AddPage(self.simple_panel, "Simple Mode")
        self.tab_panel.AddPage(self.main_panel, "Advanced Mode")
        tab_panel_sizer.Add(self.tab_panel,1,wx.EXPAND,0)   
        self.SetSizer(tab_panel_sizer)
        tab_panel_sizer.Fit(self)
    
        self.SetMinSize((WINDOW_WIDTH, WINDOW_HEIGHT))
        self.SetSize((WINDOW_WIDTH, WINDOW_HEIGHT))
        self.Layout()
        
    def ShowInitialEnv(self):#show the initial environment of the GUI   
        self.consoleOutputTxt.AppendText(WELCOME_MSG)
        self.pandaPathDir = DEFAULT_PANDA_DIR
        self.consoleOutputTxt.AppendText( "\nPanda initial path: " + self.pandaPathDir.rstrip('\\bin\\'))
        self.consoleOutputTxt.AppendText("\nIf this is not correct please use the Path options to change.")
        self.mayaVerComboBox.SetSelection(len(MAYA_VERSIONS)-1)
        self.consoleOutputTxt.AppendText("\nUsing Maya Version " + MAYA_VERSIONS[len(MAYA_VERSIONS)-1])
        self.consoleOutputTxt.AppendText("\nIf this is not correct please use the Path options to change.")
        self.pandaPathTxt.SetValue(self.pandaPathDir) 
        self.setup_grid_sizer.Add(self.maya2eggPanel, 1, wx.EXPAND|wx.LEFT|wx.TOP, 30)
        self.egg2bamPanel.Show(False)
        self.eggOptCharPanel.Show(False)
        self.eggPalettizePanel.Show(False)
        self.former = 'Maya2Egg'
        self.current = 'Maya2Egg'
        
    def ExpandPanes(self):
        #expand all the panels
        #Only called in init for now
        self.filepane.Expand()
        self.envpane.Expand()
        self.genpane.Expand()
        self.animpane.Expand()
        self.texpane.Expand()
        self.attripane.Expand()
   
    def OnPaneChanged(self,e):
        #need to refresh both the frame and the panel
        #otherwise the panes won't update
        self.maya2eggPanel.Layout()
        self.maya2eggPanel.Refresh()
        self.eggPalettizePanel.Layout()
        self.eggPalettizePanel.Refresh()
        self.eggpfile_child_panel.Layout()
        self.eggpfile_child_panel.Refresh()
        self.setup_panel.Layout()
        self.main_panel.Layout()
        self.Refresh()
        self.Layout()
        
    def OnEggpTool(self,e):
        #change egg-palettizes panels between simple egg and multiple eggs
        self.eggpformer = self.eggpcurrent
        self.eggpcurrent = self.multieggComboBox.GetValue()
       
        if self.eggpformer == 'single_egg':
            self.single_egg_panel.Show(False)
            self.eggpfile_flex_grid_sizer.Detach(self.single_egg_panel)
        if self.eggpformer == 'multiple_egg':
            self.multiple_egg_panel.Show(False)
            self.eggpfile_flex_grid_sizer.Detach(self.multiple_egg_panel)
            
        if self.eggpcurrent == 'single_egg':
            self.eggpfile_flex_grid_sizer.Add(self.single_egg_panel, 0, wx.EXPAND|wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2 )
            self.single_egg_panel.Show(True)
            self.eggpfile_child_panel.SetSizerAndFit(self.eggpfile_flex_grid_sizer)
        if self.eggpcurrent == 'multiple_egg':
            self.eggpfile_flex_grid_sizer.Add(self.multiple_egg_panel, 0,wx.EXPAND|wx.ALL|wx.ALIGN_CENTER_VERTICAL, 2 )
            self.multiple_egg_panel.Show(True)
            self.eggpfile_child_panel.SetSizerAndFit(self.eggpfile_flex_grid_sizer)

        self.eggpfile_child_panel.Layout()
        self.eggPalettizePanel.Layout()
        self.setup_panel.Layout()
        self.main_panel.Layout()
        
    def OnTool(self,e):
        #change panels of different tools
        self.former = self.current
        self.current = self.AddToolComboBox.GetValue()
       
        if self.former == 'Maya2Egg':
            self.maya2eggPanel.Show(False)
            self.setup_grid_sizer.Detach(self.maya2eggPanel)
        if self.former == 'Egg2Bam':
            self.egg2bamPanel.Show(False)
            self.setup_grid_sizer.Detach(self.egg2bamPanel)
        if self.former == 'egg-opt-char':
            self.eggOptCharPanel.Show(False)
            self.setup_grid_sizer.Detach(self.eggOptCharPanel)
        if self.former == 'egg-palettize':
            self.eggPalettizePanel.Show(False)
            self.setup_grid_sizer.Detach(self.eggPalettizePanel)
            
        if self.current == 'Maya2Egg':
            self.setup_grid_sizer.Add(self.maya2eggPanel, 1, wx.EXPAND|wx.LEFT|wx.TOP, 30)
            self.maya2eggPanel.Show(True)
        if self.current == 'Egg2Bam':
            self.setup_grid_sizer.Add(self.egg2bamPanel, 1, wx.EXPAND|wx.LEFT|wx.TOP, 30)
            self.egg2bamPanel.Show(True)
        if self.current == 'egg-opt-char':
            self.setup_grid_sizer.Add(self.eggOptCharPanel, 1, wx.EXPAND|wx.LEFT|wx.TOP, 30)
            self.eggOptCharPanel.Show(True)
        if self.current == 'egg-palettize':
            self.setup_grid_sizer.Add(self.eggPalettizePanel, 1, wx.EXPAND|wx.LEFT|wx.TOP, 30)
            self.eggPalettizePanel.Show(True)

        self.main_panel.Layout()
        
    def OnShowHelp(self,event):#show help files
        def _addBook(filename):
            if not self.help.AddBook(filename):
                wx.MessageBox("Unable to open: " + filename,
                              "Error", wx.OK|wx.ICON_EXCLAMATION)

        self.help = wx.html.HtmlHelpController()

        _addBook("helpfiles/help.hhp")
        
        self.help.DisplayContents()
        
    def OnChooseEggOutput(self, event): 
        #choose output egg for maya2egg
        dirname = ''
        filename = ''
        dlg = wx.FileDialog(self, "Choose an Egg file to load", dirname, "", "*.egg", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()            
            self.eggFileTxt.SetValue(os.path.join(dirname + os.sep , filename)) #this is for a text control
        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("Current Egg File is: " + dirname + os.sep + filename)
    
    def OnPickMayaFile(self, event):
        #choose input maya scene for maya2egg 
        filename = ''
        dirname = ''
        dlg = wx.FileDialog(self, "Choose an Egg file to load", dirname, "", "*.mb", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()  
            self.mayaFileTxt.SetValue(os.path.join(dirname + os.sep , filename)) #this is for a text control
        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("Current Scene File is: " + dirname + os.sep + filename)
        
    def OnPandaPathChoose(self, event): 
        #Choose Panda directory
        dlg = wx.DirDialog(self, "Choose your Panda directory:")
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.pandaPathTxt.SetValue(dlg.GetPath())
        dlg.Destroy() #otherwise just kill the file dialog
        
    def OnMayaVerChoose(self, event): 
        #choose maya version
        event.Skip()
        
    def OnCopyTexToggle(self,event):
        #get value from copytexture check box
        if self.copyTexCHK.GetValue():
            self.texDestPathTxt.Enable(True)
        else:
            self.texDestPathTxt.Enable(False)
    
    def ChooseTexCopyDest(self,event):
        #choose texture copy output directory
        dlg = wx.DirDialog(self, "Choose your output directory:")
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.texDestPathTxt.SetValue(dlg.GetPath()) #this is for a text control duh
        dlg.Destroy() #otherwise just kill the file dialog
        #self.statusBar.SetStatusText("Current Egg File is: " + dirname + '\\' + filename)
        
    def OnBamLoadEgg(self,event):
        #choose the input egg file for egg2bam
        filename = ''
        dirname = ''
        dlg = wx.FileDialog(self, "Choose an Egg file to BAM", dirname, "", "*.egg", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()            
            self.bamLoadText.SetValue(os.path.join(dirname + os.sep , self.filename)) #this is for a text control
        dlg.Destroy() #otherwise just kill the file dialog
        
    def OnBamChooseOutput(self,event):
        #choose the output bam file for egg2bam
        dirname = ''
        dlg = wx.FileDialog(self, "Choose an Egg file to load", dirname, "", "*.bam", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()            
            self.bamOutputText.SetValue(os.path.join(dirname + os.sep , self.filename)) #this is for a text control duh
        dlg.Destroy() #otherwise just kill the file dialog
        
    def EnableBamChoose(self,event):
        #check if we want to use a custom egg file or not for egg2bam panel
        if (not self.bamUseCurrentChk.GetValue()):
            self.bamLoadText.Enable()
            self.bamLoadEgg.Enable()
        else:
            self.bamLoadText.Disable()
            self.bamLoadEgg.Disable()
            
    def EnableOptChoose(self,event):
        #check if we want to use a custom egg file or not for egg-opt-char panel
        if (not self.optUseCurrentChk.GetValue()):
            self.optEggTxt.Enable()
            self.loadOptEggButton.Enable()
        else:
            self.optEggTxt.Disable()
            self.loadOptEggButton.Disable()
    
    def BamAddToBatch(self,event):
        #add command line to batch list for egg2bam panel
        args = self.BuildBamArgs()
        args += ' -o '
        fileDict = {}
        fileDict['cmd'] = 'egg2bam'
        fileDict['args'] = args
        fileDict['foutput'] = str(self.bamOutputText.GetValue())
        if (not self.bamUseCurrentChk.GetValue()):
            fileDict['finput'] = str(self.bamLoadText.GetValue())
        else:
            fileDict['finput'] = str(self.eggFileTxt.GetValue())
        modtime = os.path.getmtime(str(fileDict['finput']))
        fileDict['modtime'] = str(modtime)
        fileDict['quotes'] = '"'
        self.numFiles.append(fileDict)
        print "bam add"
        print self.numFiles
        
        self.AddToBatchDisplay(fileDict)
    
    def OnOptChooseEgg(self,event):
        #choose input egg for egg-opt-char panel
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a location and filename", dirname, "", "*.egg", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            self.optEggTxt.SetValue(os.path.join(dirname + os.sep, filename))
        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("Batch list exported to " + dirname + os.sep + filename)
        
    def OnOptChooseOutput(self,event):
        #choose output egg for egg-opt-char panel
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a location and filename", dirname, "", "*.egg", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            self.optOutputTxt.SetValue(os.path.join(dirname + os.sep, filename))
        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("Batch list exported to " + dirname + os.sep + filename)
        
    def OnRunOptChar(self,event):
        #add command line to batch list for egg-opt-char panel
        if (self.optEggTxt.GetValue != '' and self.optOutputTxt.GetValue() != ''):
            args = ''
            args += ' -o '
            fileDict = {}
            fileDict['cmd'] = 'egg-optchar'
            fileDict['args'] = args
            fileDict['foutput'] = str(self.optOutputTxt.GetValue())
            if (not self.optUseCurrentChk.GetValue()):
                fileDict['finput'] = str(self.optEggTxt.GetValue())
            else:
                fileDict['finput'] = str(self.eggFileTxt.GetValue())
            modtime = os.path.getmtime(str(fileDict['finput']))
            fileDict['modtime'] = str(modtime)
            fileDict['quotes'] = '"'
            self.numFiles.append(fileDict)
            self.AddToBatchDisplay(fileDict)
            
    def Onmultinegg(self,e):
        #add input egg file for multiple eggs panel in egg-palettize panel
        filename = ''
        dirname = ''
        dlg = wx.FileDialog(self, "Choose your input egg files", dirname, "", "*.egg", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory() 
            
            eggfileDict = {}
            eggfileDict = os.path.join(dirname + os.sep , filename)
            self.eggnumFiles.append(eggfileDict)

            self.AddEggDisplay(eggfileDict)
         
        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("The input egg File is: " + dirname + os.sep + filename)
        
    def Onmultineggremove(self,e):
        #remove selected input egg file from multiple eggs panel in egg-palettize panel
        item = self.mult_batchTree.GetSelection()
        if (item != self.mult_treeRoot):
            index  = self.mult_batchTree.GetItemText(item)[0]#find the number to remove from eggnumFiles
            index = int(index) - 1
            self.eggnumFiles.pop(index)
            self.mult_batchTree.Delete(item)
            self.UpdateeggBatchDisplay()
            
    def Onmultineggremoveall(self,event):
        #remove all input egg file from multiple eggs panel in egg-palettize panel
        self.mult_batchTree.DeleteAllItems()
        if self.eggnumFiles != []:
            self.eggnumFiles = []
        self.mult_treeRoot = self.mult_batchTree.AddRoot('Egg Files')
            
    def UpdateeggBatchDisplay(self):
        #update the output of the selected egg files of multiple eggs panel in egg-palettize panel
        self.mult_batchTree.DeleteAllItems()
        self.mult_treeRoot = self.mult_batchTree.AddRoot('Egg Files')
        index = 0
        for item in self.eggnumFiles:
            index += 1
            treeitem = item
            self.mult_batchTree.AppendItem(self.mult_treeRoot,str(index) + str(treeitem))
        self.mult_batchTree.ExpandAll()
            
    def Oninegg(self, event): 
        #choose the input egg file of single egg panel in egg-palettize panel
        filename = ''
        dirname = ''
        dlg = wx.FileDialog(self, "Choose an Egg file to load", dirname, "", "*.egg", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()  
            self.ineggFileTxt.SetValue(os.path.join(dirname + os.sep , filename)) #this is for a text control
        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("The input egg File is: " + dirname + os.sep + filename)
        
    def Onouttex(self,e):
        #choose the output texture directory of single egg panel in egg-palettize panel
        dlg = wx.DirDialog(self, "Choose your Output Texture directory:")
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.outtexFileTxt.SetValue(dlg.GetPath())
        dlg.Destroy()
    
    def Onmultouttex(self,e):
        #choose the output texture directory of multiple eggs panel in egg-palettize panel
        dlg = wx.DirDialog(self, "Choose your Output Texture directory:")
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.mult_outtexFileTxt.SetValue(dlg.GetPath())
        dlg.Destroy()
        
    def Onmultoutegg(self,e):
        #choose the output eggs directory of multiple eggs panel in egg-palettize panel
        dlg = wx.DirDialog(self, "Choose your Output Eggs directory:")
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            self.mult_outeggFileTxt.SetValue(dlg.GetPath())
        dlg.Destroy()
        
    def Onoutegg(self, event): 
        #choose the output egg file of single egg panel in egg-palettize panel
        filename = ''
        dirname = ''
        dlg = wx.FileDialog(self, "Specify your Egg file to output", dirname, "", "*.egg", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()  
            self.outeggFileTxt.SetValue(os.path.join(dirname + os.sep , filename)) #this is for a text control
        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("The output egg File is: " + dirname + os.sep + filename)
        
    def OnSavetxa(self,event):
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
            mydata+=str(str(self.sizeSpin1.GetValue()))
            mydata+=[' ']
            mydata+=str(str(self.sizeSpin2.GetValue()))
            mydata+=['\n:imagetype ']
            if(str(self.imagetype.GetSelection())=='0'):
                mydata+=['rgb']
            if(str(self.imagetype.GetSelection())=='1'):
                mydata+=['jpg']
            if(str(self.imagetype.GetSelection())=='2'):
                mydata+=['png']
            mydata+=['\n:powertwo ']
            mydata+=str(self.powerflag.GetSelection())
            mydata+=['\n:background ']
            mydata+=str(int(self.RSpin.GetValue()))
            mydata+=[' ']
            mydata+=str(int(self.GSpin.GetValue()))
            mydata+=[' ']
            mydata+=str(int(self.BSpin.GetValue()))
            mydata+=[' ']
            mydata+=str(int(self.ASpin.GetValue()))
            mydata+=['\n:margin ']
            mydata+=str(str(int(self.marginSpin.GetValue())))
            mydata+=['\n:coverage ']
            mydata+=str(str(self.coverSpin.GetValue()))
            for line in mydata:
               myfile.writelines(line)
            myfile.close()
            self.savetxaFileTxt.SetValue(os.path.join(dirname + os.sep , filename)) #this is for a text control
        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("The output Txa File is: " + dirname + os.sep + filename)
        
    def OnAddEggToBatch(self, event): 
        #add command line to batch list for maya2egg panel
        if ((self.mayaFileTxt.GetValue() == "") or (self.eggFileTxt.GetValue() == "")):
            dlg = wx.MessageDialog(self,"Both an input and output file must be present to add an item to the batch queue" ,"ERROR", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
            return
        finput = self.mayaFileTxt.GetValue()
        foutput = self.eggFileTxt.GetValue()
        modtime = os.path.getmtime(finput) #this is formated in seconds btw since it's used for comparisons
        modelOptions = self.BuildEggArgs()
        arguments = ' ' + modelOptions + ' -o '
        fileDict = {}
        fileDict['cmd'] = 'maya2egg' + self.mayaVerComboBox.GetStringSelection()
        fileDict['args'] = str(arguments)
        fileDict['finput'] = str(finput)
        fileDict['foutput'] = str(foutput)
        fileDict['modtime'] = str(modtime)
        fileDict['quotes'] = '"'
        self.numFiles.append(fileDict)

        self.AddToBatchDisplay(fileDict)
        
    def Onpalettizeadd(self, event):
        #add command line to batch list for egg-palettize panel
        fileDict = {}
        finput = {}
        if self.eggpcurrent == 'single_egg':
            if ((self.ineggFileTxt.GetValue() == "") or (self.outtexFileTxt.GetValue() == "") or (self.savetxaFileTxt.GetValue() == "")):
                dlg = wx.MessageDialog(self,"Need choose an egg file, a output texture file and a .txa file" ,"ERROR", wx.OK )
                dlg.ShowModal()
                dlg.Destroy()
                return
            finput = self.ineggFileTxt.GetValue()
            print finput
            foutput = self.outeggFileTxt.GetValue()
            modtime = os.path.getmtime(finput) #this is formated in seconds btw since it's used for comparisons
            arguments=' '+'-af'+' '+'"'+self.savetxaFileTxt.GetValue()+ '"'
            arguments +=' '+'-opt'+' '+' '+'-egg'+' '
            arguments +=' '+'-dm'+' '+ '"'+self.outtexFileTxt.GetValue()  + '"'
            arguments +=' '+'-o'+' '
            
            fileDict['cmd'] = 'egg-palettize' 
            fileDict['args'] = str(arguments)
            fileDict['finput'] = str(finput)
            fileDict['foutput'] = str(foutput)
            fileDict['modtime'] = str(modtime)
            fileDict['quotes'] = '"'
            self.numFiles.append(fileDict)
            
        if self.eggpcurrent == 'multiple_egg':
            if ((self.eggnumFiles == "") or (self.mult_outtexFileTxt.GetValue() == "") or (self.savetxaFileTxt.GetValue() == "")):
                dlg = wx.MessageDialog(self,"Need choose egg files, a output texture file and a .txa file" ,"ERROR", wx.OK )
                dlg.ShowModal()
                dlg.Destroy()
                return
            finput = ''
            if (self.eggnumFiles):
                for args in self.eggnumFiles:
                    finput +=  '"' + args + '"' + ' '
            print finput
            foutput = self.mult_outeggFileTxt.GetValue()
            modtime = 0 #this is formated in seconds btw since it's used for comparisons
            arguments=' '+'-af'+' '+'"'+self.savetxaFileTxt.GetValue()+ '"'
            arguments +=' '+'-opt'+' '+' '+'-egg'+' '
            arguments +=' '+'-dm'+' '+ '"'+self.mult_outtexFileTxt.GetValue()  + '"'
            arguments +=' '+'-d'+' '
            
            fileDict['cmd'] = 'egg-palettize' 
            fileDict['args'] = str(arguments)
            fileDict['finput'] = str(finput)
            fileDict['foutput'] = str(foutput)
            fileDict['modtime'] = str(modtime)
            fileDict['quotes'] = ''
            self.numFiles.append(fileDict)
        
        self.AddToBatchDisplay(fileDict)
        
    def SimpleExport(self,e):
        #This is essentially the same behavior as a batch export
        #build up the dictionary that gets passed to Run Export
        #with all relevant items to a given command and 
        #pass it to RunExport along with the batchmode boolean
        if self.simpleEggTxt.GetValue() == '':
            return
        if self.simpleMBtxt.GetValue() == '':
            return
        if self.simplemayaVerComboBox.GetValue() == MAYA_VERSIONS[0]:
            return
        args = {}
        args['cmd'] = 'maya2egg' 
        args['finput'] = str(self.simpleMBtxt.GetValue())
        args['foutput'] = str(self.simpleEggTxt.GetValue())
        args['args'] = " -a " + self.simpleAnimOptChoice.GetStringSelection() 
        args['args'] += " -o"
        
        self.outdlg.Show()
        self.RunExport(args,False)
        
    def LoadSimpleEgg(self,e):
        #choose the output egg file for simple mode
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a location and filename", dirname, "", "*.egg", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            self.simpleEggTxt.SetValue(os.path.join(dirname + os.sep, filename))
        dlg.Destroy() #otherwise just kill the file dialog
        
    def LoadSimpleMB(self,e):
        #choose the input maya scene for simple mode
        dirname = ''
        dlg = wx.FileDialog(self, "Choose a location and filename", dirname, "", "*.mb", wx.SAVE)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            self.simpleMBtxt.SetValue(os.path.join(dirname + os.sep, filename))
        dlg.Destroy() #otherwise just kill the file dialog
        
    def RunExport(self, arguments, batchmode):
        #run command lines
        
        #if it's maya2egg then we need to use the VER
        if batchmode:
            if arguments['cmd'] == 'maya2egg':        
                VER = self.GetMayaVersion()
            else:
                VER = ''
            PATH = self.pandaPathTxt.GetValue() + os.sep + 'bin' + os.sep 
        else:
            VER = self.simplemayaVerComboBox.GetStringSelection()
            PATH = ''
        #Detect the users platform to run the correct binary   
        if sys.platform == "win32":
            extension = ".exe"
        elif sys.platform == "darwin": #OSX
            extension = ""
        else: #Linux and UNIX
            extension = ""
        
        command = PATH  + arguments['cmd'] + VER + extension  +  ' ' +  arguments['args']
        command += ' ' + '"' + arguments['foutput'] + '"'
        command += ' ' + arguments['quotes'] + arguments['finput'] + arguments['quotes'] 

        print command
        try:
            p = subprocess.Popen(command, shell = True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines = True )
        except:
            dlg = wx.MessageDialog(self,"Failed To Find Or un the Exporter Application" ,"ERROR", wx.OK )
            dlg.ShowModal()
            dlg.Destroy()
        else:
            if batchmode:            
                while p.poll() is None:            
                    out = p.stdout.readline()
                    self.consoleOutputTxt.AppendText(out)
            else:
                while p.poll() is None:
                    out = p.stdout.readline()
                    self.outdlg.dlgOutText.AppendText(out)
                    
                    
    def AddToBatchDisplay(self,args):
        #add items to the display for batch list
        self.SetStatusText("Batch item added")
        item = ' '
        item += args['cmd'] + ' ' + args['args']
        item += '"' + args['foutput'] + '"' + ' '
        item += args['quotes'] + args['finput']  + args['quotes']    
        self.batchTree.AppendItem(self.treeRoot,str(len(self.numFiles)) + item)
        self.batchTree.ExpandAll()
        
    def AddEggDisplay(self,args):
        #add items to the display for egg file list for multiple egg panel of egg-palettize panel
        self.SetStatusText("Egg File added")
        item = args
        self.mult_batchTree.AppendItem(self.mult_treeRoot,str(len(self.eggnumFiles)) + ' ' + item)
        self.mult_batchTree.ExpandAll()
        
    def BuildEggArgs(self):
        #Build up all the command line arguments present in the maya2egg panel 
        result = '-a ' + self.animOptChoice.GetStringSelection()
        result += ' -ui ' + UNIT_TYPES[self.mayaUnitsCombo.GetSelection()]
        result += ' -uo ' + UNIT_TYPES[self.pandaUnitsCombo.GetSelection()]
        
        if (self.backfaceChk.GetValue()):
            result += ' -bface'
        
        if (self.tbnallChk.GetValue()):
            result += ' -tbnall'
            
        if (self.charChk.GetValue()):
            result += ' -cn ' + self.charTxt.GetValue()
        
        if (self.useSubsetChk.GetValue()):
            result += ' -subset ' + self.subsetsTxt.GetValue()
           
        if (self.useSubrootsChk.GetValue()):
            result += ' -subroot ' + self.subrootsTxt.GetValue()
            
        if (self.sfChk.GetValue()):
            result += ' -sf ' + str(self.sfSpin.GetValue())
                        
        if (self.efChk.GetValue()):
            result += ' -ef ' + str(self.efSpin.GetValue())
                    
        if (self.friChk.GetValue()):
            result += ' -fri ' + str(self.friSpin.GetValue())
            
        if (self.froChk.GetValue()):
            result += ' -fro ' + str(self.froSpin.GetValue())
        
        if (self.copyTexCHK.GetValue()):
            result += ' -copytex ' + '"' + str(self.texDestPathTxt.GetValue()) + '"'
            
        if (self.useLegacyShaderCHK.GetValue()):
            result += ' -legacy-shader '
        
        return result
    
    def BuildBamArgs(self):
        #Build up all the command line arguments present in the egg2bam panel 
        args = ''
        if self.bamFlattenChk.GetValue():
            args += ' -flatten 3 '
        if self.eggEmbedChk.GetValue():
            args += ' -rawtex '
        return args
    
    def OnRemoveBatch(self,event):
        #remove selected item from the batch list
        item = self.batchTree.GetSelection()
        if (item != self.treeRoot):
            index  = self.batchTree.GetItemText(item)[0]#find the number to remove from numFiles
            index = int(index) - 1
            self.numFiles.pop(index)
            self.batchTree.Delete(item)
            self.UpdateBatchDisplay()
            
    def UpdateBatchDisplay(self):
        #update the display of the batch list
        
        #For now just re calculate the entire list order
        self.batchTree.DeleteAllItems()
        self.treeRoot = self.batchTree.AddRoot('Batch Files')
        index = 0
        for item in self.numFiles:
            index += 1
            treeitem = ' '
            treeitem += item['cmd'] + ' ' + item['args']
            treeitem +=  item['foutput'] + ' '
            treeitem +=  item['finput'] 
            self.batchTree.AppendItem(self.treeRoot,str(index) + str(treeitem))
        self.batchTree.ExpandAll()
            
    def OnRemoveAllBatch(self,event):
        #remove all the items from the batch list
        self.batchTree.DeleteAllItems()
        if self.numFiles != []:
            self.numFiles = []
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
        
    def OnSaveBatch(self,event):
        #save current batch list       
        newdoc = Document()
        top_element = newdoc.createElement('batch')
        newdoc.appendChild(top_element)
        for item in self.numFiles:
            print "save item"
            print item
            
            batchitem = newdoc.createElement("batchitem")
            top_element.appendChild(batchitem)            
            cmd = newdoc.createTextNode(item['cmd'])
            args = newdoc.createTextNode(item['args'])
            finput = newdoc.createTextNode(item['finput'])
            foutput = newdoc.createTextNode(item['foutput'])
            modtime = newdoc.createTextNode(item['modtime'])
            quotes = newdoc.createTextNode(item['quotes'])
            cmdNode = newdoc.createElement('cmd')
            argsNode = newdoc.createElement('args')
            finputNode = newdoc.createElement('finput')
            foutputNode = newdoc.createElement('foutput')
            modtimeNode = newdoc.createElement('modtime')
            quotesNode = newdoc.createElement('quotes')
            
            batchitem.appendChild(cmdNode)
            batchitem.appendChild(argsNode)
            batchitem.appendChild(finputNode)
            batchitem.appendChild(foutputNode)
            batchitem.appendChild(modtimeNode)
            batchitem.appendChild(quotesNode)
            
            cmdNode.appendChild(cmd)
            argsNode.appendChild(args)
            finputNode.appendChild(finput)
            foutputNode.appendChild(foutput)
            modtimeNode.appendChild(modtime)
            quotesNode.appendChild(quotes)
            
        dirname = ''
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
        dirname = ''
        filename = ''
        dlg = wx.FileDialog(self, "Load Preferences file...", dirname, "", "*.xml", wx.OPEN)
        if dlg.ShowModal() == wx.ID_OK: #if the user clicked ok then we grabbed a file so load it
            filename = dlg.GetFilename()
            dirname = dlg.GetDirectory()
            file = dirname + os.sep + filename
            #sanity check to see if there's anything hanging out in the batch tree
            #May want to prompt the user if we should overwrite the list or not
            if self.numFiles != []:
                #if there is delete it from the internal list
                self.numFiles = []
            #Always delete it visually
            self.batchTree.DeleteAllItems()
            #Re-add the root item
            self.treeRoot = self.batchTree.AddRoot('Batch Files')
            doc = xml.dom.minidom.parse(str(file))
            for node in doc.getElementsByTagName('batchitem'):
                displaymap = {}
                cmd = ''
                args = ''
                finput = ''
                foutput = ''
                modtime = ''
                quotes = ''
                elemcmd = node.getElementsByTagName('cmd')
                for node2 in elemcmd:
                    for node3 in node2.childNodes:
                        cmd = node3.data
                elemargs = node.getElementsByTagName('args')
                for node2 in elemargs:
                    for node3 in node2.childNodes:
                        args = node3.data
                elemfinput = node.getElementsByTagName('finput')
                for node2 in elemfinput:
                    for node3 in node2.childNodes:
                        finput = node3.data
                elemoutput = node.getElementsByTagName('foutput')
                for node2 in elemoutput:
                    for node3 in node2.childNodes:
                        foutput = node3.data
                elemmodtime = node.getElementsByTagName('modtime')
                for node2 in elemmodtime:
                    for node3 in node2.childNodes:
                        modtime = node3.data
                elemmodtime = node.getElementsByTagName('quotes')
                for node2 in elemmodtime:
                    for node3 in node2.childNodes:
                        quotes = node3.data
                displaymap['cmd'] = str(cmd).strip()
                displaymap['args'] = str(args).strip()
                displaymap['finput'] = str(finput).strip()
                displaymap['foutput'] = str(foutput).strip()
                displaymap['modtime'] = str(modtime).strip()
                displaymap['quotes'] = str(quotes).strip()
                
                
                self.numFiles.append(displaymap)
                #print self.numFiles
                self.AddToBatchDisplay(displaymap)
                       
        dlg.Destroy() #otherwise just kill the file dialog
        self.statusBar.SetStatusText("Batch imported succesfully from:" + dirname + os.sep + filename)
        
    def OnRunBatch(self, event):
        #run current batch list        
        self.batchProgress = wx.ProgressDialog('Progress', "Running Batch export please be patient...", maximum = len(self.numFiles),  style = wx.PD_REMAINING_TIME)
        self.batchProgress.SetSizeWH(300,150)
        self.batchProgress.Show()
        index = 0
        if (self.numFiles):
            self.SetStatusText("Running Batch export please be patient...")
            for args in self.numFiles:
                self.batchProgress.Update(index ,'')
                index += 1
                if (self.ignoreModDates.GetValue()): #do we want to just egg them regardless if they're newer?
                    self.RunExport(args,True)
                else:
                    if self.CheckModDates(args):
                        self.RunExport(args,True)
                    else:
                        break
                
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
            
    def CheckModDates(self, args):
        #This function checks the modified dates of the MB files in the batch list
        #it is skipped if the check is disabled/overridden
        finput = args['finput']
        recordmodtime = args['modtime']
        checktime = os.path.getmtime(finput)

        if (checktime > recordmodtime):
            return True
        else:
            return False
            
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
        mayaver = newdoc.createTextNode(str(self.mayaVerComboBox.GetSelection()))
        pandadirElem = newdoc.createElement('pandadir')
        mayaverElem = newdoc.createElement('mayaver')
        pandadirElem.appendChild(pandadir)
        mayaverElem.appendChild(mayaver)
        envitem.appendChild(pandadirElem)
        envitem.appendChild(mayaverElem)
        
        inunits = newdoc.createTextNode(str(self.mayaUnitsCombo.GetValue()))
        outunits = newdoc.createTextNode(str(self.pandaUnitsCombo.GetValue()))
        bface = newdoc.createTextNode(str(int(self.backfaceChk.GetValue())))
        tbnall = newdoc.createTextNode(str(int(self.tbnallChk.GetValue())))
        subsets = newdoc.createTextNode(str(int(self.useSubsetChk.GetValue())))
        subsetsval = newdoc.createTextNode(str(self.subsetsTxt.GetValue()))
        
        inunitsElem = newdoc.createElement('inunits')
        outunitsElem = newdoc.createElement('outunits')
        bfaceElem = newdoc.createElement('bface')
        tbnallElem = newdoc.createElement('tbnall')
        subsetsElem = newdoc.createElement('subsets')
        subnamesElem = newdoc.createElement('subnames')
        
        inunitsElem.appendChild(inunits)
        outunitsElem.appendChild(outunits)
        bfaceElem.appendChild(bface)
        tbnallElem.appendChild(tbnall)
        subsetsElem.appendChild(subsets)
        subnamesElem.appendChild(subsetsval)
        
        genitem.appendChild(inunitsElem)
        genitem.appendChild(outunitsElem)
        genitem.appendChild(bfaceElem)
        genitem.appendChild(tbnallElem)
        genitem.appendChild(subsetsElem)
        genitem.appendChild(subnamesElem)
        
        
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
        
        modelopts = newdoc.createTextNode(str(self.animOptChoice.GetSelection()))
        cn = newdoc.createTextNode(str(int(self.charChk.GetValue())))
        charname = newdoc.createTextNode(str(self.charTxt.GetValue()))
        
        modeloptsElem.appendChild(modelopts)
        cnElem.appendChild(cn)
        charnameElem.appendChild(charname)
        
        sf = newdoc.createTextNode(str(int(self.sfChk.GetValue())))
        sfval = newdoc.createTextNode(str(self.sfSpin.GetValue()))
        ef = newdoc.createTextNode(str(int(self.efChk.GetValue())))
        efval = newdoc.createTextNode(str(self.efSpin.GetValue()))
        fri = newdoc.createTextNode(str(int(self.friChk.GetValue())))
        frival = newdoc.createTextNode(str(self.friSpin.GetValue()))
        fro = newdoc.createTextNode(str(int(self.froChk.GetValue())))
        froval = newdoc.createTextNode(str(self.froSpin.GetValue()))
        
        sfElem.appendChild(sf)
        sfvalElem.appendChild(sfval)
        efElem.appendChild(ef)
        efvalElem.appendChild(efval)
        friElem.appendChild(fri)
        frivalElem.appendChild(frival)
        froElem.appendChild(fro)
        frovalElem.appendChild(froval)
        
        subroots = newdoc.createTextNode(str(int(self.useSubrootsChk.GetValue())))
        subrnames = newdoc.createTextNode(str(self.subrootsTxt.GetValue()))
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
        legacy_shader = newdoc.createTextNode(str(int(self.useLegacyShaderCHK.GetValue())))
        copytex = newdoc.createTextNode(str(int(self.copyTexCHK.GetValue())))
        destpath = newdoc.createTextNode(str(self.texDestPathTxt.GetValue()))
        
        legacy_shaderElem.appendChild(legacy_shader)
        copytexElem.appendChild(copytex)
        destpathElem.appendChild(destpath)
        
        texitem.appendChild(legacy_shaderElem)
        texitem.appendChild(copytexElem)
        texitem.appendChild(destpathElem)
                
        override = newdoc.createTextNode(str(int(self.ignoreModDates.GetValue())))
        overitem.appendChild(override)
        
        imagetype = newdoc.createTextNode(str(self.imagetype.GetSelection()))
        powertwo = newdoc.createTextNode(str(self.powerflag.GetSelection()))
        imagetypeElem = newdoc.createElement('imagetype')
        powertwoElem = newdoc.createElement('powertwo')
        R = newdoc.createTextNode(str(int(self.RSpin.GetValue())))
        G = newdoc.createTextNode(str(int(self.GSpin.GetValue())))
        B = newdoc.createTextNode(str(int(self.BSpin.GetValue())))
        A = newdoc.createTextNode(str(int(self.ASpin.GetValue())))
        margin = newdoc.createTextNode(str(int(self.marginSpin.GetValue())))
        coverage = newdoc.createTextNode(str(self.coverSpin.GetValue()))
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
        self.mayaVerComboBox.SetSelection(int(prefs['mayaver']))
        self.mayaUnitsCombo.SetValue(prefs['inunits'])
        self.pandaUnitsCombo.SetValue(prefs['outunits'])
        self.backfaceChk.SetValue(bool(prefs['bface']))
        self.tbnallChk.SetValue(bool(int(prefs['tbnall'])))
        self.useSubsetChk.SetValue(bool(int(prefs['subsets'])))
        self.subsetsTxt.SetValue(prefs['subnames'])
        self.animOptChoice.SetSelection(int(prefs['modelopts']))
        self.charChk.SetValue(bool(int(prefs['cn'])))
        self.charTxt.SetValue(prefs['charname'])
        self.sfChk.SetValue(bool(int(prefs['sf'])))
        self.sfSpin.SetValue(int(prefs['sfval']))
        self.efChk.SetValue(bool(int(prefs['ef'])))
        self.efSpin.SetValue(int(prefs['efval']))
        self.friChk.SetValue(bool(int(prefs['fri'])))
        self.friSpin.SetValue(int(prefs['frival']))
        self.froChk.SetValue(bool(int(prefs['fro'])))
        self.froSpin.SetValue(int(prefs['froval']))
        self.useSubrootsChk.SetValue(bool(int(prefs['subroots'])))
        self.subrootsTxt.SetValue(prefs['subrnames'])
        self.ignoreModDates.SetValue(bool(int(prefs['overridemod'])))
        self.useLegacyShaderCHK.SetValue(bool(int(prefs['legacy-shader'])))
        self.copyTexCHK.SetValue(bool(int(prefs['copytex'])))
        self.texDestPathTxt.SetValue(prefs['path'])
        self.imagetype.SetSelection(int(prefs['imagetype']))
        self.powerflag.SetSelection(int(prefs['powertwo']))
        self.RSpin.SetValue(int(prefs['R']))
        self.GSpin.SetValue(int(prefs['G']))
        self.BSpin.SetValue(int(prefs['B']))
        self.ASpin.SetValue(int(prefs['A']))
        self.marginSpin.SetValue(int(prefs['margin']))
        self.coverSpin.SetValue(prefs['coverage'])

        
if __name__ == "__main__":
    app = wx.App(0)
    wx.InitAllImageHandlers()
    new_frame = main(None, -1, "")
    app.SetTopWindow(new_frame)
    new_frame.Show()
    app.MainLoop()
