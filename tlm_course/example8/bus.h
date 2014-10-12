/* ----------------------------------------------------------------------------
 * bus.h
 *
 * A tlm2 bus model
 *
 * Copyright (C) 2012 imec, glasseem@imec.be
 * For copyright and disclaimer notice, see "COPYRIGHT" 
 * ------------------------------------------------------------------------- */

/*! file
 * \brief a tlm2 bus model with multiple initiators and a single target to 
 * connect one initiator to multiple targets
 */

/* ------------------------------------------------------------------------ */
#ifndef BUS_H
#define BUS_H

/* ------------------------------------------------------------------------ */
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>

using namespace std;
using namespace tlm;
using namespace sc_core;

/* ------------------------------------------------------------------------ */
template <unsigned int NR_OF_INITIATORS=1>
class Bus: public sc_module, tlm_fw_transport_if<> {
public:
    explicit Bus(
            sc_module_name name,
            unsigned int bits_per_initiator = 8,
            unsigned int bits_extra=2);
    ~Bus();

    virtual void end_of_elaboration();

    // initiator and target ports of the Bus
    tlm_utils::simple_initiator_socket_tagged<Bus> initiators[NR_OF_INITIATORS];
    tlm_target_socket<> target;

private: 
    SC_HAS_PROCESS(Bus);

    // Disable default and copy constructor, assignment operator
    Bus();
    Bus(const Bus &);
    Bus operator= (const Bus &);

    // tlm fw interface
    virtual tlm_sync_enum nb_transport_fw(
            tlm_generic_payload &trans,
            tlm_phase &phase,
            sc_time &time );

    virtual void b_transport( tlm_generic_payload  &trans, sc_time &time);

    virtual bool get_direct_mem_ptr(
            tlm_generic_payload &trans,
            tlm_dmi &dmi_data);

    virtual unsigned int transport_dbg(tlm::tlm_generic_payload &trans);

    // Callbacks for the initiators
    tlm_sync_enum my_nb_transport_bw(
            int id,
            tlm_generic_payload &trans,
            tlm_phase &phase,
            sc_time &time);

    void my_invalidate_direct_mem_ptr(
            int id,
            sc_dt::uint64 start_range,
            sc_dt::uint64 end_range);

    unsigned int bits_per_initiator_;
    unsigned int mask_addr_;
    unsigned int mask_;
    unsigned int initiator_for_response_;
};

/* ------------------------------------------------------------------------ */
template <unsigned int NR_OF_INITIATORS>
Bus<NR_OF_INITIATORS>::Bus(
        sc_module_name name,
        unsigned int bits_per_initiator,
        unsigned int bits_extra):
        sc_module(name),
        target("target"),
        bits_per_initiator_(bits_per_initiator),
        mask_addr_((1<<bits_per_initiator)-1),
        mask_((1 << (bits_extra + bits_per_initiator))-1)
{
    if ((1 << bits_extra) < NR_OF_INITIATORS) {
        ostringstream msg;
        msg << this->name() << " @ " << sc_time_stamp() 
            << ": initiators outside bus memory range (" << NR_OF_INITIATORS
            << " initiators, only " << (1 << bits_extra) << " addressable)";
        SC_REPORT_WARNING(__FILE__, msg.str().c_str()); 
    }
    // binding the target port
    target.bind(*this);

    // creating the initiator sockets, interfaces and binding them
    for (int i = 0; i < NR_OF_INITIATORS; i++ ) {
        initiators[i].register_nb_transport_bw(
                this,
                &Bus::my_nb_transport_bw,
                i);
        initiators[i].register_invalidate_direct_mem_ptr(
                this,
                &Bus::my_invalidate_direct_mem_ptr,
                i);
    }
}

/* ------------------------------------------------------------------------ */
template <unsigned int NR_OF_INITIATORS>
Bus<NR_OF_INITIATORS>::~Bus()
{
}

/* ------------------------------------------------------------------------ */
template <unsigned int NR_OF_INITIATORS>
void
Bus<NR_OF_INITIATORS>::end_of_elaboration()
{
}

