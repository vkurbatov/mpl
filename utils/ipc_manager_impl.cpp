#include "ipc_manager_impl.h"
#include "tools/ipc/ipc_manager.h"
#include "tools/ipc/ipc_segment.h"

namespace mpl
{

class ipc_shared_data_manager : public i_shared_data_manager
{
    class ipc_shared_data : public i_sync_shared_data
    {
        mutable ipc::ipc_segment    m_ipc_segment;
    public:
        using s_ptr_t = std::shared_ptr<ipc_shared_data>;
        static s_ptr_t create(ipc::ipc_manager& ipc_manager
                              , const std::string_view& name
                              , std::size_t size)
        {
            if (ipc_manager.is_valid())
            {
                if (auto ptr = std::make_shared<ipc_shared_data>(ipc_manager
                                                                 , name
                                                                 , size))
                {
                    if (ptr->m_ipc_segment.is_valid())
                    {
                        return ptr;
                    }
                }
            }

            return nullptr;
        }

        ipc_shared_data(ipc::ipc_manager& ipc_manager
                        , const std::string_view& name
                        , std::size_t size)
            : m_ipc_segment(ipc_manager.create_segment(std::string(name)
                                                       , size))
        {

        }

        ~ipc_shared_data()
        {

        }

        bool is_valid() const
        {
            return m_ipc_segment.is_valid();
        }

        // i_shared_data interface
    public:
        const void *map() const override
        {
            return m_ipc_segment.map();
        }
        void *map() override
        {
            return m_ipc_segment.map();
        }
        bool unmap() const override
        {
            m_ipc_segment.unmap();
            return true;
        }
        bool unmap() override
        {
            m_ipc_segment.unmap();
            return true;
        }
        std::size_t size() const override
        {
            return m_ipc_segment.size();
        }

        // i_sync_shared_data interface
    public:
        std::string name() const override
        {
            return m_ipc_segment.name();
        }
        void notify() override
        {
            m_ipc_segment.notify(true);
        }

        bool wait(timestamp_t timeout) override
        {
            if (timeout == timestamp_infinite)
            {
                m_ipc_segment.wait();
                return true;
            }

            return m_ipc_segment.wait(timeout);

        }
    };

    ipc::ipc_manager        m_ipc_manager;
    // i_shared_data_manager interface
public:
    using u_ptr_t = std::unique_ptr<ipc_shared_data_manager>;

    static u_ptr_t create(const std::string_view &name, std::size_t total_size)
    {
        ipc::ipc_manager ipc_manager({name, total_size });
        if (ipc_manager.is_valid())
        {
            return std::make_unique<ipc_shared_data_manager>(std::move(ipc_manager));
        }

        return nullptr;
    }

    ipc_shared_data_manager(ipc::ipc_manager&& ipc_manager)
        : m_ipc_manager(std::move(ipc_manager))
    {

    }

    i_sync_shared_data::s_ptr_t query_data(const std::string_view &name
                                           , std::size_t size) override
    {
        return ipc_shared_data::create(m_ipc_manager
                                       , name
                                       , size);
    }

    std::size_t available_size() override
    {
        return m_ipc_manager.free_size();
    }
};


ipc_manager_factory &ipc_manager_factory::get_instance()
{
    static ipc_manager_factory single_factory;
    return single_factory;
}

i_shared_data_manager::u_ptr_t ipc_manager_factory::create_shared_data_manager(const std::string_view &name
                                                                               , std::size_t total_size)
{
    return ipc_shared_data_manager::create(name, total_size);
}



}