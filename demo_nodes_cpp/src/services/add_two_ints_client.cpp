// Copyright 2014 Open Source Robotics Foundation, Inc.
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

#include <chrono>
#include <iostream>
#include <memory>

#include "rclcpp/rclcpp.hpp"

#include "rcl/introspection.h"


#include "example_interfaces/srv/add_two_ints.hpp"

using namespace std::chrono_literals;

// TODO(wjwwood): make this into a method of rclcpp::Client.
example_interfaces::srv::AddTwoInts::Response::SharedPtr send_request(
  rclcpp::Node::SharedPtr node,
  rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedPtr client,
  example_interfaces::srv::AddTwoInts::Request::SharedPtr request)
{
  auto result = client->async_send_request(request);
  // Wait for the result.
  if (rclcpp::spin_until_future_complete(node, result) ==
    rclcpp::FutureReturnCode::SUCCESS)
  {
    return result.get();
  } else {
    return NULL;
  }
}

int main(int argc, char ** argv)
{
  // Force flush of the stdout buffer.
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  rclcpp::init(argc, argv);

  auto node = rclcpp::Node::make_shared("add_two_ints_client");

  auto client = node->create_client<example_interfaces::srv::AddTwoInts>("add_two_ints");

  auto request = std::make_shared<example_interfaces::srv::AddTwoInts::Request>();
  request->a = 2;
  request->b = 3;

  while (!client->wait_for_service(1s)) {
    if (!rclcpp::ok()) {
      RCLCPP_ERROR(node->get_logger(), "Interrupted while waiting for the service. Exiting.");
      return 0;
    }
    RCLCPP_INFO(node->get_logger(), "service not available, waiting again...");
  }

  // TODO(wjwwood): make it like `client->send_request(node, request)->sum`
  // TODO(wjwwood): consider error condition

  int i = 0;

  while (true) {
    i ++;

    auto result = send_request(node, client, request);
    if (result) {
      RCLCPP_INFO_STREAM(node->get_logger(), "Result of add_two_ints: " << result->sum);
    } else {
      RCLCPP_ERROR(node->get_logger(), "Interrupted while waiting for response. Exiting.");
    }

    // rcl_client_t * rcl_client  = client->get_client_handle().get();
    std::shared_ptr<rcl_client_t> rcl_client_ptr = client->get_client_handle();
    rcl_node_t * node_ptr = node->get_node_base_interface()->get_rcl_node_handle();

    // test enable/disable service events & content
    if (i%2 == 0) {
      // rcl_service_introspection_enable_client_content(rcl_client_ptr.get());
      rcl_service_introspection_disable_client_events(rcl_client_ptr.get(), node_ptr);
      RCLCPP_INFO(node->get_logger(), "Disabled client entirely");
    } else {
      // rcl_service_introspection_disable_client_content(rcl_client_ptr.get());
      rcl_service_introspection_enable_client_events(rcl_client_ptr.get(), node_ptr);
      RCLCPP_INFO(node->get_logger(), "Enabled client entirely");
    }








    std::this_thread::sleep_for(1s);
  }

  rclcpp::shutdown();
  return 0;
}
