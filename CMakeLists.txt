if(IDF_VER) # ESP-IDF component
	idf_component_register(SRC_DIRS "src"  INCLUDE_DIRS "src")
	target_compile_options(${COMPONENT_LIB} PUBLIC -DESP32)
else() # Raspberry Pi Pico
	add_library (JLed src/jled_base.cpp)
	target_include_directories (JLed PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif()
