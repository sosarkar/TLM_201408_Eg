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
using namespace tlm;
static const sc_time bus_clock_period = sc_time(1e6/BUS_CLOCK_FREQUENCY, SC_PS);
static const sc_time slave_clock_period = sc_time(
        1e6/SLAVE_CLOCK_FREQUENCY, 
        SC_PS);
static const unsigned int bus_width_in_bytes = BUS_WIDTH/8;

/* ------------------------------------------------------------------------- */
Slave::Slave(sc_module_name name):
        sc_module(name),
        target("target"),
        done("done"),
        sum_(0),
        running_(false)
{
    for (unsigned int ii=0; ii < NR_TERMS; ii++) {
        terms_[ii] = 0;
    }

    target.register_b_transport(this, &Slave::my_b_transport);
}

/* ------------------------------------------------------------------------- */
Slave::~Slave()
{
}

/* ------------------------------------------------------------------------- */
void 
Slave::end_of_elaboration()
{
    SC_THREAD(my_run);
    sensitive << run_event_;

}

/* ------------------------------------------------------------------------- */
unsigned int
Slave::my_dsp_function(unsigned int * terms, unsigned int length) 
{
    unsigned int sum = 0;
    for (unsigned int ii =0; ii < length; ii++) {
        sum += terms[ii];
    }
    return sum;
}

/* ------------------------------------------------------------------------- */
void 
Slave::my_run() 
{
    while (true) {
        wait();
        sum_ = 0;
        done = false;
        running_ = true;
        wait(slave_clock_period * NR_TERMS);
        sum_ = my_dsp_function(terms_, NR_TERMS);
        running_ = false ;
        done = true;
    }
}

/* ------------------------------------------------------------------------- */
void 
Slave::my_b_transport(tlm_generic_payload &trans, sc_time &local_time)
{
    sc_assert((BUS_WIDTH % 32) == 0);
    unsigned int length = trans.get_data_length();
    unsigned int address = trans.get_address();

    //some checks and error responses to unsupported features
    if (trans.get_byte_enable_ptr()){
        // not supporting byte enable
        trans.set_response_status(TLM_BYTE_ENABLE_ERROR_RESPONSE);
    } else if (trans.get_streaming_width() < length) {
        // not supporting streaming
        trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
    } else if ((length%bus_width_in_bytes) || (address%bus_width_in_bytes)){
        // not supporting unaligned access 
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
    } else {
        trans.set_response_status(TLM_OK_RESPONSE);
        if (trans.is_read()) { // only allow reading from result register
            if (address == REG_RESULT && (length == bus_width_in_bytes)) {
                unsigned int *data_ptr = (unsigned int *) trans.get_data_ptr();
                *data_ptr= sum_;
            } else {
                trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
            }
            local_time += (SLAVE_READ_DELAY+(length/bus_width_in_bytes)) 
                * bus_clock_period;
        } else {
            // write
            // only on the terms, burst access is taken into account
            if ((address == REG_START) && (length == bus_width_in_bytes)) {
                run_event_.notify(local_time+SLAVE_WRITE_DELAY*bus_clock_period);
            } else if ((address < REG_OFFSET_TERMS) 
                    || (address >= REG_OFFSET_TERMS+sizeof(int)*NR_TERMS)) {
                trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
            } else {
                unsigned int index = (address - REG_OFFSET_TERMS)/sizeof(int);
                unsigned int remaining = (NR_TERMS-index)*sizeof(int);
                const void *data_ptr = (const void *) trans.get_data_ptr();
                if (length<= remaining) {
                    memcpy((void *) &terms_[index], data_ptr, length);
                } else {
                    memcpy((void *) &terms_[index], data_ptr, remaining);
                }
            }
            local_time += (SLAVE_WRITE_DELAY+(length/bus_width_in_bytes)) 
                * bus_clock_period;
        }
    }
}

/* ------------------------------------------------------------------------- */

