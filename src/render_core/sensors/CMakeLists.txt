add_library(vision-sensor INTERFACE)
vision_add_plugin(thin_lens CATEGORY sensor SOURCES thin_lens.cpp)
vision_add_plugin(pinhole CATEGORY sensor SOURCES pinhole.cpp)