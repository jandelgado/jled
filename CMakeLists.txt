if(IDF_VER) # ESP-IDF component (ESP32)

    idf_component_register(
        SRCS "src/jled_effects.cpp"
        INCLUDE_DIRS "src"
        REQUIRES esp_driver_ledc esp_timer)

    idf_build_set_property(COMPILE_OPTIONS "-DESP32" APPEND)
    set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)

else() # Raspberry Pi Pico
	add_library (JLed src/jled_effects.cpp)
	target_include_directories (JLed PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif()
