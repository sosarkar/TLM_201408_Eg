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

/* ------------------------------------------------------------------------- */
class Master: public sc_core::sc_module {
public:
    explicit Master(sc_core::sc_module_name name );
    ~Master();

    virtual void end_of_elaboration();
    sc_in<bool> done;

    tlm_utils::simple_initiator_socket<Master> datasocket;
    tlm_utils::simple_initiator_socket<Master> ctrlsocket;

private:    
    SC_HAS_PROCESS(Master);
    
    // Disable default and copy constructor, assignment operator
    Master();
    Master(const Master &);
    Master operator= (const Master &);

    // worker thread
    void worker_thread();
};

/* ------------------------------------------------------------------------- */
#endif /* MASTER_H */

