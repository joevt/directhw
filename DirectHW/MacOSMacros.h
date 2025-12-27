#ifndef __MACOSMACROS_HPP__
#define __MACOSMACROS_HPP__

/* DirectHW - Kernel extension to pass through IO commands to user space
 *
 * Copyright © 2008-2010 coresystems GmbH <info@coresystems.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef MAC_OS_X_VERSION_SDK
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_SDK
#else
#include <TargetConditionals.h>
#include <AvailabilityMacros.h>
#endif

#ifndef MAC_OS_X_VERSION_10_3
#define MAC_OS_X_VERSION_10_3 1030
#endif
#ifndef MAC_OS_X_VERSION_10_4
#define MAC_OS_X_VERSION_10_4 1040
#endif
#ifndef MAC_OS_X_VERSION_10_5
#define MAC_OS_X_VERSION_10_5 1050
#endif
#ifndef MAC_OS_X_VERSION_10_6
#define MAC_OS_X_VERSION_10_6 1060
#endif
#ifndef MAC_OS_X_VERSION_10_7
#define MAC_OS_X_VERSION_10_7 1070
#endif
#ifndef MAC_OS_X_VERSION_10_12
#define MAC_OS_X_VERSION_10_12 101200
#endif

#ifdef AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER
    #undef MAC_OS_X_VERSION_SDK
    #define MAC_OS_X_VERSION_SDK MAC_OS_X_VERSION_10_4
#endif

#if defined(AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER) && defined(TARGET_OS_EMBEDDED)
    #undef MAC_OS_X_VERSION_SDK
    #define MAC_OS_X_VERSION_SDK MAC_OS_X_VERSION_10_5
#endif

#ifdef AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER
    #undef MAC_OS_X_VERSION_SDK
    #define MAC_OS_X_VERSION_SDK MAC_OS_X_VERSION_10_6
#endif

#ifdef AVAILABLE_MAC_OS_X_VERSION_10_7_AND_LATER
    #undef MAC_OS_X_VERSION_SDK
    #define MAC_OS_X_VERSION_SDK MAC_OS_X_VERSION_10_7
#endif

#if !defined(MAC_OS_X_VERSION_MAX_ALLOWED) || !defined(MAC_OS_X_VERSION_SDK)
    #error missing #include AvailabilityMacros.h
#endif

#if defined(__ppc64__)
    #warning ppc64
#elif defined(__ppc__)
    #warning ppc
#elif defined(__x86_64__)
    #warning x86_64
#elif defined(__i386__)
    #warning i386
#elif defined(__arm64e__)
    #warning arm64e
#elif defined(__arm64__)
    #warning arm64
#else
    #error other architecture
#endif

#if MAC_OS_X_VERSION_SDK == MAC_OS_X_VERSION_10_3
    #warning SDK 10.3
#elif MAC_OS_X_VERSION_SDK == MAC_OS_X_VERSION_10_4
    #warning SDK 10.4
#elif MAC_OS_X_VERSION_SDK == MAC_OS_X_VERSION_10_5
    #warning SDK 10.5
#elif MAC_OS_X_VERSION_SDK == MAC_OS_X_VERSION_10_6
    #warning SDK 10.6
#elif MAC_OS_X_VERSION_SDK == MAC_OS_X_VERSION_10_7
    #warning SDK 10.7+
#else
    #error unknown SDK
#endif

#endif /* __MACOSMACROS_H__ */
