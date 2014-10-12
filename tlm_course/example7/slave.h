/* ----------------------------------------------------------------------------
 * slave.h
 *
 * A simple slave block
 *
 * Copyright (C) 2012 imec, glasseem@imec.be
 * For copyright and disclaimer notice, see "COPYRIGHT" 
 * ------------------------------------------------------------------------- */
#ifndef SLAVE_H
#define SLAVE_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/peq_with_cb_and_phase.h>

/* ------------------------------------------------------------------------- */
class Slave: public sc_core::sc_module {
public:
    explicit Slave(sc_core::sc_module_name name );
    ~Slave();

    virtual void end_of_elaboration();

    sc_out<bool> done;

    tlm_utils::simple_target_socket<Slave> target;

private:    
    SC_HAS_PROCESS(Slave);
    
    // Disable default and copy constructor, assignment operator
    Slave();
    Slave(const Slave &);
    Slave operator= (const Slave &);

    // internal registers/state variables
    unsigned int terms_[10];
    bool running_;
    unsigned int sum_;
    bool resp_pending;

    // The payload event queue
    tlm_utils::peq_with_cb_and_phase<Slave> peq_;

    // end of run event
    sc_core::sc_event run_event_;

    // run function
    void my_run();

    // some processing function
    unsigned int my_dsp_function(unsigned int * terms, unsigned int length);

    //non-blocking transport function
    tlm::tlm_sync_enum my_nb_transport_fw(
            tlm::tlm_generic_payload &trans, 
            tlm::tlm_phase &phase, 
            sc_core::sc_time &local_time);

    // payload event queue callback
    void peq_cb(tlm::tlm_generic_payload &trans, const tlm::tlm_phase &phase);
};

/* ------------------------------------------------------------------------- */
#endif /* SLAVE_H */

