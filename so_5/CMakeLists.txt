cmake_minimum_required(VERSION 3.21..)

if(NOT SOBJECTIZER_MASTER_PROJECT)
	set(SOBJECTIZER_MASTER_PROJECT OFF)
	if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
		set(SOBJECTIZER_MASTER_PROJECT ON)
	endif()
endif()

if((NOT SOBJECTIZER_BUILD_STATIC) AND (NOT SOBJECTIZER_BUILD_SHARED))
	set(SOBJECTIZER_BUILD_STATIC ON)
	set(SOBJECTIZER_BUILD_SHARED ON)
endif()

if(NOT SOBJECTIZER_INSTALL)
	set(SOBJECTIZER_INSTALL ${SOBJECTIZER_MASTER_PROJECT})
endif()

include(cmake/target.cmake)

project(sobjectizer VERSION ${SO_5_VERSION} LANGUAGES CXX)

set(PTHREDS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(SO_5_DEFS "-DSO_5_PRJ")

if(WIN32)
	list(APPEND SO_5_DEFS "-DSO_5__PLATFORM_REQUIRES_CDECL")
endif()

get_filename_component(CURRENT_FILE_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
get_filename_component(CURRENT_FILE_DIR ${CURRENT_FILE_DIR} DIRECTORY)
set(SO_5_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
unset(CURRENT_FILE_DIR)

set(SO_5_SRC exception.cpp
	exception.cpp
	error_logger.cpp
	timers.cpp
	msg_tracing.cpp
	wrapped_env.cpp
	message.cpp
	enveloped_msg.cpp
	handler_makers.cpp
	message_limit.cpp
	event_queue_hook.cpp
	mbox.cpp
	mchain.cpp
	event_exception_logger.cpp
	agent.cpp
	coop.cpp
	queue_locks_defaults_manager.cpp
	environment.cpp
	so_layer.cpp

	impl/msg_tracing_helpers.cpp
	impl/subscription_storage_iface.cpp
	impl/subscr_storage_vector_based.cpp
	impl/subscr_storage_flat_set_based.cpp
	impl/subscr_storage_map_based.cpp
	impl/subscr_storage_hash_table_based.cpp
	impl/subscr_storage_adaptive.cpp
	impl/process_unhandled_exception.cpp
	impl/named_local_mbox.cpp
	impl/mbox_core.cpp
	impl/coop_repository_basis.cpp
	impl/layer_core.cpp
	impl/mt_env_infrastructure.cpp
	impl/simple_mtsafe_st_env_infrastructure.cpp
	impl/simple_not_mtsafe_st_env_infrastructure.cpp

	stats/repository.cpp
	stats/std_names.cpp

	stats/impl/std_controller.cpp
	stats/impl/ds_agent_core_stats.cpp
	stats/impl/ds_mbox_core_stats.cpp
	stats/impl/ds_timer_thread_stats.cpp

	disp/abstract_work_thread.cpp
	disp/mpsc_queue_traits/pub.cpp
	disp/mpmc_queue_traits/pub.cpp
	disp/one_thread/pub.cpp
	disp/nef_one_thread/pub.cpp
	disp/active_obj/pub.cpp
	disp/active_group/pub.cpp
	disp/thread_pool/pub.cpp
	disp/adv_thread_pool/pub.cpp
	disp/nef_thread_pool/pub.cpp
	disp/prio_one_thread/strictly_ordered/pub.cpp
	disp/prio_one_thread/quoted_round_robin/pub.cpp
	disp/prio_dedicated_threads/one_per_prio/pub.cpp

	experimental/testing/v1/all.cpp
)

function(setup_target_cxx_std TARGET_NAME)
	if(${CMAKE_VERSION} VERSION_LESS "3.8.0")
		set_target_properties(${TARGET_NAME} PROPERTIES
			CXX_STANDARD 17
			CXX_STANDARD_REQUIRED YES
			CXX_EXTENSIONS NO
		)
	else()
		target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
	endif()
endfunction()

set(SO_5_TARGETS_TO_INSTALL)

if(SOBJECTIZER_BUILD_SHARED)
	add_library(${SO_5_SHARED_LIB} SHARED ${SO_5_SRC})
	add_library(sobjectizer::SharedLib ALIAS ${SO_5_SHARED_LIB})
	set_property(TARGET ${SO_5_SHARED_LIB} PROPERTY EXPORT_NAME SharedLib)
	setup_target_cxx_std(${SO_5_SHARED_LIB})

	if(NOT WIN32)
		target_compile_options(${SO_5_SHARED_LIB}
			PRIVATE -fvisibility=hidden -fvisibility-inlines-hidden
		)
	endif()

	target_compile_definitions(${SO_5_SHARED_LIB}
		PRIVATE ${SO_5_DEFS}
	)
	target_include_directories(${SO_5_SHARED_LIB}
		PUBLIC
			$<BUILD_INTERFACE:${SO_5_INCLUDE_PATH}/..>
			$<INSTALL_INTERFACE:include>
	)
	target_link_libraries(${SO_5_SHARED_LIB} ${CMAKE_THREAD_LIBS_INIT})

	set(SO_5_EXT_LIBS )
	if( ANDROID )
		list(APPEND SO_5_EXT_LIBS ${ANDROID_LIBCRYSTAX_FILE})
		list(APPEND SO_5_EXT_LIBS ${ANDROID_LIBSTDCXX_FILE})
	endif()

	foreach(__extLibrary ${SO_5_EXT_LIBS})
		add_custom_command(TARGET ${SO_5_SHARED_LIB} PRE_LINK
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${__extLibrary} $<TARGET_FILE_DIR:${SO_5_SHARED_LIB}>)
	endforeach()

	list(APPEND SO_5_TARGETS_TO_INSTALL ${SO_5_SHARED_LIB})
endif()

if(SOBJECTIZER_BUILD_STATIC)
	add_library(${SO_5_STATIC_LIB} STATIC ${SO_5_SRC})
	add_library(sobjectizer::StaticLib ALIAS ${SO_5_STATIC_LIB})
	set_property(TARGET ${SO_5_STATIC_LIB} PROPERTY EXPORT_NAME StaticLib)
	setup_target_cxx_std(${SO_5_STATIC_LIB})
	target_compile_definitions(${SO_5_STATIC_LIB}
		PRIVATE ${SO_5_DEFS}
	)
	target_compile_definitions(${SO_5_STATIC_LIB}
		PUBLIC -DSO_5_STATIC_LIB
	)
	target_include_directories(${SO_5_STATIC_LIB}
		PUBLIC
			$<BUILD_INTERFACE:${SO_5_INCLUDE_PATH}/..>
			$<INSTALL_INTERFACE:include>
	)
	target_link_libraries(${SO_5_STATIC_LIB} ${CMAKE_THREAD_LIBS_INIT})

	list(APPEND SO_5_TARGETS_TO_INSTALL ${SO_5_STATIC_LIB})
endif()

if(SOBJECTIZER_INSTALL)
	set(SO_5_CMAKE_FILES_DEST "lib/cmake/sobjectizer")

	install(
		TARGETS ${SO_5_TARGETS_TO_INSTALL}
		EXPORT SO_5_ALL_TARGETS
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
	)
	install(
		EXPORT SO_5_ALL_TARGETS
		FILE sobjectizer-targets.cmake
		NAMESPACE sobjectizer::
		DESTINATION ${SO_5_CMAKE_FILES_DEST}
	)

	if(SOBJECTIZER_BUILD_SHARED)
		foreach(__extLibrary ${SO_5_EXT_LIBS})
			install(FILES ${__extLibrary} DESTINATION lib)
		endforeach()
	endif()

	file(GLOB_RECURSE ALL_SO_5_HEADERS RELATIVE ${CMAKE_CURRENT_LIST_DIR} *.hpp)

	foreach( HEADER_FILE ${ALL_SO_5_HEADERS} )
		# Since v.5.7.2.3 install even files from impl subfolders
		# because it could be necessary for projects like so5extra.
		get_filename_component( DIR ${HEADER_FILE} PATH )
		install( FILES ${HEADER_FILE} DESTINATION include/so_5/${DIR} )
	endforeach()

	set(SO_5_CONFIG_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/sobjectizer-config-version.cmake")
	set(SO_5_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/sobjectizer-config.cmake")

	include(CMakePackageConfigHelpers)
	write_basic_package_version_file(
		${SO_5_CONFIG_VERSION_FILE}
		VERSION ${SO_5_VERSION}
		COMPATIBILITY ExactVersion
	)
	configure_package_config_file(
		"cmake/sobjectizer-config.cmake.in"
		${SO_5_CONFIG_FILE}
		INSTALL_DESTINATION ${SO_5_CMAKE_FILES_DEST}
		PATH_VARS SO_5_VERSION
	)

	install(
		FILES ${SO_5_CONFIG_FILE} ${SO_5_CONFIG_VERSION_FILE}
		DESTINATION ${SO_5_CMAKE_FILES_DEST}
	)
endif()

