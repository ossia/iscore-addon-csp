cmake_minimum_required(VERSION 3.1)
project(iscore_addon_csp LANGUAGES CXX)

iscore_common_setup()

# Packages
find_package(Qt5 5.3 REQUIRED COMPONENTS Core)

# Files & main target
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/kiwi")

set(HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/ApplicationPlugin.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/DisplacementComputer.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/DocumentPlugin.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Policies/FlexDisplacementPolicy.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Policies/DisplacementPolicy.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Policies/ExecutionPolicy.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/Scenario.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/TimeNode.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/TimeRelation.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/tools/ConstraintHolder.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/MoveEventCSPFactory.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/MoveEventCSPFlexFactory.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/iscore_addon_csp.hpp"

)
set(SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/ApplicationPlugin.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/DisplacementComputer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/DocumentPlugin.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Policies/FlexDisplacementPolicy.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Policies/DisplacementPolicy.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Policies/ExecutionPolicy.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/Scenario.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/TimeNode.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/TimeRelation.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/MoveEventCSPFactory.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/MoveEventCSPFlexFactory.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/iscore_addon_csp.cpp"
)

add_library(${PROJECT_NAME} ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PUBLIC
                        Qt5::Core
                        iscore_lib_base iscore_lib_process iscore_plugin_scenario)

setup_iscore_plugin(${PROJECT_NAME})
