/* ----------------------------------------------------------------------------
 * reset.h
 *
 * A reset generator
 *
 * Copyright (C) 2012 imec, glasseem@imec.be
 * For copyright and disclaimer notice, see "COPYRIGHT" 
 * ------------------------------------------------------------------------- */
#include <systemc.h>
/* ------------------------------------------------------------------------- */
class Reset: public sc_core::sc_module {
public:
    explicit Reset(sc_core::sc_module_name name, sc_core::sc_time duration);
    ~Reset();

    virtual void end_of_elaboration();
    sc_out<bool> resetn;

private:    
    SC_HAS_PROCESS(Reset);
    
    // Disable default and copy constructor, assignment operator
    Reset();
    Reset(const Reset &);
    Reset operator= (const Reset &);

    // Reset thread
    void Generate();

    // member variables
    sc_core::sc_time duration_;
};
/* ------------------------------------------------------------------------- */

