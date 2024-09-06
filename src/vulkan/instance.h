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
		UniqueVkInstance instance_{nullptr};
		bool             enable_validation_layers_{false};

		[[nodiscard]]
		static bool check_validation_layer_support();

	public:
		Instance(
		        std::string_view application_name, std::string_view engine_name, bool enable_validation_layers = false
		);

		[[nodiscard]]
		VkInstance get() const noexcept;
	};
}// namespace raytracing::vulkan

#endif//INSTANCE_H
