###############################################################################
# Name: visualbasic.py                                                        #
# Purpose: Define Visual Basic syntax for highlighting and other features     #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2007 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
FILE: visualbasic.py
AUTHOR: Cody Precord
@summary: Lexer configuration module for Visual Basic.
@todo: Incomplete requires color/kw tuning

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: visualbasic.py 52852 2008-03-27 13:45:40Z CJP $"
__revision__ = "$Revision: 52852 $"

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

# Visual Basic Keywords (Statements)
VB_KW = (0, "AppActivate Base Beep Begin Call Case ChDir ChDrive Const Declare "
            "DefByte DefCur DefDate DefDbl DefDec DefInt DefLng DefObj DefSng "
            "DefStr Deftype DefVar DeleteSetting Dim Do Else End Enum Erase "
            "Event Exit Explicit FileCopy For ForEach Function Get GoSub GoTo "
            "If Implements Kill Let LineInput Lock LSet MkDir Name Next "
            "On Option Private Property Public Put RaiseEvent Randomize ReDim "
            "Rem Reset Resume Return RmDir RSet SavePicture SaveSetting With "
            "SendKeys SetAttr Static Sub Then Type Unlock Wend While Width "
            "Write Height DefBool OnError ")

# Visual Basic User Keywords 1 (Functions)
VB_UKW1 = (1, "Abs Array Asc AscB AscW Atn Avg CBool CByte CCur CDate CDbl "
              "Choose Chr ChrB ChrW CInt CLng Command Cos Count CreateObject "
              "CSng CStr CurDir CVar CVDate CVErr Date DateAdd DateDiff Cdec "
              "DatePart DateSerial DateValue Day DDB Dir DoEvents Environ EOF "
              "Error Exp FileAttr FileDateTime FileLen Fix Format FreeFile FV "
              "GetAllStrings GetAttr GetAutoServerSettings GetObject NPV "
              "Hex Hour IIf IMEStatus Input InputB InputBox InStr InstB Int "
              "IPmt IsArray IsDate IsEmpty IsError IsMissing IsNull IsNumeric "
              "IsObject LBound LCase Left LeftB Len LenB LoadPicture Loc LOF "
              "Log LTrim Max Mid MidB Min Minute MIRR Month MsgBox Now NPer "
              "Oct Partition Pmt PPmt PV QBColor Rate RGB Right RightB Rnd "
              "RTrim Second Seek Sgn Shell Sin SLN Space Spc Sqr StDev StDevP "
              "Str StrComp StrConv String Switch Sum SYD Tab Tan Time Timer "
              "TimeSerial TimeValue Trim TypeName UBound UCase Val Var VarP "
              "VarType Weekday Year GetSetting ")

