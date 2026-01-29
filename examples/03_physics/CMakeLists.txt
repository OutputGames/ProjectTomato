cmake_minimum_required(VERSION 3.15)

project(PhysicsExample)

add_executable(PhysicsExample main.cpp)

target_link_libraries(PhysicsExample PRIVATE TomatoEngine)
