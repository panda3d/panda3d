###############################################################################
# Name: stata.py                                                              #
# Purpose: Define Stata syntax for highlighting and other features            #
# Author: Jean Eid<jeid@wlu.ca>                                               #
# Note: stata keywords are taken from TextWrangler module for stata           #
###############################################################################

"""
FILE: stata.py                                                                
@author: Jean Eid                                                      
@summary: Lexer configuration file for STATA source files.
                                                                         
"""

__author__ = "Jean Eid <jeid@wlu.ca>"
__svnid__ = "$Id: _stata.py 63834 2010-04-03 06:04:33Z CJP $"
__revision__ = "$Revision: 0$"

#-----------------------------------------------------------------------------#
# Imports
import wx.stc as stc

# Local imports
import synglob
import syndata
from _cpp import AutoIndenter

#-----------------------------------------------------------------------------#

#---- Keyword Specifications ----#

#  Documentation Keywords (includes Doxygen keywords)
SECONDARY_KEYWORDS =(1,
                     """
                     __GEEBT __GEERC  __GEEUC _3dax0 _3daxmin _3daxout if
                     _3daxtbl _3ddflts _3dmkdta _3dmnmx _3dproj _3drproj 
                     _3drshow _3dshad _3dsvusr _ac _addl _addop _adjksm 
                     _all _bsqreg _byobs _callerr _cpmatnm _cr1form 
                     _cr1invt _cr1se _cr1t _crc2use _crc4fld _crcacnt 
                     _crcar1 _crcause _crcbcrt _crcbin _crcbygr _crcchi2 
                     _crcchkt _crcchkw _crcci _crccip _crceprs _crcexn1 
                     _crcexn2 _crcexn4 _crcexn5 _crcexn6 _crcexn7 _crcexn8 
                     _crcexn9 _crcexna _crcexnb _crcexnc _crcexnd _crcexne 
                     _crcexnf _crcexnt _crcgldv _crcglil _crcichi _crcird 
                     _crcirr _crcksm _crclf _crcmeq _crcmiss _crcnlou 
                     _crcnms2 _crcnuse _crcor _crcphdr _crcplst _crcra 
                     _crcrd _crcrnfd _crcrr _crcrsfl _crcseq _crcshdr 
                     _crcslbl _crcsrv2 _crcsrvc _crcstep _crcswxx _crct 
                     _crctmge _crcunab _crcunit _crcvarl _crcwsrv _crczsku 
                     _cu_c0 _diparm _evlist _fracchk _fraccox _fracddp 
                     _fracdis _fracdv _fracin _fracmdp _fracord _fracpp 
                     _fracpv _fracrep _fracwgt _fracxo _gcount _gcut _gdiff 
                     _getbv _getrhs _getrres _gfill _ggroup _giqr _gladder 
                     _glmfl _glmilnk _glmmapf _glmmapl _glmresd _glmrpt 
                     _glmwgt _gma _gmad _gmax _gmdmean _gmdmed _gmean 
                     _gmedian _gmin _gmtr _gpctile _grank _grank2 _grfirst 
                     _grlast _grmax _grmean _grmin _grmiss _grmiss2 _grobs 
                     _grsd _grsum _gsd _gsrank _gstd _gsum _gtma _gtrank 
                     _hu _hub _hube _huber _inlist _invlist _isfit _jprfpdt 
                     _jprfpdx _jprfpfp _jprfpgn _jprfpin _jprfplx _jprfpmm 
                     _jprfppp _jprfpre _jprfprp _jprfpse _jprfptp _jprfpxo 
                     _jprglef _jprglfl _jprglil _jprglld _jprglwz _jprxrpa 
                     _kalman1 _ksmwrk _ldrtest _linemax _maked _merge 
                     _mfrmvec _mkvec _mvec _newey _nlout _nobs _opnum 
                     _parsevl _parsewt _partset _pctile _pred_me _pred_se 
                     _predict _qreg _repart _result _rmcoll _robksm _robust 
                     _sfran _subchar _svy _sw_lik _sw_lik2 _sw_ood _ts 
                     _ts_dsmp _ts_flag _ts_gdat _ts_meqn _ts_pars _ts_peri 
                     _tsheadr _ttest _ttest1 _ttest2 _tutends _tx_mtr1 
                     _tx_mtr2 _tx_mtr3 _tx_mtr4 _tx_mtr5 _tx_rpl _wkapm 
                     _wsrvcrv _xtrenorm
                     """)

