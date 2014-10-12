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
#include "bus.h"

static const unsigned int NR_SLAVES = 2;
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
    sc_signal<bool> done[NR_SLAVES];

    //modules
    Master master_;
    Slave *slave_[NR_SLAVES];
    Bus<NR_SLAVES> bus_;
};

/* ------------------------------------------------------------------------- */
#endif /* TOP_H */