# Visual Basic User Keywords 2 (Methods)
VB_UKW2 = (2, "Accept Activate Add AddCustom AddFile AddFromFile AddItem "
              "AddFromTemplate AddNew AddToAddInToolbar AddToolboxProgID "
              "Append AppendChunk Arrange Assert AsyncRead BatchUpdate "
              "BeginTrans Bind Cancel CancelAsyncRead CancelBatch CancelUpdate "
              "CanPropertyChange CaptureImage CellText CellValue Circle Clear "
              "ClearFields ClearSel ClearSelCols Clone Close Cls ColContaining "
              "ColumnSize CommitTrans CompactDatabase Compose Connect Copy "
              "CopyQueryDef CreateDatabase CreateDragImage CreateEmbed "
              "CreateField CreateGroup CreateIndex CreateLink Customize"
              "CreatePreparedStatement CreatePropery CreateQueryCreateQueryDef "
              "CreateRelation CreateTableDef CreateUser CreateWorkspace "
              "Delete DeleteColumnLabels DeleteColumns DeleteRowLabels Open "
              "DeleteRows DoVerb Drag Draw Edit EditCopy EditPaste EndDoc "
              "EnsureVisible EstablishConnection Execute ExtractIcon Fetch "
              "FetchVerbs Files FillCache Find FindFirst FindItem FindLast "
              "FindNext GoForward KillDoc LoadFile MakeCompileFile MoveNext "
              "FindPrevious Forward GetBookmark GetChunk GetClipString GetData "
              "GetFirstVisible GetFormat GetHeader GetLineFromChar GetNumTicks "
              "GetRows GetSelectedPart GetText GetVisibleCount GoBack OLEDrag "
              "Hide HitTest HoldFields Idle InitializeLabels InsertRows Item "
              "InsertColumnLabels InsertColumns InsertObjDlg InsertRowLabels "
              "Layout Line LinkExecute LinkPoke LinkRequest LinkSend Listen "
              "LoadResData LoadResPicture LoadResString LogEvent OpenResultset "
              "MakeReplica MoreResults Move MoveData MoveFirst MoveLast Point "
              "MovePrevious NavigateTo NewPage NewPassword NextRecordset Quit "
              "OnAddinsUpdate OnConnection OnDisconnection OnStartupComplete "
              "OpenConnection OpenDatabase OpenQueryDef OpenRecordset Reload "
              "OpenURL Overlay PaintPicture Paste PastSpecialDlg PeekData Play "
              "PopulatePartial PopupMenu Print PrintForm PropertyChanged PSet "
              "Raise RandomDataFill RandomFillColumns RandomFillRows Remove "
              "rdoCreateEnvironment rdoRegisterDataSource ReadFromFile "
              "Rebind ReFill Refresh RefreshLink RegisterDatabase ReadProperty "
              "RemoveAddInFromToolbar RemoveItem Render RepairDatabase Reply "
              "ReplyAll Requery ResetCustom ResetCustomLabel ResolveName "
              "RestoreToolbar Resync Rollback RollbackTrans RowBookmark "
              "RowContaining RowTop Save SaveAs SaveFile SaveToFile SelectAll "
              "SaveToolbar SaveToOle1File Scale ScaleX ScaleY Scroll Select "
              "SelectPart SelPrint Send SendData Set SetAutoServerSettings "
              "SetData SetFocus SetOption SetSize SetText SetViewport Show "
              "ShowColor ShowFont ShowHelp ShowOpen ShowPrinter ShowSave "
              "ShowWhatsThis SignOff SignOn Size Span SplitContaining "
              "StartLabelEdit StartLogging Stop Synchronize TextHeight "
              "TextWidth ToDefaults TwipsToChartPart TypeByChartType "
              "Update UpdateControls UpdateRecord UpdateRow Upto WhatsThisMode "
              "WriteProperty ZOrder")

