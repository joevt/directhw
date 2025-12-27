/*
 * DirectHW.c - userspace part for DirectHW
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

#include "MacOSMacros.h"
#include "DirectHW.h"
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

/* define DEBUG to print Framework debugging information */
#undef DEBUG

#ifndef err_get_system
#define err_get_system(err) (((err)>>26)&0x3f)
#endif

#ifndef err_get_sub
#define err_get_sub(err)    (((err)>>14)&0xfff)
#endif

#ifndef err_get_code
#define err_get_code(err)   ((err)&0x3fff)
#endif

#include "DirectHWShared.h"

static io_connect_t darwin_connect = MACH_PORT_NULL;
static io_service_t iokit_uc;

static int darwin_init(void)
{
    kern_return_t err;

    /* Note the actual security happens in the kernel module.
     * This check is just candy to be able to get nicer output
     */
    if (getuid() != 0) {
        /* Fun's reserved for root */
        errno = EPERM;
        return -1;
    }

    /* Get the DirectHW driver service */
    /* Use weak linking for maximum compatibility with all SDKs and architectures */
    /* kIOMainPortDefault exists since macOS 12.1 SDK, kIOMasterPortDefault exists since macOS 10.2.8 */
    extern const mach_port_t kIOMainPortDefault __attribute__((weak));
    extern const mach_port_t kIOMasterPortDefault __attribute__((weak));

    /* Create compatibility macro as suggested by Joevt */
    #define kOurMasterPort (kIOMasterPortDefault ? kIOMasterPortDefault : kIOMainPortDefault)

    /* Use the compatible master port that works with all SDKs and targets */
    iokit_uc = IOServiceGetMatchingService(kOurMasterPort, IOServiceMatching("DirectHWService"));

    if (!iokit_uc) {
        printf("DirectHW.kext not loaded.\n");
        errno = ENOSYS;
        return -1;
    }

    /* Create an instance */
    err = IOServiceOpen(iokit_uc, mach_task_self(), 0, &darwin_connect);

    /* Should not go further if error with service open */
    if (err != KERN_SUCCESS) {
        printf("Could not create DirectHW instance.\n");
        errno = ENOSYS;
        return -1;
    }

    return 0;
}

static void darwin_cleanup(void)
{
    if (darwin_connect != MACH_PORT_NULL) {
        IOServiceClose(darwin_connect);
        darwin_connect = MACH_PORT_NULL;
    }
}

kern_return_t MyIOConnectCallStructMethod(
    io_connect_t    connect,
    unsigned int    index,
    void *          in,
    size_t          dataInLen,
    void *          out,
    size_t *        dataOutLen
)
{
    kern_return_t err;
#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_4 || MAC_OS_X_VERSION_SDK <= MAC_OS_X_VERSION_10_4
    /* Use legacy IOConnectMethodStructureIStructureO for Mac OS X 10.4 and earlier */
    err = IOConnectMethodStructureIStructureO(connect, index, dataInLen, dataOutLen, in, out);
#elif defined(__LP64__)
    /* Use modern IOConnectCallStructMethod for 64-bit systems */
    err = IOConnectCallStructMethod(connect, index, in, dataInLen, out, dataOutLen);
#else
    /* For 32-bit systems with transitional APIs (Mac OS X 10.5-10.7),
     * determine which API to use based on availability at compile time.
     * Use weak linking to check API availability at runtime. */
    if (&IOConnectCallStructMethod != NULL) {
        /* Modern API is available, use it */
        err = IOConnectCallStructMethod(connect, index, in, dataInLen, out, dataOutLen);
    } else {
        /* Modern API not available, use legacy API */
        err = IOConnectMethodStructureIStructureO(connect, index, dataInLen, dataOutLen, in, out);
    }
#endif
    return err;
}

static kern_return_t dhw_IOConnectCallStructMethod(
    unsigned int    index,
    void *          in,
    size_t          dataInLen,
    void *          out,
    size_t *        dataOutLen
)
{
    if (darwin_connect == MACH_PORT_NULL) {
        iopl(3);
    }
    if (darwin_connect != MACH_PORT_NULL) {
        return MyIOConnectCallStructMethod(darwin_connect, index, in, dataInLen, out, dataOutLen);
    }
    return kIOReturnError;
}

