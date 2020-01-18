project(kdoubanfm)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(KDE4 REQUIRED 4.5)
find_package(QJson REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(MacroLogFeature)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-Wall -Wextra)

set(kdoubanfm_SRCS
    kdoubanfm.cpp
    kiogetdevice.cpp
    main.cpp
    mpris2.cpp
    trayicon.cpp
)

qt4_add_dbus_adaptor(kdoubanfm_SRCS
    org.mpris.MediaPlayer2.xml
    mpris2.h
    Mpris2
    mprisadaptor
    MprisAdaptor
)

qt4_add_dbus_adaptor(kdoubanfm_SRCS
    org.mpris.MediaPlayer2.Player.xml
    mpris2.h
    Mpris2
    mprisplayeradaptor
    MprisPlayerAdaptor
)

kde4_add_ui_files(kdoubanfm_SRCS account.ui)

kde4_add_executable(kdoubanfm ${kdoubanfm_SRCS})

target_link_libraries(kdoubanfm
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${PHONON_LIBRARY}
    ${QJSON_LIBRARY}
)

install(TARGETS kdoubanfm ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kdoubanfm.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

add_subdirectory(icons)
add_subdirectory(po)
