#ifndef MPL_NET_ICE_SOCKET_H
#define MPL_NET_ICE_SOCKET_H

#include "net/socket/i_socket_transport.h"
#include "utils/message_sink_impl.h"
#include "ice_controller.h"
#include "ice_candidate.h"
#include "ice_server_params.h"

namespace mpl::net
{

class i_socket_packet;

class ice_socket_basic : private ice_controller::i_listener
{
protected:
    i_timer_manager&                m_timer_manager;
    i_socket_transport::u_ptr_t     m_socket;
    ice_component_id_t              m_component_id;

    message_sink_impl               m_socket_sink;
    ice_controller                  m_ice_controller;

    ice_candidate_t::array_t        m_candidates;
    socket_address_t                m_mapped_address;


    ice_gathering_state_t           m_gathering_state;
    i_timer::u_ptr_t                m_gathering_timer;
    bool                            m_established;

public:
    static constexpr std::uint64_t ice_gathering_id = 0;
    static constexpr std::uint64_t ice_binding_id = 1;

    ice_socket_basic(i_socket_transport::u_ptr_t&& socket
                     , i_timer_manager& timer_manager
                     , ice_component_id_t component_id);

    virtual ~ice_socket_basic();

    bool control(const channel_control_t& control);
    bool start_gathering(const ice_server_params_t::array_t& stun_servers
                         , timestamp_t timeout = timestamp_infinite);
    bool stop_gathering();

    bool is_open() const;
    bool is_established() const;
    channel_state_t channel_state() const;
    ice_gathering_state_t gathering_state() const;

    bool send_transaction(ice_transaction_t&& transaction
                          , bool flush = false);

private:
    ice_candidate_t create_candidate(const socket_address_t& address
                                     , ice_candidate_type_t type = ice_candidate_type_t::host);

    void set_gathering_state(ice_gathering_state_t new_state
                             , const std::string_view& reason = {});
    void set_established(bool established);

    bool on_socket_mesage(const i_message& message);
    void on_gathering_timeout();
    void on_mapped_address(const socket_address_t& address);
    void check_mapped_address(const socket_address_t& address);
    bool add_candidate(ice_candidate_t&& candidate);

    // i_listener interface
private:
    bool on_send_packet(const i_stun_packet &stun_packet) override final;
    std::string on_autorized(const ice_transaction_t &transaction) override final;
    bool on_request(ice_transaction_t &transaction) override final;
    bool on_response(ice_transaction_t &transaction) override final;

protected:

    virtual std::string query_foundation();
    virtual std::string get_password() const;
    virtual void on_socket_state(channel_state_t new_state
                                 , const std::string_view& reason);
    virtual void on_gathering_state(ice_gathering_state_t new_state
                                    , const std::string_view& reason);
    virtual bool on_socket_packet(const i_socket_packet& socket_packet);
    virtual bool on_candidate(const ice_candidate_t& candidate);
    virtual void on_established(bool established);
    virtual bool on_ice_request(ice_transaction_t&& transaction);
    virtual bool on_ice_response(ice_transaction_t&& transaction);

};

}

#endif // MPL_NET_ICE_SOCKET_H