int darwin_ioread(int pos, unsigned char * buf, int len)
{
    kern_return_t err;
    size_t dataInLen;
    size_t dataOutLen;
    void *in;
    void *out;
    iomem_t in32;
    iomem_t out32;
    iomem64_t in64;
    iomem64_t out64;
    UInt64 tmpdata64;
    UInt32 tmpdata;

    if (len <= 4) {
        in = &in32;
        out = &out32;
        dataInLen = sizeof(in32);
        dataOutLen = sizeof(out32);
        in32.width = len;
        in32.offset = pos;
    }
    else if (len <= 8) {
        in = &in64;
        out = &out64;
        dataInLen = sizeof(in64);
        dataOutLen = sizeof(out64);
        in64.width = len;
        in64.offset = pos;
    }
    else {
        return 1;
    }

    err = dhw_IOConnectCallStructMethod(kReadIO, in, dataInLen, out, &dataOutLen);
    if (err != KERN_SUCCESS)
        return 1;

    if (len <= 4) {
        tmpdata = out32.data;
        switch (len) {
            case 1: memcpy(buf, &tmpdata, 1); break;
            case 2: memcpy(buf, &tmpdata, 2); break;
            case 4: memcpy(buf, &tmpdata, 4); break;
            case 8: memcpy(buf, &tmpdata, 8); break;
            default:
                fprintf(stderr, "ERROR: unsupported ioRead length %d\n", len);
                return 1;
        }
    }
    else {
        tmpdata64 = out64.data;
        switch (len) {
            case 8: memcpy(buf, &tmpdata64, 8); break;
            default:
                fprintf(stderr, "ERROR: unsupported ioRead length %d\n", len);
                return 1;
        }
    }

    return 0;
}

static int darwin_iowrite(int pos, unsigned char * buf, int len)
{
    kern_return_t err;
    size_t dataInLen;
    size_t dataOutLen;
    void *in;
    void *out;
    iomem_t in32;
    iomem_t out32;
    iomem64_t in64;
    iomem64_t out64;

    if (len <= 4) {
        in = &in32;
        out = &out32;
        dataInLen = sizeof(in32);
        dataOutLen = sizeof(out32);
        in32.width = len;
        in32.offset = pos;
        memcpy(&in32.data, buf, len);
    }
    else if (len <= 8) {
        in = &in64;
        out = &out64;
        dataInLen = sizeof(in64);
        dataOutLen = sizeof(out64);
        in64.width = len;
        in64.offset = pos;
        memcpy(&in64.data, buf, len);
    }
    else {
        return 1;
    }

    err = dhw_IOConnectCallStructMethod(kWriteIO, in, dataInLen, out, &dataOutLen);
    if (err != KERN_SUCCESS) {
        return 1;
    }

    return 0;
}


/* Compatibility interface */

unsigned char inb(unsigned short addr)
{
    unsigned char ret = 0;
    darwin_ioread(addr, &ret, 1);
    return ret;
}

unsigned short inw(unsigned short addr)
{
    unsigned short ret = 0;
    darwin_ioread(addr, (unsigned char *)&ret, 2);
    return ret;
}

unsigned int inl(unsigned short addr)
{
    unsigned int ret = 0;
    darwin_ioread(addr, (unsigned char *)&ret, 4);
    return ret;
}

#ifdef __LP64__
unsigned long inq(unsigned short addr)
{
    unsigned long ret = 0;
    darwin_ioread(addr, (unsigned char *)&ret, 8);
    return ret;
}
#endif

void outb(unsigned char val, unsigned short addr)
{
    darwin_iowrite(addr, &val, 1);
}

void outw(unsigned short val, unsigned short addr)
{
    darwin_iowrite(addr, (unsigned char *)&val, 2);
}

void outl(unsigned int val, unsigned short addr)
{
    darwin_iowrite(addr, (unsigned char *)&val, 4);
}

#ifdef __LP64__
void outq(unsigned long val, unsigned short addr)
{
    darwin_iowrite(addr, (unsigned char *)&val, 8);
}
#endif

int iopl(int level)
{
    if (level) {
        if (darwin_connect != MACH_PORT_NULL) {
            return 0;
        }
        atexit(darwin_cleanup);
        return darwin_init();
    }
    else {
        darwin_cleanup();
        return 0;
    }
}

