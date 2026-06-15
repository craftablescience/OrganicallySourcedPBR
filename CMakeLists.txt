cmake_minimum_required(VERSION 3.31)
project(OrganicallySourcedPBR
        DESCRIPTION "An improved C++ remake of PBR-2-Source"
        HOMEPAGE_URL "https://github.com/Trico-Everfire/OrganicallySourcedPBR")

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

qt_add_executable(OrganicallySourcedPBR
        main.cpp
        res/res.qrc
        src/nl_NL.ts
        src/mainwindow.cpp
        src/mainwindow.h
        "$<$<BOOL:${WIN32}>:res/logo.rc>")


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
    if(WIN32)
        install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
            DESTINATION .)

        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)

        # Qt
        install(IMPORTED_RUNTIME_ARTIFACTS
                Qt6::Core Qt6::Gui Qt6::Widgets
                RUNTIME DESTINATION .
                LIBRARY DESTINATION .)

        install(FILES "${QT_BASEDIR}/plugins/platforms/qwindows${QT_LIB_SUFFIX}.dll"
                DESTINATION "platforms")

        if(EXISTS "${QT_BASEDIR}/plugins/styles/qmodernwindowsstyle${QT_LIB_SUFFIX}.dll")
            install(FILES "${QT_BASEDIR}/plugins/styles/qmodernwindowsstyle${QT_LIB_SUFFIX}.dll"
                    DESTINATION "styles")
        else()
            install(FILES "${QT_BASEDIR}/plugins/styles/qwindowsvistastyle${QT_LIB_SUFFIX}.dll"
                    DESTINATION "styles")
        endif()
    elseif(UNIX)
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
        if(CPACK_GENERATOR STREQUAL "NSIS")
            set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
            set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
            set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})
            set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME})
            set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.ico")
            set(CPACK_NSIS_INSTALLED_ICON_NAME "${PROJECT_NAME}.exe")
            set(CPACK_NSIS_URL_INFO_ABOUT "${PROJECT_HOMEPAGE_URL}")
            set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
        else()
            message(FATAL_ERROR "CPACK_GENERATOR is unset! Only NSIS generator is supported.")
        endif()
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
