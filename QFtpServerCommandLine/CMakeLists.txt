# Author: Kang Lin <kl222@126.com>

project(QFtpServerCommandLine)

set(SOURCE_FILES
    main.cpp
    )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} QFtpServerLib ${QT_LIBRARIES})
install(TARGETS ${PROJECT_NAME})