void *map_physical(uint64_t phys_addr, size_t len)
{
    kern_return_t err;
#if defined(__LP64__) && (MAC_OS_X_VERSION_SDK >= MAC_OS_X_VERSION_10_5)
    mach_vm_address_t addr;
    mach_vm_size_t size;
#else
    vm_address_t addr;
    vm_size_t size;
#endif
    size_t dataInLen = sizeof(map_t);
    size_t dataOutLen = sizeof(map_t);

    map_t in;
    map_t out;

    in.addr = phys_addr;
    in.size = len;

#ifdef DEBUG
    printf("map_phys: phys %08lx, %08x\n", phys_addr, len);
#endif

    err = dhw_IOConnectCallStructMethod(kPrepareMap, &in, dataInLen, &out, &dataOutLen);
    if (err != KERN_SUCCESS) {
        printf("\nError(kPrepareMap): system 0x%x subsystem 0x%x code 0x%x ",
               err_get_system(err), err_get_sub(err), err_get_code(err));

        printf("physical 0x%16lx[0x%lx]\n", (unsigned long)phys_addr, (unsigned long)len);

        switch (err_get_code(err)) {
            case 0x2c2: printf("Invalid argument.\n"); errno = EINVAL; break;
            case 0x2cd: printf("Device not open.\n"); errno = ENOENT; break;
        }

        return MAP_FAILED;
    }

    err = IOConnectMapMemory(darwin_connect, 0, mach_task_self(),
                             &addr, &size, kIOMapAnywhere | kIOMapInhibitCache);

    /* Now this is odd; The above connect seems to be unfinished at the
     * time the function returns. So wait a little bit, or the calling
     * program will just segfault. Bummer. Who knows a better solution?
     */
    usleep(1000);

    if (err != KERN_SUCCESS) {
        printf("\nError(IOConnectMapMemory): system 0x%x subsystem 0x%x code 0x%x ",
               err_get_system(err), err_get_sub(err), err_get_code(err));

        printf("physical 0x%16lx[0x%lx]\n", (unsigned long)phys_addr, (unsigned long)len);

        switch (err_get_code(err)) {
            case 0x2c2: printf("Invalid argument.\n"); errno = EINVAL; break;
            case 0x2cd: printf("Device not open.\n"); errno = ENOENT; break;
        }

        return MAP_FAILED;
    }

#ifdef DEBUG
    printf("map_phys: virt %16lx, %16lx\n", (unsigned  long)addr, (unsigned long)size);
#endif /* DEBUG */

    return (void *)addr;
}

void unmap_physical(void *virt_addr __attribute__((unused)), size_t len __attribute__((unused)))
{
    // Nut'n Honey
}

static int current_logical_cpu = 0;

msr_t rdmsr(int addr)
{
    kern_return_t err;
    size_t dataInLen = sizeof(msrcmd_t);
    size_t dataOutLen = sizeof(msrcmd_t);
    msrcmd_t in, out;
    msr_t ret;
    ret.lo = INVALID_MSR_LO;
    ret.hi = INVALID_MSR_HI;

    in.core = current_logical_cpu;
    in.index = addr;

    err = dhw_IOConnectCallStructMethod(kReadMSR, &in, dataInLen, &out, &dataOutLen);
    if (err != KERN_SUCCESS) {
        return ret;
    }

    ret.lo = out.lo;
    ret.hi = out.hi;

    return ret;
}

int rdcpuid(uint32_t eax, uint32_t ecx, uint32_t cpudata[4])
{
    kern_return_t err;
    size_t dataInLen = sizeof(cpuid_t);
    size_t dataOutLen = sizeof(cpuid_t);
    cpuid_t in, out;

    in.core = current_logical_cpu;
    in.eax = eax;
    in.ecx = ecx;

    err = dhw_IOConnectCallStructMethod(kReadCpuId, &in, dataInLen, &out, &dataOutLen);
    if (err != KERN_SUCCESS)
        return -1;

    memcpy(cpudata, out.cpudata, sizeof(uint32_t) * 4);
    return 0;
}

