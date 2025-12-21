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

#ifndef DUMMY_ROBOT__DUMMY_ROBOT_ORCHESTRATOR_HPP_
#define DUMMY_ROBOT__DUMMY_ROBOT_ORCHESTRATOR_HPP_

#include "behavior_architecture/base_orchestrator.hpp"

namespace dummy_robot
{

/**
 * @brief Dummy robot orchestrator - manages robot behavior states
 * 
 * Simple finite state machine with two states that execute different behaviors
 */
class DummyRobotOrchestrator : public behavior_architecture::BaseOrchestrator
{
public:
  /**
   * @brief Constructor
   * @param blackboard Shared blackboard for communication
   */
  explicit DummyRobotOrchestrator(BT::Blackboard::Ptr blackboard);

protected:
  /**
   * @brief Main control cycle - implements FSM logic
   */
  void control_cycle() override;

  /**
   * @brief Transition to a specific state
   * @param state State identifier
   */
  void go_to_state(int state);

private:
  enum class State {
    INIT,
    STATE_1,
    STATE_2,
    STOP
  };

  State state_;
};

}  // namespace dummy_robot

#endif  // DUMMY_ROBOT__DUMMY_ROBOT_ORCHESTRATOR_HPP_
