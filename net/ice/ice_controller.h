#ifndef MPL_NET_ICE_CONTROLLER_H
#define MPL_NET_ICE_CONTROLLER_H

#include "core/i_timer_manager.h"
#include "net/stun/i_stun_packet.h"
#include "ice_transaction.h"
#include "tools/utils/sync_base.h"

#include <queue>

namespace mpl::net
{

using mutex_t = pt::utils::shared_spin_lock;

class ice_controller
{
    using ice_transaction_id_t = stun_transaction_id_t;
    using timer_queue_t = std::queue<i_timer::u_ptr_t>;
    struct transaction_hasher_t
    {
        std::size_t operator()(const ice_transaction_id_t& request_id) const;
    };

    struct transaction_t : public ice_transaction_t
    {
        using map_t = std::unordered_map<ice_transaction_id_t
                                        , transaction_t
                                        , transaction_hasher_t>;

        ice_controller&             owner;
        ice_transaction_id_t        id;

        i_timer::u_ptr_t            timeout_timer;

        transaction_t(ice_controller& owner
                      , ice_transaction_id_t id
                      , ice_transaction_t&& ice_transaction);
        ~transaction_t();

        bool start();
        bool cancel();

    };
public:
    class i_listener
    {
    public:
        virtual ~i_listener() = default;
        virtual bool on_send_packet(const i_stun_packet& stun_packet) = 0;
        virtual std::string on_autorized(const ice_transaction_t& transaction) = 0;
        virtual bool on_request(ice_transaction_t& transaction) = 0;
        virtual bool on_response(ice_transaction_t& transaction) = 0;
    };
private:

    mutable mutex_t         m_safe_mutex;
    i_listener&             m_listener;
    i_timer_manager&        m_timer_manager;
    timer_queue_t           m_timers;

    transaction_t::map_t    m_transactions;

public:

    static std::size_t read_attributes(const i_stun_packet& stun_packet
                                       , ice_transaction_t& transaction);

    ice_controller(i_listener& listener
                   , i_timer_manager& timer_manager);

    ~ice_controller();

    bool send_request(const ice_transaction_t& transaction);
    bool send_request(ice_transaction_t&& transaction);

    bool push_packet(const i_stun_packet& stun_packet);

    void reset();
    void reset(std::uint64_t tag);
    std::size_t pending_count() const;
    std::size_t pending_count(std::uint64_t tag) const;

private:
    i_timer::u_ptr_t create_timer(transaction_t& transaction);
    void release_timer(i_timer::u_ptr_t&& timer);
    bool complete_transaction(ice_transaction_id_t id
                              , ice_transaction_t::ice_result_t result);
    bool complete_transaction(const i_stun_packet& response_packet);
    bool pop_transaction(const i_stun_packet& response_packet
                         , ice_transaction_t& ice_transaction);
    bool send_transaction(transaction_t& transaction);
    bool request(ice_transaction_id_t id);
    void on_timeout(ice_transaction_id_t id);
};

}

#endif // MPL_NET_ICE_CONTROLLER_H
