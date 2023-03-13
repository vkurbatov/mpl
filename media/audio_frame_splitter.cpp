#include "audio_frame_splitter.h"
#include "audio_format_helper.h"

namespace mpl::media
{

audio_frame_splitter::audio_frame_splitter()
    : m_duration(0)
{

}

audio_frame_splitter::audio_frame_splitter(const i_audio_format &format
                                           , uint32_t duration)
    : m_format(format)
    , m_duration(duration)
{
    reset();
}

void audio_frame_splitter::setup(const i_audio_format &format, uint32_t duration)
{
    m_format.assign(format);
    m_duration = duration;
    reset();
}

void audio_frame_splitter::reset()
{
    m_splitters.clear();

    if (m_format.is_convertable())
    {
        audio_format_helper audio_info(m_format);
        auto buffer_size = audio_info.sample_size() * m_duration;
        if (audio_info.is_planar())
        {
            buffer_size /= m_format.channels();
            for (auto c = 0; c < m_format.channels(); c++)
            {
                m_splitters.emplace_back(buffer_size);
            }
        }
        else
        {
            m_splitters.emplace_back(buffer_size);
        }
    }
}

const i_audio_format &audio_frame_splitter::format() const
{
    return m_format;
}

uint32_t audio_frame_splitter::duration() const
{
    return m_duration;
}

std::size_t audio_frame_splitter::buffered_samples() const
{
    if (!m_splitters.empty())
    {
        audio_format_helper audio_info(m_format);
        auto samples = (m_splitters.back().buffered_size() * m_splitters.size()) / audio_info.sample_size();
        return samples;
    }
    return 0;
}

data_fragment_queue_t audio_frame_splitter::push_frame(const void *data
                                                       , std::size_t size)
{
    data_fragment_queue_t result;
    if (!m_splitters.empty())
    {
        if (m_splitters.size() == 1)
        {
            return m_splitters.front().push_stream(data
                                                   , size);
        }
        std::vector<data_fragment_t> fragments;
        auto data_ptr = static_cast<const std::uint8_t*>(data);
        auto part_size = size / m_splitters.size();
        for (auto& s : m_splitters)
        {
            auto queue = s.push_stream(data_ptr
                                       , part_size);

            if (fragments.empty())
            {
                while(!queue.empty())
                {
                    fragments.emplace_back(std::move(queue.front()));
                    queue.pop();
                }
            }
            else
            {
                auto idx = 0;
                while(!queue.empty())
                {
                    fragments[idx].insert(fragments[idx].end()
                                          , queue.front().begin()
                                          , queue.front().end());
                    queue.pop();
                    idx++;
                }
            }

            data_ptr += part_size;
        }

        for (auto& f : fragments)
        {
            result.push(std::move(f));
        }
    }

    return result;
}

}
