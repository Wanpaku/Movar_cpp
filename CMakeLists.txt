cmake_minimum_required(VERSION 3.5)

project(Movar_cpp VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets
    LinguistTools WebEngineWidgets TextToSpeech)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES en uk ja)

qt_add_executable(${PROJECT_NAME}
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
    resources.qrc
    logger.h
    logger.cpp
)

qt_add_translations(${PROJECT_NAME}
    TS_FILES translations/Movar_cpp_en_US.ts
             translations/Movar_cpp_uk_UA.ts
             translations/Movar_cpp_ja_JP.ts
)

add_dependencies(${PROJECT_NAME} update_translations)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::TextToSpeech
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}

)


