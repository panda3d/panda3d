/* Syntax highlight test file for Progress 4GL */

DEFINE TEMP-TABLE tt
 FIELDS site like ld_site
 FIELDS loc like ld_loc
 FIELDS part like ld_part
 FIELDS lot like ld_lot
 FIELDS ref like ld_ref
 FIELDS ldt like ld_date
 FIELDS ldst like ld_status
 FIELDS qoh like ld_qty_oh
 FIELDS coh like ld_cust_consign_qty.

INPUT FROM "ld-ncstkinconsignloc.d".
 REPEAT TRANSACTION.
 CREATE tt.
 IMPORT tt.
END.
 INPUT CLOSE.

OUTPUT TO "cyc-amt-02.d".
FOR EACH tt:
     FINF FIRST pt_mstr WHERE pt_part = part NO-LOCK NO-ERROR.

   FIND FIRST pl_mstr WHERE pl_prod_line = pt_prod_line NO-LOCK NO-ERROR.
     FIND FIRST sct_det where sct_sim = "standard"
     AND sct_site = "g1"
     AND sct_part = part NO-LOCK NO-ERROR.
     EXPORT site loc part lot ref 
     pt_prod_line
     pl_inv_acct
     pl_inv_cc
     qoh sct_cst_tot qoh * sct_cst_tot.
END.
