cmake_minimum_required(VERSION 3.8)
project(bike_game)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(bike_game src/bike_game.cpp)
target_include_directories(bike_game PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(bike_game ${SDL2_LIBRARIES})
ament_target_dependencies(bike_game rclcpp std_msgs geometry_msgs)

install(TARGETS bike_game DESTINATION lib/${PROJECT_NAME})

ament_package()
