/* ----------------------------------------------------------------------------
 * master.cpp
 *
 * a simple master block
 *
 * Copyright (C) 2012 imec, glasseem@imec.be
 * For copyright and disclaimer notice, see "COPYRIGHT" 
 * ------------------------------------------------------------------------- */
#include "master.h"
using namespace sc_core;

/* ------------------------------------------------------------------------- */
Master::Master(sc_module_name name):
        sc_module(name),
        clock("clock"),
        resetn("resetn"),
        start("start"),
        done("done"),
        iterations_("iterations_"),
        counter_("counter_"),
        slave_running_("slave_running_")
{
}

/* ------------------------------------------------------------------------- */
Master::~Master()
{
}

/* ------------------------------------------------------------------------- */
void
Master::end_of_elaboration()
{
    SC_METHOD(ClockMethod);
    sensitive << clock.posedge_event();
}

/* ------------------------------------------------------------------------- */
void
Master::ClockMethod()
{
    if (resetn == false) {
        start = false; // writing to signal interface through port
        iterations_ = 0;
        counter_ = 0;
        slave_running_ = false;
    } else {
        if (start) { 
            // reset start signal after one cycle
            start = false;
        } else if (iterations_ == 5) {
            std::ostringstream msg;
            msg << name() << " @ " << sc_time_stamp() << ": end of simulation";
            SC_REPORT_INFO(__FILE__, msg.str().c_str()); 
            sc_stop();
        } else if (!slave_running_) {
            // if the slave is not running, count till 50 and start
            if (counter_ == 50) {
                start = true;
                slave_running_ = true;
                counter_ = 0;
            } else {
                counter_ = counter_ + 1;
            }
        } else if (done) {
            // increment the iteration count if slave gives the done signal
            iterations_ = iterations_ + 1 ;
            slave_running_ = false;
        } // the slave is running; wait until it is done
    }
}

/* ------------------------------------------------------------------------- */

