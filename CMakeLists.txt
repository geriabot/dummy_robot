cmake_minimum_required(VERSION 3.8)
project(dummy_robot)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(rclcpp_cascade_lifecycle REQUIRED)
find_package(behavior_architecture REQUIRED)
find_package(social_bt_nodes REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(std_msgs REQUIRED)

set(dependencies
  rclcpp
  rclcpp_lifecycle
  rclcpp_cascade_lifecycle
  behavior_architecture
  social_bt_nodes
  behaviortree_cpp
  std_msgs
)

# Include directories
include_directories(
  include
)

# Dummy robot orchestrator executable
add_executable(dummy_robot_main
  src/dummy_robot_main.cpp
  src/dummy_robot_orchestrator.cpp
  src/bt_nodes/log_message_action.cpp
)

ament_target_dependencies(dummy_robot_main ${dependencies})

# Install executables
install(TARGETS
  dummy_robot_main
  DESTINATION lib/${PROJECT_NAME}
)

# Install launch files
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

# Install behavior tree files
install(DIRECTORY
  behaviors
  DESTINATION share/${PROJECT_NAME}
)

# Install include directory
install(DIRECTORY include/
  DESTINATION include/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
