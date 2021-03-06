/* Copyright (c) 2018-2019, Arm Limited and Contributors
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

#include <memory>
#include <mutex>

#include "scene_graph/components/camera.h"
#include "scene_graph/components/image.h"
#include "scene_graph/components/mesh.h"
#include "scene_graph/components/pbr_material.h"
#include "scene_graph/components/sampler.h"
#include "scene_graph/components/sub_mesh.h"
#include "scene_graph/components/texture.h"
#include "scene_graph/node.h"
#include "scene_graph/scene.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include <android/asset_manager.h>

namespace tinygltf
{
extern AAssetManager *asset_manager;
}
#endif

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
// Disable warnings for external header
#pragma clang diagnostic ignored "-Wall"
#include "tiny_gltf.h"
#pragma clang diagnostic pop

#include "core/device.h"

namespace vkb
{
/// Read a gltf file and return a scene object. Converts the gltf objects
/// to our internal scene implementation. Mesh data is copied to vulkan buffers and
/// images are loaded from the folder of gltf file to vulkan images.
class GLTFLoader
{
  public:
	GLTFLoader(Device &device);

	bool read_scene_from_file(const std::string &file_name, sg::Scene &scene);

  protected:
	virtual std::shared_ptr<sg::Node> parse_node(const tinygltf::Node &gltf_node);

	virtual std::shared_ptr<sg::Camera> parse_camera(const tinygltf::Camera &gltf_camera);

	virtual std::shared_ptr<sg::Mesh> parse_mesh(const tinygltf::Mesh &gltf_mesh);

	virtual std::shared_ptr<sg::SubMesh> parse_primitive(const tinygltf::Primitive &gltf_primitive);

	virtual std::shared_ptr<sg::PBRMaterial> parse_material(const tinygltf::Material &gltf_material);

	virtual std::shared_ptr<sg::Image> parse_image(tinygltf::Image &gltf_image);

	virtual std::shared_ptr<sg::Sampler> parse_sampler(const tinygltf::Sampler &gltf_sampler);

	virtual std::shared_ptr<sg::Texture> parse_texture(const tinygltf::Texture &gltf_texture);

	virtual std::shared_ptr<sg::PBRMaterial> create_default_material();

	virtual std::shared_ptr<sg::Sampler> create_default_sampler();

	virtual std::shared_ptr<sg::Camera> create_default_camera();

	Device &device;

	tinygltf::Model model;

	std::string model_path;

  private:
	void load_scene(sg::Scene &scene);
};
}        // namespace vkb
