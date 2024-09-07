#ifndef VKB_RAII
#define VKB_RAII

#include <VkBootstrap.h>
#include <concepts>
#include <optional>
#include <utility>

namespace raytracing {
	template<class V, class Destroyer>
	    requires std::invocable<Destroyer, V const &>
	class UniqueContainer final {
		std::optional<V> m_Value;
		Destroyer        m_Destroyer;

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
			if (m_Value.has_value())
				m_Destroyer(m_Value.value());
		}

		UniqueContainer(UniqueContainer const &) = delete;

		UniqueContainer &operator=(UniqueContainer const &) = delete;

		UniqueContainer(UniqueContainer &&other)
		    : m_Value{std::move(other.m_Value)}
		    , m_Destroyer{std::move(other.m_Destroyer)} {
			other.m_Value = std::nullopt;
		}

		[[nodiscard]]
		UniqueContainer &
		operator=(UniqueContainer &&other) {
			if (&other == this)
				return *this;

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

	class VkbInstanceDestroyer {
	public:
		void operator()(vkb::Instance const &instance);
	};

	using UniqueVkbInstance = UniqueContainer<vkb::Instance, VkbInstanceDestroyer>;
}// namespace raytracing
#endif
