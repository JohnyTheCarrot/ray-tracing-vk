#ifndef INSTANCE_H
#define INSTANCE_H

#include <memory>
#include <string_view>

struct VkInstance_T;
using VkInstance = VkInstance_T *;

namespace raytracing::vulkan {
	class VkInstanceDestroyer final {


	public:
		void operator()(VkInstance instance) const;
	};

	using UniqueVkInstance = std::unique_ptr<VkInstance_T, VkInstanceDestroyer>;

	class Instance final {
		UniqueVkInstance m_Instance{nullptr};
		bool             m_EnableValidationLayers{false};

		[[nodiscard]]
		static bool CheckValidationLayerSupport();

	public:
		Instance(std::string_view applicationName, std::string_view engineName, bool enableValidationLayers = false);
	};
}// namespace raytracing::vulkan

#endif//INSTANCE_H
