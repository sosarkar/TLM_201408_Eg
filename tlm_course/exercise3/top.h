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
    sc_signal<bool> done;

    //modules
    Master master_;
    Slave slave_;
};

/* ------------------------------------------------------------------------- */
#endif /* TOP_H */

