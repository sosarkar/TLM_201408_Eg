/* ----------------------------------------------------------------------------
 * slave.cpp
 *
 * A simple slave block
 *
 * Copyright (C) 2012 imec, glasseem@imec.be
 * For copyright and disclaimer notice, see "COPYRIGHT" 
 * ------------------------------------------------------------------------- */
#include "slave.h"
using namespace sc_core;

/* ------------------------------------------------------------------------- */
Slave::Slave(sc_module_name name):
        sc_module(name),
        clock("clock"),
        resetn("resetn"),
        start("start"),
        done("done"),
        index_("index_"),
        sum_("sum_"),
        running_("running_")
{
    terms_[0] = 0xEEF;
    terms_[1] = 0xBEE;
    terms_[2] = 0xDEA;
    terms_[3] = 0x0B0;
    terms_[4] = 0xB00;
    terms_[5] = 0x123;
    terms_[6] = 0x987;
    terms_[7] = 0x645;
    terms_[8] = 0xB23;
    terms_[9] = 0xB63;
}

/* ------------------------------------------------------------------------- */
Slave::~Slave()
{
}

/* ------------------------------------------------------------------------- */
void 
Slave::end_of_elaboration()
{
    SC_METHOD(ClockMethod);
    sensitive << clock.posedge_event();
}

/* ------------------------------------------------------------------------- */
void 
Slave::ClockMethod()
{
    if (resetn == false) {
        // bring in reset state
        index_ = 0;
        sum_ = 0; 
        running_ = false;
        done = false;
    } else {
        if (start) {
            // start restarts the slave
            index_ = 0;
            sum_ = 0; 
            running_ = true;
            done = false;
        } else if (running_) {
            // in running , a term is added until end index reached
           unsigned int index_local = index_.read(); 
            if (index_local == 10 ) {
                done = true;
                running_ = false;
            } else {
                sum_ = sum_.read() + terms_[index_local];
                index_ =  index_local + 1;
            }
        } 
        // else do nothing if not running and no start given
    }
}

/* ------------------------------------------------------------------------- */