# Visual Basic User Keywords 3 (Events)
VB_UKW3 = (3, "AccessKeyPress AfterAddFile AfterChangeFileName AfterCloseFile "
              "AfterColEdit AfterColUpdate AfterDelete AfterInsert "
              "AfterLabelEdit AfterRemoveFile AfterUpdate AfterWriteFile "
              "AmbienChanged ApplyChanges Associate AsyncReadComplete "
              "AxisActivated AxisLabelActivated AxisLabelSelected Collapse "
              "AxisLabelUpdated AxisSelected AxisTitleActivated BeforeColEdit "
              "AxisTitleSelected AxisTitleUpdated AxisUpdated BeforeClick "
              "BeforeColUpdate BeforeConnect BeforeDelete BeforeInsert "
              "BeforeLabelEdit BeforeLoadFile BeforeUpdate ButtonClick "
              "ButtonCompleted ButtonGotFocus ButtonLostFocus Change ColResize "
              "ChartActivated ChartSelected ChartUpdated Click ColEdit "
              "ColumnClick Compare ConfigChageCancelled ConfigChanged "
              "ConnectionRequest DataArrival DataChanged DataUpdated DblClick "
              "Deactivate DeviceArrival DeviceOtherEvent DeviceQueryRemove "
              "DeviceQueryRemoveFailed DeviceRemoveComplete DoGetNewFileName "
              "DeviceRemovePending DevModeChange Disconnect DisplayChanged "
              "Dissociate Done DonePainting DownClick DragDrop DragOver "
              "DropDown EditProperty EnterCell EnterFocus ExitFocus Expand "
              "FootnoteActivated FootnoteSelected FootnoteUpdated GotFocus "
              "HeadClick InfoMessage Initialize IniProperties ItemActivated "
              "ItemAdded ItemCheck ItemClick ItemReloaded ItemRemoved "
              "ItemRenamed ItemSeletected KeyDown KeyPress KeyUp LeaveCell "
              "LegendActivated LegendSelected LegendUpdated LinkClose "
              "LinkError LinkNotify LinkOpen Load LostFocus MouseDown "
              "MouseMove MouseUp NodeClick ObjectMove OLECompleteDrag "
              "OLEDragDrop OLEDragOver OLEGiveFeedback OLESetData OLEStartDrag "
              "OnAddNew OnComm Paint PanelClick PanelDblClick PathChange "
              "PatternChange PlotActivated PlotSelected PlotUpdated "
              "PointActivated Reposition SelChange StateChanged TitleActivated "
              "PointLabelActivated PointLabelSelected PointLabelUpdated "
              "PointSelected PointUpdated PowerQuerySuspend PowerResume "
              "PowerStatusChanged PowerSuspend QueryChangeConfig QueryComplete "
              "QueryCompleted QueryTimeout QueryUnload ReadProperties "
              "RequestChangeFileName RequestWriteFile Resize ResultsChanged "
              "RowColChange RowCurrencyChange RowResize RowStatusChanged "
              "SelectionChanged SendComplete SendProgress SeriesActivated "
              "SeriesSelected SeriesUpdated SettingChanged SplitChange Unload "
              "StatusUpdate SysColorsChanged Terminate TimeChanged "
              "TitleSelected TitleActivated UnboundAddData UnboundDeleteRow "
              "UnboundGetRelativeBookmark UnboundReadData UnboundWriteData "
              "UpClick Updated Validate ValidationError WillAssociate "
              "WillDissociate WillExecute WillUpdateRows WriteProperties "
              "WillChangeData")

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ ('STC_B_ASM', 'asm_style'),
                 ('STC_B_BINNUMBER', 'default_style'), # STYLE NEEDED
                 ('STC_B_COMMENT', 'comment_style'),
                 ('STC_B_CONSTANT', 'const_style'),
                 ('STC_B_DATE', 'default_style'), # STYLE NEEDED
                 ('STC_B_DEFAULT', 'default_style'),
                 ('STC_B_ERROR', 'error_style'),
                 ('STC_B_HEXNUMBER', 'number_style'),
                 ('STC_B_IDENTIFIER', 'default_style'),
                 ('STC_B_KEYWORD', 'keyword_style'),
                 ('STC_B_KEYWORD2', 'class_style'),   # STYLE NEEDED
                 ('STC_B_KEYWORD3', 'funct_style'), # STYLE NEEDED
                 ('STC_B_KEYWORD4', 'scalar_style'), # STYLE NEEDED
                 ('STC_B_LABEL', 'directive_style'), # STYLE NEEDED
                 ('STC_B_NUMBER', 'number_style'),
                 ('STC_B_OPERATOR', 'operator_style'),
                 ('STC_B_PREPROCESSOR', 'pre_style'),
                 ('STC_B_STRING', 'string_style'),
                 ('STC_B_STRINGEOL', 'stringeol_style')
               ]

#---- Extra Properties ----#
FOLD = ("fold", "1")

#-----------------------------------------------------------------------------#

#---- Required Module Functions ----#
def Keywords(lang_id=0):
    """Returns Specified Keywords List
    @param lang_id: used to select specific subset of keywords

    """
    keywords = list()
    tmp = [VB_KW, VB_UKW1, VB_UKW2, VB_UKW3]
    for keyw in tmp:
        keywords.append((keyw[0], keyw[1].lower()))
    return keywords

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @param lang_id: used for selecting a specific subset of syntax specs

    """
    return SYNTAX_ITEMS

def Properties(lang_id=0):
    """Returns a list of Extra Properties to set
    @param lang_id: used to select a specific set of properties

    """
    return [FOLD]

def CommentPattern(lang_id=0):
    """Returns a list of characters used to comment a block of code
    @param lang_id: used to select a specific subset of comment pattern(s)

    """
    return [u'\'']
#---- End Required Module Functions ----#

#---- Syntax Modules Internal Functions ----#
def KeywordString(option=0):
    """Returns the specified Keyword String
    @note: not used by most modules

    """
    return None

#---- End Syntax Modules Internal Functions ----#
