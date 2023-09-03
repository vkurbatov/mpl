#include "ipc_manager.h"
#include "ipc_segment.h"

namespace ipc
{

namespace detail
{


}

ipc_manager::config_t::config_t(const std::string_view &name
                                , std::size_t size)
    : name(name)
    , size(size)
{

}

bool ipc_manager::config_t::is_valid() const
{
    return !name.empty();
}

bool ipc_manager::config_t::has_create() const
{
    return is_valid()
            && size > 0;
}

void ipc_manager::remove_by_name(const std::string &name)
{
    boost::interprocess::shared_memory_object::remove(name.c_str());
}

ipc_manager::ipc_shmem_manager_ptr_t ipc_manager::create_native_manager(const config_t &config)
{
    if (config.is_valid())
    {
        try
        {
            if (config.size > 0)
            {
                //remove_by_name(config.name);

                return std::make_shared<ipc_shmem_manager_t>(boost::interprocess::open_or_create
                                                             , config.name.c_str()
                                                             , config.size);


            }
            else
            {
                return std::make_shared<ipc_shmem_manager_t>(boost::interprocess::open_only
                                                            , config.name.c_str());
            }
        }
        catch(const boost::interprocess::interprocess_exception& e)
        {
            // log
        }
        catch(const std::exception& e)
        {
            // log
        }
        catch(...)
        {
            // ???
        }

        return nullptr;
    }

    return nullptr;
}

ipc_manager::ipc_manager(const config_t &config)
    : m_config(config)
    , m_native_manager(create_native_manager(m_config))
{

}

ipc_manager::~ipc_manager()
{
/*
    bool need_remove = m_native_manager != nullptr
            && m_config.has_create();
    m_native_manager.reset();
    if (need_remove)
    {
        remove_by_name(m_config.name);
    }*/
}

ipc_segment ipc_manager::create_segment(const std::string &name
                                        , std::size_t size)
{
    return ipc_segment(m_native_manager
                       , name
                       , size);
}

std::string ipc_manager::name() const
{
    return m_config.name;
}

void *ipc_manager::pointer()
{
    return m_native_manager->get_address();
}

std::size_t ipc_manager::total_size() const
{
    return m_native_manager->get_size();
}

std::size_t ipc_manager::free_size() const
{
    return m_native_manager->get_free_memory();
}


ipc_shmem_manager_t &ipc_manager::native_manager()
{
    return *m_native_manager;
}

bool ipc_manager::is_valid() const
{
    return m_native_manager != nullptr;
}

bool ipc_manager::is_writeble() const
{
    return is_valid()
            && m_config.has_create();
}



}
