/* ----------------------------------------------------------------------------
 * top.cpp
 *
 * Top level to connect master and slave module
 *
 * Copyright (C) 2012 imec, glasseem@imec.be
 * For copyright and disclaimer notice, see "COPYRIGHT" 
 * ------------------------------------------------------------------------- */
#include "top.h"
using namespace sc_core;

/* ------------------------------------------------------------------------- */
Top::Top(sc_module_name name):
        sc_module(name),
        done("done"),
        master_("master_"),
        slave_("slave_")
{
    master_.done(done);
    slave_.done(done);
    master_.datasocket(slave_.datasocket);
    master_.ctrlsocket(slave_.ctrlsocket);
}

/* ------------------------------------------------------------------------- */
Top::~Top()
{
}

/* ------------------------------------------------------------------------- */

