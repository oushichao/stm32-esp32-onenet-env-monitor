# 收集 src 文件夹内的所有 .c 源文件
FILE(GLOB_RECURSE src_files ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

# 收集 根目录/iot 文件夹内的所有源文件
FILE(GLOB_RECURSE iot_files ${CMAKE_CURRENT_SOURCE_DIR}/../iot/*.*)

# 合并所有文件
set(APP_SRCS ${src_files} ${iot_files})

# 注册编译 + 正确包含头文件路径
idf_component_register(
    SRCS ${APP_SRCS}
    INCLUDE_DIRS ../iot  # 退回根目录找 iot 文件夹
)
# FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

# idf_component_register(SRCS ${app_sources})