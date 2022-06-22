/*
 * Copyright © 2019 Red Hat.
 * Copyright © 2022 Collabora, LTD
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

#include "vk_alloc.h"
#include "vk_cmd_enqueue_entrypoints.h"
#include "vk_command_buffer.h"
#include "vk_device.h"
#include "vk_util.h"

VKAPI_ATTR void VKAPI_CALL
vk_cmd_enqueue_CmdDrawMultiEXT(VkCommandBuffer commandBuffer,
                               uint32_t drawCount,
                               const VkMultiDrawInfoEXT *pVertexInfo,
                               uint32_t instanceCount,
                               uint32_t firstInstance,
                               uint32_t stride)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);

   struct vk_cmd_queue_entry *cmd =
      vk_zalloc(cmd_buffer->cmd_queue.alloc, sizeof(*cmd), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmd)
      return;

   cmd->type = VK_CMD_DRAW_MULTI_EXT;
   list_addtail(&cmd->cmd_link, &cmd_buffer->cmd_queue.cmds);

   cmd->u.draw_multi_ext.draw_count = drawCount;
   if (pVertexInfo) {
      unsigned i = 0;
      cmd->u.draw_multi_ext.vertex_info =
         vk_zalloc(cmd_buffer->cmd_queue.alloc,
                   sizeof(*cmd->u.draw_multi_ext.vertex_info) * drawCount, 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      vk_foreach_multi_draw(draw, i, pVertexInfo, drawCount, stride) {
         memcpy(&cmd->u.draw_multi_ext.vertex_info[i], draw,
                sizeof(*cmd->u.draw_multi_ext.vertex_info));
      }
   }
   cmd->u.draw_multi_ext.instance_count = instanceCount;
   cmd->u.draw_multi_ext.first_instance = firstInstance;
   cmd->u.draw_multi_ext.stride = stride;
}

VKAPI_ATTR void VKAPI_CALL
vk_cmd_enqueue_CmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer,
                                      uint32_t drawCount,
                                      const VkMultiDrawIndexedInfoEXT *pIndexInfo,
                                      uint32_t instanceCount,
                                      uint32_t firstInstance,
                                      uint32_t stride,
                                      const int32_t *pVertexOffset)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);

   struct vk_cmd_queue_entry *cmd =
      vk_zalloc(cmd_buffer->cmd_queue.alloc, sizeof(*cmd), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmd)
      return;

   cmd->type = VK_CMD_DRAW_MULTI_INDEXED_EXT;
   list_addtail(&cmd->cmd_link, &cmd_buffer->cmd_queue.cmds);

   cmd->u.draw_multi_indexed_ext.draw_count = drawCount;

   if (pIndexInfo) {
      unsigned i = 0;
      cmd->u.draw_multi_indexed_ext.index_info =
         vk_zalloc(cmd_buffer->cmd_queue.alloc,
                   sizeof(*cmd->u.draw_multi_indexed_ext.index_info) * drawCount, 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      vk_foreach_multi_draw_indexed(draw, i, pIndexInfo, drawCount, stride) {
         cmd->u.draw_multi_indexed_ext.index_info[i].firstIndex = draw->firstIndex;
         cmd->u.draw_multi_indexed_ext.index_info[i].indexCount = draw->indexCount;
         if (pVertexOffset == NULL)
            cmd->u.draw_multi_indexed_ext.index_info[i].vertexOffset = draw->vertexOffset;
      }
   }

   cmd->u.draw_multi_indexed_ext.instance_count = instanceCount;
   cmd->u.draw_multi_indexed_ext.first_instance = firstInstance;
   cmd->u.draw_multi_indexed_ext.stride = stride;

   if (pVertexOffset) {
      cmd->u.draw_multi_indexed_ext.vertex_offset =
         vk_zalloc(cmd_buffer->cmd_queue.alloc,
                   sizeof(*cmd->u.draw_multi_indexed_ext.vertex_offset), 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      memcpy(cmd->u.draw_multi_indexed_ext.vertex_offset, pVertexOffset,
             sizeof(*cmd->u.draw_multi_indexed_ext.vertex_offset));
   }
}

VKAPI_ATTR void VKAPI_CALL
vk_cmd_enqueue_CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                       VkPipelineBindPoint pipelineBindPoint,
                                       VkPipelineLayout layout,
                                       uint32_t set,
                                       uint32_t descriptorWriteCount,
                                       const VkWriteDescriptorSet *pDescriptorWrites)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);
   struct vk_cmd_push_descriptor_set_khr *pds;

   struct vk_cmd_queue_entry *cmd =
      vk_zalloc(cmd_buffer->cmd_queue.alloc, sizeof(*cmd), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmd)
      return;

   pds = &cmd->u.push_descriptor_set_khr;

   cmd->type = VK_CMD_PUSH_DESCRIPTOR_SET_KHR;
   list_addtail(&cmd->cmd_link, &cmd_buffer->cmd_queue.cmds);

   pds->pipeline_bind_point = pipelineBindPoint;
   pds->layout = layout;
   pds->set = set;
   pds->descriptor_write_count = descriptorWriteCount;

   if (pDescriptorWrites) {
      pds->descriptor_writes =
         vk_zalloc(cmd_buffer->cmd_queue.alloc,
                   sizeof(*pds->descriptor_writes) * descriptorWriteCount, 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      memcpy(pds->descriptor_writes,
             pDescriptorWrites,
             sizeof(*pds->descriptor_writes) * descriptorWriteCount);

      for (unsigned i = 0; i < descriptorWriteCount; i++) {
         switch (pds->descriptor_writes[i].descriptorType) {
         case VK_DESCRIPTOR_TYPE_SAMPLER:
         case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
         case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
         case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            pds->descriptor_writes[i].pImageInfo =
               vk_zalloc(cmd_buffer->cmd_queue.alloc,
                         sizeof(VkDescriptorImageInfo) * pds->descriptor_writes[i].descriptorCount, 8,
                         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            memcpy((VkDescriptorImageInfo *)pds->descriptor_writes[i].pImageInfo,
                   pDescriptorWrites[i].pImageInfo,
                   sizeof(VkDescriptorImageInfo) * pds->descriptor_writes[i].descriptorCount);
            break;
         case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            pds->descriptor_writes[i].pTexelBufferView =
               vk_zalloc(cmd_buffer->cmd_queue.alloc,
                         sizeof(VkBufferView) * pds->descriptor_writes[i].descriptorCount, 8,
                         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            memcpy((VkBufferView *)pds->descriptor_writes[i].pTexelBufferView,
                   pDescriptorWrites[i].pTexelBufferView,
                   sizeof(VkBufferView) * pds->descriptor_writes[i].descriptorCount);
            break;
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         default:
            pds->descriptor_writes[i].pBufferInfo =
               vk_zalloc(cmd_buffer->cmd_queue.alloc,
                         sizeof(VkDescriptorBufferInfo) * pds->descriptor_writes[i].descriptorCount, 8,
                         VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            memcpy((VkDescriptorBufferInfo *)pds->descriptor_writes[i].pBufferInfo,
                   pDescriptorWrites[i].pBufferInfo,
                   sizeof(VkDescriptorBufferInfo) * pds->descriptor_writes[i].descriptorCount);
            break;
         }
      }
   }
}

static void
unref_pipeline_layout(struct vk_cmd_queue *queue,
                      struct vk_cmd_queue_entry *cmd)
{
   struct vk_command_buffer *cmd_buffer =
      container_of(queue, struct vk_command_buffer, cmd_queue);
   struct vk_device *device = cmd_buffer->base.device;

   assert(cmd->type == VK_CMD_BIND_DESCRIPTOR_SETS);

   device->unref_pipeline_layout(device, cmd->u.bind_descriptor_sets.layout);
}

VKAPI_ATTR void VKAPI_CALL
vk_cmd_enqueue_CmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                                     VkPipelineBindPoint pipelineBindPoint,
                                     VkPipelineLayout layout,
                                     uint32_t firstSet,
                                     uint32_t descriptorSetCount,
                                     const VkDescriptorSet* pDescriptorSets,
                                     uint32_t dynamicOffsetCount,
                                     const uint32_t *pDynamicOffsets)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd_buffer, commandBuffer);
   struct vk_device *device = cmd_buffer->base.device;

   struct vk_cmd_queue_entry *cmd =
      vk_zalloc(cmd_buffer->cmd_queue.alloc, sizeof(*cmd), 8,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!cmd)
      return;

   cmd->type = VK_CMD_BIND_DESCRIPTOR_SETS;
   list_addtail(&cmd->cmd_link, &cmd_buffer->cmd_queue.cmds);

   /* We need to hold a reference to the descriptor set as long as this
    * command is in the queue.  Otherwise, it may get deleted out from under
    * us before the command is replayed.
    */
   device->ref_pipeline_layout(device, layout);
   cmd->u.bind_descriptor_sets.layout = layout;
   cmd->driver_free_cb = unref_pipeline_layout;

   cmd->u.bind_descriptor_sets.pipeline_bind_point = pipelineBindPoint;
   cmd->u.bind_descriptor_sets.first_set = firstSet;
   cmd->u.bind_descriptor_sets.descriptor_set_count = descriptorSetCount;
   if (pDescriptorSets) {
      cmd->u.bind_descriptor_sets.descriptor_sets =
         vk_zalloc(cmd_buffer->cmd_queue.alloc,
                   sizeof(*cmd->u.bind_descriptor_sets.descriptor_sets) * descriptorSetCount, 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      memcpy(cmd->u.bind_descriptor_sets.descriptor_sets, pDescriptorSets,
             sizeof(*cmd->u.bind_descriptor_sets.descriptor_sets) * descriptorSetCount);
   }
   cmd->u.bind_descriptor_sets.dynamic_offset_count = dynamicOffsetCount;
   if (pDynamicOffsets) {
      cmd->u.bind_descriptor_sets.dynamic_offsets =
         vk_zalloc(cmd_buffer->cmd_queue.alloc,
                   sizeof(*cmd->u.bind_descriptor_sets.dynamic_offsets) * dynamicOffsetCount, 8,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

      memcpy(cmd->u.bind_descriptor_sets.dynamic_offsets, pDynamicOffsets,
             sizeof(*cmd->u.bind_descriptor_sets.dynamic_offsets) * dynamicOffsetCount);
   }
}
