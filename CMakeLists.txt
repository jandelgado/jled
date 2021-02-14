# this CMakeLists.txt file is needed to include JLed in raspi pico projects
add_library (JLed src/jled_base.cpp)
target_include_directories (JLed PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

