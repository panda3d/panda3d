###############################################################################
# Name: progress.py                                                           #
# Purpose: Define Progress 4gl syntax for highlighting and other features     #
# Author: Cody Precord <cprecord@editra.org>                                  #
# Copyright: (c) 2008 Cody Precord <staff@editra.org>                         #
# License: wxWindows License                                                  #
###############################################################################

"""
Syntax highlighting definition for Progress 4GL programming language.

"""

__author__ = "Cody Precord <cprecord@editra.org>"
__svnid__ = "$Id: progress.py 57048 2008-12-01 00:09:05Z CJP $"
__revision__ = "$Revision: 57048 $"

#-----------------------------------------------------------------------------#
# Imports
import synglob
import sql

#-----------------------------------------------------------------------------#

# Reserved Progress 4GL keywords
PROG_KW = (0, "accumulate active-window add alias all alter ambiguous analyze " 
              "and any apply as ascending assign at attr-space authorization "
              "auto-return available background before-hide begins bell "
              "between blank break btos by call can-do can-find centered "
              "character check chr clear clipboard col colon color column "
              "column-label columns compiler connected control count-of "
              "cpstream create ctos current current-changed current-language "
              "current-window current_date cursor database dataservers "
              "dbcodepage dbcollation dbname dbrestrictions dbtaskid dbtype "
              "dbversion dde deblank debug-list debugger decimal decimals "
              "declare def default default-noxlate default-window define "
              "delete delimiter descending dictionary disable disconnect disp "
              "display distinct dos down drop editing enable encode entry "
              "error-status escape etime except exclusive exclusive-lock "
              "exclusive-web-user exists export false fetch field fields "
              "file-information fill find find-case-sensitive find-global "
              "find-next-occurrence find-prev-occurrence find-select "
              "find-wrap-around first first-of focus font form format frame "
              "frame-col frame-db frame-down frame-field frame-file "
              "frame-index frame-line frame-name frame-row frame-value from "
              "from-chars from-pixels gateways get-byte get-codepages "
              "get-collations get-key-value getbyte global go-on go-pending "
              "grant graphic-edge group having header help hide import in "
              "index indicator input input-output insert integer into is "
              "is-attr-space join kblabel key-code key-function key-label "
              "keycode keyfunction keylabel keys keyword label last last-event "
              "last-key last-of lastkey ldbname leave library like "
              "line-counter listing locked lookup machine-class map member "
              "message message-lines mouse mpe new next next-prompt no "
              "no-attr-space no-error no-fill no-help no-hide no-labels "
              "no-lock no-map no-message no-pause no-prefetch no-undo "
              "no-validate no-wait not null num-aliases num-dbs num-entries "
              "of off old on open opsys option or os-append os-command "
              "os-copy os-create-dir os-delete os-dir os-drives os-error "
              "os-rename os2 os400 output overlay page page-bottom page-number "
              "page-top parameter pause pdbname persistent pixels preprocess "
              "privileges proc-handle proc-status process program-name "
              "Progress prompt prompt-for promsgs propath proversion put "
              "put-byte put-key-value putbyte query query-tuning quit r-index "
              "rcode-information readkey recid record-length rectangle release "
              "reposition retain retry return return-value revert revoke run "
              "save schema screen screen-io screen-lines scroll sdbname search "
              "seek select self session set setuserid share-lock shared "
              "show-stats skip some space status stream stream-io string-xref "
              "system-dialog table term terminal text text-cursor "
              "text-seg-growth this-procedure time title to today top-only "
              "trans transaction trigger triggers trim true underline undo "
              "unformatted union unique unix up update use-index use-revvideo "
              "use-underline user userid using v6frame value values variable "
              "view view-as vms wait-for web-context window window-maximized "
              "window-minimized window-normal with work-table workfile write "
              "xcode xref yes _cbit _control _list _memory _msg _pcontrol "
              "_serial-num _trace "
              "repeat transaction for each end finf where if then else skip "
              "close"
)

# Progress 4GL Types
PROG_TYPES = (1, "char character int integer format var variable log logical "
                 "da date")

