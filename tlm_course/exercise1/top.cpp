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
        resetn("resetn"),
        start("start"),
        result("result"),
        done("done"),
        clock_("clock_", 5, SC_NS),
        reset_("reset_", sc_time(50, SC_NS)),
        master_("master_"),
        slave_("slave_")
{

    reset_.resetn(resetn);

    master_.clock(clock_);
    master_.resetn(resetn);
    master_.start(start);
    master_.result(result);
    master_.done(done);
    
    slave_.clock(clock_);
    slave_.resetn(resetn);
    slave_.start(start);
    slave_.result(result);
    slave_.done(done);
}

/* ------------------------------------------------------------------------- */
Top::~Top()
{
}

/* ------------------------------------------------------------------------- */

