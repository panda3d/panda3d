# scale.tcl - Copyright (C) 2004 Pat Thoyts <patthoyts@users.sourceforge.net>
#
# Bindings for the TScale widget
#
# $Id: scale.tcl,v 1.1 2006/10/31 01:42:27 hobbs Exp $

namespace eval ttk::scale {
    variable State
    array set State  {
	dragging 0
    }
}

bind TScale <ButtonPress-1>   { ttk::scale::Press %W %x %y }
bind TScale <B1-Motion>       { ttk::scale::Drag %W %x %y }
bind TScale <ButtonRelease-1> { ttk::scale::Release %W %x %y }

proc ttk::scale::Press {w x y} {
    variable State
    set State(dragging) 0

    switch -glob -- [$w identify $x $y] {
	*track -
        *trough {
	    if {[$w get $x $y] <= [$w get]} {
		ttk::Repeatedly Increment $w -1
	    } else {
		ttk::Repeatedly Increment $w 1
	    }
        }
        *slider {
            set State(dragging) 1
            set State(initial) [$w get]
        }
    }
}

proc ttk::scale::Drag {w x y} {
    variable State
    if {$State(dragging)} {
	$w set [$w get $x $y]
    }
}

proc ttk::scale::Release {w x y} {
    variable State
    set State(dragging) 0
    ttk::CancelRepeat
}

proc ttk::scale::Increment {w delta} {
    if {![winfo exists $w]} return
    $w set [expr {[$w get] + $delta}]
}
