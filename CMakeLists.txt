cmake_minimum_required(VERSION 3.20)

set(BOARD blackpill_f401cc)

find_package(Zephyr)
project(imu_logger)

target_include_directories(app PRIVATE ${PROJECT_SOURCE_DIR}/src/modules/imu/include)
target_include_directories(app PRIVATE ${PROJECT_SOURCE_DIR}/src/modules/common/include)

file(GLOB IMU_SRCS ${PROJECT_SOURCE_DIR}/src/modules/imu/*.c)
file(GLOB COMMON_SRCS ${PROJECT_SOURCE_DIR}/src/modules/common/*.c)

target_sources(app PRIVATE src/main.c ${IMU_SRCS} ${COMMON_SRCS})

