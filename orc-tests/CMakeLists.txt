cmake_minimum_required(VERSION 3.20)

if(BUILD_UNIT_TESTS)
    add_subdirectory(core/unit)
endif()

if(BUILD_GUI_TESTS)
    add_subdirectory(gui/unit)
endif()