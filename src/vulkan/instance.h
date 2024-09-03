#ifndef INSTANCE_H
#define INSTANCE_H
#include <memory>

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

	public:
		Instance(std::string_view applicationName, std::string_view engineName);
	};
}// namespace raytracing::vulkan

#endif//INSTANCE_H
