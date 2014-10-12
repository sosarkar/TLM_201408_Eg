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

#include <systemc.h>
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

    //connecting signals
    sc_signal<bool> resetn;
    sc_signal< sc_uint<12> > start;
    sc_signal< bool > done;
    sc_signal< sc_uint<16> > result;

    //modules
    sc_clock clock_;
    Reset reset_;
    Master master_;
    Slave slave_;
};

/* ------------------------------------------------------------------------- */
#endif /* TOP_H */

