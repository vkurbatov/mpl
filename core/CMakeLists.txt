cmake_minimum_required(VERSION 3.16)

set(TARGET mplcore)

set(SOURCES
    test.cpp
    type_info.cpp
    thread_info.cpp
    command.cpp
    channel_control.cpp
    event.cpp
    event_channel_state.cpp
    ntp_time.cpp
)

set(PUBLIC_HEADERS
    common_types.h
    type_info.h
    property_types.h
    packet_types.h
    time_types.h
    thread_info.h
    ntp_time.h
    message_types.h
    channel_types.h
    event_types.h
    channel_control.h
    option_types.h
    event.h
    event_channel_state.h
    command_types.h
    command.h
    i_task.h
    i_task_manager.h
    i_timer.h
    i_timer_manager.h
    i_property.h
    i_property_value.h
    i_property_tree.h
    i_parametrizable.h
    i_option.h
    i_option_value.h
    i_data_object.h
    i_buffer.h
    i_mapped_buffer.h
    i_dynamic_buffer.h
    i_buffer_collection.h
    i_fifo_buffer.h
    i_fifo_buffer_reader.h
    i_fifo_buffer_writer.h
    i_shared_data.h
    i_sync_shared_data.h
    i_shared_data_manager.h
    i_channel.h
    i_message.h
    i_message_sink.h
    i_message_source.h
    i_message_data.h
    i_message_event.h
    i_message_command.h
    i_message_packet.h
    i_message_channel.h
    )

set(PRIVATE_HEADERS
    test.h
)


add_library(${TARGET}
    ${SOURCES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

