/*
 * Copyright © 2019 Valve Corporation.
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

#ifndef RADV_SHADER_ARGS_H
#define RADV_SHADER_ARGS_H

#include "compiler/shader_enums.h"
#include "util/list.h"
#include "util/macros.h"
#include "ac_shader_args.h"
#include "amd_family.h"
#include "radv_constants.h"
#include "radv_shader.h"

struct radv_shader_args {
   struct ac_shader_args ac;

   struct ac_arg descriptor_sets[MAX_SETS];
   struct ac_arg ring_offsets;

   /* Streamout */
   struct ac_arg streamout_buffers;

   /* NGG GS */
   struct ac_arg ngg_gs_state;
   struct ac_arg ngg_culling_settings;
   struct ac_arg ngg_viewport_scale[2];
   struct ac_arg ngg_viewport_translate[2];

   struct ac_arg prolog_inputs;
   struct ac_arg vs_inputs[MAX_VERTEX_ATTRIBS];

   struct radv_userdata_locations user_sgprs_locs;
   unsigned num_user_sgprs;

   bool explicit_scratch_args;
   bool remap_spi_ps_input;
   bool load_grid_size_from_user_sgpr;
   bool is_gs_copy_shader;
   bool is_trap_handler_shader;
};

static inline struct radv_shader_args *
radv_shader_args_from_ac(struct ac_shader_args *args)
{
   return container_of(args, struct radv_shader_args, ac);
}

struct radv_pipeline_key;
struct radv_shader_info;

void radv_declare_shader_args(enum chip_class chip_class, const struct radv_pipeline_key *key,
                              const struct radv_shader_info *info, gl_shader_stage stage,
                              bool has_previous_stage, gl_shader_stage previous_stage,
                              struct radv_shader_args *args);

#endif
