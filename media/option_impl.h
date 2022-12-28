#ifndef MPL_OPTION_IMPL_H
#define MPL_OPTION_IMPL_H

#include "i_option.h"
#include <unordered_map>

namespace mpl
{

class option_impl : public i_option
{
public:
    using option_map_t = std::unordered_map<i_option::option_key_t
                                            , i_option::option_value_t>;
private:
    option_map_t    m_options;
public:

    using u_ptr_t = std::unique_ptr<option_impl>;
    using s_ptr_t = std::shared_ptr<option_impl>;

    static u_ptr_t create(const option_map_t& options = {});
    static u_ptr_t create(option_map_t&& options);
    static u_ptr_t create(const i_option& options);

    option_impl(const option_map_t& options = {});
    option_impl(option_map_t&& options);
    option_impl(const i_option& options);

    option_impl& assign(const i_option& options);

    option_map_t release();

    // i_option interface
public:
    bool has_option(const option_key_t &key) const override;
    bool set(const option_key_t &key
             , option_value_t &&value) override;
    bool set(const option_key_t &key
             , const option_value_t &value) override;
    option_value_t get(const option_key_t &key) const override;
    option_key_list_t keys() const override;
    void foreach(const foreach_handler_t &handler) const override;
    void clear() override;
    std::size_t merge(const i_option &other) override;
    bool is_equal(const i_option &other) const override;
    std::size_t count() const override;
    i_option::u_ptr_t clone() const override;
};

}

#endif // MPL_OPTION_IMPL_H
