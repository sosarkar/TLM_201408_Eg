/* ----------------------------------------------------------------------------
 * settings.h
 *
 * Settings for master-slave system with bus
 *
 * Copyright (C) 2013 imec, glasseem@imec.be
 * For copyright and disclaimer notice, see "COPYRIGHT"
 * ------------------------------------------------------------------------- */
#ifndef SETTINGS_H
#define SETTINGS_H

// bus width in bits, should be multiple of 32
static const unsigned int BUS_WIDTH = 64;

// burst size in bus words (of size BUS_WIDTH)
static const unsigned int BURST_WIDTH = 8;

static const unsigned int NR_SLAVES = 3;

// Bus clock frequency in MHz.  Masters uses this clock too
static const unsigned int BUS_CLOCK_FREQUENCY = 200;

// umount of bus cycles delay for read and write
static const unsigned int SLAVE_READ_DELAY = 3;
static const unsigned int SLAVE_WRITE_DELAY = 1;

// Slave clock frequency in MHz
static const unsigned int SLAVE_CLOCK_FREQUENCY = 100;

// number of times a sum needs to be calculated 
static const unsigned int NR_SUMS = 10;

// number of terms per sum
static const unsigned int NR_TERMS = 100;

//registers within a slave, should be aligned with bus width
static const unsigned int REG_START = 0x0;
static const unsigned int REG_RESULT = 0x80;
static const unsigned int REG_OFFSET_TERMS = 0x100;
//alignment with 7 address LSB's 0, allows bus_width up to 2^7 = 128 bytes or 
//1024 bits

//width n of the address per slave, the range in bytes (2^n) should be greater 
//than the range required for all the terms + the offset 
//(NR_TERMS*4)+REG_OFFSET_TERMS
static const unsigned int ADDRES_BITS_PER_SLAVE = 12;
// allows up to (2^12-0x100)/4 = 960 terms

//The number of address bits m to address the different slaves
//The maximum number of slaves is limited to 2^m
static const unsigned int ADDRESS_BITS_EXTRA = 4;//allows up to 2^4 = 16 slaves
#endif /* SETTINGS_H */

