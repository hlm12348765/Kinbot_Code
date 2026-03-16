find_package(Protobuf CONFIG QUIET)
if(NOT Protobuf_FOUND)
  find_package(Protobuf REQUIRED)
endif()

# 收集所有 .proto 文件
file(GLOB_RECURSE PROTO_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/kinbot/*.proto"
)

# 将 contracts/ 作为 proto import 根目录，保证
# import "kinbot/common/v1/common.proto" 这类路径在独立子目录构建时也能解析。
add_library(kinbot_contracts)

protobuf_generate(
  TARGET kinbot_contracts
  LANGUAGE cpp
  PROTOS ${PROTO_FILES}
  IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
  PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(kinbot_contracts PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Protobuf_INCLUDE_DIRS}
)

if(TARGET protobuf::libprotobuf)
  target_link_libraries(kinbot_contracts PUBLIC protobuf::libprotobuf)
else()
  target_link_libraries(kinbot_contracts PUBLIC ${Protobuf_LIBRARIES})
endif()
