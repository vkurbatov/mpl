#include "seq_packet_builder.h"
#include "mapped_seq_header.h"

#include <cstring>

namespace mpl::seq
{

seq_packet_builder_t::seq_packet_builder_t(uint8_t session_id
                                           , uint16_t packet_id
                                           , uint32_t max_fragment_size
                                           , std::size_t max_nack_group_size)
    : session_id(session_id)
    , packet_id(packet_id)
    , max_fragment_size(max_fragment_size)
    , max_nack_group_size(max_nack_group_size)
{

}

smart_buffer::array_t seq_packet_builder_t::build_fragments(const void *data
                                                                          , std::size_t size)
{
    smart_buffer::array_t packet_list;

    auto payload_ptr = static_cast<const std::uint8_t*>(data);
    while (size > 0)
    {
        auto part_size = size;
        if (max_fragment_size > 0
                && part_size > max_fragment_size)
        {
            part_size = max_fragment_size;
        }

        smart_buffer fragment(nullptr, sizeof(mapped_packet_header_t) + part_size, true);
        mapped_packet_header_t& header = *static_cast<mapped_packet_header_t*>(fragment.map());
        auto payload = static_cast<std::uint8_t*>(fragment.map()) + sizeof(mapped_packet_header_t);

        header.session_id = session_id;
        header.id = packet_id;
        header.set_packet_type(packet_type_t::fragment);
        header.head = static_cast<std::uint8_t>(payload_ptr == data);
        header.tail =  static_cast<std::uint8_t>(part_size == size);
        header.tune();
        header.length = part_size;

        std::memcpy(payload
                    , payload_ptr
                    , part_size);

        packet_list.emplace_back(std::move(fragment));


        packet_id++;
        payload_ptr += part_size;
        size -= part_size;
    }

    return packet_list;

}

smart_buffer::array_t seq_packet_builder_t::build_nack_request(const std::set<uint16_t> &nack_ids)
{
    smart_buffer::array_t packet_list;

    if (!nack_ids.empty())
    {
        std::vector<std::vector<std::uint16_t>> groups_ids;

        for (const auto& id: nack_ids)
        {
            if (groups_ids.empty()
                    || groups_ids.back().empty()
                    || (id - groups_ids.back().front()) > (max_nack_group_size * 8))
            {
                groups_ids.push_back({id});
            }
            else
            {
                groups_ids.back().push_back(id);
            }
        }

        for (const auto& group : groups_ids)
        {
            auto ids_dist = group.back() - group.front();
            auto nack_record_size = 2 + (ids_dist + 7) / 8;

            smart_buffer request(nullptr, sizeof(mapped_packet_header_t) + nack_record_size);
            auto it = group.begin();
            mapped_packet_header_t& header = *static_cast<mapped_packet_header_t*>(request.map());
            mapped_nack_record_t& nack_record = *reinterpret_cast<mapped_nack_record_t*>(static_cast<std::uint8_t*>(request.map()) + sizeof(mapped_packet_header_t));

            header.session_id = session_id;
            header.id = packet_id;
            header.set_packet_type(packet_type_t::request_nack);
            header.head = 1;
            header.tail = 1;
            header.tune();
            header.length = nack_record_size;

            nack_record.id = *it;
            it = std::next(it);

            while (it != group.end())
            {
                auto i = (*it - nack_record.id) / 8;
                auto bit = (*it - nack_record.id) % 8;
                nack_record.ids[i] |= 1 << bit;
                it = std::next(it);
            }

            packet_list.emplace_back(request);

        }
    }

    return packet_list;
}

}
