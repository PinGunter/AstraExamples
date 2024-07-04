cmake_minimum_required(VERSION 3.9.6 FATAL_ERROR)
project(Astra)

include(AstraEngine/setup.cmake)

#--------------------------------------------------------------------------------------------------
# Source files for this project
#

add_subdirectory(SceneEditor)

add_subdirectory(Pathtracer)


