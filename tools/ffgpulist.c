/*
 * Created by John Othwolo on 6/13/22.
 * Copyright © 2022 John Othwolo.
 *
 * File: fflistgpus.c
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>

static int
displayGraphicsInfo(void)
{
    // Get dictionary of all the 'IOAccelerator' entries instead of 'PCIDevice' entries
    CFMutableDictionaryRef matchDict = IOServiceMatching("IOAccelerator");

    // Create an iterator
    io_iterator_t iterator;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, matchDict, &iterator) == kIOReturnSuccess)
    {
        // Iterator for devices found
        io_registry_entry_t acceleratorEntry;

        while ((acceleratorEntry = IOIteratorNext(iterator))) {
            io_registry_entry_t gpuEntry; // entry for IOAccel parent, which is gpu pci device
            unsigned long long entryID; // graphics card IOACcel registryID property
            unsigned char gpuName[sizeof(io_string_t)];
            CFMutableDictionaryRef serviceDictionary; // Put this services object into a dictionary object.
            
            // get IOAccel entryID. This is the ID Metal and OpenGL use.
            if(IORegistryEntryGetRegistryEntryID(acceleratorEntry, &entryID) != kIOReturnSuccess)
                goto release_accelerator_entry;
            
            // get the parent device for this IOAccel entry
            if (IORegistryEntryGetParentEntry(acceleratorEntry, kIOServicePlane, &gpuEntry) != kIOReturnSuccess)
                goto release_accelerator_entry;
            
            // get properties for the gpu
            if (IORegistryEntryCreateCFProperties(gpuEntry, &serviceDictionary,
                                                  kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
            {
                // Service dictionary creation failed.
                goto release_gpu_entry;
            }
            
            
            CFDataGetBytes((CFDataRef)CFDictionaryGetValue(serviceDictionary, CFSTR("model")),
                           CFRangeMake(0, sizeof(gpuName)), gpuName);
            
            printf("Device: %40s - ID: 0x%llx\n", gpuName, entryID);

            // Release the dictionary
            CFRelease(serviceDictionary);
release_gpu_entry:
            IOObjectRelease(gpuEntry);
release_accelerator_entry:
            // Release the acceleratorEntry object
            IOObjectRelease(acceleratorEntry);
        }
        // Release the iterator
        IOObjectRelease(iterator);
    }
    return 0;
}

int main(int argc, const char * argv[]) {
    struct utsname utsname;
    if(uname(&utsname)){
        perror("Error: ");
        abort();
    }
    if (strncmp(utsname.sysname, "Darwin", sizeof("Darwin") - 1) != 0)
        fprintf(stderr, "This command is only supported on macOS");
    
    return displayGraphicsInfo();
}
