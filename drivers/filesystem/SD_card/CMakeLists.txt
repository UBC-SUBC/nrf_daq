zephyr_library()
zephyr_library_sources(disc_operations.c)

zephyr_include_directories(${ZEPHYR_BASE}/subsys/fs/fat_fs)
message(STATUS "ZEPHYR_BASE is set to: ${ZEPHYR_BASE}")