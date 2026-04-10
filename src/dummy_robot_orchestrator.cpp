// Copyright 2025 Your Name
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dummy_robot/dummy_robot_orchestrator.hpp"
#include "behavior_architecture/behavior_config.hpp"
#include "behavior_architecture/behavior_runner.hpp"
#include "behavior_architecture/orchestrator_factory.hpp"
#include "dummy_robot/bt_nodes/log_message_action.hpp"

namespace dummy_robot
{

// Register this orchestrator with the factory
static behavior_architecture::OrchestratorRegistrar<DummyRobotOrchestrator> 
  dummy_robot_registrar("dummy_robot");

DummyRobotOrchestrator::DummyRobotOrchestrator(BT::Blackboard::Ptr blackboard)
: BaseOrchestrator("dummy_robot_orchestrator", blackboard),
  state_(State::INIT)
{
  RCLCPP_INFO(get_logger(), "DummyRobotOrchestrator created");
}

rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
DummyRobotOrchestrator::on_configure(const rclcpp_lifecycle::State & state)
{
  if (runners_.empty()) {
    auto registrar = [](BT::BehaviorTreeFactory & factory) {
      factory.registerNodeType<bt_nodes::LogMessageAction>("LogMessage");
    };

    // Prefer config from blackboard (set by mission_executor or main)
    std::vector<behavior_architecture::BehaviorConfig> behaviors;
    std::vector<std::string> plugins;
    std::string pkg_name = "dummy_robot";

    if (!blackboard_->get("behaviors_config", behaviors) ||
      !blackboard_->get("plugin_libraries", plugins))
    {
      // Standalone fallback — hardcoded defaults
      behaviors = {
        {"state1_runner", "behaviors/dummy_state1.xml", "dummy_robot", 50},
        {"state2_runner", "behaviors/dummy_state2.xml", "dummy_robot", 50},
      };
    } else {
      std::ignore = blackboard_->get("package_name", pkg_name);
    }

    for (auto & bc : behaviors) {
      auto runner = std::make_shared<behavior_architecture::BehaviorRunner>(
        blackboard_, bc.name, bc.behavior_file, plugins,
        bc.package_name.empty() ? pkg_name : bc.package_name,
        bc.control_period_ms, registrar);
      register_runner(bc.name, runner);
    }
  }
  return BaseOrchestrator::on_configure(state);
}

void DummyRobotOrchestrator::control_cycle()
{
  switch (state_) {
    case State::INIT:
      RCLCPP_INFO(get_logger(), "State: INIT");
      go_to_state(static_cast<int>(State::STATE_1));
      break;

    case State::STATE_1:
      if (check_behavior_finished()) {
        RCLCPP_INFO(get_logger(), "State 1 behavior finished with status: %s", 
          last_status_.c_str());
        if (last_status_ == "SUCCESS") {
          go_to_state(static_cast<int>(State::STATE_2));
        } else {
          RCLCPP_WARN(get_logger(), "State 1 failed, retrying...");
          go_to_state(static_cast<int>(State::STATE_1));
        }
      }
      break;

    case State::STATE_2:
      if (check_behavior_finished()) {
        RCLCPP_INFO(get_logger(), "State 2 behavior finished with status: %s", 
          last_status_.c_str());
        if (last_status_ == "SUCCESS") {
          go_to_state(static_cast<int>(State::STOP));
        } else {
          RCLCPP_WARN(get_logger(), "State 2 failed, retrying...");
          go_to_state(static_cast<int>(State::STATE_2));
        }
      }
      break;

    case State::STOP:
      RCLCPP_INFO_ONCE(get_logger(), "State: STOP - All behaviors completed");
      break;
  }
}

void DummyRobotOrchestrator::go_to_state(int state)
{
  state_ = static_cast<State>(state);

  switch (state_) {
    case State::INIT:
      RCLCPP_INFO(get_logger(), "Transitioning to INIT");
      deactivate_all_runners();
      break;

    case State::STATE_1:
      RCLCPP_INFO(get_logger(), "Transitioning to STATE_1");
      deactivate_all_runners();
      activate_runner("state1_runner");
      break;

    case State::STATE_2:
      RCLCPP_INFO(get_logger(), "Transitioning to STATE_2");
      deactivate_all_runners();
      activate_runner("state2_runner");
      break;

    case State::STOP:
      RCLCPP_INFO(get_logger(), "Transitioning to STOP");
      deactivate_all_runners();
      break;
  }
}

}  // namespace dummy_robot
