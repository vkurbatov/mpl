#ifndef MPL_OPTION_IMPL_H
#define MPL_OPTION_IMPL_H

#include "i_option.h"
#include <unordered_map>

namespace mpl
{

class option_impl : public i_option
{
public:
    using option_map_t = std::unordered_map<i_option::option_id_t
                                            , i_option_value::u_ptr_t>;
private:
    option_map_t    m_options;
public:
    using u_ptr_t = std::unique_ptr<option_impl>;
    using s_ptr_t = std::shared_ptr<option_impl>;

    static option_map_t clone_options(const option_map_t& options);

    static u_ptr_t create(const option_map_t& options = {});
    static u_ptr_t create(option_map_t&& options);
    static u_ptr_t create(const i_option& options);

    option_impl(const option_impl& option);
    option_impl(option_impl&& other) = default;

    option_impl& operator=(const option_impl& option);
    option_impl& operator=(option_impl&& other) = default;

    option_impl(const option_map_t& options = {});
    option_impl(option_map_t&& options);
    option_impl(const i_option& options);

    option_impl& assign(const i_option& options);

    option_map_t release();

    // i_option interface
public:
    bool has_option(const option_id_t &id) const override;
    bool set(const option_id_t &id
             , i_option_value::u_ptr_t &&value) override;
    bool set(const option_id_t &id
             , const i_option_value &value) override;
    const i_option_value* get(const option_id_t &id) const override;
    option_id_list_t ids() const override;
    void foreach(const foreach_handler_t &handler) const override;
    void clear() override;
    std::size_t merge(const i_option &other) override;
    bool is_equal(const i_option &other) const override;
    std::size_t count() const override;
    i_option::u_ptr_t clone() const override;
};

}

#endif // MPL_OPTION_IMPL_H
