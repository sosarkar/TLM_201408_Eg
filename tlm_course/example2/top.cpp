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
        read_ena("read_ena"),
        write_ena("write_ena"),
        addr("addr"),
        write_data("write_data"),
        read_data("read_data"),
        ready("ready"),
        done("done"),
        clock_("clock_", 5, SC_NS),
        reset_("reset_", sc_time(50, SC_NS)),
        master_("master_"),
        slave_("slave_")
{

    reset_.resetn(resetn);

    master_.clock(clock_);
    master_.resetn(resetn);
    master_.read_ena(read_ena);
    master_.write_ena(write_ena);
    master_.addr(addr);
    master_.write_data(write_data);
    master_.read_data(read_data);
    master_.ready(ready);
    master_.done(done);
    
    slave_.clock(clock_);
    slave_.resetn(resetn);
    slave_.read_ena(read_ena);
    slave_.write_ena(write_ena);
    slave_.addr(addr);
    slave_.write_data(write_data);
    slave_.read_data(read_data);
    slave_.ready(ready);
    slave_.done(done);
}

/* ------------------------------------------------------------------------- */
Top::~Top()
{
}

/* ------------------------------------------------------------------------- */

