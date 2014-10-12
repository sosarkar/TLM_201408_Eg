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
        master_("master_"),
        bus_("bus_")
{
    master_.init(bus_.target);

    std::ostringstream nm;
    for (unsigned int ii = 0; ii < NR_SLAVES; ii++) {
        nm.str("");
        nm << "slave_" << ii;
        slave_[ii] = new Slave(nm.str().c_str());
       
        bus_.initiators[ii](slave_[ii]->target);
        slave_[ii]->done(done[ii]);
    }
    master_.done(done[0]);
}

/* ------------------------------------------------------------------------- */
Top::~Top()
{
}

/* ------------------------------------------------------------------------- */

