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
static const sc_time clock_period = sc_time(1e6/BUS_CLOCK_FREQUENCY, SC_PS);
static const unsigned int bus_width_in_bytes = BUS_WIDTH/8;
#include "init_data.h" // 1024 init values
/* ------------------------------------------------------------------------- */
Master::Master(sc_module_name name):
        sc_module(name),
        init("init")
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
Master::start_slave(
        tlm::tlm_generic_payload &trans,
        unsigned int slave,
        sc_time &local_time)
{
    unsigned int base_index = (slave << ADDRES_BITS_PER_SLAVE);

    trans.set_write();
    trans.set_streaming_width(bus_width_in_bytes);
    trans.set_data_length(bus_width_in_bytes);

    //allocate right amount of data;
    unsigned char data[bus_width_in_bytes];
    unsigned int *idata = (unsigned int *) data;
    trans.set_data_ptr((unsigned char *) &data);
    
    trans.set_dmi_allowed(false);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans.set_address(base_index+REG_START);

    *idata = 0x1;
    init->b_transport(trans, local_time);

    ostringstream msg;
    msg << name() << " @ " << sc_time_stamp() << "+" << local_time 
        << ": Transaction returned " << trans.get_response_string();
    if (trans.is_response_error()) {
        SC_REPORT_ERROR(__FILE__, msg.str().c_str()); 
    } 
    SC_REPORT_INFO(__FILE__, msg.str().c_str()); 
}

/* ------------------------------------------------------------------------- */
unsigned int 
Master::get_result(
        tlm::tlm_generic_payload &trans,
        unsigned int slave,
        sc_time &local_time)
{
    // base address of the slave
    unsigned int base_index = (slave << ADDRES_BITS_PER_SLAVE);

    trans.set_read();
    trans.set_streaming_width(bus_width_in_bytes);
    trans.set_data_length(bus_width_in_bytes);
    //allocate right amount of data;
    unsigned char data[bus_width_in_bytes];
    trans.set_data_ptr(data);

    // read the result
    trans.set_dmi_allowed(false);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans.set_address(base_index+REG_RESULT);
    init->b_transport(trans, local_time);

    ostringstream msg;
    msg << name() << " @ " << sc_time_stamp() << "+" << local_time 
        << ": Transaction returned " << trans.get_response_string();
    if (trans.is_response_error()) {
        SC_REPORT_ERROR(__FILE__, msg.str().c_str()); 
    }
    unsigned int result = *(unsigned int *) data;
    msg << ", result = " << result << " (0x" << hex << result << dec << ")";
    SC_REPORT_INFO(__FILE__, msg.str().c_str()); 

    return result;
}

/* ------------------------------------------------------------------------- */
void
Master::program_terms(
        tlm::tlm_generic_payload &trans, 
        unsigned char * data_ptr,
        unsigned int slave,
        sc_time &local_time)
{
    // base address of the slave
    unsigned int base_index = (slave << ADDRES_BITS_PER_SLAVE);

    // The number of bus words required to program all terms of a sum
    unsigned int bus_words_required = 
        (NR_TERMS * sizeof(int)+bus_width_in_bytes-1)/bus_width_in_bytes;

    trans.set_write();
    unsigned int length = (BURST_WIDTH*bus_width_in_bytes);
    trans.set_streaming_width(length);
    trans.set_data_length(length);
    unsigned int index = 0;
    
    ostringstream msg;
    while ( bus_words_required  > BURST_WIDTH) {
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        trans.set_address(base_index + REG_OFFSET_TERMS+index);
        trans.set_data_ptr(data_ptr + index);
        init->b_transport(trans, local_time);

        msg.str("");
        msg << name() << " @ " << sc_time_stamp() << "+" << local_time 
            << ": Transaction returned " << trans.get_response_string();
        if (trans.is_response_error()) {
            SC_REPORT_ERROR(__FILE__, msg.str().c_str()); 
        }
        SC_REPORT_INFO(__FILE__, msg.str().c_str()); 

        index += BURST_WIDTH * bus_width_in_bytes;
        bus_words_required -= BURST_WIDTH;
    }
    // program last chunk
    length = (bus_words_required*bus_width_in_bytes);
    trans.set_streaming_width(length);
    trans.set_data_length(length);
    trans.set_dmi_allowed(false);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans.set_address(base_index + REG_OFFSET_TERMS+index);
    trans.set_data_ptr(data_ptr + index);
    init->b_transport(trans, local_time);

    msg.str("");
    msg << name() << " @ " << sc_time_stamp() << "+" << local_time 
        << ": Transaction returned " << trans.get_response_string();
    if (trans.is_response_error()) {
        SC_REPORT_ERROR(__FILE__, msg.str().c_str()); 
    }
    SC_REPORT_INFO(__FILE__, msg.str().c_str()); 
}
/* ------------------------------------------------------------------------- */
void
Master::worker_thread()
{
    // keep track of the state of the slaves
    bool slave_running[NR_SLAVES] ;
    unsigned int result_index[NR_SLAVES];
    for (unsigned int ii = 0; ii < NR_SLAVES; ii++) {
        slave_running[ii] = false;
    }

    //  using single transaction throughout the process
    tlm::tlm_generic_payload trans;

    // disable byte_enable 
    trans.set_byte_enable_ptr(0);

    sc_time local_time(50, SC_NS);  // account for reset

    ostringstream msg;

    unsigned int current_slave = 0;

    unsigned int results[NR_SUMS];
    for (unsigned int ii = 0; ii < NR_SUMS; ii++) {
        // if the slave we want to use is running, wait until it is done and 
        // get result
        if (slave_running[current_slave]) {
            wait(local_time);
            local_time = SC_ZERO_TIME;
            if (done[current_slave] == 0) {
                wait(done[current_slave].posedge_event());
            }
            results[result_index[current_slave]] = 
                get_result(trans, current_slave, local_time);
            slave_running[current_slave] = false;
        }
        // program terms and start slave
        unsigned char *ptr= (unsigned char*) &init_data[ii*NR_TERMS];
        program_terms(trans, ptr, current_slave, local_time);
        start_slave(trans, current_slave, local_time);

        //keep track of state;
        slave_running[current_slave] = true;
        result_index[current_slave] = ii;

        current_slave++;
        if (current_slave == NR_SLAVES) {
            current_slave = 0;
        }
    }

    // collect outstanding results 
    for (unsigned int ii = 0; ii < NR_SLAVES; ii++) {
        if (slave_running[ii]) {
            wait(local_time);
            local_time = SC_ZERO_TIME;
            if (done[ii] == 0) {
                wait(done[ii].posedge_event());
            }
            results[result_index[ii]] = get_result(trans, ii, local_time);
            slave_running[ii] = false;
        }
    }

    wait(local_time);
    msg.str("");
    msg << name() << " @ " << sc_time_stamp() << ": end of simulation";
    for (unsigned int ii = 0; ii < NR_SUMS; ii++) {
        msg << endl << "sum[" << ii << "] = " << results[ii] << hex << " (0x" 
            << results[ii] << ")" << dec;
    }
    SC_REPORT_INFO(__FILE__, msg.str().c_str());

    sc_stop();
}

/* ------------------------------------------------------------------------- */

