#!/usr/bin/env python3
# Copyright 2025 Your Name
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction, TimerAction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def launch_setup(context, *args, **kwargs):
    """Build the node list from the mission YAML config.

    This launcher is intentionally minimal and only starts:
        1. mission_executor
        2. start_mission (delayed)

    Planner and BT builder nodes are expected to be started separately.
    """
    mission_file = LaunchConfiguration('mission_file').perform(context)
    skills_file = LaunchConfiguration('skills_file').perform(context)
    save_exec_arg = LaunchConfiguration('save_exec').perform(context).lower() == 'true'

    # If it's just a filename, assume it's in dummy_robot/config
    if not os.path.isabs(mission_file):
        pkg_dir = get_package_share_directory('dummy_robot')
        mission_file = os.path.join(pkg_dir, 'config', mission_file)

    with open(mission_file, 'r') as f:
        config = yaml.safe_load(f)

    # Allow save_exec to be defined either via launch CLI or inside the YAML
    save_exec = save_exec_arg or config.get('save_exec', False)

    nodes = []

    # ── 1. Mission Executor ────────────────────────────────────────────────────
    executor_args = [mission_file]
    if save_exec:
        executor_args.append('--save-exec')
    nodes.append(Node(
        package='behavior_architecture',
        executable='mission_executor',
        name='mission_executor',
        output='screen',
        emulate_tty=True,
        arguments=executor_args,
    ))

    # ── 2. Goal sender ─────────────────────────────────────────────────────────
    nodes.append(TimerAction(
        period=3.0,
        actions=[Node(
            package='behavior_architecture',
            executable='start_mission',
            name='start_mission',
            output='screen',
            emulate_tty=True,
            parameters=[{
                'mission_file': mission_file,
                'skills_file': skills_file,
            }],
        )],
    ))

    return nodes


def generate_launch_description():
    """Launch mission_executor + start_mission using a mission YAML file.

    This launcher does not start planner/builder LLM nodes.
    """
    pkg_dir = get_package_share_directory('dummy_robot')
    default_config = os.path.join(pkg_dir, 'config', 'waiter_mission_checking.yaml')
    default_skills = os.path.join(pkg_dir, 'config', 'dummy_robot_skills.yaml')

    mission_file_arg = DeclareLaunchArgument(
        'mission_file',
        default_value=default_config,
        description=(
            'Path to the unified YAML mission file '
            '(orchestrator_type must be "llm"). Can be a filename in dummy_robot/config.'
        ),
    )

    skills_file_arg = DeclareLaunchArgument(
        'skills_file',
        default_value=default_skills,
        description='Path to the YAML file listing the robot skills (skills: [...]).',
    )

    save_exec_arg = DeclareLaunchArgument(
        'save_exec',
        default_value='false',
        description=(
            'Save generated BT XMLs to exec/<mission_name>/<timestamp>/. '
            'Set to "true" to enable execution logging.'
        ),
    )

    return LaunchDescription([
        mission_file_arg,
        skills_file_arg,
        save_exec_arg,
        OpaqueFunction(function=launch_setup),
    ])
