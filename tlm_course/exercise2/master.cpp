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
using namespace std;
static sc_time clock_period = sc_time(5, SC_NS);
static const unsigned int REG_START = 0x0;
static const unsigned int REG_RESULT = 0x10;
static const unsigned int REG_OFFSET_TERMS = 0x20;

/* ------------------------------------------------------------------------- */
Master::Master(sc_module_name name):
        sc_module(name),
        done("done"),
        ctrlsocket("ctrlsocket"),
        datasocket("datasocket")
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
    SC_THREAD(worker_thread);
}

/* ------------------------------------------------------------------------- */
void
Master::worker_thread()
{
    // the data for the transaction
    unsigned int data;

    //  using single transaction throughout the process
    tlm::tlm_generic_payload trans;

    // disable byte_enable 
    trans.set_byte_enable_ptr(0);
   
    // make the transaction use data for its data
    trans.set_data_ptr((unsigned char *) &data);

    // set transaction length to 4 bytes = 1 integer
    trans.set_streaming_width(4);
    trans.set_data_length(4);

    sc_time local_time(50, SC_NS);  // account for reset

    ostringstream msg;
    for (int ii =0; ii < 5; ii++) {
        // wait 25 cycles
        local_time += 25 * clock_period;

        // program a term
        trans.set_write();
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        trans.set_address(REG_OFFSET_TERMS + (ii<<2));
        data = (ii << 8)  + 25;
        datasocket->b_transport(trans, local_time);
        
        msg.str("");
        msg << name() << " @ " << sc_time_stamp() << "+" << local_time 
            << ": Transaction returned " << trans.get_response_string();
        if (trans.is_response_error()) {
            SC_REPORT_ERROR(__FILE__, msg.str().c_str()); 
        } 
        SC_REPORT_INFO(__FILE__, msg.str().c_str()); 

        // wait 25 cycles
        local_time += 25 * clock_period;

        // start the slave
        trans.set_write();
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        trans.set_address(REG_START);
        data = 0x1;
        ctrlsocket->b_transport(trans, local_time);
        
        msg.str("");
        msg << name() << " @ " << sc_time_stamp() << "+" << local_time 
            << ": Transaction returned " << trans.get_response_string();
        if (trans.is_response_error()) {
            SC_REPORT_ERROR(__FILE__, msg.str().c_str()); 
        } 
        SC_REPORT_INFO(__FILE__, msg.str().c_str()); 

        wait(local_time);
        wait(done.posedge_event());
        // reading can start one clock after done
        local_time = clock_period;

        // read the result
        trans.set_read();
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        trans.set_address(REG_RESULT);
        ctrlsocket->b_transport(trans, local_time);
        
        msg.str("");
        msg << name() << " @ " << sc_time_stamp() << "+" << local_time 
            << ": Transaction returned " << trans.get_response_string();
        if (trans.is_response_error()) {
            SC_REPORT_ERROR(__FILE__, msg.str().c_str()); 
        } 
        msg << ", result = " << data << " (0x" << hex << data << dec << ")";
        SC_REPORT_INFO(__FILE__, msg.str().c_str()); 
        
        local_time += clock_period;
    }
    wait(local_time);

    msg.str("");
    msg << name() << " @ " << sc_time_stamp() << ": end of simulation";
    SC_REPORT_INFO(__FILE__, msg.str().c_str()); 
    
    sc_stop();
}

/* ------------------------------------------------------------------------- */

