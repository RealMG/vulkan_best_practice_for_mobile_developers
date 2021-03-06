/* Copyright (c) 2019, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <list>

#include "common.h"

#include "core/descriptor_set.h"
#include "core/framebuffer.h"
#include "core/image_view.h"
#include "core/pipeline.h"
#include "core/pipeline_layout.h"
#include "core/render_pass.h"
#include "graphics_pipeline_state.h"
#include "render_target.h"
#include "resource_binding_state.h"

namespace vkb
{
class RenderContext;

/*
 * @brief Temporary pipeline descriptor structure for a drawcall 
 *        in a subpass to be used during command recording.
 */
struct PipelineDesc
{
	std::streampos event_id{};

	GraphicsPipelineState graphics_pipeline_state;
};

/*
 * @brief Temporary subpass descriptor structure for a renderpass to
 *        to be used during command recording.
 */
struct SubpassDesc
{
	std::streampos event_id{};

	std::set<uint32_t> input_attachments;

	std::set<uint32_t> output_attachments;

	std::list<PipelineDesc> pipeline_states;
};

/*
 * @brief Render pass binding to be used during command replay.
 */
struct RenderPassBinding
{
	std::streampos event_id;

	const RenderTarget &render_target;

	std::vector<LoadStoreInfo> load_store_infos;

	std::vector<VkClearValue> clear_values;

	std::vector<SubpassDesc> subpasses;

	const RenderPass *render_pass;

	const Framebuffer *framebuffer;
};

/*
 * @brief Pipeline binding structure to be used during command replay.
 */
struct PipelineBinding
{
	std::streampos event_id;

	VkPipelineBindPoint pipeline_bind_point;

	const Pipeline &pipeline;
};

/*
 * @brief Descriptor set binding structure to be used during command replay.
 */
struct DescriptorSetBinding
{
	std::streampos event_id;

	VkPipelineBindPoint pipeline_bind_point;

	const PipelineLayout &pipeline_layout;

	uint32_t set_index;

	const DescriptorSet &descriptor_set;
};

enum class CommandType
{
	Begin,
	End,
	BeginRenderPass,
	NextSubpass,
	EndRenderPass,
	BindPipelineLayout,
	PushConstants,
	BindBuffer,
	BindImage,
	BindVertexBuffers,
	BindIndexBuffer,
	SetVertexInputFormat,
	SetViewportState,
	SetInputAssemblyState,
	SetRasterizationState,
	SetMultisampleState,
	SetDepthStencilState,
	SetColorBlendState,
	SetViewport,
	SetScissor,
	SetLineWidth,
	SetDepthBias,
	SetBlendConstants,
	SetDepthBounds,
	Draw,
	DrawIndexed,
	UpdateBuffer,
	CopyImage,
	CopyBufferToImage,
	ImageMemoryBarrier
};

/*
 * @brief Writes Vulkan commands in a memory stream while building 
 *        Vulkan pipelines and descriptor sets for each draw only if state changes.
 */
class CommandRecord
{
  public:
	CommandRecord(Device &device);

	void reset();

	Device &get_device();

	const std::ostringstream &get_stream() const;

	const std::vector<RenderPassBinding> &get_render_pass_bindings() const;

	const std::vector<PipelineBinding> &get_pipeline_bindings() const;

	const std::vector<DescriptorSetBinding> &get_descriptor_set_bindings() const;

	void begin(VkCommandBufferUsageFlags flags);

	void end();

	void begin_render_pass(const RenderTarget &render_target, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<VkClearValue> &clear_values);

	void next_subpass();

	void end_render_pass();

	void bind_pipeline_layout(PipelineLayout &pipeline_layout);

	void push_constants(uint32_t offset, const std::vector<uint8_t> &values);

	void bind_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element);

	void bind_image(const ImageView &image_view, VkSampler sampler, uint32_t set, uint32_t binding, uint32_t array_element);

	void bind_vertex_buffers(uint32_t first_binding, const std::vector<std::reference_wrapper<const vkb::core::Buffer>> &buffers, const std::vector<VkDeviceSize> &offsets);

	void bind_index_buffer(const core::Buffer &buffer, VkDeviceSize offset, VkIndexType index_type);

	void set_viewport_state(const ViewportState &state_info);

	void set_vertex_input_state(const VertexInputState &state_info);

	void set_input_assembly_state(const InputAssemblyState &state_info);

	void set_rasterization_state(const RasterizationState &state_info);

	void set_multisample_state(const MultisampleState &state_info);

	void set_depth_stencil_state(const DepthStencilState &state_info);

	void set_color_blend_state(const ColorBlendState &state_info);

	void set_viewport(uint32_t first_viewport, const std::vector<VkViewport> &viewports);

	void set_scissor(uint32_t first_scissor, const std::vector<VkRect2D> &scissors);

	void set_line_width(float line_width);

	void set_depth_bias(float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor);

	void set_blend_constants(const std::array<float, 4> &blend_constants);

	void set_depth_bounds(float min_depth_bounds, float max_depth_bounds);

	void draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);

	void draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);

	void update_buffer(const core::Buffer &buffer, VkDeviceSize offset, const std::vector<uint8_t> &data);

	void copy_image(const core::Image &src_img, const core::Image &dst_img, const std::vector<VkImageCopy> &regions);

	void copy_buffer_to_image(const core::Buffer &buffer, const core::Image &image, const std::vector<VkBufferImageCopy> &regions);

	void image_memory_barrier(const ImageView &image_view, const ImageMemoryBarrier &memory_barrier);

  private:
	Device &device;

	std::ostringstream stream;

	std::vector<RenderPassBinding> render_pass_bindings;

	std::vector<DescriptorSetBinding> descriptor_set_bindings;

	std::vector<PipelineBinding> pipeline_bindings;

	GraphicsPipelineState graphics_pipeline_state;

	ResourceBindingState resource_binding_state;

	std::unordered_map<uint32_t, DescriptorSetLayout *> descriptor_set_layout_state;

	void FlushPipelineState();

	void FlushDescriptorState();
};
}        // namespace vkb
