/* ----------------------------------------------------------------------------
 * top.h
 *
 * Top level to connect master and slave module
 *
 * Copyright (C) 2012 imec, glasseem@imec.be
 * For copyright and disclaimer notice, see "COPYRIGHT" 
 * ------------------------------------------------------------------------- */
#ifndef TOP_H
#define TOP_H

#include "master.h"
#include "slave.h"
#include "reset.h"

/* ------------------------------------------------------------------------- */
class Top: public sc_core::sc_module {
public:
    explicit Top(sc_core::sc_module_name name );
    ~Top();

private:    
    SC_HAS_PROCESS(Top);
    
    // Disable default and copy constructor, assignment operator
    Top();
    Top(const Top &);
    Top operator= (const Top &);

    // connecting signals
    // signals are channels implementing a signal inout interface (of one bit 
    // wide in case of bool) they model a single wire
    sc_signal<bool> resetn;
    sc_signal<bool> start;
    sc_signal<bool> done;
   
    sc_clock clock_; // channel providing a clock-like behaviour
    //modules
    Reset reset_;
    Master master_;
    Slave slave_;
};

/* ------------------------------------------------------------------------- */
#endif /* TOP_H */

