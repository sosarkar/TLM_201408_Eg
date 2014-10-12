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

/* ------------------------------------------------------------------------- */
class Slave: public sc_core::sc_module {
public:
    explicit Slave(sc_core::sc_module_name name );
    ~Slave();

    virtual void end_of_elaboration();

    //sc_in is a port requiring a signal in interface
    sc_in<bool> clock;
    sc_in<bool> resetn;

    sc_in<bool> start;
    sc_out<bool> done;
    //sc_out is a port requiring a signal inout interface
private:    
    SC_HAS_PROCESS(Slave);
    
    // Disable default and copy constructor, assignment operator
    Slave();
    Slave(const Slave &);
    Slave operator= (const Slave &);

    //clocked method
    void ClockMethod();

    // signals
    sc_signal< sc_uint<16> > index_;
    sc_signal< sc_uint<16> > sum_;
    sc_signal<bool> running_ ;

    sc_uint<12> terms_[10];
};

/* ------------------------------------------------------------------------- */
#endif /* SLAVE_H */

