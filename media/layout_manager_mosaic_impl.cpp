#include "layout_manager_mosaic_impl.h"

#include "tools/base/sync_base.h"

#include <cmath>
#include <vector>
#include <mutex>

namespace mpl::media
{

class mosaic_layout_impl : public i_layout
{
    using layouts_t = std::vector<relative_frame_rect_t>;
    using array_t = std::vector<mosaic_layout_impl>;
    layouts_t           m_layouts;
public:

    static layouts_t generate_mosaic_layout(std::size_t streams)
    {
        layouts_t layouts;

        if (streams < 2)
        {
            layouts.emplace_back(0.0, 0.0, 1.0, 1.0);
        }
        else if (streams == 2)
        {
            layouts.emplace_back(0.0, 0.25, 0.5, 0.5);
            layouts.emplace_back(0.5, 0.25, 0.5, 0.5);
        }
        else
        {
            auto col_count = static_cast<std::int32_t>(std::sqrt(streams) + 0.999);
            auto last_col_count = streams % col_count;
            auto row_count = static_cast<std::int32_t>(streams) / col_count + static_cast<std::int32_t>(last_col_count != 0);

            double l_width = 1.0 / col_count;
            double l_heigth = 1.0 / row_count;

            for (auto row = 0; row < row_count; row ++)
            {
                auto l_y = static_cast<double>(row) * l_heigth;

                auto x_offset = 0.0;

                if ((row + 1 == row_count) && (last_col_count != 0))
                {
                    x_offset = (static_cast<double>(col_count - last_col_count) * l_width) / 2.0;
                }

                for (auto col = 0; col < col_count && layouts.size() < streams; col++)
                {
                    auto l_x = x_offset + static_cast<double>(col) * l_width;
                    layouts.emplace_back(l_x
                                        , l_y
                                        , l_width
                                        , l_heigth);
                }
            }
        }

        return layouts;
    }

    static mosaic_layout_impl* get_layout(std::size_t streams)
    {
        if (streams <= 1000)
        {
            static mosaic_layout_impl::array_t single_layouts;
            static base::spin_lock safe_mutex;

            if (single_layouts.size() <= streams)
            {
                std::lock_guard lock(safe_mutex);
                while (single_layouts.size() <= (streams + 5))
                {
                    single_layouts.emplace_back(single_layouts.size());
                }
            }

            return &single_layouts[streams];
        }

        return nullptr;
    }

    mosaic_layout_impl(std::size_t streams)
        : m_layouts(generate_mosaic_layout(streams))
    {

    }

    // i_layout interface
public:
    std::size_t size() const override
    {
        return m_layouts.size();
    }

    relative_frame_rect_t get_rect(std::size_t index) const override
    {
        if (index < m_layouts.size())
        {
            return m_layouts[index];
        }

        return {};
    }
};

layout_manager_mosaic_impl &layout_manager_mosaic_impl::get_instance()
{
    static layout_manager_mosaic_impl single_instance;
    return single_instance;
}

layout_manager_mosaic_impl::layout_manager_mosaic_impl()
{

}

const i_layout *layout_manager_mosaic_impl::query_layout(std::size_t streams)
{
    return mosaic_layout_impl::get_layout(streams);
}

}
