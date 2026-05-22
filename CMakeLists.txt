cmake_minimum_required(VERSION 3.31)
project(OrganicallySourcedPBR DESCRIPTION "An improved C++ remake of PBR-2-Source")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.rc")

qt_add_executable(OrganicallySourcedPBR main.cpp
        res/res.qrc
        src/nl_NL.ts
        src/mainwindow.cpp
        src/mainwindow.h
        ${app_icon_resource_windows})


set(SOURCEPP_LIBS_START_ENABLED OFF CACHE INTERNAL "" FORCE)
set(SOURCEPP_USE_VTFPP ON CACHE INTERNAL "" FORCE)

add_subdirectory(libs/sourcepp)

target_link_libraries(OrganicallySourcedPBR PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        sourcepp::vtfpp
)

option(OGP_INSTALL "Configure install and CPack rules for OrganicallySourcedPBR" OFF)
if(OGP_INSTALL)
    if(UNIX)
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
                DESTINATION "share/licenses/${PROJECT_NAME}")

        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin")

        # Desktop file
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/install/linux/desktop.in"
                "${CMAKE_CURRENT_SOURCE_DIR}/install/linux/generated/${PROJECT_NAME}.desktop")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/install/linux/generated/${PROJECT_NAME}.desktop"
                DESTINATION "share/applications")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.png"
                DESTINATION "share/icons/hicolor/512x512/apps"
                RENAME "${PROJECT_NAME}.png")

        # Use system Qt - no install rules
    else()
        message(FATAL_ERROR "No install rules for selected platform.")
    endif()

    # CPack setup
    set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
    set(CPACK_PACKAGE_VENDOR "Project Collapse Studios")
    set(CPACK_PACKAGE_CONTACT "trico-everfire@project-collapse-studios.org")
    set(CPACK_PACKAGE_DESCRIPTION ${CMAKE_PROJECT_DESCRIPTION})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${CPACK_PACKAGE_DESCRIPTION})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.png")
    set(CPACK_MONOLITHIC_INSTALL ON)
    set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} ${PROJECT_NAME}) # duplication is correct here
    set(CPACK_STRIP_FILES ON)
    set(CPACK_THREADS 0)
    set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    if(WIN32)
        # todo
    else()
        if(CPACK_GENERATOR STREQUAL "DEB")
            set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/Trico-Everfire/OrganicallySourcedPBR")
            set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
            set(CPACK_DEBIAN_PACKAGE_DEPENDS "libxcb-cursor0, libqt6core6 (>= 6.4.2), libqt6gui6 (>= 6.4.2), libqt6widgets6 (>= 6.4.2)")
            set(CPACK_DEBIAN_COMPRESSION_TYPE "zstd")
            set(CPACK_DEBIAN_PACKAGE_VERSION "1.0")
        elseif(CPACK_GENERATOR STREQUAL "RPM")
            set(CPACK_RPM_PACKAGE_LICENSE "MIT")
            set(CPACK_RPM_PACKAGE_REQUIRES "libxcb, qt6-qtbase >= 6.4.2, qt6-qtbase-gui >= 6.4.2")
            set(CPACK_RPM_COMPRESSION_TYPE "zstd")
        else()
            message(FATAL_ERROR "CPACK_GENERATOR is unset! Only DEB and RPM generators are supported.")
        endif()
    endif()
    include(CPack)
endif()
