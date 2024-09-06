#ifndef SINGLETON_H
#define SINGLETON_H

namespace raytracing::engine {
	template<typename T>
	class Singleton {
	public:
		static T &get_instance() {
			static T instance{};
			return instance;
		}

		virtual ~Singleton() = default;

		Singleton(Singleton const &other) = delete;

		Singleton(Singleton &&other) = delete;

		Singleton &operator=(Singleton const &other) = delete;

		Singleton &operator=(Singleton &&other) = delete;

	protected:
		Singleton() = default;
	};
}// namespace raytracing::engine
#endif// SINGLETON_H
