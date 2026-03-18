find_package(Protobuf REQUIRED)

# 收集所有 .proto 文件
file(GLOB_RECURSE PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/kinbot/*.proto")

# 设置 proto import 搜索路径，使 import "kinbot/..." 能正确解析
set(Protobuf_IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")

# 生成 C++ 绑定
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(kinbot_contracts ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(kinbot_contracts PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Protobuf_INCLUDE_DIRS}
)
target_link_libraries(kinbot_contracts PUBLIC ${Protobuf_LIBRARIES})
