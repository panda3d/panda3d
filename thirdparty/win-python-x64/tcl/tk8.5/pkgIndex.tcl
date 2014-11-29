if {[package vcompare [package provide Tcl] 8.5.2] != 0} { return }
package ifneeded Tk 8.5.2 [list load [file join $dir .. .. bin tk85.dll] Tk]
