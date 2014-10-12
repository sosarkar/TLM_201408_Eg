/* ----------------------------------------------------------------------------
 * master.h
 *
 * a simple master block
 *
 * Copyright (C) 2012 imec, glasseem@imec.be
 * For copyright and disclaimer notice, see "COPYRIGHT" 
 * ------------------------------------------------------------------------- */
#ifndef MASTER_H
#define MASTER_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <queue>
using namespace tlm;

/* ------------------------------------------------------------------------- */
class TransactionPool: public tlm_mm_interface {
public:
    TransactionPool();
    ~TransactionPool();
    void free(tlm_generic_payload *trans);
    tlm_generic_payload *get_transaction();
private:
    std::queue<tlm_generic_payload *> transaction_queue_;
};

/* ------------------------------------------------------------------------- */
class Master: public sc_core::sc_module {
public:
    explicit Master(sc_core::sc_module_name name );
    ~Master();

    virtual void end_of_elaboration();
    sc_in<bool> done;

    tlm_utils::simple_initiator_socket<Master> init;

private:
    SC_HAS_PROCESS(Master);

    // Disable default and copy constructor, assignment operator
    Master();
    Master(const Master &);
    Master operator= (const Master &);

    // worker thread
    void worker_thread();
    void run_transaction(
            tlm_generic_payload &trans,
            sc_core::sc_time &time);

    // non-blocking backward transport function
    tlm_sync_enum my_nb_transport_bw(
            tlm_generic_payload &trans,
            tlm_phase &phase,
            sc_core::sc_time &time);

    // state variables
    bool req_pending_;
    bool resp_pending_;

    sc_core::sc_event resp_event_;
    TransactionPool pool_;
};

/* ------------------------------------------------------------------------- */
#endif /* MASTER_H */