// fw interface implementation
/* ------------------------------------------------------------------------ */
template <unsigned int NR_OF_INITIATORS>
tlm_sync_enum 
Bus<NR_OF_INITIATORS>::nb_transport_fw(
        tlm_generic_payload &trans,
        tlm_phase &phase,
        sc_time &time )
{
    tlm_sync_enum ret;
    if (phase == BEGIN_REQ) {
        unsigned int addr = trans.get_address() & mask_;
        unsigned int length = trans.get_data_length();

        ostringstream msg;
        msg << name() << " @ " << sc_time_stamp() << "+" << time << hex
        << ": non-blocking forward transport, address 0x" << addr 
        << ", length 0x" << length << dec;

        //a transaction should go to a single target
        unsigned int index = (addr >> bits_per_initiator_);
        if (index >= NR_OF_INITIATORS) {
            msg << ": address out of range";
            SC_REPORT_WARNING(__FILE__, msg.str().c_str()); 
            trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
            ret = TLM_COMPLETED;
        } else if (((addr + length-1) >> bits_per_initiator_ ) != index) {
            msg << ": address out of range";
            SC_REPORT_WARNING(__FILE__, msg.str().c_str()); 
            trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
            ret = TLM_COMPLETED;
        } else {
            msg << ": forwarding nb_transport_fw to initiator " << index;
            SC_REPORT_INFO(__FILE__, msg.str().c_str()); 
            trans.set_address(addr & mask_addr_);
            ret= (initiators[index])->nb_transport_fw(trans, phase, time);
            if ((ret == TLM_UPDATED) && (phase == BEGIN_RESP)) {
                initiator_for_response_ = index; 
            }
        }
    } else {
        sc_assert(phase == END_RESP);
        ret= (initiators[initiator_for_response_])->nb_transport_fw(
                trans, 
                phase, 
                time);
    }
    return ret;
}

/* ------------------------------------------------------------------------ */
template <unsigned int NR_OF_INITIATORS>
void
Bus<NR_OF_INITIATORS>::b_transport(tlm_generic_payload &trans, sc_time &time)
{
    unsigned int addr = trans.get_address() & mask_;
    unsigned int length = trans.get_data_length();

    ostringstream msg;
    msg << name() << " @ " << sc_time_stamp() << "+" << time << hex
        << ": blocking transport, address 0x" << addr << ", length 0x"
        << length << dec;

    //a transaction should go to a single target
    unsigned int index = (addr >> bits_per_initiator_);
    if (index >= NR_OF_INITIATORS) {
        msg << ": address out of range";
        SC_REPORT_WARNING(__FILE__, msg.str().c_str()); 
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
    } else if (((addr + length-1) >> bits_per_initiator_ ) != index) {
        msg << ": address out of range";
        SC_REPORT_WARNING(__FILE__, msg.str().c_str()); 
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
    } else {
        msg << ": forwarding b_transport to initiator " << index;
        SC_REPORT_INFO(__FILE__, msg.str().c_str()); 
        trans.set_address(addr & mask_addr_);
        (initiators[index])->b_transport(trans, time);
    }
}

/* ------------------------------------------------------------------------ */
template <unsigned int NR_OF_INITIATORS>
bool
Bus<NR_OF_INITIATORS>::get_direct_mem_ptr(
        tlm_generic_payload &trans, 
        tlm_dmi &dmi_data)
{
    unsigned int addr = trans.get_address() & mask_;
    unsigned int length = trans.get_data_length();
    bool ret;

    //a transaction should go to a single target
    unsigned int index = (addr >> bits_per_initiator_);
    if (((addr + length-1) >> bits_per_initiator_ ) != index) {
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
        ret = false;
    } else {
        trans.set_address(addr & mask_addr_);
        ret = (initiators[index])->get_direct_mem_ptr(trans, dmi_data);
        trans.set_address(addr);
    }
    return ret;
}

/* ------------------------------------------------------------------------ */
template <unsigned int NR_OF_INITIATORS>
unsigned int
Bus<NR_OF_INITIATORS>::transport_dbg(tlm::tlm_generic_payload &trans)
{
    unsigned int addr = trans.get_address() & mask_;
    unsigned int length = trans.get_data_length();
    unsigned int ret = 0;

    //a transaction should go to a single target
    unsigned int index = (addr >> bits_per_initiator_);
    if (((addr + length-1) >> bits_per_initiator_ ) != index) {
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
    } else {
        trans.set_address(addr & mask_addr_);
        ret = (initiators[index])->transport_dbg(trans);
        trans.set_address(addr);
    }
    return ret;
}

/* ------------------------------------------------------------------------ */
template <unsigned int NR_OF_INITIATORS>
tlm_sync_enum
Bus<NR_OF_INITIATORS>::my_nb_transport_bw(
            int id,
            tlm_generic_payload &trans,
            tlm_phase &phase,
            sc_time &time)
{
    ostringstream msg;
    msg << name() << " @ " << sc_time_stamp() << "+" << time 
         << ": forwarding nb_transport_bw to target";
    SC_REPORT_INFO(__FILE__, msg.str().c_str()); 
    tlm_sync_enum ret = target->nb_transport_bw(trans, phase, time);
    if ((ret != TLM_COMPLETED) && (phase == BEGIN_RESP)) {
       initiator_for_response_ = id; 
    }
    return ret;
}

/* ------------------------------------------------------------------------ */
template <unsigned int NR_OF_INITIATORS>
void
Bus<NR_OF_INITIATORS>::my_invalidate_direct_mem_ptr(
            int id,
            sc_dt::uint64 start_range,
            sc_dt::uint64 end_range)
{
}

/* ------------------------------------------------------------------------ */
#endif /* BUS_H */

