# stm32-bluepill-toolchain.cmake
# CMake toolchain file for cross-compiling to STM32F103C8T6 (Blue Pill).
#
# Usage:
#   cmake -B build -DCMAKE_TOOLCHAIN_FILE=../stm32-bluepill-toolchain.cmake \
#         -DCMAKE_BUILD_TYPE=MinSizeRel
#   cmake --build build
#
# Prerequisites (Debian/Ubuntu):
#   sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi

# ── System / architecture ─────────────────────────────────────────────────────
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# ── Cross-compiler executables ────────────────────────────────────────────────
find_program(ARM_CC      arm-none-eabi-gcc     REQUIRED)
find_program(ARM_CXX     arm-none-eabi-g++     REQUIRED)
find_program(ARM_AR      arm-none-eabi-ar      REQUIRED)
find_program(ARM_OBJCOPY arm-none-eabi-objcopy REQUIRED)
find_program(ARM_SIZE    arm-none-eabi-size    REQUIRED)

set(CMAKE_C_COMPILER   "${ARM_CC}")
set(CMAKE_CXX_COMPILER "${ARM_CXX}")
set(CMAKE_AR           "${ARM_AR}")

# Prevent CMake from testing the cross-compiler with a host executable.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ── Target MCU ────────────────────────────────────────────────────────────────
# STM32F103C8T6: ARM Cortex-M3, no hardware FPU.
set(STM32_CPU_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft")

# ── Compiler flags ────────────────────────────────────────────────────────────
# HSE_VALUE must match the crystal fitted to the board (Blue Pill: 8 MHz).
set(STM32_COMMON_FLAGS
  "${STM32_CPU_FLAGS} -DSTM32F103xB -DHSE_VALUE=8000000UL \
-Os -ffunction-sections -fdata-sections -fno-exceptions"
)

set(CMAKE_C_FLAGS_INIT   "${STM32_COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${STM32_COMMON_FLAGS} -fno-rtti -std=c++17")

# ── Linker flags ──────────────────────────────────────────────────────────────
# nosys.specs  – stub out OS syscalls (bare metal, no OS).
# nano.specs   – link against newlib-nano for a smaller footprint.
set(CMAKE_EXE_LINKER_FLAGS_INIT
  "${STM32_CPU_FLAGS} -Wl,--gc-sections -specs=nosys.specs -specs=nano.specs"
)

# ── Post-build: generate .hex and .bin files ready for flashing ───────────────
# Call this macro after add_executable() with the path to the linker script.
macro(stm32_generate_bin TARGET LINKER_SCRIPT)
  target_link_options("${TARGET}" PRIVATE
    "-T${LINKER_SCRIPT}"
    "-Wl,-Map=$<TARGET_FILE_DIR:${TARGET}>/${TARGET}.map"
  )
  add_custom_command(TARGET "${TARGET}" POST_BUILD
    COMMAND "${ARM_OBJCOPY}" -O ihex
            "$<TARGET_FILE:${TARGET}>"
            "$<TARGET_FILE_DIR:${TARGET}>/${TARGET}.hex"
    COMMENT "Generating HEX for ${TARGET}"
  )
  add_custom_command(TARGET "${TARGET}" POST_BUILD
    COMMAND "${ARM_OBJCOPY}" -O binary
            "$<TARGET_FILE:${TARGET}>"
            "$<TARGET_FILE_DIR:${TARGET}>/${TARGET}.bin"
    COMMENT "Generating BIN for ${TARGET}"
  )
  add_custom_command(TARGET "${TARGET}" POST_BUILD
    COMMAND "${ARM_SIZE}" "$<TARGET_FILE:${TARGET}>"
    COMMENT "Flash / RAM usage for ${TARGET}"
  )
endmacro()
