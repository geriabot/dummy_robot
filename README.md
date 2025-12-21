# Dummy Robot

**Reference example package demonstrating the usage of the `behavior_architecture` framework.**

> **Note**: This package serves as a reference implementation for developers who want to use `behavior_architecture` in their own robot projects. It provides a complete, working example of how to structure your package, create orchestrators, define behavior trees, and implement custom BT nodes.

## Overview

This package implements a simple two-state behavior system for a dummy robot using the behavior_architecture framework. It showcases how to create a separate package that imports and uses the generic behavior architecture components.

## Package Structure

```
dummy_robot/
├── behaviors/               # Behavior tree XML files
│   ├── dummy_state1.xml    # First state behavior
│   └── dummy_state2.xml    # Second state behavior
├── include/
│   └── dummy_robot/
│       ├── bt_nodes/                      # Custom BehaviorTree nodes
│       │   └── log_message_action.hpp
│       └── dummy_robot_orchestrator.hpp
├── src/
│   ├── bt_nodes/                          # Custom BT node implementations
│   │   └── log_message_action.cpp
│   ├── dummy_robot_main.cpp          # Main executable
│   └── dummy_robot_orchestrator.cpp  # FSM orchestrator
├── launch/
│   └── dummy_robot.launch.py
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

## Running

### Using launch file:
```bash
ros2 launch dummy_robot dummy_robot.launch.py
```

### Direct execution:
```bash
ros2 run dummy_robot dummy_robot_main
```

## Architecture

### Components

1. **DummyRobotOrchestrator**: FSM that manages state transitions
   - Inherits from `behavior_architecture::BaseOrchestrator`
   - Implements a simple 4-state FSM: INIT → STATE_1 → STATE_2 → STOP

2. **BehaviorRunners**: Execute behavior trees for each state
   - `state1_runner`: Executes dummy_state1.xml (includes custom LogMessage node)
   - `state2_runner`: Executes dummy_state2.xml
   - Created using `behavior_architecture::BehaviorRunner`

3. **Behavior Trees**: Define robot actions for each state
   - State 1: Logs custom message, speaks "Hola 1", logs completion
   - State 2: Speaks "Hola 2"

4. **Custom BT Nodes**: Example of creating custom BehaviorTree nodes
   - `LogMessage`: Simple action that logs a message to the console
   - Demonstrates how to create custom nodes within your package
   - Shows registration via callback function

### Execution Flow

```
INIT → Activate state1_runner
         ↓ (on SUCCESS)
     STATE_1 → Activate state2_runner
         ↓ (on SUCCESS)
     STATE_2 → STOP
```

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
