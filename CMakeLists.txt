cmake_minimum_required(VERSION 3.1)
project(iscore_addon_csp LANGUAGES CXX)

iscore_common_setup()

# Packages
find_package(Qt5 5.3 REQUIRED COMPONENTS Core)

# Files & main target
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/kiwi")

set(HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/CSPApplicationPlugin.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/CSPDisplacementPolicy.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/CSPDocumentPlugin.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/CSPFlexDisplacementPolicy.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/CSPScenario.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/CSPTimeNode.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/CSPTimeRelation.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/tools/CSPConstraintHolder.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/MoveEventCSPFactory.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/MoveEventCSPFlexFactory.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/iscore_addon_csp.hpp"

)
set(SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/CSPApplicationPlugin.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/CSPDisplacementPolicy.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/CSPDocumentPlugin.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/CSPFlexDisplacementPolicy.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/CSPScenario.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/CSPTimeNode.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/Model/CSPTimeRelation.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/MoveEventCSPFactory.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/CSP/MoveEventCSPFlexFactory.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/iscore_addon_csp.cpp"
)

add_library(${PROJECT_NAME} ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE
                        Qt5::Core
                        iscore_lib_base iscore_lib_process iscore_plugin_scenario)

setup_iscore_plugin(${PROJECT_NAME})
