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
static sc_time clock_period = sc_time(5, SC_NS);
static const unsigned int REG_START = 0x0;
static const unsigned int REG_RESULT = 0x10;
static const unsigned int REG_OFFSET_TERMS = 0x20;

/* ------------------------------------------------------------------------- */
Slave::Slave(sc_module_name name):
        sc_module(name),
        ctrlsocket("ctrlsocket"),
        datasocket("datasocket"),
        done("done"),
        sum_(0),
        running_(false)
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

    datasocket.register_b_transport(this, &Slave::my_b_transport_data);
    ctrlsocket.register_b_transport(this, &Slave::my_b_transport_ctrl);
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
        wait(clock_period * 10);
        sum_ = my_dsp_function(terms_, 10);
        running_ = false ;
        done = true;
    }
}

/* ------------------------------------------------------------------------- */
void 
Slave::my_b_transport_ctrl(tlm_generic_payload &trans, sc_time &local_time)
{
    unsigned int length = trans.get_data_length();
    unsigned int address = trans.get_address();
    //some checks and error responses to unsupported features
    if (trans.get_byte_enable_ptr()){
        // not supporting byte enable
        trans.set_response_status(TLM_BYTE_ENABLE_ERROR_RESPONSE);
    } else if (trans.get_streaming_width() < length) {
        // not supporting streaming
        trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
    } else if ((length%4) || (address%4)){
        // not supporting unaligned access 
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
    } else {
        unsigned int *data_ptr = (unsigned int *) trans.get_data_ptr();
        if (trans.is_read()) {
            // let's assume that we only can read the result
            if ((address ==  REG_RESULT) && (length == 4) ) {
                trans.set_response_status(TLM_OK_RESPONSE);
                *data_ptr= sum_;
            } else {
                trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
            }
            local_time += 4 * clock_period;
        } else { // write
            if ((address ==  REG_START) && (length == 4) ) {
                trans.set_response_status(TLM_OK_RESPONSE);
                run_event_.notify(local_time+2*clock_period);
            } else {
                trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
            }
            local_time += 2 * clock_period;
        }
    } 
}
/* ------------------------------------------------------------------------- */
void 
Slave::my_b_transport_data(tlm_generic_payload &trans, sc_time &local_time)
{
    unsigned int length = trans.get_data_length();
    unsigned int address = trans.get_address();

    //some checks and error responses to unsupported features
    if (trans.get_byte_enable_ptr()){
        // not supporting byte enable
        trans.set_response_status(TLM_BYTE_ENABLE_ERROR_RESPONSE);
    } else if (trans.get_streaming_width() < length) {
        // not supporting streaming
        trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
    } else if ((length%4) || (address%4)){
        // not supporting unaligned access 
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
    } else if ( address < REG_OFFSET_TERMS) {
        // invalid address
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
    } else {
        unsigned int begin_index = (address-REG_OFFSET_TERMS)/4;
        unsigned int end_index = begin_index + length/4;
        if (end_index > 10 ) {
            trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
        } else {
            trans.set_response_status(TLM_OK_RESPONSE);
            unsigned int *data_ptr = (unsigned int *) trans.get_data_ptr();
            if (trans.is_read()) {
                for (unsigned int ii = begin_index; ii < end_index; ii++)  {
                    *data_ptr = terms_[ii]; 
                    data_ptr++;
                }
                local_time += (3+(length/4)) * clock_period;
            } else {
                // write
                for (unsigned int ii = begin_index; ii < end_index; ii++)  {
                    terms_[ii]=  (*data_ptr & 0xFFF); 
                    data_ptr++;
                }
                local_time += (1+(length/4)) * clock_period;
            }
        }
    }
}

/* ------------------------------------------------------------------------- */

