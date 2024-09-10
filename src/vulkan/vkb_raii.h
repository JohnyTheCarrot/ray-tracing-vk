#ifndef SRC_VULKAN_VKB_RAII_H_
#define SRC_VULKAN_VKB_RAII_H_

#include <VkBootstrap.h>
#include <concepts>
#include <memory>
#include <optional>
#include <utility>
#include <vma/vk_mem_alloc.h>

namespace raytracing::vulkan {
	template<class V, class Destroyer>
	    requires std::invocable<Destroyer, V const &>
	class UniqueContainer final {
		std::optional<V> m_Value;
		Destroyer        m_Destroyer;

		void cleanup() noexcept {
			if (m_Value.has_value())
				m_Destroyer(m_Value.value());
		}

	public:
		template<typename... Args>
		    requires std::constructible_from<V, Args...> and std::default_initializable<Destroyer>
		UniqueContainer(Args &&...args)
		    : m_Value{std::forward<Args>(args)...}
		    , m_Destroyer{} {
		}

		template<typename... Args>
		    requires std::constructible_from<V, Args...>
		UniqueContainer(Destroyer destroyer, Args &&...args)
		    : m_Value{std::forward<Args>(args)...}
		    , m_Destroyer{std::move(destroyer)} {
		}

		~UniqueContainer() noexcept {
			cleanup();
		}

		UniqueContainer(UniqueContainer const &) = delete;

		UniqueContainer &operator=(UniqueContainer const &) = delete;

		UniqueContainer(UniqueContainer &&other)
		    : m_Value{std::move(other.m_Value)}
		    , m_Destroyer{std::move(other.m_Destroyer)} {
			other.m_Value = std::nullopt;
		}

		UniqueContainer &operator=(UniqueContainer &&other) {
			if (&other == this)
				return *this;

			cleanup();
			m_Value     = std::move(other.m_Value);
			m_Destroyer = std::move(other.m_Destroyer);

			other.m_Value = std::nullopt;

			return *this;
		}

		[[nodiscard]]
		V &get() noexcept {
			return m_Value.value();
		}

		[[nodiscard]]
		V const &get() const noexcept {
			return m_Value.value();
		}

		V *operator->() {
			return &m_Value.value();
		}
	};

	class VkbInstanceDestroyer final {
	public:
		void operator()(vkb::Instance const &instance);
	};

	using UniqueVkbInstance = UniqueContainer<vkb::Instance, VkbInstanceDestroyer>;

	class VkbDeviceDestroyer final {
	public:
		void operator()(vkb::Device const &device);
	};

	using UniqueVkbDevice = UniqueContainer<vkb::Device, VkbDeviceDestroyer>;

	class VkbSwapchainDestroyer final {
	public:
		void operator()(vkb::Swapchain const &swapchain);
	};

	using UniqueVkbSwapchain = UniqueContainer<vkb::Swapchain, VkbSwapchainDestroyer>;

	class VmaAllocatorDestroyer final {
	public:
		void operator()(VmaAllocator const &allocator);
	};

	using UniqueVmaAllocator = UniqueContainer<VmaAllocator, VmaAllocatorDestroyer>;

	class VkShaderModuleDestroyer final {
		VkDevice device_;

	public:
		explicit VkShaderModuleDestroyer(VkDevice device);

		void operator()(VkShaderModule shader_module) const;
	};

	using UniqueVkShaderModule = std::unique_ptr<VkShaderModule_T, VkShaderModuleDestroyer>;

	class VkPipelineLayoutDestroyer final {
		VkDevice device_;

	public:
		explicit VkPipelineLayoutDestroyer(VkDevice device);

		void operator()(VkPipelineLayout pipeline_layout) const;
	};

	using UniqueVkPipelineLayout = std::unique_ptr<VkPipelineLayout_T, VkPipelineLayoutDestroyer>;
}// namespace raytracing::vulkan
#endif
