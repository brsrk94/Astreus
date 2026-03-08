cmake_minimum_required(VERSION 3.21)
project(AstreusPackageDownloader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(astreus-package-downloader
    src/main.cpp
    src/PackageInstallerService.cpp
)

qt_add_qml_module(astreus-package-downloader
    URI Astreus
    VERSION 1.0
    QML_FILES
        qml/Main.qml
    RESOURCES
        resources/fonts/Outfit-Variable.ttf
)

target_include_directories(astreus-package-downloader PRIVATE src)

target_link_libraries(astreus-package-downloader
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
)

install(TARGETS astreus-package-downloader
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
