#include "ice_controller.h"

#include "net/stun/stun_packet_impl.h"
#include "net/stun/stun_message.h"
#include "net/stun/stun_utils.h"

#include "utils/common_utils.h"
#include "utils/hash_utils.h"

#include <shared_mutex>

namespace mpl::net
{

using lock_t = std::lock_guard<pt::utils::shared_spin_lock>;
using shared_lock_t = std::lock_guard<pt::utils::shared_spin_lock>;

std::size_t ice_controller::transaction_hasher_t::operator()(const ice_transaction_id_t &request_id) const
{
    return utils::calc_hash(request_id.data()
                            , request_id.size());
}

ice_controller::transaction_t::transaction_t(ice_controller &owner
                                             , ice_transaction_id_t id
                                             , ice_transaction_t &&ice_transaction)
    : ice_transaction_t(std::move(ice_transaction))
    , owner(owner)
    , id(id)
    , timeout_timer(owner.create_timer(*this))
{

}

ice_controller::transaction_t::~transaction_t()
{
    cancel();
    owner.release_timer(std::move(timeout_timer));
}

bool ice_controller::transaction_t::start()
{
    if (timeout != timestamp_infinite)
    {
        timeout_timer->start(timeout);
    }

    return true;
}

bool ice_controller::transaction_t::cancel()
{
    if (timeout_timer->processed())
    {
        return timeout_timer->stop();
    }

    return false;
}


std::size_t ice_controller::read_attributes(const i_stun_packet &stun_packet
                                            , ice_transaction_t &transaction)
{
    std::size_t result = 0;

    if (stun_packet.address().is_valid())
    {
        transaction.address = stun_packet.address();
    }

    switch(stun_packet.stun_class())
    {
        case stun_message_class_t::request:
        case stun_message_class_t::indication:

        break;
        case stun_message_class_t::success_response:
            transaction.response.result = ice_transaction_t::ice_result_t::success;
        break;
        case stun_message_class_t::error_response:
            transaction.response.result = ice_transaction_t::ice_result_t::failed;
        break;
        default:;
    }

    for (const auto& a : stun_packet.attributes())
    {
        result++;
        switch(a->attribute_id)
        {
            case stun_attribute_id_t::mapped_address:
                transaction.response.mapped_address = a->cast<stun_attribute_mapped_address_t>().endpoint;
                transaction.response.xor_address = false;
            break;
            case stun_attribute_id_t::xor_mapped_address:
                transaction.response.mapped_address = a->cast<stun_attribute_xor_mapped_address_t>().endpoint;
                transaction.response.xor_address = true;
            break;
            case stun_attribute_id_t::username:
                transaction.request.username = a->cast<stun_attribute_username_t>().username;
            break;
            case stun_attribute_id_t::ice_controlling:
                transaction.request.controlling = true;
                transaction.request.tie_breaker = a->cast<stun_attribute_ice_controlling_t>().tie_breaker;
            break;
            case stun_attribute_id_t::ice_controlled:
                transaction.request.controlling = false;
                transaction.request.tie_breaker = a->cast<stun_attribute_ice_controlled_t>().tie_breaker;
            break;
            case stun_attribute_id_t::use_candidate:
                transaction.request.use_candidate = true;
            break;
            case stun_attribute_id_t::priority:
                transaction.request.priority = a->cast<stun_attribute_priority_t>().priority;
            break;
            case stun_attribute_id_t::error_code:
                transaction.response.error_code = a->cast<stun_attribute_error_code_t>().error_code;
            break;
            default:
                result--;
        }
    }

    return result;
}

ice_controller::ice_controller(i_listener &listener
                               , i_timer_manager &timer_manager)
    : m_listener(listener)
    , m_timer_manager(timer_manager)
{

}

ice_controller::~ice_controller()
{
    reset();
}

bool ice_controller::send_request(const ice_transaction_t &transaction)
{
    return send_request(ice_transaction_t(transaction));
}

bool ice_controller::send_request(ice_transaction_t &&transaction)
{
    auto id = stun_message_t::generate_transaction_id();
    {
        lock_t lock(m_safe_mutex);
        auto it = m_transactions.emplace(std::piecewise_construct
                                        , std::forward_as_tuple(id)
                                        , std::forward_as_tuple(*this
                                                                , id
                                                                , std::move(transaction)));

        if (it.second)
        {
            transaction_t& transaction = it.first->second;
            if (send_transaction(transaction))
            {
                return true;
            }

        }
    }

    complete_transaction(id
                         , ice_transaction_t::ice_result_t::failed);

    return false;
}

bool ice_controller::push_packet(const i_stun_packet &stun_packet)
{
    switch(stun_packet.stun_method())
    {
        case stun_method_t::binding:
        {
            switch(stun_packet.stun_class())
            {
                case stun_message_class_t::request:
                {
                    ice_transaction_t transaction;
                    read_attributes(stun_packet
                                    , transaction);
                    transaction.request.password = m_listener.on_autorized(transaction);
                    if (utils::stun_verify_auth(transaction.request.password
                                                , stun_packet) == stun_authentification_result_t::ok)
                    // if (stun_packet.check_authentification(transaction.request.password) == stun_authentification_result_t::ok)
                    {
                        transaction.response.mapped_address = transaction.address;
                        if (m_listener.on_request(transaction))
                        {

                            stun_message_t stun_message(transaction.response.is_success()
                                                              ? stun_message_class_t::success_response
                                                              : stun_message_class_t::error_response
                                                              , stun_packet.stun_method()
                                                              , stun_packet.transaction_id());

                            if (stun_message.message_class == stun_message_class_t::error_response)
                            {
                                stun_message.attributes.push_back(stun_attribute_error_code_t::create(transaction.response.error_code));
                            }
                            else
                            {
                                if (transaction.response.xor_address)
                                {
                                    stun_message.attributes.push_back(stun_attribute_xor_mapped_address_t::create(transaction.response.mapped_address));
                                }
                                else
                                {
                                    stun_message.attributes.push_back(stun_attribute_mapped_address_t::create(transaction.response.mapped_address));
                                }
                            }

                            stun_packet_impl response_packet(stun_message.build_packet(transaction.request.password)
                                                             , stun_packet.address());


                            return m_listener.on_send_packet(response_packet);
                        }

                    }
                }
                break;
                case stun_message_class_t::success_response:
                case stun_message_class_t::error_response:
                    return complete_transaction(stun_packet);
                break;
                default:;
            }
        }
        default:;
    }

    return false;
}

void ice_controller::reset()
{
    lock_t lock(m_safe_mutex);
    m_transactions.clear();
}

void ice_controller::reset(uint64_t tag)
{
    lock_t lock(m_safe_mutex);
    for (auto it = m_transactions.begin(); it != m_transactions.end();)
    {
        if (it->second.tag == tag)
        {
            it = m_transactions.erase(it);
        }
        else
        {
            it = std::next(it);
        }
    }

}

std::size_t ice_controller::pending_count() const
{
    shared_lock_t lock(m_safe_mutex);
    return m_transactions.size();
}

std::size_t ice_controller::pending_count(uint64_t tag) const
{
    std::size_t result = 0;
    shared_lock_t lock(m_safe_mutex);

    for (const auto& t : m_transactions)
    {
        if (t.second.tag == tag)
        {
            result++;
        }
    }

    return result;
}

i_timer::u_ptr_t ice_controller::create_timer(transaction_t &transaction)
{
    auto timer_handler = [&]{ on_timeout(transaction.id); };

    if (!m_timers.empty())
    {
        auto timer = std::move(m_timers.front());
        m_timers.pop();
        timer->set_handler(timer_handler);
        return timer;
    }

    return m_timer_manager.create_timer(timer_handler);
}

void ice_controller::release_timer(i_timer::u_ptr_t &&timer)
{
    if (timer)
    {
        timer->set_handler(nullptr);
        m_timers.push(std::move(timer));
    }
}

bool ice_controller::complete_transaction(ice_transaction_id_t id
                                          , ice_transaction_t::ice_result_t result)
{
    ice_transaction_t transaction;
    {
        lock_t lock(m_safe_mutex);

        if (auto it = m_transactions.find(id); it != m_transactions.end())
        {
            it->second.response.result = result;
            transaction = it->second;
            m_transactions.erase(it);
        }
        else
        {
            return false;
        }
    }

    return m_listener.on_response(transaction);
}

bool ice_controller::complete_transaction(const i_stun_packet &response_packet)
{
    ice_transaction_t transaction;
    if (pop_transaction(response_packet
                        , transaction))
    {
        m_listener.on_response(transaction);
        return true;
    }

    return false;
}

bool ice_controller::pop_transaction(const i_stun_packet &response_packet
                                     , ice_transaction_t &ice_transaction)
{
    lock_t lock(m_safe_mutex);

    auto it = m_transactions.find(response_packet.transaction_id());
    if (it != m_transactions.end())
    {
        transaction_t& transaction = it->second;
        if (response_packet.stun_class() == stun_message_class_t::error_response
                || utils::stun_verify_auth(transaction.request.password
                                           , response_packet)
                    == stun_authentification_result_t::ok)
        {
            ice_transaction = std::move(transaction);

            read_attributes(response_packet
                            , ice_transaction);

            m_transactions.erase(it);
            return true;
        }
    }

    return false;
}

bool ice_controller::send_transaction(transaction_t &transaction)
{
    stun_message_t message(stun_message_class_t::request
                                 , stun_method_t::binding);

    if (!transaction.request.username.empty())
    {
        message.attributes.push_back(stun_attribute_username_t::create(transaction.request.username));
    }

    if (transaction.request.tie_breaker != 0)
    {
        if (transaction.request.controlling)
        {
            message.attributes.push_back(stun_attribute_ice_controlling_t::create(transaction.request.tie_breaker));
            if (transaction.request.use_candidate)
            {
                message.attributes.push_back(stun_attribute_use_candidate_t::create());
            }
        }
        else
        {
            message.attributes.push_back(stun_attribute_ice_controlled_t::create(transaction.request.tie_breaker));
        }

        if (transaction.request.priority  != 0)
        {
            message.attributes.push_back(stun_attribute_priority_t::create(transaction.request.priority));
        }
    }

    message.transaction_id = transaction.id;
    stun_packet_impl stun_packet(message.build_packet(transaction.request.password)
                                 , transaction.address);

    if (m_listener.on_send_packet(stun_packet))
    {
        transaction.start();
        return true;
    }

    return false;
}

void ice_controller::on_timeout(ice_transaction_id_t id)
{
    ice_transaction_t ice_transaction;

    lock_t lock(m_safe_mutex);
    if (auto it = m_transactions.find(id)
            ; it != m_transactions.end())
    {
        auto result = ice_transaction_t::ice_result_t::timeout;
        transaction_t& transaction = it->second;
        if (transaction.retries > 0)
        {
            transaction.retries--;
            if (send_transaction(transaction))
            {
                return;
            }

            result = ice_transaction_t::ice_result_t::failed;
        }

        transaction.response.result = result;
        ice_transaction = transaction;
        m_transactions.erase(it);
    }
    else
    {
        return;
    }

    m_listener.on_response(ice_transaction);
    /*
    auto result = ice_transaction_t::ice_result_t::timeout;
    {
        if (transaction.retries > 0)
        {
            lock_t lock(m_safe_mutex);
            transaction.retries--;
            if (send_transaction(transaction))
            {
                return;
            }

            result = ice_transaction_t::ice_result_t::failed;
        }
    }

    complete_transaction(transaction.id
                         , result);*/

}

}
