#include "swapchain.h"
#include "logical_device.h"
#include "src/diagnostics.h"
#include "src/vulkan/vk_exception.h"
#include <algorithm>
#include <cstdint>
#include <format>
#include <iterator>
#include <vulkan/vulkan_core.h>

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
		    std::vector<UniqueVkImageView> unique_views;
		    std::transform(
		            views->cbegin(), views->cend(), std::back_inserter(unique_views),
		            [&](VkImageView image_view) {
			            return UniqueVkImageView{image_view, VkImageViewDestroyer{device.get()}};
		            }
		    );

		    return unique_views;
	    }()}
	    , device_{&device} {
		std::uint32_t image_count{};
		if (VkResult const result{vkGetSwapchainImagesKHR(device_->get(), swapchain_->swapchain, &image_count, nullptr)
		    };
		    result != VK_SUCCESS) {
			throw VkException{"Could not get swapchain image count", result};
		}

		images_.resize(image_count);
		if (VkResult const result{
		            vkGetSwapchainImagesKHR(device_->get(), swapchain_->swapchain, &image_count, images_.data())
		    };
		    result != VK_SUCCESS) {
			throw VkException{"Could not get swapchain images", result};
		}
	}

	vkb::Swapchain &Swapchain::get() noexcept {
		return swapchain_.get();
	}

	vkb::Swapchain const &Swapchain::get() const noexcept {
		return swapchain_.get();
	}

	std::vector<UniqueVkImageView> const &Swapchain::get_views() const {
		return image_views_;
	}
}// namespace raytracing::vulkan
