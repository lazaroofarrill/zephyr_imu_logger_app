cmake_minimum_required(VERSION 3.20)

find_package(Zephyr)
project(imu_logger)

target_include_directories(app PRIVATE src/modules/include src/modules/common/include)

target_sources(app PRIVATE src/main.c src/modules/icm20948.c src/modules/common/i2c_utils.c)

