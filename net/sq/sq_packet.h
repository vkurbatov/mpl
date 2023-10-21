#ifndef MPL_NET_SEQ_PACKET_H
#define MPL_NET_SEQ_PACKET_H

#include "sq_types.h"
#include "utils/smart_buffer.h"

namespace mpl::net
{

struct sq_mapped_packet_header_t;

class sq_packet
{
    smart_buffer  m_fragment_data;
public:

    using array_t = std::vector<sq_packet>;

    sq_packet(const smart_buffer& fragment_data = {});
    sq_packet(smart_buffer&& fragment_data);

    void set_fragment_data(const smart_buffer& fragment_data);
    void set_fragment_data(smart_buffer&& fragment_data);

    smart_buffer fetch_fragment_data();

    bool is_valid() const;

    sq_packet& detach_refs();
    void clear();

    const void* payload_data() const;
    std::size_t payload_size() const;
    std::uint16_t id() const;
    bool is_first() const;
    bool is_last() const;
    bool is_full() const;


private:
    const sq_mapped_packet_header_t& header() const;
};

}

#endif // MPL_NET_SEQ_PACKET_H
