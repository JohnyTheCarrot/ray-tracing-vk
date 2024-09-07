#include "swapchain.h"
#include "logical_device.h"
#include "src/diagnostics.h"
#include <algorithm>
#include <format>
#include <iterator>

namespace raytracing::vulkan {
	Swapchain::Swapchain(LogicalDevice const &device, std::optional<Swapchain> &&old)
	    : swapchain_{[&] {
		    vkb::SwapchainBuilder builder{device.get()};
		    if (old.has_value()) {
			    Logger::get_instance().log(LogLevel::Debug, "Recreating swapchain");
			    builder.set_old_swapchain(old.value().get());
		    } else {
			    Logger::get_instance().log(LogLevel::Debug, "Creating swapchain");
		    }

		    auto const swapchain_ret{builder.build()};

		    if (!swapchain_ret) {
			    if (old.has_value()) {
				    old->swapchain_->swapchain = VK_NULL_HANDLE;
			    }
			    std::string message{std::format("Failed to create swapchain: {}", swapchain_ret.error().message())};
			    throw std::runtime_error{std::move(message)};
		    }

		    return swapchain_ret.value();
	    }()}
	    , image_views_{[&] {
		    Logger::get_instance().log(LogLevel::Debug, "Creating image views for swapchain");
		    auto const views{swapchain_->get_image_views()};

		    if (!views) {
			    std::string message{std::format("Couldn't get swapchain image views: {}", views.error().message())};
			    throw std::runtime_error{std::move(message)};
		    }
		    std::vector<UniqueImageView> unique_views;
		    std::transform(
		            views->cbegin(), views->cend(), std::back_inserter(unique_views),
		            [&](VkImageView image_view) {
			            return UniqueImageView{image_view, ImageViewDestroyer{device.get()}};
		            }
		    );

		    return unique_views;
	    }()}
	    , device_{&device} {
	}

	vkb::Swapchain &Swapchain::get() noexcept {
		return swapchain_.get();
	}

	vkb::Swapchain const &Swapchain::get() const noexcept {
		return swapchain_.get();
	}
}// namespace raytracing::vulkan
