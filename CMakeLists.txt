cmake_minimum_required(VERSION 3.5)

project(Movar_cpp VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets
    LinguistTools WebEngineWidgets TextToSpeech)
qt_standard_project_setup()

qt_add_lupdate(
    TS_FILES
    translations/Movar_cpp_en_US.ts
    translations/Movar_cpp_ja_JP.ts
    translations/Movar_cpp_uk_UA.ts

    SOURCES
    mainwindow.cpp
    dictionarysettings.cpp
    fileloader.cpp
    mainwindow.ui
    dictionarysettings.ui
)

qt_add_lrelease(
    TS_FILES
    translations/Movar_cpp_en_US.ts
    translations/Movar_cpp_ja_JP.ts
    translations/Movar_cpp_uk_UA.ts
)

qt_add_executable(Movar_cpp
    mainwindow.ui
    mainwindow.h
    mainwindow.cpp
    main.cpp
    dictionarysettings.h
    dictionarysettings.cpp
    dictionarysettings.ui
    fileloader.h
    fileloader.cpp
    adaptedcompleter.h
    adaptedcompleter.cpp
    adaptedstringlistmodel.h
    adaptedstringlistmodel.cpp
)

add_dependencies(${CMAKE_PROJECT_NAME} update_translations)

target_link_libraries(Movar_cpp PRIVATE
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::TextToSpeech)

set_target_properties(Movar_cpp PROPERTIES
    WIN32_EXECUTABLE ON
)

install(TARGETS Movar_cpp
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}

)


