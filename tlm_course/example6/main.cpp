/* ----------------------------------------------------------------------------
 * main.cpp
 *
 * a generic main for systemC tests
 *
 * Copyright (C) 2012 imec, glasseem@imec.be
 * For copyright and disclaimer notice, see "COPYRIGHT" 
 * ------------------------------------------------------------------------- */

#include "top.h"
/* ------------------------------------------------------------------------- */
int sc_main (int argc , char *argv[]) {
    Top top("top");
    sc_core::sc_start();
    return 0;
}
/* ------------------------------------------------------------------------- */

