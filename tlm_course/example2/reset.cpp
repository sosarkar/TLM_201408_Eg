 /* ----------------------------------------------------------------------------
  * reset.cpp
  *
  * A reset generator
  *
  * Copyright (C) 2012 imec, glasseem@imec.be
  * For copyright and disclaimer notice, see "COPYRIGHT" 
  * ------------------------------------------------------------------------- */
#include "reset.h"
using namespace sc_core;

/* ------------------------------------------------------------------------- */
Reset::Reset(sc_module_name name, sc_time duration):
        sc_module(name),
        resetn("resetn"),
        duration_(duration)
{
}

/* ------------------------------------------------------------------------- */
Reset::~Reset()
{
}

/* ------------------------------------------------------------------------- */
void 
Reset::end_of_elaboration()
{
    SC_THREAD(Generate);
}

/* ------------------------------------------------------------------------- */
void 
Reset::Generate()
{
    resetn.write(false);
    wait(duration_);
    resetn.write(true);
}

/* ------------------------------------------------------------------------- */

