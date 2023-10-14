#include "ice_transport_factory.h"
#include "i_ice_transport.h"
#include "utils/property_utils.h"

#include "ice_transport_params.h"

#include "tools/base/sync_base.h"
#include <shared_mutex>

namespace mpl::net
{

namespace detail
{


ice_transport_params_t create_ice_params(const i_property& ice_property)
{
    ice_transport_params_t ice_params;

    utils::property::deserialize(ice_params
                                 , ice_property);

    return ice_params;
}

}

class ice_transport_impl : public i_ice_transport
{
    using mutex_t = std::shared_mutex;
    using lock_t = std::lock_guard<mutex_t>;
    using shared_lock_t = std::shared_lock<mutex_t>;

    using u_ptr_t = std::unique_ptr<ice_transport_impl>;

    ice_config_t                m_ice_config;
    i_timer_manager&            m_timer_manager;
    ice_transport_params_t      m_ice_params;

    bool                        m_open;
    channel_state_t             m_state;
    ice_gathering_state_t       m_gathering_state;

    u_ptr_t create(const ice_config_t& ice_config
                   , i_timer_manager& timer_manager
                   , const i_property& params)
    {
        return nullptr;
    }

    ice_transport_impl(const ice_config_t& ice_config
                       , i_timer_manager& timer_manager
                       , const i_property& params)
        : m_ice_config(ice_config)
        , m_timer_manager(timer_manager)
        , m_ice_params(detail::create_ice_params(params))
        , m_open(false)
        , m_state(channel_state_t::ready)
        , m_gathering_state(ice_gathering_state_t::ready)
    {

    }

    // i_channel interface
public:
    bool control(const channel_control_t &control) override
    {
        return false;
    }

    bool is_open() const override
    {
        return m_open;
    }

    channel_state_t state() const override
    {
        return m_state;
    }

    // i_parametrizable interface
public:
    bool set_params(const i_property &params) override
    {
        auto ice_params = m_ice_params;
        if (utils::property::deserialize(ice_params
                                          , params))
        {
            ice_params.local_endpoint = m_ice_params.local_endpoint;
            return true;

        }

        return false;
    }

    bool get_params(i_property &params) const override
    {
        return utils::property::serialize(m_ice_params
                                          , params);
    }

    // i_message_channel interface
public:
    i_message_sink *sink(std::size_t index) override
    {
        if (index == 0)
        {
            return nullptr;
        }

        return nullptr;
    }

    i_message_source *source(std::size_t index) override
    {
        if (index == 0)
        {
            return nullptr;
        }

        return nullptr;
    }

    // i_transport_channel interface
public:
    transport_id_t transport_id() const override
    {
        return transport_id_t::ice;
    }

    // i_ice_transport interface
public:
    ice_component_id_t component_id() const override
    {
        return m_ice_params.component_id;
    }

    ice_mode_t mode() const override
    {
        return m_ice_params.mode;
    }

    ice_gathering_state_t gathering_state() const override
    {
        return m_gathering_state;
    }

    ice_endpoint_t local_endpoint() const override
    {
        return m_ice_params.local_endpoint;
    }

    ice_endpoint_t remote_endpoint() const override
    {
        return m_ice_params.remote_endpoint;
    }

    bool add_remote_candidate(const ice_candidate_t &candidate) override
    {
        return false;
    }
};

ice_transport_factory::u_ptr_t ice_transport_factory::create(const ice_config_t& ice_config
                                                             , i_timer_manager &timer_manager)
{
    return std::make_unique<ice_transport_factory>(ice_config
                                                   , timer_manager);
}

ice_transport_factory::ice_transport_factory(const ice_config_t& ice_config
                                             , i_timer_manager &timer_manager)
    : m_ice_config(ice_config)
    , m_timer_manager(timer_manager)
{

}

i_transport_channel::u_ptr_t ice_transport_factory::create_transport(const i_property &params)
{
    return nullptr;
}


}
