add_executable(compressor compressor.cxx)
target_link_libraries(compressor ${Boost_LIBRARIES} AliceO2::TOFCompression)
install(TARGETS compressor RUNTIME DESTINATION bin)
