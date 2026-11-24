add_executable (CMakeProject1 "k580_ableton.cpp" "k580_ableton.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakeProject1 PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(CMakeProject1 PRIVATE winmm)