int readmem32(uint64_t addr, uint32_t* data)
{
    kern_return_t err;
    size_t dataInLen = sizeof(readmem_t);
    size_t dataOutLen = sizeof(readmem_t);
    readmem_t in, out;

    in.core = current_logical_cpu;
    in.addr = addr;

    err = dhw_IOConnectCallStructMethod(kReadMem, &in, dataInLen, &out, &dataOutLen);
    if (err != KERN_SUCCESS)
        return -1;

    *data = out.data;
    return 0;
}

int wrmsr(int addr, msr_t msr)
{
    kern_return_t err;
    size_t dataInLen = sizeof(msrcmd_t);
    size_t dataOutLen = sizeof(msrcmd_t);
    msrcmd_t in;
    msrcmd_t out;

    in.core = current_logical_cpu;
    in.index = addr;
    in.lo = msr.lo;
    in.hi = msr.hi;

    err = dhw_IOConnectCallStructMethod(kWriteMSR, &in, dataInLen, &out, &dataOutLen);
    if (err != KERN_SUCCESS)
        return 1;

    return 0;
}

int logical_cpu_select(int cpu)
{
    current_logical_cpu = cpu;
    return current_logical_cpu;
}

int allocate_physically_contiguous_32(size_t len, uint32_t *phys, void* *user, uint32_t *type)
{
    kern_return_t err;

    MemParams in;
    MemParams out;
    size_t dataInLen = sizeof(MemParams);
    size_t dataOutLen = sizeof(MemParams);

    in.allocOptions = kPhysContig;
    in.size = len;
    in.physMask = 0xfffff000; // 32-bit page aligned
    in.mapOptions = kIOMapInhibitCache;

    err = dhw_IOConnectCallStructMethod(kAllocatePhysicalMemory, &in, dataInLen, &out, &dataOutLen);
    if (err != KERN_SUCCESS) {
        printf("\nError(kAllocatePhysicalMemory): system 0x%x subsystem 0x%x code 0x%x\n",
               err_get_system(err), err_get_sub(err), err_get_code(err));
        return -1;
    }

    if (phys) *phys = (UInt32)out.physAddr;
#ifdef __LP64__
    if (user) *user = (void*)out.userAddr;
#else
    if (user) *user = (void*)(UInt32)out.userAddr;
#endif
    if (type) *type = out.memoryType;
    return 0;
}

int unallocate_mem(uint32_t type)
{
    kern_return_t err;

    MemParams in;
    MemParams out;
    size_t dataInLen = sizeof(MemParams);
    size_t dataOutLen = sizeof(MemParams);

    in.memoryType = type;

    err = dhw_IOConnectCallStructMethod(kUnallocatePhysicalMemory, &in, dataInLen, &out, &dataOutLen);
    if (err != KERN_SUCCESS) {
        printf("\nError(kUnallocatePhysicalMemory): system 0x%x subsystem 0x%x code 0x%x\n",
               err_get_system(err), err_get_sub(err), err_get_code(err));
        return -1;
    }
    return 0;
}

void *map_physical_v2(uint64_t phys_addr, size_t len)
{
    kern_return_t err;

    MemParams in;
    MemParams out;
    size_t dataInLen = sizeof(MemParams);
    size_t dataOutLen = sizeof(MemParams);

    in.allocOptions = kUsePhys;
    in.physAddr = phys_addr;
    in.size = len;
    in.mapOptions = kIOMapInhibitCache;

#ifdef DEBUG
    printf("map_phys: phys %08llx, %08zx\n", phys_addr, len);
#endif

    err = dhw_IOConnectCallStructMethod(kAllocatePhysicalMemory, &in, dataInLen, &out, &dataOutLen);
    if (err != KERN_SUCCESS) {
        printf("\nError(kPrepareMap): system 0x%x subsystem 0x%x code 0x%x ",
               err_get_system(err), err_get_sub(err), err_get_code(err));

        printf("physical 0x%16lx[0x%lx]\n", (unsigned long)phys_addr, (unsigned long)len);

        return MAP_FAILED;
    }

#ifdef DEBUG
    printf("map_phys: virt %16lx, %16llx\n", out.userAddr, out.size);
#endif /* DEBUG */

#ifdef __LP64__
    return (void *)out.userAddr;
#else
    return (void *)(UInt32)out.userAddr;
#endif
}
