#ifndef MPL_SEQ_PACKET_H
#define MPL_SEQ_PACKET_H

#include "seq_types.h"
#include "core/smart_buffer.h"

namespace mpl::seq
{

struct mapped_packet_header_t;

class seq_packet
{
    smart_buffer  m_fragment_data;
public:

    using array_t = std::vector<seq_packet>;

    seq_packet(const smart_buffer& fragment_data = {});
    seq_packet(smart_buffer&& fragment_data);

    void set_fragment_data(const smart_buffer& fragment_data);
    void set_fragment_data(smart_buffer&& fragment_data);

    smart_buffer fetch_fragment_data();

    bool is_valid() const;

    void detach_refs();

    const void* payload_data() const;
    std::size_t payload_size() const;
    std::uint16_t id() const;
    bool is_first() const;
    bool is_last() const;


private:
    const mapped_packet_header_t& header() const;
};

}

#endif // MPL_SEQ_PACKET_H