MAIN_KEYWORDS =  (0, 
                  """
                  abbrev about abs acprplot add 
                  adjust ado adopath alpha an ano anov anova anovadef 
                  aorder ap app appe appen append arch arch_dr arch_p 
                  areg areg_p args arima arima_dr arima_p as ass asse 
                  asser assert at avplot avplots aw aweight bcskew0 be 
                  bee beep binreg bipp_lf bipr_lf bipr_p biprobit bitest 
                  bitesti bitowt blogit bmemsize boot bootsamp boxcox 
                  boxcox_p bprobit break brier bs bsampl_w bsample 
                  bsqreg bstat bstrap by bys bysort byte c_local canon 
                  canon_p capture cat cc cchart cci cd cell cellsize 
                  centile cf char char chdir checksum chi2 chi2tail ci 
                  cii cksum clear clo clocal clog clog_lf clog_p clogi 
                  clogi_sw clogit clogit_p clogitp clogl_sw cloglog 
                  close cluster cmdlog cnr cnre cnreg cnreg_sw cnsreg 
                  codebook col collaps4 collapse compare compress 
                  compute cond conf confi confir confirm cons const 
                  constr constra constrai constrain constraint contract 
                  copy cor corc corr corr2data corre correl correla 
                  correlat correlate corrgram cou coun count cox cox_p 
                  cox_sw coxbase coxhaz coxvar cp cprplot crc cross cs 
                  cscript csi ct ct_is ctset ctst_5 ctst_st cttost cumsp 
                  cumul cusum d datetof dbeta de debug debugbuf dec deco 
                  decod decode def deff define des desc descr descri 
                  describ describe dfbeta dfuller di dir dis discard 
                  disp disp_res disp_s displ displa display do doe doed 
                  doedi doedit dotplot dprobit drawnorm drop ds dstdize 
                  dwstat dydx dyex dynre dynren e ed edi edit egen 
                  eivreg else emdef en enc enco encod encode end eq 
                  eqlist erase ereg ereg_lf ereg_p ereg_sw err erro 
                  error est esti estim estima estimat estimate estimates 
                  etodow etof etomdy ex exact exec execu execut execute 
                  exi exit expand export eydx eyex F fac fact facto 
                  factor fast fft fillin findit fit float for for5_0 
                  force form forma format fpredict frac_154 frac_adj 
                  frac_chk frac_cox frac_ddp frac_dis frac_dv frac_in 
                  frac_mun frac_pp frac_pq frac_pv frac_wgt frac_xo 
                  fracgen fracplot fracpoly fracpred freq frequency 
                  Ftail ftodate ftoe ftomdy ftowdate fw fweight g gamma 
                  gamma_lf gamma_p gamma_sw ge gen gene gener genera 
                  generat generate genrank genstd genvmean gettoken gl 
                  gladder glm glm_p glm_sw glmpred glo glob globa global 
                  glogit glogit_p gnbre_lf gnbreg gnbreg_5 gnbreg_p 
                  gomp_lf gompe_sw gomper_p gompertz gph gphdot gphpen 
                  gphprint gprobi_p gprobit gr gr_print gra grap graph 
                  grebar greigen grmeanby group gsort gwood h hadimvo 
                  hareg hausman 
                  he heck_d2 heckma_p heckman heckp_lf heckpr_p heckprob 
                  hel help helpchk hereg hetpr_lf hetpr_p hetprob 
                  hettest hilite hist hlogit hlu hotel hprobit hreg icd9 
                  icd9p  iis impute in index inf infi infil infile 
                  infix inlist inp inpu input ins insh inshe inshee 
                  insheet insp inspe inspec inspect int integ intreg 
                  intrg_ll invchi2 invchi2tail invF invFtail invnchi2 
                  invnorm invttail ipolate iqreg ir iri istdize ivreg iw 
                  iweight joinby kalarma1 kap kap_3 kapmeier kappa 
                  kapwgt kdensity keep ksm ksmirnov ktau kwallis l la 
                  lab labe label ladder length level leverage lfit 
                  lfit_p li lincom linesize linktest lis list llogi_sw 
                  llogis_p llogist ln lnorm_lf lnorm_sw lnorma_p lnormal 
                  lnskew0 lo loc loca local log logi logis_lf logistic 
                  logit logit_p loglogs logrank logtype loneway long loo 
                  look lookfor looku lookup lower lpredict lroc lrtest 
                  ls lsens lsens_x lstat ltable ltrim lv lvr2plot m ma 
                  mac macr macro man mantel mark markout marksample mat 
                  matcell match matcol matcproc matname matr matri 
                  matrix matrow matsize matstrik max mcc mcci md0_ md1_ 
                  md1debu_ md2_ md2debu_ mdytoe mdytof mean means median 
                  memory memsize meqparse mer merg merge mfx mhodds min 
                  missing mkdir mkmat mkspline ml ml_5 ml_adjs ml_bhhhs 
                  ml_c_d ml_check ml_clear ml_cnt ml_debug ml_defd ml_e0 
                  ml_e0i ml_e1 ml_e2 ml_ebfg0 ml_ebfr0 ml_ebh0q ml_ebhh0 
                  ml_ebhr0 ml_ebr0i ml_ecr0i ml_edfp0 ml_edfr0 ml_edr0i 
                  ml_eds ml_eer0i ml_egr0i ml_elf ml_elfi ml_elfs 
                  ml_enr0i ml_enrr0 ml_exde ml_geqnr ml_grad0 ml_graph 
                  ml_hbhhh ml_hd0 ml_init ml_inv ml_log ml_max ml_mlout 
                  ml_model ml_nb0 ml_opt ml_plot ml_query ml_rdgrd 
                  ml_repor ml_s_e ml_searc mleval mlf_ mlmatsum mlog 
                  mlogi mlogit mlogit_p mlopts mlsum mlvecsum mnl0_ mor 
                  more mov move mrdu0_ mvdecode mvencode mvreg n nbreg 
                  nbreg_al nbreg_lf nbreg_sw nchi2 net newey newey_p 
                  news nl nl_p nlexp2 nlexp2a nlexp3 nlgom3 nlgom4 
                  nlinit nllog3 nllog4 nlogit nlpred no nobreak nod 
                  nodiscrete noe noesample nof nofreq noi nois noisi 
                  noisil noisily nol nolabel nonl nonlinear normden nose 
                  note notes notify now nowght npnchi2 nptrend numlist 
                  obs off old_ver olo olog ologi ologi_sw ologit 
                  ologit_p ologitp on one onew onewa oneway op_colnm 
                  op_comp op_diff op_inv op_str opr opro oprob oprob_sw 
                  oprobi oprobi_p oprobit oprobitp order orthog orthpoly 
                  ou out outf outfi outfil outfile outs outsh outshe 
                  outshee outsheet ovtest pac par pars parse pause pc 
                  pchart pchi pcorr pctile pentium percent pergram 
                  permanent personal pkcollapse pkcross pkequiv 
                  pkexamine pkshape pksumm pl playsnd plo plot plug 
                  plugi plugin pnorm poisgof poiss_lf poiss_sw poisso_p 
                  poisson pop popu popup post postclose postfile pperron 
                  pr prais prais_e prais_p pred predi predic predict 
                  predict preserve printgr priorest pro prob probi 
                  probit probit_p prog progr progra program prove prtest 
                  prtesti push pw pwcorr pwd pweight q qby qchi qnorm 
                  qqplot qreg qreg_c qreg_p qreg_sw qu quadchk quantile 
                  que quer query qui quie quiet quietl quietly range 
                  ranksum rawsum rchart rcof real recast recode reg reg3 
                  reg3_p regdw regr regre regre_p2 regres regres_p 
                  regress regriv_p remap ren rena renam rename renpfix 
                  repeat replace replay reshape restore ret retu retur 
                  return reverse rm roccomp rocfit rocgold roctab rot 
                  rota rotat rotate round row rreg rreg_p rtrim ru run 
                  runtest rvfplot rvpplot sa safesum sample sampsi sav 
                  save saving say sca scal scala scalar sco scob_lf 
                  scob_p scobi_sw scobit scor score sd sdtest sdtesti se 
                  search separate seperate serrbar set sfrancia sh she 
                  shel shell shewhart showpoint signrank signtest simul 
                  simulinit sktest sleep smcl smooth snapspan so sor 
                  sort spearman speedchk1 speekchk2 spikeplt spline_x 
                  sqreg sret sretu sretur sreturn st st_ct st_hc st_hcd 
                  st_is st_issys st_note st_promo st_set st_show st_smpl 
                  st_subid stack stackdepth stackreset statsby stbase 
                  stci stcox stcox_p stcoxkm stcurv stcurve stdes stem 
                  stereg stfill stgen stinit stir stjoin stmc stmh 
                  stphplot stphtest stptime strate streg streset string 
                  sts stset stsplit stsum sttocc sttoct stvary stweib su 
                  subinstr subinword subpop substr subwin sum summ summa 
                  summar summari summariz summarize sureg survcurv 
                  survsum svmat svy_disp svy_dreg svy_est svy_get 
                  svy_head svy_sub svy_x svydes svyintrg svyivreg svylc 
                  svylog_p svylogit svymean svymlog svyolog svyoprob 
                  svypois svyprobt svyprop svyratio svyreg svyreg_p 
                  svyset svytab svytest svytotal sw swcnreg swcox swereg 
                  swilk swlogis swlogit swologit swoprbt swpois swprobit 
                  swqreg swtobit swweib symmetry symmi symplot syntax 
                  sysdir sysmenu ta tab tab_or tab1 tab2 tabd tabdi 
                  tabdis tabdisp tabi table tabodds tabstat tabu tabul 
                  tabula tabulat tabulate te tempfile tempname tempvar 
                  tes test testnl testparm teststd text timer tis tob 
                  tobi tobit tobit_p tobit_sw token tokeni tokeniz 
                  tokenize touch treatreg trim truncreg tset tsfill 
                  tsreport tsrevar tsset tsunab ttail ttest ttesti 
                  tut_chk tut_wait tutorial ty typ type typeof u unab 
                  unabbrev uniform update upper us use using val values 
                  var variable varlabelpos vce verinst vers versi versio 
                  version vif vwls wdatetof wdctl wdlg wdupdate weib_lf 
                  weib_lf0 weibu_sw weibul_p weibull wh whelp whi whic 
                  which whil while wilc_st wilcoxon win wind windo 
                  window winexec winhelp wmenu wntestb wntestq xchart 
                  xcorr xi xpose xt_iis xt_tis xtabond xtbin_p xtclog 
                  xtcnt_p xtcorr xtdata xtdes xtgee xtgee_p xtgls 
                  xtgls_p xthaus xtile xtint_p xtintreg xtivreg xtlogit 
                  xtnb_fe xtnb_lf xtnbreg xtpcse xtpois xtpred xtprobit 
                  xtps_fe xtps_lf xtps_ren xtrch_p xtrchh xtrefe_p xtreg 
                  xtreg_be xtreg_fe xtreg_ml xtreg_re xtregar xtrere_p 
                  xtsum xttab xttest0 xttobit xttrans xwin xwind xwindo 
                  xwindow zap_s zinb zinb_llf zinb_plf zip zip_llf zip_p 
                  zip_plf zt_ct_5 zt_hc_5 zt_hcd_5 zt_is_5 zt_iss_5 
                  zt_sho_5 zt_smp_5 ztbase_5 ztcox_5 ztdes_5 ztereg_5 
                  ztfill_5 ztgen_5 ztir_5 ztjoin_5 zts_5 ztset_5 
                  ztspli_5 ztsum_5 zttoct_5 ztvary_5 ztweib_5 comma tab 
                  robust mvsktest oglm dlist mahapick viewresults 
                  valuesof textbarplot freduse hcavar distrate zipsave 
                  hutchens svylorenz goprobit regoprob digits cprplot2 
                  devcon oaxaca supclust convert_top_lines graphbinary 
                  trellis traces wtd glst lxpct_2 clv backrasch mmsrm 
                  diplot kountry gologit2 intcens svygei_svyatk fitint 
                  xtarsim eclpci beamplot samplepps classplot spearman2 
                  fixsort casefat mehetprob svyselmlog svybsamp2 
                  moremata checkfor2 dbmscopybatch palette_all ivreset 
                  lookfor_all nmissing isvar descogini akdensity 
                  biplotvlab genscore rrlogit ivvif hnblogit hnbclg 
                  hglogit hgclg hplogit hpclg ztg cdfplot kdens cpoisson 
                  samplesize gnbstrat nbstrat genass outreg2 kapprevi 
                  sampsi_reg lgamma2 glgamma2 ivgauss2 givgauss2 
                  sampsi_mcc cnbreg xtivreg2 sliceplot cquantile primes 
                  scheme_rbn1mono subsave insob geekel2d censornb 
                  estadd_plus estadd xtregre2 clorenz usmaps2 usswm 
                  tablemat tmap mif2dta surface kdens2 onespell diffpi 
                  plotbeta batplot relrank invcdf jmp jmp2 smithwelch 
                  reswage dfl p2ci heterogi shuffle8 shuffle lincheck 
                  bicdrop1 pre optifact matsort clustsens listmiss bic 
                  hdquantile dirlist variog svypxcon svypxcat seast 
                  xtlsdvc avplots4 eststo vclose gausshermite simirt 
                  cprplots glcurve sscsubmit sdecode harmby ellip 
                  mlboolean splitvallabels circular betacoef bnormpdf 
                  grqreg genhwcci mrtab zandrews cvxhull abar mat2txt 
                  gmlabvpos tableplot sortlistby mgen hlist ppplot rocss 
                  pnrcheck stexpect rbounds regaxis pgmhaz8 meta_lr 
                  mcqscore mvcorr ipf hapblock swblock raschtest 
                  gammasym raschtestv7 vanelteren fs estout1 estout 
                  loevh msp detect disjoint adjacent plotmatrix nnmatch 
                  senspec twoway_estfit mkdat soepren alphawgt center 
                  decompose wgttest cochran examples rollreg clemao_io 
                  heckprob2 gipf metagraph hshaz tslist collapse2 
                  gen_tail carryforward floattolong doubletofloat 
                  margeff matin4-matout4 perturb coldiag2 sim_arma ndbci 
                  labelmiss mcenter sslope reorder scat3 dummieslab rfl 
                  xtvc metareg rc2 moments sxpose kaputil bystore mice 
                  gzsave witch cureregr hprescott tabout gamet duncan 
                  fview eret2 rc_spline tolerance modeldiag metaparm 
                  profhap nsplit hlm fieller xtfisher matwrite usmaps 
                  ellip6 ellip7 xi3 qhapipf slist nearest fedit extremes 
                  mypkg pairplot cycleplot ciplot selectvars stcompet 
                  full_palette catplot eclplot spellutil metadialog 
                  psmatch2 bygap ingap mylabels metaaggr cleanlog gpfobl 
                  mvprobit eqprhistogram slideplot majority hireg bigtab 
                  vartyp codebook2 dmariano whotdeck crtest 
                  collapseunique stripplot linkplot statsbyfast parplot 
                  mitools groups wclogit xcontract xcollapse metafunnel 
                  corrtab dmerge makematrix cibplot vreverse msplot 
                  nicedates mkcorr nearmrg tabstatmat panelunit 
                  panelauto safedrop gammafit gumbelfit betafit 
                  lookforit savasas usesas etime usagelog tmpdir 
                  confirmdir shortdir lambda survtime xtabond2 
                  factortest checkvar vtokenize reshape8 pcorr2 tarow 
                  cb2html survwgt svr jonter xsampsi ci2 domdiag 
                  xtpattern nbfit distinct maketex triprobit smileplot 
                  tabmerge avplot3 datesum _gclsort cltest varlab decomp 
                  overlay tab3way tab2way ivreg2 hapipf varlag vececm 
                  isco isko ptrend dpplot cipolate tknz levels reformat 
                  xtab mvsumm fsum stkerhaz explist qlognorm tsspell 
                  gphepssj texteditors alphawgt decompose inccat adjksm 
                  outdat reshape7 qsim allpossible glmcorr gcause 
                  selmlog matsave est2tex log2do2 hansen2 gam ivhettest 
                  newey2 intext matrixof mrdum fastcd ivendog tabcount 
                  tabcond minap qrowname cij ciw textgph latab autolog 
                  histbox kdbox sunflower charlist adoedit lincomest 
                  stylerules strgen wntstmvq grnote xttrans2 inequal7 
                  tablepc hegy4 regdplot denormix chi2fit bstut aboutreg 
                  _gstd01 cpcorr mktab vecar xdatelist strdate thinplate 
                  gfields takelogs catgraph dsconcat tosql outseries 
                  glcurve7 omninorm summdate sencode rgroup cf3 hlpdir 
                  descsave outmat svytabs mstore savesome stbget spsurv 
                  xtgraph effects stpm madfuller aformat todate _gwtmean 
                  tsgraph soreg fbar metaninf blist johans vecar6 
                  outtable factmerg factref factext hadrilm levinlin 
                  nharvey ipshin gpreset geneigen dotex outtex sutex 
                  dsearch chiplot idonepsu statsmat ds3 dthaz paran 
                  gprefscode _gsoundex bpass bking labsort intreg2 sq 
                  powercal denton corr_svy log2html dfao xpredict mcl 
                  listtex raschcvt diagt estsave egenmore labutil 
                  concord avplot2 tablecol metabias coldiag fitstat 
                  doubmass cortesti fndmtch2 cusum6 ghistcum findval 
                  centcalc xrigls dfgls charutil icomp enlarge kpss 
                  metatrim ivgmm0 smhsiao matvsort roblpr modlpr recode2 
                  showgph copydesc shapley rnd himatrix bspline stcascoh 
                  stselpre nct ncf hist3 dolog dologx tscollap bcoeff 
                  grfreq grlogit lrchg lrmatx lrplot lrutil forfile 
                  printgph readlog spaces title dashln lomodrs ctabstat 
                  expandby finddup fitmacro normtest predcalc _grmedf 
                  ranvar matmap svmatf lincom2 csjl shownear fracdiff 
                  genvars calibr fracirf xriml rowranks tgraph ordplot 
                  cpr mlcoint stcmd xttest3 atkplot fsreg ciform rowsort 
                  expgen epsigr dashgph addtxt swboot stak _grprod 
                  sskapp xttest2 trinary ivprob-ivtobit6 torumm split q 
                  linesize keyb expr xtlist xtcount xtcorr2 vce2 summvl 
                  ststrata stcount simul2 regh pcamv pca mvpatt mokken 
                  lrtest2 linest iia hordered htest elogit diag wraplist 
                  qsort precmd modfycmd matfunc listuniq multgof hilite2 
                  unlabeld to_msp sample2 placevar listby listblck hh 
                  genl for2 dropvars countby bys icslib varcase tsplot 
                  diagtest ssizebi studysi poverty ineq dissim geivars 
                  ineqdec0 ineqdeco ineqfac povdeco sumdist xfrac 
                  dagumfit smfit cpyxplot mkbilogn univstat hotdeck
                  matodd p_vlists gennorm tab_chi sbplot5 sbplot
                  mfracpol keyplot taba ds5 tabplot cistat probitiv
                  tobitiv _gslope sphdist ginidesc inequal2 kernreg1
                  kernreg2 outfix outfix2 seg outreg rfregk spautoc
                  onewplot pwcorrs ltable2 tabhbar hbox tabhplot cihplot 
                  civplot sf36 pwploti partgam cf2 xtile2 ivglog 
                  kwallis2 far5 jb6 gby strparse _gprod mfilegr desmat 
                  areg2 margfx arimafit moreobs tsmktim durbinh bgtest 
                  mnthplot archlm gphudak renames skewplot cnsrsig 
                  recast2 doub2flt feldti tolower lfsum whitetst bpagan 
                  listutil mdensity kdmany stquant byvar cflpois 
                  workdays flower _grpos stcoxgof stcoxplt stpiece 
                  overid overidxt swapval adotype fndmtch svvarlbl 
                  gentrun storecmd sto lrdrop1 lrseq dmexogxt 
                  probexog-tobexog predxcon predxcat mmerge tablab 
                  for211 gmci grand nbinreg spikeplt ocratio biplot 
                  coranal mca heckman2 marker markov pgamma qgamma 
                  somersd pantest2 datmat distan missing quantil2 
                  distplot tpred contrast cid rglm dtobit2 ljs ewma 
                  barplot genfreq hbar hplot fodstr catdev rmanova 
                  ranova seq intterms lmoments regresby reglike pweibull 
                  wbull qweibull regpred logpred adjmean adjprop spell 
                  switchr trnbin0 mvsamp1i mvsampsi tpvar mvtest addtex 
                  pwcorrw vlist violin eba mstdize orthog stcumh 
                  ccweight psbayes oprobpr cndnmb3 pbeta qbeta vmatch 
                  kr20 sbrowni canon stbtcalc stgtcalc zb_qrm catenate 
                  lprplot nnest longplot parmest qqplot2 jb zip zinb 
                  hetprob unique longch gwhet williams adjust barplot2 
                  grand2 histplot xcorplot clarify mlogpred nproc 
                  labgraph vallist pexp qexp lms levene centroid medoid 
                  cluster fulltab t2way5 epiconf lstack deaton colelms 
                  confsvy median winsor bys torats venndiag chaos 
                  muxyplot muxplot irrepro triplot tomode circstat tryem 
                  white strip ralloc acplot stack symmetry omodel 
                  allcross dups iia sdtest vplplot summvl labsumm 
                  loopplot elapse istdize blogit2 sparl vallab gologit 
                  mkstrsn poisml trpois0 cenpois sssplot hausman stcstat
                  forvalues
                  """
)

