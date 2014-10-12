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

/* ------------------------------------------------------------------------- */
class Master: public sc_core::sc_module {
public:
    explicit Master(sc_core::sc_module_name name );
    ~Master();

    virtual void end_of_elaboration();
    sc_in<bool> clock;
    sc_in<bool> resetn;

    sc_out<bool> start;
    sc_in<bool> done;

private:    
    SC_HAS_PROCESS(Master);
    
    // Disable default and copy constructor, assignment operator
    Master();
    Master(const Master &);
    Master operator= (const Master &);

    // clocked method
    void ClockMethod();

    // signals
    sc_signal< unsigned int > iterations_;
    sc_signal< unsigned int > counter_;
    sc_signal<bool> slave_running_;
};

/* ------------------------------------------------------------------------- */
#endif /* MASTER_H */

