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
using namespace tlm;
using namespace std;
static sc_time clock_period = sc_time(5, SC_NS);
static const unsigned int REG_START = 0x0;
static const unsigned int REG_RESULT = 0x10;
static const unsigned int REG_OFFSET_TERMS = 0x20;

// the memory manager implementation
/* ------------------------------------------------------------------------- */
TransactionPool::TransactionPool()
{}

/* ------------------------------------------------------------------------- */
TransactionPool::~TransactionPool()
{
    while (transaction_queue_.size()) {
        tlm_generic_payload *trans = transaction_queue_.front();
        transaction_queue_.pop();
        delete[] trans->get_data_ptr();
        delete trans;
    }
}

/* ------------------------------------------------------------------------- */
void
TransactionPool::free(tlm_generic_payload *trans)
{
    transaction_queue_.push(trans);
}

/* ------------------------------------------------------------------------- */
tlm_generic_payload *
TransactionPool::get_transaction()
{
    tlm_generic_payload *trans=0;
    if (transaction_queue_.size()) {
        trans = transaction_queue_.front();
        transaction_queue_.pop();
    } else {
        trans = new tlm_generic_payload(this);
        // standard transaction params
        //not using byte enable
        trans->set_byte_enable_ptr(0);

        // set transaction length to 4 bytes = 1 integer
        trans->set_streaming_width(4);
        trans->set_data_length(4);

        // allocate data for data pointer
        unsigned char *data= new unsigned char[4];
        // make the transaction use data for its data
        trans->set_data_ptr(data);
    }
    return trans;
}

/* ------------------------------------------------------------------------- */
// The master implementation
/* ------------------------------------------------------------------------- */
Master::Master(sc_module_name name):
        sc_module(name),
        done("done"),
        init("init"),
        req_pending_(false),
        resp_pending_(false)
{
    init.register_nb_transport_bw(this, &Master::my_nb_transport_bw);
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
    wait(50, SC_NS);  // account for reset
    sc_time local_time = SC_ZERO_TIME;

    ostringstream msg;

    for (int ii =0; ii < 5; ii++) {
        // wait 25 cycles
        wait(local_time + 25 * clock_period);
        local_time = SC_ZERO_TIME;

        // get a transaction object
        tlm_generic_payload *trans=pool_.get_transaction();
        trans->acquire();
        unsigned int *data = (unsigned int *)trans->get_data_ptr();

        // program a term
        trans->set_write();
        trans->set_address(REG_OFFSET_TERMS + (ii<<2));
        *data = (ii << 8)  + 25;
        run_transaction(*trans, local_time);
        if (resp_pending_) {
            wait(resp_event_);
        } else {
            wait(local_time);
        }
        local_time = SC_ZERO_TIME;

        // wait 25 cycles
        wait(25 * clock_period);

        // get a transaction object
        trans=pool_.get_transaction();
        trans->acquire();
        data = (unsigned int *)trans->get_data_ptr();

        // start the slave
        trans->set_write();
        trans->set_address(REG_START);
        *data = 0x1;
        run_transaction(*trans, local_time);
        if (resp_pending_) {
            wait(resp_event_);
        } else {
            wait(local_time);
        }
        local_time = SC_ZERO_TIME;

        wait(done.posedge_event());

        // reading can start one clock after done
        wait(clock_period);

        // read the result
        //Get a transaction object
        trans=pool_.get_transaction();
        trans->acquire();

        trans->set_read();
        trans->set_address(REG_RESULT);
        run_transaction(*trans, local_time);
        if (resp_pending_) {
            wait(resp_event_);
        } else {
            wait(local_time);
        }
        local_time = SC_ZERO_TIME;

        data = (unsigned int *)trans->get_data_ptr();
        msg.str("");
        msg << name() << " @ " << sc_time_stamp() << ": result = " 
            << *data << " (0x" << hex << *data << dec << ")";
        SC_REPORT_INFO(__FILE__, msg.str().c_str());
        wait(clock_period);
    }

    msg.str("");
    msg << name() << " @ " << sc_time_stamp() << ": end of simulation";
    SC_REPORT_INFO(__FILE__, msg.str().c_str()); 

    sc_stop();
}

/* ------------------------------------------------------------------------- */
tlm_sync_enum 
Master::my_nb_transport_bw(
            tlm_generic_payload &trans,
            tlm_phase &phase,
            sc_time &time)
{
    tlm_sync_enum ret = TLM_ACCEPTED;
    switch (phase) {
    case END_REQ:
        assert(req_pending_);
        req_pending_ = false;
        break;
    case BEGIN_RESP:
        assert(resp_pending_);
        resp_pending_ = false;
        req_pending_ = false;
        resp_event_.notify(time);
        ostringstream msg;
        msg << name() << " @ " << sc_time_stamp() << "+" << time
            << ": Transaction returned " << trans.get_response_string();
        if (trans.is_response_error()) {
            SC_REPORT_ERROR(__FILE__, msg.str().c_str());
        }
        SC_REPORT_INFO(__FILE__, msg.str().c_str()); 
        ret = TLM_COMPLETED;

        //release transaction object
        trans.release();
    }
    return ret;
}

/* ------------------------------------------------------------------------- */
void
Master::run_transaction(tlm_generic_payload &trans, sc_time &time)
{
    trans.set_dmi_allowed(false);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    tlm_phase phase = BEGIN_REQ;
    tlm_sync_enum res = init->nb_transport_fw(trans, phase, time);
    ostringstream msg;

    switch (res) {
    case TLM_ACCEPTED:
        req_pending_ = true;
        resp_pending_ = true;
        break;
    case TLM_UPDATED:
        switch (phase) {
        case END_REQ:
            resp_pending_ =true;
            break;
        case BEGIN_RESP:
            msg.str("");
            msg << name() << " @ " << sc_time_stamp() << "+" << time
                << ": Transaction returned " << trans.get_response_string();
            if (trans.is_response_error()) {
                SC_REPORT_ERROR(__FILE__, msg.str().c_str());
            }
            SC_REPORT_INFO(__FILE__, msg.str().c_str());
            phase = END_RESP;
            res = init->nb_transport_fw(trans, phase, time);

            //release transaction object
            trans.release();
            break;
        default:
            msg.str("");
            msg << name() << " @ " << sc_time_stamp() << "+" << time
                << ": ignoring unknown phase";
            SC_REPORT_WARNING(__FILE__, msg.str().c_str());
            break;
        }
        break;
    case TLM_COMPLETED:
        msg.str("");
        msg << name() << " @ " << sc_time_stamp() << "+" << time
            << ": Transaction returned " << trans.get_response_string();
        if (trans.is_response_error()) {
            SC_REPORT_ERROR(__FILE__, msg.str().c_str());
        }
        SC_REPORT_INFO(__FILE__, msg.str().c_str());

        //release transaction object
        trans.release();
        break;
    }
}

/* ------------------------------------------------------------------------- */

