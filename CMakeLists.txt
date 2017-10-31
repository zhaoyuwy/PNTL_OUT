cmake_minimum_required(VERSION 2.8)
project(PNTL_Agent)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES PNTL_agent/src/AgentCommon.h PNTL_agent/src/AgentJsonAPI.cpp PNTL_agent/src/AgentJsonAPI.h PNTL_agent/src/DetectWorker.cpp PNTL_agent/src/DetectWorker.h PNTL_agent/src/FileNotifier.cpp PNTL_agent/src/FileNotifier.h PNTL_agent/src/FlowManager.cpp PNTL_agent/src/FlowManager.h PNTL_agent/src/GetLocalCfg.cpp PNTL_agent/src/GetLocalCfg.h PNTL_agent/src/Log.cpp PNTL_agent/src/Log.h PNTL_agent/src/main.cpp PNTL_agent/src/MessagePlatform.cpp PNTL_agent/src/MessagePlatform.h PNTL_agent/src/Sal.cpp PNTL_agent/src/Sal.h PNTL_agent/src/ServerAntAgentCfg.cpp PNTL_agent/src/ServerAntAgentCfg.h PNTL_agent/src/ThreadClass.cpp PNTL_agent/src/ThreadClass.h PNTL_agent/src/common/common.cpp PNTL_agent/src/MessagePlatformClient.cpp PNTL_agent/src/MessagePlatformClient.h)
add_executable(PNTL_Agent ${SOURCE_FILES})

#Set link library.
TARGET_LINK_LIBRARIES(PNTL_Agent pthread curl)
