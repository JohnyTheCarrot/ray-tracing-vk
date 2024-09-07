#include "swapchain.h"
#include "logical_device.h"
#include "src/diagnostics.h"
#include <format>

namespace raytracing::vulkan {
	Swapchain::Swapchain(LogicalDevice const &device)
	    : swapchain_{[&] {
		    Logger::get_instance().log(LogLevel::Debug, "Creating swapchain");
		    vkb::SwapchainBuilder const builder{device.get()};
		    auto const                  swapchain_ret{builder.build()};
		    if (!swapchain_ret) {
			    std::string message{std::format("Failed to create swapchain: {}", swapchain_ret.error().message())};
			    throw std::runtime_error{std::move(message)};
		    }

		    return swapchain_ret.value();
	    }()}
	    , device_{&device.get()} {
	}

	void Swapchain::recreate() {
		Logger::get_instance().log(LogLevel::Debug, "Recreating swapchain");
		vkb::SwapchainBuilder builder{*device_};
		builder.set_old_swapchain(swapchain_.get());

		auto const swapchain_ret{builder.build()};
		if (!swapchain_ret) {
			swapchain_->swapchain = VK_NULL_HANDLE;

			std::string message{std::format("Failed to create swapchain: {}", swapchain_ret.error().message())};
			throw std::runtime_error{std::move(message)};
		}

		swapchain_ = swapchain_ret.value();
	}

	vkb::Swapchain &Swapchain::get() noexcept {
		return swapchain_.get();
	}

	vkb::Swapchain const &Swapchain::get() const noexcept {
		return swapchain_.get();
	}
}// namespace raytracing::vulkan
