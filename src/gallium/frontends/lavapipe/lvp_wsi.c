/*
 * Copyright © 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "lvp_wsi.h"

#include "vk_fence.h"
#include "vk_semaphore.h"
#include "vk_sync_dummy.h"

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
lvp_wsi_proc_addr(VkPhysicalDevice physicalDevice, const char *pName)
{
   LVP_FROM_HANDLE(lvp_physical_device, pdevice, physicalDevice);
   return vk_instance_get_proc_addr_unchecked(pdevice->vk.instance, pName);
}

VkResult
lvp_init_wsi(struct lvp_physical_device *physical_device)
{
   VkResult result;

   result = wsi_device_init(&physical_device->wsi_device,
                            lvp_physical_device_to_handle(physical_device),
                            lvp_wsi_proc_addr,
                            &physical_device->vk.instance->alloc,
                            -1, NULL, true);
   if (result != VK_SUCCESS)
      return result;

   physical_device->vk.wsi_device = &physical_device->wsi_device;

   return VK_SUCCESS;
}

void
lvp_finish_wsi(struct lvp_physical_device *physical_device)
{
   physical_device->vk.wsi_device = NULL;
   wsi_device_finish(&physical_device->wsi_device,
                     &physical_device->vk.instance->alloc);
}

VKAPI_ATTR VkResult VKAPI_CALL lvp_AcquireNextImage2KHR(
   VkDevice                                     _device,
   const VkAcquireNextImageInfoKHR*             pAcquireInfo,
   uint32_t*                                    pImageIndex)
{
   LVP_FROM_HANDLE(lvp_device, device, _device);
   struct lvp_physical_device *pdevice = device->physical_device;

   VkResult result = wsi_common_acquire_next_image2(&pdevice->wsi_device,
                                                    _device,
                                                    pAcquireInfo,
                                                    pImageIndex);

   VK_FROM_HANDLE(vk_fence, fence, pAcquireInfo->fence);
   VK_FROM_HANDLE(vk_semaphore, sem, pAcquireInfo->semaphore);
   if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
      VkResult sync_res;
      if (fence) {
         vk_fence_reset_temporary(&device->vk, fence);
         sync_res = vk_sync_create(&device->vk, &vk_sync_dummy_type,
                                   0 /* flags */, 0 /* initial_value */,
                                   &fence->temporary);
         if (sync_res != VK_SUCCESS)
            return sync_res;
      }

      if (sem) {
         vk_semaphore_reset_temporary(&device->vk, sem);
         sync_res = vk_sync_create(&device->vk, &vk_sync_dummy_type,
                                   0 /* flags */, 0 /* initial_value */,
                                   &sem->temporary);
         if (sync_res != VK_SUCCESS)
            return sync_res;
      }
   }

   return result;
}
