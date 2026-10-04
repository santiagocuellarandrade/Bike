cmake_minimum_required(VERSION 3.8)
project(bike_game)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Buscar dependencias de ROS 2 y SDL2 base
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(SDL2 REQUIRED)

# Buscar la librería SDL2_net de forma correcta
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_NET REQUIRED IMPORTED_TARGET SDL2_net)

# Crear el ejecutable del juego
add_executable(bike_game src/bike_game.cpp)

# Incluir los directorios de cabeceras (.h) de SDL2 y SDL2_net
target_include_directories(bike_game PRIVATE 
  ${SDL2_INCLUDE_DIRS}
  ${SDL2_NET_INCLUDE_DIRS}
)

# Enlazar físicamente las librerías binarias al ejecutable
target_link_libraries(bike_game 
  ${SDL2_LIBRARIES}
  PkgConfig::SDL2_NET
)

# Enlazar dependencias de ROS 2
ament_target_dependencies(bike_game rclcpp std_msgs geometry_msgs)

# Instalación del ejecutable
install(TARGETS bike_game DESTINATION lib/${PROJECT_NAME})

ament_package()
