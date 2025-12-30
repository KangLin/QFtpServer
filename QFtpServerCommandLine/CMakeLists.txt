# Author: Kang Lin <kl222@126.com>

project(QFtpServerCommandLine)

SET(_COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${_COMPONENTS})
if(Qt${QT_VERSION_MAJOR}_FOUND)
    FOREACH(_COMPONENT ${_COMPONENTS})
        list(APPEND QT_LIBRARIES Qt${QT_VERSION_MAJOR}::${_COMPONENT})
    ENDFOREACH()
endif()
message("${PROJECT_NAME} QT_LIBRARIES:${QT_LIBRARIES}")

set(SOURCE_FILES
    main.cpp
    )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} QFtpServerLib ${QT_LIBRARIES})
install(TARGETS ${PROJECT_NAME})
