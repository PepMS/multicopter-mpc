message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_EXECUTABLE = ${PYTHON_EXECUTABLE}")
message(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")

set(${PROJECT_NAME}_PYTHON_BINDINGS_SOURCES python/multicopter_mpc/multicopter-mpc.cpp)
set(${PROJECT_NAME}_PYTHON_BINDINGS_FILES __init__.py)

add_library(${PROJECT_NAME}_pywrap SHARED ${${PROJECT_NAME}_PYTHON_BINDINGS_SOURCES})

target_include_directories(${PROJECT_NAME}_pywrap PUBLIC  ${Boost_INCLUDE_DIRS} 
                                                          ${PYTHON_INCLUDE_DIRS}
                                                          ${eigenpy_INCLUDE_DIRS}
                                                          ${EIGEN3_INCLUDE_DIRS}
                                                          include/
                                                          -lpython2.7
                                                          bindings/)

target_link_libraries(${PROJECT_NAME}_pywrap ${Boost_LIBRARIES} 
                                             ${PYTHON_LIBRARIES}
                                             ${eigenpy_LIBRARIES}
                                             -lpython2.7
                                             multicopter_mpc)

target_compile_options(${PROJECT_NAME}_pywrap PRIVATE "-Wno-conversion")

install(TARGETS ${PROJECT_NAME}_pywrap
        LIBRARY DESTINATION lib/python2.7/dist-packages/multicopter-mpc)
install(DIRECTORY bindings/python/multicopter/utils DESTINATION lib/python2.7/dist-packages/multicopter FILES_MATCHING PATTERN "*.py")
install(FILES bindings/python/multicopter/__init__.py DESTINATION lib/python2.7/dist-packages/multicopter)