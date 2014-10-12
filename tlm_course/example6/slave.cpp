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
using namespace std;
static sc_time clock_period = sc_time(5, SC_NS);
static const unsigned int REG_START = 0x0;
static const unsigned int REG_RESULT = 0x10;
static const unsigned int REG_OFFSET_TERMS = 0x20;

/* ------------------------------------------------------------------------- */
Slave::Slave(sc_module_name name):
        sc_module(name),
        target("target"),
        done("done"),
        sum_(0),
        running_(false),
        resp_pending(false),
        peq_("peq_", this, &Slave::peq_cb)
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

    target.register_nb_transport_fw(this, &Slave::my_nb_transport_fw);
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
tlm_sync_enum 
Slave::my_nb_transport_fw(
        tlm_generic_payload &trans, 
        tlm_phase &phase, 
        sc_time &local_time)
{
    // memory manager is mandatory for non-blocking transports
    sc_assert(trans.has_mm()); 
    tlm_sync_enum ret= TLM_COMPLETED;
    unsigned int length;
    unsigned int address;
    ostringstream msg;

    switch (phase) {
    case BEGIN_REQ:
        length = trans.get_data_length();
        address = trans.get_address();
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
            ret = TLM_ACCEPTED;
            trans.acquire();
            // one clock cycle accept delay
            sc_time event_time = local_time + clock_period;
            tlm_phase  phase_peq= END_REQ;
            peq_.notify(trans, phase_peq, event_time);

            //response delay depends on read or write 
            if (trans.is_read()) {
                event_time = local_time + (3+(length/4)) * clock_period;
            } else {
                event_time = local_time + (1+(length/4)) * clock_period;
            }
            phase_peq= BEGIN_RESP;
            peq_.notify(trans, phase_peq, event_time);
        }
        break;
    case END_RESP:
        sc_assert(resp_pending);
        resp_pending = false;
        break;
    case BEGIN_RESP:
    case END_REQ:
        msg << name() << " @ " << sc_time_stamp() << "+" << local_time 
            << ": illegal phase in forward path"; 
        SC_REPORT_ERROR(__FILE__, msg.str().c_str());
        break;
    default:
        ret = TLM_ACCEPTED;
        ostringstream msg;
        msg << name() << " @ " << sc_time_stamp() << "+" << local_time 
            << ": ignoring unknown phase"; 
        SC_REPORT_WARNING(__FILE__, msg.str().c_str());
        break;
    }
    return ret;
}

/* ------------------------------------------------------------------------- */
void
Slave::peq_cb(tlm_generic_payload &trans, const tlm_phase &phase)
{
    sc_time time = SC_ZERO_TIME;
    tlm_phase phase_bw = phase;
    if (phase == END_REQ) {
        tlm_sync_enum res= target->nb_transport_bw(trans, phase_bw, time);
    } else {
        sc_assert(phase == BEGIN_RESP);
        unsigned int length = trans.get_data_length();
        unsigned int address = trans.get_address();

        trans.set_response_status(TLM_OK_RESPONSE);
        unsigned int *data_ptr = (unsigned int *) trans.get_data_ptr();
        if (trans.is_read()) {
            for (unsigned int ii = address; ii < (address+length); ii += 4)  {
                switch (ii) {
                case REG_START:
                    *data_ptr= running_;
                    break;
                case REG_RESULT:
                    *data_ptr= sum_;
                    break;
                default:
                    if (ii < REG_OFFSET_TERMS) {
                        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
                    } else {
                        unsigned int index = ( ii - REG_OFFSET_TERMS)/4;
                        if (index >= 10) {
                            trans.set_response_status(
                                    TLM_ADDRESS_ERROR_RESPONSE);
                        } else {
                            *data_ptr = terms_[index]; 
                        }
                    }
                    break;
                }
                data_ptr++;
            }
        } else {
            // write
            for (unsigned int ii = address; ii < (address+length); ii += 4)  {
                switch (ii) {
                case REG_START:
                    run_event_.notify(time);
                    break;
                default:
                    if (ii < REG_OFFSET_TERMS) {
                        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
                    } else {
                        unsigned int index = ( ii - REG_OFFSET_TERMS)/4;
                        if (index >= 10) {
                            trans.set_response_status(
                                    TLM_ADDRESS_ERROR_RESPONSE);
                        } else {
                            terms_[index]=  ((*data_ptr) & 0xFFF); 
                        }
                    }
                    break;
                }
                data_ptr++;
            }
        }
        tlm_sync_enum res= target->nb_transport_bw(trans, phase_bw, time);
        trans.release();
        if ((res == TLM_ACCEPTED) 
                || ((res == TLM_UPDATED) && (phase != END_RESP))) {
            resp_pending = true;
        } // all other cases end the transaction
    }
}

/* ------------------------------------------------------------------------- */

