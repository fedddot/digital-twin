# avr-toolchain.cmake
# CMake toolchain file for cross-compiling to AVR (Arduino Nano / ATmega328P).
#
# Usage:
#   cmake .. -DCMAKE_TOOLCHAIN_FILE=../avr-toolchain.cmake \
#            -DF_CPU=16000000UL \
#            -DCMAKE_BUILD_TYPE=MinSizeRel

# ── System / architecture ─────────────────────────────────────────────────────
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

# ── Cross-compiler executables ────────────────────────────────────────────────
# Install on Debian/Ubuntu:  sudo apt install gcc-avr binutils-avr avr-libc
find_program(AVR_CC  avr-gcc  REQUIRED)
find_program(AVR_CXX avr-g++  REQUIRED)
find_program(AVR_AR  avr-ar   REQUIRED)
find_program(AVR_OBJCOPY avr-objcopy REQUIRED)
find_program(AVR_SIZE    avr-size    REQUIRED)

set(CMAKE_C_COMPILER   "${AVR_CC}")
set(CMAKE_CXX_COMPILER "${AVR_CXX}")
set(CMAKE_AR           "${AVR_AR}")

# Prevent CMake from testing the cross-compiler with a host executable.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ── Target MCU ────────────────────────────────────────────────────────────────
# Arduino Nano uses the ATmega328P running at 16 MHz.
set(AVR_MCU "atmega328p" CACHE STRING "Target AVR MCU")
if(NOT DEFINED F_CPU)
  set(F_CPU "16000000UL" CACHE STRING "MCU clock frequency in Hz")
endif()

# ── Compiler flags ────────────────────────────────────────────────────────────
set(AVR_COMMON_FLAGS "-mmcu=${AVR_MCU} -DF_CPU=${F_CPU} -Os -ffunction-sections -fdata-sections -fno-exceptions")

set(CMAKE_C_FLAGS_INIT   "${AVR_COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${AVR_COMMON_FLAGS} -fno-rtti -std=c++17")

# ── Linker flags ──────────────────────────────────────────────────────────────
# -Wl,--gc-sections removes unused code/data sections to minimise flash usage.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-mmcu=${AVR_MCU} -Wl,--gc-sections")

# ── Post-build: generate Intel HEX file for avrdude ──────────────────────────
# Call this macro after add_executable() to create <target>.hex alongside the ELF.
macro(avr_generate_hex TARGET)
  add_custom_command(TARGET "${TARGET}" POST_BUILD
    COMMAND "${AVR_OBJCOPY}" -O ihex -R .eeprom
            "$<TARGET_FILE:${TARGET}>"
            "$<TARGET_FILE_DIR:${TARGET}>/${TARGET}.hex"
    COMMENT "Generating HEX file for ${TARGET}"
  )
  add_custom_command(TARGET "${TARGET}" POST_BUILD
    COMMAND "${AVR_SIZE}" --format=avr --mcu=${AVR_MCU} "$<TARGET_FILE:${TARGET}>"
    COMMENT "Flash / RAM usage for ${TARGET}"
  )
endmacro()
