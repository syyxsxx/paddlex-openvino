# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if("/wangsiyuan06/PaddleX/deploy/openvino/deps/yaml-cpp.zip" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "/wangsiyuan06/PaddleX/deploy/openvino/deps/yaml-cpp.zip")
  message(FATAL_ERROR "File not found: /wangsiyuan06/PaddleX/deploy/openvino/deps/yaml-cpp.zip")
endif()

if("" STREQUAL "")
  message(WARNING "File will not be verified since no URL_HASH specified")
  return()
endif()

if("" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(STATUS "verifying file...
     file='/wangsiyuan06/PaddleX/deploy/openvino/deps/yaml-cpp.zip'")

file("" "/wangsiyuan06/PaddleX/deploy/openvino/deps/yaml-cpp.zip" actual_value)

if(NOT "${actual_value}" STREQUAL "")
  message(FATAL_ERROR "error:  hash of
  /wangsiyuan06/PaddleX/deploy/openvino/deps/yaml-cpp.zip
does not match expected value
  expected: ''
    actual: '${actual_value}'
")
endif()

message(STATUS "verifying file... done")
