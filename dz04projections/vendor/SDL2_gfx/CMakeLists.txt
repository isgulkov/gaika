
# A restructured SDL2_gfx 1.0.4 (http://www.ferzkopp.net/wordpress/2016/01/02/sdl_gfx-sdl2_gfx/)

add_library(SDL2_gfx SDL2_framerate.c SDL2_gfxPrimitives.c SDL2_imageFilter.c SDL2_rotozoom.c)
target_include_directories(SDL2_gfx PUBLIC include)
