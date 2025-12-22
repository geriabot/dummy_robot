# Dummy Robot

**Reference example package demonstrating the usage of the `behavior_architecture` framework with YAML-configured action executor.**

> **Note**: This package serves as a reference implementation for developers who want to use `behavior_architecture` in their own robot projects. It provides a complete, working example of how to structure your package, create orchestrators, define behavior trees, implement custom BT nodes, and configure everything via YAML.

## Overview

This package implements a simple two-state behavior system for a dummy robot using the behavior_architecture framework. It showcases the recommended approach: using the generic `action_executor` with YAML configuration instead of writing custom main() programs.

## Package Structure

```
dummy_robot/
├── behaviors/                  # Behavior tree XML files
│   ├── dummy_state1.xml       # First state behavior
│   └── dummy_state2.xml       # Second state behavior
├── config/                     # YAML configuration
│   └── dummy_robot_config.yaml
├── include/
│   └── dummy_robot/
│       ├── bt_nodes/                      # Custom BehaviorTree nodes
│       │   └── log_message_action.hpp
│       └── dummy_robot_orchestrator.hpp   # FSM orchestrator
├── src/
│   ├── bt_nodes/
│   │   ├── bt_plugins.cpp                 # BT plugin registration
│   │   └── log_message_action.cpp         # Custom BT node
│   └── dummy_robot_orchestrator.cpp       # FSM orchestrator implementation
├── launch/
│   └── dummy_robot_action_executor.launch.py
├── CMakeLists.txt
└── package.xml
```

## Dependencies

- `behavior_architecture`: Provides BaseOrchestrator and BehaviorRunner classes
- `social_bt_nodes`: Behavior tree node plugins (Speak, etc.)
- `rclcpp_cascade_lifecycle`: Lifecycle management
- `behaviortree_cpp`: BehaviorTree.CPP library

## Getting Started

### Clone and Setup Dependencies

```bash
# Create a workspace
mkdir -p ~/my_robot_ws/src
cd ~/my_robot_ws/src

# Clone this package
git clone <your-repo-url> dummy_robot

# Import third-party dependencies
vcs import < dummy_robot/thirdparty.repos

# The behavior_architecture package has its own dependencies
vcs import < behavior_architecture/thirdparty.repos
```

## Building

```bash
cd ~/my_robot_ws

# Install dependencies
rosdep install --from-paths src --ignore-src -r -y

# Build the workspace
colcon build --symlink-install

# Source the workspace
source install/setup.bash
```

```bash
ros2 launch dummy_robot dummy_robot_action_executor.launch.py
```

The system will:
1. Load the orchestrator library dynamically
2. Load custom BT node plugins
3. Create behavior runners from YAML configuration
4. Execute the FSM: INIT → STATE_1 → STATE_2 → STOPbash
ros2 run dummy_robot dummy_robot_main
```

## Architecture

### Components

1. **DummyRobotOrchestrator**: FSM that manages state transitions
   - Inherits from `behavior_architecture::BaseOrchestrator`
   - Implements a simple 4-state FSM: INIT → STATE_1 → STATE_2 → STOP
   - Registered with factory for dynamic loading

2. **BehaviorRunners**: Execute behavior trees for each state
   - `state1_runner`: Executes dummy_state1.xml (includes custom LogMessage node)
   - `state2_runner`: Executes dummy_state2.xml
   - Created automatically by action_executor from YAML config

3. **Behavior Trees**: Define robot actions for each state
   - State 1: Logs custom message, speaks "Hola 1", logs completion
   - State 2: Speaks "Hola 2"

4. **Custom BT Nodes Plugin**: Example of creating custom BehaviorTree nodes
   - `LogMessage`: Simple action that logs a message to the console
   - Built as a separate plugin library (`libdummy_robot_bt_nodes.so`)
   - Loaded dynamically at runtime

5. **YAML Configuration**: Declarative system setup
   - Specifies orchestrator type and library
   - Lists BT plugin libraries to load
   - Defines behaviors with their XML files and control periods

### Execution Flow

```
INIT → Activate state1_runner
   Key Implementation Details

### 1. Orchestrator Registration

The orchestrator is registered with the factory for dynamic loading:

```cpp
// In dummy_robot_orchestrator.cpp
#include "behavior_architecture/orchestrator_factory.hpp"

static behavior_architecture::OrchestratorRegistrar<DummyRobotOrchestrator> 
  dummy_robot_registrar("dummy_robot");
```

### 2. Built as Shared Libraries

Both orchestrator and BT nodes are built as shared libraries:

