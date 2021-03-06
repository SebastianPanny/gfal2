cmake_minimum_required (VERSION 2.6)


if (PLUGIN_SRM)
    file (GLOB src_srm "*.c*")

    pkg_check_modules(SRM_IFCE_PKG REQUIRED srm-ifce>=1.15.0)
    pkg_check_modules(GLOBUS_GSSAPI_PKG REQUIRED globus-gssapi-gsi)

    add_definitions(${SRM_IFCE_PKG_CFLAGS} ${GLOBUS_GSSAPI_PKG_CFLAGS})
    add_definitions(${gfal2_mds_def})
    include_directories(${SRM_IFCE_PKG_INCLUDE_DIR} ${GLOBUS_GSSAPI_PKG_INCLUDE_DIR})

    add_library(plugin_srm SHARED ${src_srm} ${gfal2_mds_src} )
    target_link_libraries(plugin_srm gfal2 gfal2_transfer ${mds_cache_link}
                                     ${SRM_IFCE_PKG_LIBRARIES} ${is_ifce_link} gomp)

    set_target_properties(plugin_srm PROPERTIES
                            CLEAN_DIRECT_OUTPUT 1
                            OUTPUT_NAME "gfal_plugin_srm"
                            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

    install(TARGETS plugin_srm
            LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
    install(FILES "README_PLUGIN_SRM"
            DESTINATION ${DOC_INSTALL_DIR})


    # install srm configuration files
    LIST(APPEND srm_conf_file "${CMAKE_SOURCE_DIR}/dist/etc/gfal2.d/srm_plugin.conf")
    install(FILES ${srm_conf_file}
                        DESTINATION ${SYSCONF_INSTALL_DIR}/gfal2.d/)

endif (PLUGIN_SRM)
