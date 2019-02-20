add_executable(testwindow testwindow.cpp)
target_link_libraries(testwindow PRIVATE Vulkan::Vulkan SDL2)