```cmake
# Orchestrator library
add_library(${PROJECT_NAME}_orchestrator SHARED
  src/dummy_robot_orchestrator.cpp
)

# BT nodes plugin library
add_library(${PROJECT_NAME}_bt_nodes SHARED
  src/bt_nodes/bt_plugins.cpp
  src/bt_nodes/log_message_action.cpp
)

# Critical: Required for BT plugin export
target_compile_definitions(${PROJECT_NAME}_bt_nodes PRIVATE BT_PLUGIN_EXPORT)
```

### 3. BT Plugin Registration

Custom nodes are registered in a separate file:

```cpp
// src/bt_nodes/bt_plugins.cpp
#include "behaviortree_cpp/bt_factory.h"
#include "dummy_robot/bt_nodes/log_message_action.hpp"

BT_REGISTER_NODES(factory)
{
  factory.registerNodeType<dummy_robot::bt_nodes::LogMessageAction>("LogMessage");
}
```

### 4. YAML Configuration

Everything is configured declaratively in `config/dummy_robot_config.yaml`:

```yaml
node_name: "dummy_robot_bt_node"
orchestrator_type: "dummy_robot"  # Matches registration name
package_name: "dummy_robot"

# Load orchestrator library first
orchestrator_libraries:
  - "libdummy_robot_orchestrator.so"

# Load BT plugin libraries
plugin_libraries:
  - "libsocial_bt_nodes_plugin.so"
  - "libdummy_robot_bt_nodes.so"

# Define behaviors
behaviors:
  - name: "state1_runner"
    behavior_file: "behaviors/dummy_state1.xml"
    control_period_ms: 50
  - name: "state2_runner"
    behavior_file: "behaviors/dummy_state2.xml"
    control_period_ms: 50
```

### 5. Launch File

Simple launch file that uses the generic action_executor:

```python
# launch/dummy_robot_action_executor.launch.py
from launch import LaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    config_file = PathJoinSubstitution([
        FindPackageShare('dummy_robot'),
        'config',
        'dummy_robot_config.yaml'
    ])
    
    return LaunchDescription([
        Node(
            package='behavior_architecture',
            executable='action_executor',  # Generic executable
            output='screen',
            emulate_tty=True,
            arguments=[config_file]
        )
    ])
```

## Customization

### Adding New States

1. Create new behavior tree XML file in `behaviors/`
2. Add new state to `DummyRobotOrchestrator::State` enum
3. Update `control_cycle()` with new state logic
4. Update `go_to_state()` to handle new state activation
5. Add behavior to YAML config

### Creating Custom BT Nodes

Follow this example structure

## Customization

### Creating Custom BT Nodes

The package includes an example of creating custom BehaviorTree nodes in the `bt_nodes` folder:

1. **Define the node class** (see [log_message_action.hpp](include/dummy_robot/bt_nodes/log_message_action.hpp))
   ```cpp
   namespace dummy_robot {
   namespace bt_nodes {
   
   class LogMessageAction : public BT::SyncActionNode {
     static BT::PortsList providedPorts();
     BT::NodeStatus tick() override;
   };
   
   }  // namespace bt_nodes
   }  // namespace dummy_robot
   ```

2. **Implement the node** (see [log_message_action.cpp](src/bt_nodes/log_message_action.cpp))
   ```cpp
   BT::NodeStatus LogMessageAction::tick() {
     auto message = getInput<std::string>("message");
     RCLCPP_INFO(node_->get_logger(), "LogMessageAction: %s", message.value().c_str());
     return BT::NodeStatus::SUCCESS;
   }
   ```

3. **Register the node** in main.cpp using a lambda:
   ```cpp
   auto register_custom_nodes = [](BT::BehaviorTreeFactory& factory) {
     factory.registerNodeType<dummy_robot::bt_nodes::LogMessageAction>("LogMessage");
   };
   
   auto runner = std::make_shared<behavior_architecture::BehaviorRunner>(
     blackboard, "runner_name", "path.xml", plugins, 
     "package_name", 50, register_custom_nodes
   );
   ```

4. **Use in XML** behavior tree files:
   ```xml
   <LogMessage message="Your custom message here"/>
   ```

### Adding New States

1. Create new behavior tree XML file in `behaviors/`
2. Add new state to `DummyRobotOrchestrator::State` enum
3. Update `control_cycle()` with new state logic
4. Update `go_to_state()` to handle new state activation

### Modifying Behaviors

Edit the XML files in `behaviors/` to change robot actions. You can use any BehaviorTree nodes provided by `social_bt_nodes`.

### Adjusting Control Cycle Period

Modify the last parameter when creating BehaviorRunner instances in `dummy_robot_main.cpp`:

```cpp
auto state1_runner = std::make_shared<behavior_architecture::BehaviorRunner>(
  blackboard, "state1_runner", "behaviors/dummy_state1.xml", 
  plugins, "dummy_robot",
  100  // Control cycle period in milliseconds
);
```

## License

Apache-2.0
