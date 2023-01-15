#ifndef BASE_ANY_H
#define BASE_ANY_H

#include <any>
#include <functional>

namespace base
{

class any
{
public:
    using comparator_t = std::function<bool(const std::any& lhs
                                            , const std::any& rhs)>;
    using merger_t = std::function<bool(std::any& lhs
                                   , const std::any& rhs)>;
private:

    std::any        m_value;
    comparator_t    m_comparator;
    merger_t        m_merger;

public:

    template<typename T, typename TV = std::decay_t<T>>
    struct default_comporator_t
    {
        bool operator()(const std::any& lhs
                        , const std::any& rhs)
        {
            const TV* l = std::any_cast<TV>(&lhs);
            const TV* r = std::any_cast<TV>(&rhs);

            return l != nullptr
                    && r != nullptr
                    && *l == *r;
        }
    };

    template<typename T, typename TV = std::decay_t<T>>
    struct default_merger_t
    {
        bool operator()(std::any& lhs
                        , const std::any& rhs)
        {
            auto l = std::any_cast<TV>(&lhs);
            auto r = std::any_cast<TV>(&rhs);

            if (l != nullptr
                    && r != nullptr)
            {
                *l = *r;
                return true;
            }

            return false;
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
        , comparator_t comporator = default_comporator_t<T>()
        , merger_t merger = default_merger_t<T>())
        : m_value(std::move(value))
        , m_comparator(std::move(comporator))
        , m_merger(std::move(merger))
    {

    }

    template <typename T, typename VT = decay_if_not_any<T>>
    any(const T& value
        , comparator_t comporator = default_comporator_t<T>()
        , merger_t merger = default_merger_t<T>())
        : m_value(value)
        , m_comparator(std::move(comporator))
        , m_merger(std::move(merger))
    {

    }

    template<typename T, typename VT = std::enable_if_t<!std::is_pointer_v<T>>>
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

    bool merge(const any& other);

    bool has_value() const;
    bool has_comparable() const;
    operator bool() const;

};

}

#endif // BASE_ANY_H
