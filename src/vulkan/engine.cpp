#include "engine.h"
#include "src/scene.h"
#include "src/vulkan/device_manager.h"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "src/vulkan/command_pool.h"
#include "src/vulkan/graphics_pipeline.h"
#include "src/vulkan/logical_device.h"

namespace raytracing::vulkan {
	Engine::Engine(std::string_view app_name)
	    : core_{app_name}
	    , device_manager_{core_.create_device_manager()}
	    , swapchain_{device_manager_.get_logical()}
	    , rasterizer_{device_manager_.get_logical(), device_manager_.get_allocator(), swapchain_}
	    , scene_{device_manager_.get_logical(), device_manager_.get_command_pool(),
	             device_manager_.get_allocator().get(), "resources/maps/p2-map.glb", GltfScene{}} {
	}

	DeviceManager const &Engine::get_device_manager() const {
		return device_manager_;
	}

	Scene Engine::load_scene(std::filesystem::path const &path, SceneFormat format) {
		auto const &device{device_manager_.get_logical()};
		auto const &command_pool{device_manager_.get_command_pool()};
		auto const &allocator{device_manager_.get_allocator()};

		switch (format) {
			case SceneFormat::Gltf:
				return Scene{device, command_pool, allocator.get(), path, GltfScene{}};
		};

		throw std::runtime_error{"Invalid format"};
	}

	void Engine::main_loop() {
		while (!core_.get_close_requested()) {
			core_.update();
			rasterizer_.render(scene_);
		}

		device_manager_.get_logical().wait_idle();
	}
}// namespace raytracing::vulkan
