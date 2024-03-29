
set(${PROJECT_NAME}_m_evacu ${m})
set(m ${PROJECT_NAME})
list(APPEND ${m}_unsetter )


if(NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeKautilHeader.cmake)
    file(DOWNLOAD https://raw.githubusercontent.com/kautils/CMakeKautilHeader/v0.0.1/CMakeKautilHeader.cmake ${CMAKE_BINARY_DIR}/CMakeKautilHeader.cmake)
endif()
include(${CMAKE_BINARY_DIR}/CMakeKautilHeader.cmake)
git_clone(https://raw.githubusercontent.com/kautils/CMakeLibrarytemplate/v0.0.1/CMakeLibrarytemplate.cmake)
git_clone(https://raw.githubusercontent.com/kautils/CMakeFetchKautilModule/v0.0.1/CMakeFetchKautilModule.cmake)


CMakeFetchKautilModule(${m}_kautil_btree GIT https://github.com/kautils/btree_search.git REMOTE origin BRANCH v1.0)
find_package(KautilAlgorithmBtreeSearch.1.0.1.interface REQUIRED)


file(GLOB ${m}_range_exists ${CMAKE_CURRENT_LIST_DIR}/*.hpp)
install(FILES ${${m}_range_exists} DESTINATION include/kautil/range/exists )
install(SCRIPT "${${m}_kautil_btree.BUILD_DIR}/cmake_install.cmake")

string(APPEND ${m}_findpkgs
    "if(EXISTS \"\\$\\{PACKAGE_PREFIX_DIR}/lib/cmake/KautilAlgorithmBtreeSearch.1.0.1\")\n"
    "\t set(KautilAlgorithmBtreeSearch.1.0.1_DIR \"\\$\\{PACKAGE_PREFIX_DIR}/lib/cmake/KautilAlgorithmBtreeSearch.1.0.1\")\n"
    "\t find_package(KautilAlgorithmBtreeSearch.1.0.1.interface REQUIRED)\n"
    "endif()\n"
)


set(module_name exists)
unset(srcs)
file(GLOB srcs ${CMAKE_CURRENT_LIST_DIR}/*.hpp)
set(${module_name}_common_pref
    MODULE_PREFIX kautil range
    MODULE_NAME ${module_name}
    INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}> $<INSTALL_INTERFACE:include> 
    SOURCES ${srcs}
    LINK_LIBS kautil::algorithm::btree_search::1.0.1::interface
    EXPORT_NAME_PREFIX ${PROJECT_NAME}
    EXPORT_VERSION ${PROJECT_VERSION}
    EXPORT_VERSION_COMPATIBILITY AnyNewerVersion
        
    EXPORT_CONFIG_IN_ADDITIONAL_CONTENT_AFTER ${${m}_findpkgs}
    DESTINATION_INCLUDE_DIR include/kautil/range
    DESTINATION_CMAKE_DIR cmake
    DESTINATION_LIB_DIR lib
)

CMakeLibraryTemplate(${module_name} EXPORT_LIB_TYPE interface ${${module_name}_common_pref} )


if(${BUILD_TEST})
    
set(__t ${${module_name}_interface_tmain})
add_executable(${__t})
target_sources(${__t} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/unit_test.cc)
target_link_libraries(${__t} PRIVATE ${${module_name}_interface})
target_compile_definitions(${__t} PRIVATE ${${module_name}_interface_tmain_ppcs})

endif()


foreach(__v ${${m}_unsetter})
    unset(${__v})
endforeach()
unset(${m}_unsetter)
set(m ${${PROJECT_NAME}_m_evacu})
