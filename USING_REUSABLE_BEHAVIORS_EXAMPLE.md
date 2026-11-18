# Example: Using Reusable Behaviors in dummy_robot

This example demonstrates how to use reusable behaviors from the `koch2` catalogue in your own robot package.

## What This Example Shows

The `dummy_mission_with_follow.yaml` config demonstrates:

1. **Loading your own behaviors** from your package
2. **Loading reusable behaviors** from `koch2` package
3. **Using per-behavior package specification** to mix behaviors from different packages

## Configuration

See [config/dummy_mission_with_follow.yaml](config/dummy_mission_with_follow.yaml):

```yaml
# Global package for dummy_robot's own behaviors
package_name: "dummy_robot"

behaviors:
  # Your robot's behaviors (uses global package_name)
  - name: "state1_runner"
    behavior_file: "behaviors/dummy_state1.xml"
  
  - name: "state2_runner"
    behavior_file: "behaviors/dummy_state2.xml"
  
  # Reusable behavior from koch2
  - name: "follow_runner"
    behavior_file: "behaviors/reusable/follow_behavior.xml"
    package_name: "koch2"  # Load from different package!
```

## Key Points

✅ **No copying needed** - The follow_behavior.xml stays in koch2  
✅ **Per-behavior package** - Each behavior can specify its source package  
✅ **Single source of truth** - Updates to follow_behavior benefit all users  
✅ **Clean separation** - Your behaviors vs. shared behaviors

## Running the Example

```bash
# Build both packages
cd ~/social_nao_ws
colcon build --packages-select koch2 dummy_robot
source install/setup.bash

# Run with the reusable follow behavior
ros2 launch dummy_robot dummy_robot_with_follow.launch.py
```

## Using in Your Orchestrator

To actually use the `follow_runner` in your state machine, modify your orchestrator:

```cpp
// In dummy_robot_orchestrator.cpp
case State::STATE_2:
  if (check_behavior_finished()) {
    if (last_status_ == "SUCCESS") {
      // Instead of going to STOP, go to FOLLOW state
      go_to_state(static_cast<int>(State::FOLLOW));
    }
  }
  break;

case State::FOLLOW:
  if (check_behavior_finished()) {
    RCLCPP_INFO(get_logger(), "Follow behavior finished");
    go_to_state(static_cast<int>(State::STOP));
  }
  break;
```

And in `go_to_state()`:

```cpp
case State::FOLLOW:
  RCLCPP_INFO(get_logger(), "Transitioning to FOLLOW");
  deactivate_all_runners();
  activate_runner("follow_runner");  // Activate the reusable behavior
  break;
```

## Benefits of This Approach

1. **Reuse tested behaviors** - Don't reinvent the wheel
2. **Consistent behavior** - Same follow logic across all robots
3. **Easy updates** - Fix bugs once, benefit everywhere
4. **Mix and match** - Combine reusable behaviors with custom ones

## See Also

- [koch2 behaviors catalogue](../../koch2/behaviors/README.md)
- [Using Reusable Behaviors Guide](../../koch2/docs/USING_REUSABLE_BEHAVIORS.md)
- [koch2 README](../../koch2/README.md)
