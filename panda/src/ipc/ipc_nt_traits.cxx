// Filename: ipc_nt_traits.cxx
// Created by:  mike (23Oct98)
// 
////////////////////////////////////////////////////////////////////

#ifdef WIN32_VC

#include "ipc_nt_traits.h"

const int ipc_traits::days_in_preceeding_months[12] = { 0, 31, 59, 90,
                                                120, 151, 181, 212,
                                                243, 273, 304, 334 };
const int ipc_traits::days_in_preceeding_months_leap[12] = { 0, 31, 60,
                                                91, 121, 152, 182, 213,
                                                244, 274, 305, 335 };

#endif
