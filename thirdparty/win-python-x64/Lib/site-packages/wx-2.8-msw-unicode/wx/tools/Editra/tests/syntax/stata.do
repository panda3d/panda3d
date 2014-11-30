/* STATA Test File /*
/* Comments look like this */

set mem 100m
use "this\is\where\stata\file\is\file.dta", clear
use "file.dta", clear


forvalues i = 1(1)10 {
			
		keep if auction==`i'
                tabstat V1, by(V2) s(mean sd median count) 
                reg V1 V2 V3,robust
              }
