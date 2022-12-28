#ifndef BASE_ANY_H
#define BASE_ANY_H

#include <any>
#include <functional>

namespace base
{

class any
{
    using comparator_t = std::function<bool(const std::any& lhs
                                            , const std::any& rhs)>;
    std::any        m_value;
    comparator_t    m_comparator;

public:

    template<typename T>
    struct default_comporator_t
    {
        bool operator()(const std::any& lhs
                        , const std::any& rhs)
        {
            auto l = std::any_cast<T>(&lhs);
            auto r = std::any_cast<T>(&rhs);

            return l != nullptr
                    && r != nullptr
                    && *l == *r;
        }
    };

    any();
    any(const any& other) = default;
    any(any&& other) = default;
    any& operator = (const any& other) = default;
    any& operator = (any&& other) = default;

    template<typename T
             , typename VT = std::decay_t<T>>
    using decay_if_not_any = std::enable_if_t<!std::is_same_v<VT, base::any>>;

    template <typename T, typename VT = decay_if_not_any<T>>
    any(T&& value
        , comparator_t comporator = default_comporator_t<T>())
        : m_value(std::move(value))
        , m_comparator(std::move(comporator))
    {

    }

    template <typename T, typename VT = decay_if_not_any<T>>
    any(const T& value
        , comparator_t comporator = default_comporator_t<T>())
        : m_value(value)
        , m_comparator(std::move(comporator))
    {

    }

    template<typename T>
    T cast() const
    {
        return std::any_cast<T>(m_value);
    }

    template<typename T, typename VT = std::enable_if_t<std::is_pointer_v<T>>>
    const T cast() const
    {
        return std::any_cast<std::remove_pointer_t<T>>(&m_value);
    }

    template<typename T, typename VT = std::enable_if_t<std::is_pointer_v<T>>>
    T cast()
    {
        return std::any_cast<typename std::remove_pointer_t<T>>(&m_value);
    }

    template<typename T>
    bool has_type() const
    {
        return cast<T*>() != nullptr;
    }

    const std::type_info& type() const;

    bool operator == (const any& other) const;
    bool operator != (const any& other) const;

    bool has_value() const;
    bool has_comparable() const;
    operator bool() const;

};

}

#endif // BASE_ANY_H
