# Install script for directory: /home/cuellarmaster/Bike/src/bike_game

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/cuellarmaster/Bike/install/bike_game")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/bike_game/bike_game" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/bike_game/bike_game")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/bike_game/bike_game"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/bike_game" TYPE EXECUTABLE FILES "/home/cuellarmaster/Bike/build/bike_game/bike_game")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/bike_game/bike_game" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/bike_game/bike_game")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/bike_game/bike_game"
         OLD_RPATH "/home/cuellarmaster/ros2_jazzy/install/rclcpp/lib:/home/cuellarmaster/ros2_jazzy/install/geometry_msgs/lib:/home/cuellarmaster/ros2_jazzy/install/libstatistics_collector/lib:/home/cuellarmaster/ros2_jazzy/install/rcl/lib:/home/cuellarmaster/ros2_jazzy/install/rmw_implementation/lib:/home/cuellarmaster/ros2_jazzy/install/type_description_interfaces/lib:/home/cuellarmaster/ros2_jazzy/install/rcl_yaml_param_parser/lib:/home/cuellarmaster/ros2_jazzy/install/rosgraph_msgs/lib:/home/cuellarmaster/ros2_jazzy/install/rcl_interfaces/lib:/home/cuellarmaster/ros2_jazzy/install/service_msgs/lib:/home/cuellarmaster/ros2_jazzy/install/statistics_msgs/lib:/home/cuellarmaster/ros2_jazzy/install/tracetools/lib:/home/cuellarmaster/ros2_jazzy/install/rcl_logging_interface/lib:/home/cuellarmaster/ros2_jazzy/install/ament_index_cpp/lib:/home/cuellarmaster/ros2_jazzy/install/std_msgs/lib:/home/cuellarmaster/ros2_jazzy/install/builtin_interfaces/lib:/home/cuellarmaster/ros2_jazzy/install/rosidl_typesupport_fastrtps_c/lib:/home/cuellarmaster/ros2_jazzy/install/rosidl_typesupport_introspection_cpp/lib:/home/cuellarmaster/ros2_jazzy/install/rosidl_typesupport_introspection_c/lib:/home/cuellarmaster/ros2_jazzy/install/rosidl_typesupport_fastrtps_cpp/lib:/home/cuellarmaster/ros2_jazzy/install/rmw/lib:/home/cuellarmaster/ros2_jazzy/install/rosidl_dynamic_typesupport/lib:/home/cuellarmaster/ros2_jazzy/install/fastcdr/lib:/home/cuellarmaster/ros2_jazzy/install/rosidl_typesupport_cpp/lib:/home/cuellarmaster/ros2_jazzy/install/rosidl_typesupport_c/lib:/home/cuellarmaster/ros2_jazzy/install/rcpputils/lib:/home/cuellarmaster/ros2_jazzy/install/rosidl_runtime_c/lib:/home/cuellarmaster/ros2_jazzy/install/rcutils/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/bike_game/bike_game")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/cuellarmaster/Bike/build/bike_game/CMakeFiles/bike_game.dir/install-cxx-module-bmi-noconfig.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ament_index/resource_index/package_run_dependencies" TYPE FILE FILES "/home/cuellarmaster/Bike/build/bike_game/ament_cmake_index/share/ament_index/resource_index/package_run_dependencies/bike_game")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ament_index/resource_index/parent_prefix_path" TYPE FILE FILES "/home/cuellarmaster/Bike/build/bike_game/ament_cmake_index/share/ament_index/resource_index/parent_prefix_path/bike_game")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bike_game/environment" TYPE FILE FILES "/home/cuellarmaster/ros2_jazzy/install/ament_cmake_core/share/ament_cmake_core/cmake/environment_hooks/environment/ament_prefix_path.sh")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bike_game/environment" TYPE FILE FILES "/home/cuellarmaster/Bike/build/bike_game/ament_cmake_environment_hooks/ament_prefix_path.dsv")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bike_game/environment" TYPE FILE FILES "/home/cuellarmaster/ros2_jazzy/install/ament_cmake_core/share/ament_cmake_core/cmake/environment_hooks/environment/path.sh")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bike_game/environment" TYPE FILE FILES "/home/cuellarmaster/Bike/build/bike_game/ament_cmake_environment_hooks/path.dsv")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bike_game" TYPE FILE FILES "/home/cuellarmaster/Bike/build/bike_game/ament_cmake_environment_hooks/local_setup.bash")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bike_game" TYPE FILE FILES "/home/cuellarmaster/Bike/build/bike_game/ament_cmake_environment_hooks/local_setup.sh")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bike_game" TYPE FILE FILES "/home/cuellarmaster/Bike/build/bike_game/ament_cmake_environment_hooks/local_setup.zsh")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bike_game" TYPE FILE FILES "/home/cuellarmaster/Bike/build/bike_game/ament_cmake_environment_hooks/local_setup.dsv")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bike_game" TYPE FILE FILES "/home/cuellarmaster/Bike/build/bike_game/ament_cmake_environment_hooks/package.dsv")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ament_index/resource_index/packages" TYPE FILE FILES "/home/cuellarmaster/Bike/build/bike_game/ament_cmake_index/share/ament_index/resource_index/packages/bike_game")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bike_game/cmake" TYPE FILE FILES
    "/home/cuellarmaster/Bike/build/bike_game/ament_cmake_core/bike_gameConfig.cmake"
    "/home/cuellarmaster/Bike/build/bike_game/ament_cmake_core/bike_gameConfig-version.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bike_game" TYPE FILE FILES "/home/cuellarmaster/Bike/src/bike_game/package.xml")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/cuellarmaster/Bike/build/bike_game/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