#---- Syntax Style Specs ----#
SYNTAX_ITEMS = [ (stc.STC_C_DEFAULT, 'default_style'),
                 (stc.STC_C_COMMENT, 'comment_style'),
                 (stc.STC_C_COMMENTLINE, 'comment_style'),
                 (stc.STC_C_COMMENTDOC, 'comment_style'),
                 (stc.STC_C_COMMENTDOCKEYWORD, 'dockey_style'),
                 (stc.STC_C_COMMENTDOCKEYWORDERROR, 'error_style'),
                 (stc.STC_C_COMMENTLINE, 'comment_style'),
                 (stc.STC_C_COMMENTLINEDOC, 'comment_style'),
                 (stc.STC_C_CHARACTER, 'char_style'),
                 (stc.STC_C_GLOBALCLASS, 'global_style'),
                 (stc.STC_C_IDENTIFIER, 'default_style'),
                 (stc.STC_C_NUMBER, 'number_style'),
                 (stc.STC_C_OPERATOR, 'operator_style'),
                 (stc.STC_C_PREPROCESSOR, 'pre_style'),
                 (stc.STC_C_REGEX, 'pre_style'),
                 (stc.STC_C_STRING, 'string_style'),
                 (stc.STC_C_STRINGEOL, 'default_style'),
                 (stc.STC_C_UUID, 'pre_style'),
                 (stc.STC_C_VERBATIM, 'number2_style'),
                 (stc.STC_C_WORD, 'keyword_style'),
                 (stc.STC_C_WORD2, 'keyword2_style') ]

#---- Extra Properties ----#
FOLD = ("fold", "1")
FOLD_PRE = ("styling.within.preprocessor", "0")
FOLD_COM = ("fold.comment", "1")
FOLD_COMP = ("fold.compact", "1")
FOLD_ELSE = ("fold.at.else", "0")

#------------------------------------------------------------------------------#

class SyntaxData(syndata.SyntaxDataBase):
    """SyntaxData object for STATA""" 
    def __init__(self, langid):
        syndata.SyntaxDataBase.__init__(self, langid)

        # Setup
        self.SetLexer(stc.STC_LEX_CPP)
        self.RegisterFeature(synglob.FEATURE_AUTOINDENT, AutoIndenter)

    def GetKeywords(self):
        """Returns Specified Keywords List """
        return [MAIN_KEYWORDS, SECONDARY_KEYWORDS]

    def GetSyntaxSpec(self):
        """Syntax Specifications """
        return SYNTAX_ITEMS

    def GetProperties(self):
        """Returns a list of Extra Properties to set """
        return [FOLD, FOLD_PRE, FOLD_COM]

    # TODO: this doesnt' look right...
    def GetCommentPattern(self):
        """Returns a list of characters used to comment a block of code """
        return [u'//', u'/*', u'*/',u'*' ]
