/*******************************************************************************
 * Copyright (C) 2021 - 2026, winsoft666, <winsoft666@outlook.com>.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Expect bugs
 *
 * Please use and enjoy. Please let me know of any bugs/improvements
 * that you have found/implemented and I will fix/incorporate them into this
 * file.
 *******************************************************************************/

#ifndef AKALI_ARCH_HPP__
#define AKALI_ARCH_HPP__

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define AKALI_WIN 1
#ifdef _WIN64
#define AKALI_WIN64 1
#else
#define AKALI_WIN32 1
#endif
#elif defined(__APPLE__)
#define AKALI_MACOS 1
#include <TargetConditionals.h>
#if defined(TARGET_IPHONE_SIMULATOR)
#elif defined(TARGET_OS_IPHONE)
#elif defined(TARGET_OS_MAC)
#else
#error "Unknown Apple platform"
#endif
#elif defined(__linux__) || defined(linux) || defined(__linux)
#define AKALI_LINUX 1
#elif defined(__unix__) || defined(unix) || defined(__unix)
#define AKALI_UNIX 1
#elif defined(_POSIX_VERSION)
#define AKALI_POSIX 1
#else
#error "Unknown compiler"
#endif

// Processor architecture detection.
// For more info on what's defined, see:
//   http://msdn.microsoft.com/en-us/library/b0084kay.aspx
//   http://www.agner.org/optimize/calling_conventions.pdf
//   or with gcc, run: "echo | gcc -E -dM -"
#if defined(_M_X64) || defined(__x86_64__)
#define AKALI_ARCH_X86_FAMILY 1
#define AKALI_ARCH_X86_64 1
#define AKALI_ARCH_64_BITS 1
#define AKALI_ARCH_LITTLE_ENDIAN 1
#elif defined(_M_ARM64) || defined(__aarch64__)
#define AKALI_ARCH_ARM_FAMILY 1
#define AKALI_ARCH_64_BITS 1
#define AKALI_ARCH_LITTLE_ENDIAN 1
#elif defined(_M_IX86) || defined(__i386__)
#define AKALI_ARCH_X86_FAMILY 1
#define AKALI_ARCH_X86 1
#define AKALI_ARCH_32_BITS 1
#define AKALI_ARCH_LITTLE_ENDIAN 1
#elif defined(__ARMEL__)
#define AKALI_ARCH_ARM_FAMILY 1
#define AKALI_ARCH_32_BITS 1
#define AKALI_ARCH_LITTLE_ENDIAN 1
#elif defined(__MIPSEL__)
#define AKALI_ARCH_MIPS_FAMILY 1
#if defined(__LP64__)
#define AKALI_ARCH_64_BITS 1
#else
#define AKALI_ARCH_32_BITS 1
#endif
#define AKALI_ARCH_LITTLE_ENDIAN 1
#elif defined(__pnacl__)
#define AKALI_ARCH_32_BITS 1
#define AKALI_ARCH_LITTLE_ENDIAN 1
#elif defined(__EMSCRIPTEN__)
#define AKALI_ARCH_32_BITS 1
#define AKALI_ARCH_LITTLE_ENDIAN 1
#else
#error Please add support for your architecture in rtc_base/system/arch.h
#endif

#if !(defined(AKALI_ARCH_LITTLE_ENDIAN) ^ defined(AKALI_ARCH_BIG_ENDIAN))
#error Define either AKALI_ARCH_LITTLE_ENDIAN or AKALI_ARCH_BIG_ENDIAN
#endif

namespace akali_hpp {
class Arch {
   public:
    static bool IsHostBigEndian() {
        int nl = 0x12345678;
        short ns = 0x1234;
        bool big_endian = false;

        unsigned char* p = (unsigned char*)(&nl);
        unsigned char* sp = (unsigned char*)(&ns);

        if (p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78) {
            big_endian = true;
        }
        else if (p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12) {
            big_endian = false;
        }
        else {
            big_endian = (*sp != 0x12);
        }

        return big_endian;
    }
};
}  // namespace akali_hpp

#endif  // !AKALI_ARCH_HPP__