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
static const unsigned int REG_START = 0x0;
static const unsigned int REG_RESULT = 0x10;
static const unsigned int REG_OFFSET_TERMS = 0x20;

/* ------------------------------------------------------------------------- */
Slave::Slave(sc_module_name name):
        sc_module(name),
        clock("clock"),
        resetn("resetn"),
        read_ena("read_ena"),
        write_ena("write_ena"),
        addr("addr"),
        write_data("write_data"),
        read_data("read_data"),
        ready("ready"),
        done("done"),
        index_("index_"),
        sum_("sum_"),
        running_("running_"),
        read_cnt_("read_cnt_"),
        read_pending_("read_pending_"),
        addr_("addr_")
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
        read_data = 0;
        ready = false;
        read_cnt_ = 3; //assume read takes 3 cycles
        read_pending_ = false;
        addr_ = 0;
    } else {
        ready = false; // default, can be rewritten in specific cases
        if (read_pending_) {
            sc_uint<2> read_cnt_local = read_cnt_.read();
            if (read_cnt_local == 0) {    
                sc_uint<32> addr_local = addr_.read();
                switch (addr_local) {
                case REG_RESULT: 
                    read_data = sum_.read();
                    break;
                case REG_START:
                    if (running_) {
                        read_data = 0xFF;
                    } else {
                        read_data = 0;
                    }
                    break;
                default:
                    if ((addr_local >= REG_OFFSET_TERMS) 
                            && ((addr_local & 0x3)  == 0)) {
                        // aligned access of the terms
                        unsigned int index_local = 
                            (addr_local - REG_OFFSET_TERMS) >> 2;
                        if (index_local < 10) {
                            read_data = terms_[index_local]; 
                        }
                    }
                    break;
                }
                ready = true;
                read_pending_ = false;
            } else {
                read_cnt_ = (read_cnt_local - 1);
            }
        } else if (read_ena) {
            // loosing a cycle to see the read_ena, lossing a cycle to see the 
            // first counter value 
            read_cnt_ = 1;
            addr_ = addr.read();
            read_pending_ = true;
        } else if (write_ena) {
            unsigned int addr_local = addr.read().to_uint();
            switch (addr_local) {
            case REG_START:
                if (write_data.read() != 0x0) {
                    //restart
                    running_ = true;
                    index_ = 0;
                    sum_ = 0;
                    done = false;
                }
                break;
            default:
                if ((addr_local >= REG_OFFSET_TERMS) 
                        && ((addr_local & 0x3)  == 0)) {
                    // aligned access of the terms
                    unsigned int index_local = 
                        (addr_local - REG_OFFSET_TERMS) >> 2;
                    if (index_local < 10) {
                        terms_[index_local]  = write_data.read(); 
                    }
                }
                break;
            }
            ready = true;
        } 

        if (running_) {
            // in running state, a term is added until end index reached
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

