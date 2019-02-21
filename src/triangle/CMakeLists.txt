add_executable(triangle 
    application.cpp
    application.h
    triangle.cpp
    util.cpp
    util.h)
target_link_libraries(triangle PRIVATE Vulkan::Vulkan SDL2::SDL2 SDL2::SDL2main)