/** @file

  Copyright (c) 2018, ARM Ltd. All rights reserved.<BR>

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Guid/SgiVirtioDevicesFormSet.h>
#include <Library/VirtioMmioDeviceLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <SgiPlatform.h>

#pragma pack (1)
typedef struct {
  VENDOR_DEVICE_PATH                  VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL            End;
} VIRTIO_DEVICE_PATH;
#pragma pack ()

STATIC VIRTIO_DEVICE_PATH mVirtioBlockDevicePath =
{
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8)(sizeof (VENDOR_DEVICE_PATH)),
        (UINT8)((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    SGI_VIRTIO_BLOCK_GUID,
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      sizeof (EFI_DEVICE_PATH_PROTOCOL),
      0
    }
  }
};

/**
  Initialize platform Virtio devices.

  @return None.
**/
VOID
InitVirtioDevices (
  VOID
  )
{
  EFI_STATUS Status;
  STATIC EFI_HANDLE mVirtIoBlkController = NULL;

  // Install protocol interface for storage device
  if ((FeaturePcdGet (PcdVirtioBlkSupported)) &&
      (FixedPcdGet32 (PcdVirtioBlkBaseAddress))) {
    Status = gBS->InstallProtocolInterface (&mVirtIoBlkController,
                    &gEfiDevicePathProtocolGuid, EFI_NATIVE_INTERFACE,
                    &mVirtioBlockDevicePath);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: Failed to install EFI_DEVICE_PATH protocol "
        "for Virtio Block device (Status = %r)\n",
        __FUNCTION__, Status));
    } else {
      // Declare the Virtio BlockIo device
      Status = VirtioMmioInstallDevice (FixedPcdGet32 (PcdVirtioBlkBaseAddress),
                 mVirtIoBlkController);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a: Unable to find Virtio Block MMIO device "
          "(Status == %r)\n", __FUNCTION__, Status));
        gBS->UninstallProtocolInterface (
               mVirtIoBlkController,
               &gEfiDevicePathProtocolGuid,
               &mVirtioBlockDevicePath
             );
      } else {
        DEBUG ((DEBUG_INIT, "%a: Installed Virtio Block device\n",
          __FUNCTION__));
      }
    }
  }
}
