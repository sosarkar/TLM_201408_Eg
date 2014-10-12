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
static const unsigned int REG_START = 0x0;
static const unsigned int REG_RESULT = 0x10;
static const unsigned int REG_OFFSET_TERMS = 0x20;

/* ------------------------------------------------------------------------- */
Master::Master(sc_module_name name):
        sc_module(name),
        clock("clock"),
        resetn("resetn"),
        done("done"),
        read_ena("read_ena"),
        write_ena("write_ena"),
        addr("addr"),
        write_data("write_data"),
        read_data("read_data"),
        ready("ready"),
        iterations_("iterations_"),
        counter_("counter_"),
        slave_running_("slave_running_"),
        read_pending_("read_pending_"),
        write_pending_("write_pending_")
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
        read_ena = false;
        write_ena = false;
        addr = 0;
        write_data = 0;
        iterations_ = 0;
        counter_ = 0;
        slave_running_ = false;
        read_pending_ = false;
        write_pending_ = false;
    } else {
        if (read_ena.read()) {
            // clear the enable after one cycle
            read_ena = false ;
        } else if (write_ena) {
            // clear the enable after one cycle
            write_ena = false ;
        }

        if (read_pending_) {
            if (ready) {
                // slave has processed the read, get the data
                sc_uint<32> data_local = read_data;
                sc_uint<32> addr_local = addr;

                std::ostringstream msg;
                msg << name() << " @ " << sc_time_stamp() << ": read 0x" << hex
                    << data_local << " from address 0x" << addr_local;
                SC_REPORT_INFO(__FILE__, msg.str().c_str());

                iterations_ = iterations_ + 1 ;
                read_pending_ = false;
            }
        } else if (write_pending_) {
            if (ready) {
                // slave has processed the write
                sc_uint<32> addr_local = addr;
               if (addr_local == REG_START) { 
                   slave_running_ = true;
                   counter_ = 0;
               }
                write_pending_ = false;
            }
        } else if (iterations_ == 5) {
            std::ostringstream msg;
            msg << name() << " @ " << sc_time_stamp() << ": end of simulation";
            SC_REPORT_INFO(__FILE__, msg.str().c_str()); 
            sc_stop();
        } else if (!slave_running_) {
            // if the slave is not running, count till 50 and start
            counter_ = counter_ + 1;
            if (counter_ == 50) {
                write_ena = true;
                addr = REG_START;
                write_data = 0x1;
                write_pending_ = true;
            } else if (counter_ == 25) {
                // for every iteration, at count 25, program a term
                write_ena = true;
                addr = REG_OFFSET_TERMS + (iterations_ << 2);
                write_data = counter_ + (iterations_ << 8);
                write_pending_ = true;
            } 
        } else if (done) {
            // start reading result
            slave_running_ = false;
            read_ena = true;
            addr = REG_RESULT;
            read_pending_ = true;
        } // the slave is running; wait until it is done
    }
}

/* ------------------------------------------------------------------------- */

