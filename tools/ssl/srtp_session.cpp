#include "srtp_session.h"
#include "srtp_session_config.h"
#include "srtp_packet.h"
#include <cstring>


// #include "toolset/portable/sync_base.h"
#include <srtp2/srtp.h>
#include <mutex>
#include <iostream>

namespace ssl
{

static constexpr size_t crypto_buffer_size = 1500 + SRTP_MAX_TRAILER_LEN;
using crypto_buffer_t = std::array<uint8_t, crypto_buffer_size>;

static bool srtp_init_flag = false;


using srtp_ctx_ptr_t = std::unique_ptr<srtp_ctx_t, decltype(&srtp_dealloc)>;

namespace detail
{
    srtp_ctx_ptr_t create_srtp_session(const srtp_session_config_t& config)
    {
        srtp_policy_t   policy = {};

        switch(config.key_info.profile_id)
        {
            case srtp_profile_id_t::aes_cm_128_hmac_sha1_80:
                srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtp);
                srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp);
            break;
            case srtp_profile_id_t::aes_cm_128_hmac_sha1_32:
                srtp_crypto_policy_set_aes_cm_128_hmac_sha1_32(&policy.rtp);
                // NOTE: Must be 80 for RTCP.
                srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&policy.rtcp);
            break;
            case srtp_profile_id_t::aead_aes_256_gcm:
                srtp_crypto_policy_set_aes_gcm_256_16_auth(&policy.rtp);
                srtp_crypto_policy_set_aes_gcm_256_16_auth(&policy.rtcp);
            break;
            case srtp_profile_id_t::aead_aes_128_gcm:
                srtp_crypto_policy_set_aes_gcm_128_16_auth(&policy.rtp);
                srtp_crypto_policy_set_aes_gcm_128_16_auth(&policy.rtcp);
            break;
        }

        if (config.key_info.key.size() == policy.rtp.cipher_key_len)
        {
            switch(config.direction)
            {
                case srtp_direction_t::inbound:
                    policy.ssrc.type = ssrc_any_inbound;
                break;
                case srtp_direction_t::outbound:
                    policy.ssrc.type = ssrc_any_outbound;
                break;
            }

            policy.ssrc.value = 0;
            policy.key        = const_cast<std::uint8_t*>(config.key_info.key.data());
            // Required for sending RTP retransmission without RTX.
            policy.allow_repeat_tx = 1;
            policy.window_size     = 1024;
            policy.next            = nullptr;

            srtp_t srtp_ctx = nullptr;

            srtp_err_status_t error = srtp_create(&srtp_ctx, &policy);
            if (error == srtp_err_status_ok
                    && srtp_ctx != nullptr)
            {
                return srtp_ctx_ptr_t(srtp_ctx
                                      , srtp_dealloc);
            }

        }

        return srtp_ctx_ptr_t(nullptr
                              , nullptr);
    }
}

struct srtp_session::context_t
{
    using mutex_t = std::mutex;
    using lock_t = std::lock_guard<mutex_t>;

    using s_ptr_t = srtp_session::context_ptr_t;

    mutable mutex_t             m_safe_mutex;

    srtp_session_config_t       m_config;
    srtp_packet_handler_t       m_packet_handler;

    srtp_ctx_ptr_t              m_session;
    crypto_buffer_t             m_crypto_buffer;

    static s_ptr_t create(const srtp_session_config_t& config
                            , srtp_packet_handler_t packet_handler)
    {

        if (auto session = detail::create_srtp_session(config))
        {
            return std::make_shared<context_t>(std::move(session)
                                               , config
                                               , std::move(packet_handler));
        }

        return nullptr;
    }

    context_t(srtp_ctx_ptr_t&& session
              , const srtp_session_config_t& config
              , srtp_packet_handler_t packet_handler)
        : m_config(config)
        , m_packet_handler(std::move(packet_handler))
        , m_session(std::move(session))
    {

    }

    ~context_t()
    {

    }

    const srtp_session_config_t& config() const
    {
        return m_config;
    }


    inline void on_packet(const srtp_packet_t& packet)
    {
        if (m_packet_handler != nullptr)
        {
            m_packet_handler(packet);
        }
    }

    inline void set_packet_handler(srtp_packet_handler_t packet_handler)
    {
        m_packet_handler = std::move(packet_handler);
    }


    bool push_packet(const srtp_packet_t& packet)
    {
        if (packet.is_valid())
        {
            if ((packet.size + SRTP_MAX_TRAILER_LEN) <= crypto_buffer_size)
            {
                if (auto buffer = m_crypto_buffer.data())
                {
                    lock_t lock(m_safe_mutex);

                    std::memcpy(buffer
                                , packet.data
                                , packet.size);

                    std::int32_t len = packet.size;
                    srtp_err_status_t err;
                    srtp_packet_t output_packet;

                    switch (packet.type)
                    {
                        case srtp_packet_type_t::rtp:
                            output_packet.type = srtp_packet_type_t::srtp;
                            err = srtp_protect(m_session.get(), buffer, &len);
                        break;
                        case srtp_packet_type_t::rtcp:

                            output_packet.type = srtp_packet_type_t::srtcp;
                            err = srtp_protect_rtcp(m_session.get(), buffer, &len);
                        break;
                        case srtp_packet_type_t::srtp:
                            output_packet.type = srtp_packet_type_t::rtp;
                            err = srtp_unprotect(m_session.get(), buffer, &len);
                        break;
                        case srtp_packet_type_t::srtcp:
                            output_packet.type = srtp_packet_type_t::rtcp;
                            err = srtp_unprotect_rtcp(m_session.get(), buffer, &len);

                        break;
                    }

                    if (err == srtp_err_status_ok)
                    {
                        output_packet.data = buffer;
                        output_packet.size = len;
                        on_packet(output_packet);
                        return true;
                    }
                }
            }
        }

        return false;
    }

    inline bool is_valid() const
    {
        return m_session != nullptr;
    }
};

bool srtp_session::init()
{
    if (!srtp_init_flag)
    {
        srtp_init_flag = srtp_init() == srtp_err_status_ok;
        return true;
    }

    return false;
}

bool srtp_session::cleanup()
{
    if (srtp_init_flag)
    {
        if (srtp_shutdown() == srtp_err_status_ok)
        {
            srtp_init_flag = false;
            return true;
        }
    }

    return false;
}

bool srtp_session::is_init()
{
    return srtp_init_flag;
}

srtp_session::s_ptr_t srtp_session::create(const srtp_session_config_t &config
                                             , srtp_packet_handler_t packet_handler)
{
    return std::make_shared<srtp_session>(config
                                          , std::move(packet_handler));
}

srtp_session::srtp_session(const srtp_session_config_t &config
                           , srtp_packet_handler_t packet_handler)
    : m_context(context_t::create(config
                                  , std::move(packet_handler)))
{

}

const srtp_session_config_t &srtp_session::config() const
{
    return m_context->config();
}

void srtp_session::set_packet_handler(srtp_packet_handler_t packet_handler)
{
    m_context->set_packet_handler(std::move(packet_handler));
}

void srtp_session::push_packet(const srtp_packet_t &packet)
{
    m_context->push_packet(packet);
}

bool srtp_session::is_valid() const
{
    return m_context->is_valid();
}

}