# Progress 4GL Operators
PROG_OP = (7, "absolute accelerator across add-first add-last advise alert-box "
              "allow-replication ansi-only anywhere append appl-alert-boxes "
              "application as-cursor ask-overwrite attachment auto-end-key "
              "auto-endkey auto-go auto-indent auto-resize auto-zap "
              "available-formats average avg backwards base-key batch-mode "
              "bgcolor binary bind-where block-iteration-display border-bottom "
              "border-bottom-chars border-bottom-pixels border-left "
              "border-left-chars border-left-pixels border-right "
              "border-right-chars border-right-pixels border-top "
              "border-top-chars border-top-pixels both bottom box "
              "box-selectable browse browse-header buffer buffer-chars "
              "buffer-lines button buttons byte cache cache-size can-query "
              "can-set cancel-break cancel-button caps careful-paint "
              "case-sensitive cdecl character character_length charset checked "
              "choose clear-selection close code codepage codepage-convert "
              "col-of colon-aligned color-table column-bgcolor column-dcolor "
              "column-fgcolor column-font column-label-bgcolor "
              "column-label-dcolor column-label-fgcolor column-label-font "
              "column-of column-pfcolor column-scrolling combo-box command "
              "compile complete connect constrained contents context "
              "context-popup control-container control-form convert-to-offset "
              "convert count cpcase cpcoll cpinternal cplog cpprint cprcodein "
              "cprcodeout cpterm crc-value create-control "
              "create-result-list-entry create-test-file current-column "
              "current-environment current-iteration current-result-row "
              "current-row-modified current-value cursor-char cursor-line "
              "cursor-offset data-entry-return data-type date date-format day "
              "db-references dcolor dde-error dde-id dde-item dde-name "
              "dde-topic debug decimal default-button default-extension "
              "defer-lob-fetch define defined delete-char delete-current-row "
              "delete-line delete-selected-row delete-selected-rows "
              "deselect-focused-row deselect-rows deselect-selected-row "
              "design-mode dialog-box dialog-help dir disabled display-message "
              "display-type double drag-enabled drop-down drop-down-list dump "
              "dynamic echo edge edge-chars edge-pixels editor empty end-key "
              "endkey entered eq error error-column error-row event-type "
              "events exclusive-id execute exp expand extended extent external "
              "extract fetch-selected-row fgcolor file file-name file-offset "
              "file-type filename fill-in filled filters first-child "
              "first-column first-procedure first-tab-item fixed-only float "
              "focused-row font-based-layout font-table force-file foreground "
              "forult-row current-row-modified current-value cursor-char "
              "cursor-line cursor-offset data-entry-return data-type date "
              "date-format day db-references full-width full-width-chars "
              "full-width-pixels ge get get-blue-value get-char-property "
              "get-double get-dynamic get-file get-float get-green-value "
              "get-iteration get-license get-long get-message get-number "
              "get-pointer-value get-red-value get-repositioned-row "
              "get-selected-widget get-short get-signature get-size get-string "
              "get-tab-item get-text-height get-text-height-chars "
              "get-text-height-pixels get-text-width get-text-width-chars "
              "get-text-width-pixels get-unsigned-short grayed "
              "grid-factor-horizontal grid-factor-vertical grid-set grid-snap "
              "grid-unit-height grid-unit-height-chars grid-unit-height-pixels "
              "grid-unit-width grid-unit-width-chars grid-unit-width-pixels "
              "grid-visible gt handle height height-chars height-pixels "
              "help-context helpfile-name hidden hint horizontal hwnd image "
              "image-down image-insensitive image-size image-size-chars "
              "image-size-pixels image-up immediate-display index-hint "
              "indexed-reposition information init initial initial-dir "
              "initial-filter initiate inner inner-chars inner-lines "
              "insert-backtab insert-file insert-row insert-string insert-tab "
              "integer internal-entries is-lead-byte is-row-selected "
              "is-selected item items-per-row join-by-sqldb keep-frame-z-order "
              "keep-messages keep-tab-order key keyword-all label-bgcolor "
              "label-dcolor label-fgcolor label-font label-pfcolor labels "
              "languages large large-to-small last-child last-tab-item "
              "last-procedure lc le leading left left-aligned left-trim length "
              "line list-events list-items list-query-attrs list-set-attrs "
              "list-widgets load load-control load-icon load-image "
              "load-image-down load-image-insensitive load-image-up "
              "load-mouse-pointer load-small-icon log logical lookahead lower "
              "lt manual-highlight margin-extra margin-height "
              "margin-height-chars margin-height-pixels margin-width "
              "margin-width-chars margin-width-pixels matches max max-chars "
              "max-data-guess max-height max-height-chars max-height-pixels "
              "max-rows max-size max-value max-width max-width-chars "
              "max-width-pixels maximize maximum memory menu menu-bar "
              "menu-item menu-key menu-mouse menubar message-area "
              "message-area-font message-line min min-height min-height-chars "
              "min-height-pixels min-size min-value min-width min-width-chars "
              "min-width-pixels minimum mod modified modulo month "
              "mouse-pointer movable move-after-tab-item move-before-tab-item "
              "move-column move-to-bottom move-to-eof move-to-top multiple "
              "multiple-key multitasking-interval must-exist name native ne "
              "new-row next-column next-sibling next-tab-item next-value "
              "no-apply no-assign no-bind-where no-box no-column-scrolling "
              "no-convert no-current-value no-debug no-drag no-echo "
              "no-index-hint no-join-by-sqldb no-lookahead no-row-markers "
              "no-scrolling no-separate-connection no-separators no-underline "
              "no-word-wrap none num-buttons num-columns num-copies "
              "num-formats num-items num-iterations num-lines "
              "num-locked-columns num-messages num-results num-selected "
              "num-selected-rows num-selected-widgets num-tabs num-to-retain "
              "numeric numeric-format octet_length ok ok-cancel "
              "on-frame-border ordered-join ordinal orientation os-getenv "
              "outer outer-join override owner page-size page-width paged "
              "parent partial-key pascal pathname pfcolor pinnable "
              "pixels-per-column pixels-per-row popup-menu popup-only position "
              "precision preselect prev prev-column prev-sibling prev-tab-item "
              "primary printer-control-handle printer-setup private-data "
              "profiler Progress-source publish put-double put-float put-long "
              "put-short put-string put-unsigned-short query-off-end question "
              "radio-buttons radio-set random raw raw-transfer read-file "
              "read-only real recursive refresh refreshable replace "
              "replace-selection-text replication-create replication-delete "
              "replication-write request resizable resize retry-cancel "
              "return-inserted return-to-start-dir reverse-from right "
              "right-aligned right-trim round row row-markers row-of rowid "
              "rule rule-row rule-y save-as save-file screen-value scroll-bars "
              "scroll-delta scroll-horiz-value scroll-offset "
              "scroll-to-current-row scroll-to-item scroll-to-selected-row "
              "scroll-vert-value scrollable scrollbar-horizontal "
              "scrollbar-vertical scrolled-row-position scrolling "
              "se-check-pools se-enable-off se-enable-on se-num-pools "
              "se-use-message section select-focused-row select-next-row "
              "select-prev-row select-repositioned-row select-row selectable "
              "selected selected-items selection-end selection-list "
              "selection-start selection-text send sensitive "
              "separate-connection separators set-blue-value set-break "
              "set-cell-focus set-contents set-dynamic set-green-value "
              "set-leakpoint set-pointer-value set-property set-red-value "
              "set-repositioned-row set-selection set-size set-wait-state "
              "side-lab side-labe side-label side-label-handle side-labels "
              "silent simple single size size-chars size-pixels slider "
              "smallint sort source source-procedure sql sqrt start "
              "status-area status-area-font status-bar stdcall stenciled stop "
              "stopped stored-procedure string sub-average sub-count "
              "sub-maximum sub-menu sub-menu-help sub-minimum sub-total "
              "subscribe substitute substring subtype sum super "
              "suppress-warnings system-alert-boxes system-help tab-position "
              "tabbable target target-procedure temp-directory temp-table "
              "terminate text-selected three-d through thru tic-marks "
              "time-source title-bgcolor title-dcolor title-fgcolor title-font "
              "to-rowid toggle-box tool-bar top topic total trailing truncate "
              "type unbuffered unique-id unload unsubscribe upper use "
              "use-dict-exps use-filename use-text v6display valid-event "
              "valid-handle validate validate-condition validate-message "
              "variable vertical virtual-height virtual-height-chars "
              "use-filename use-text v6display valid-event valid-handle "
              "validate validate-condition validate-message variable vertical "
              "virtual-height virtual-height-chars widget-pool width "
              "width-chars width-pixels window-name window-state window-system "
              "word-wrap x-of y-of year yes-no yes-no-cancel _dcm")


#-----------------------------------------------------------------------------#

def Keywords(lang_id=0):
    """Progress 4GL keyword specifications
    @keyword lang_id: Language ID

    """
    if lang_id == synglob.ID_LANG_4GL:
        return [PROG_KW, PROG_TYPES, PROG_OP]
    else:
        return list()

def SyntaxSpec(lang_id=0):
    """Syntax Specifications
    @keyword lang_id: Language ID

    """
    if lang_id == synglob.ID_LANG_4GL:
        return sql.SYNTAX_ITEMS
    else:
        return list()

def Properties(lang_id=0):
    """Language properties for folding ect...
    @keyword lang_id: Language ID

    """
    if lang_id == synglob.ID_LANG_4GL:
        return [sql.FOLD,]
    else:
        return list()

def CommentPattern(lang_id=0):
    """Comment pattern
    @keyword lang_id: Language ID

    """
    if lang_id == synglob.ID_LANG_4GL:
        return [u'/*', u'*/']
    else:
        return list()

