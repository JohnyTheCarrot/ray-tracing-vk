#ifndef SRC_CAMERA_H_
#define SRC_CAMERA_H_

#include "src/Singleton.h"
#include <glm/glm.hpp>

namespace raytracing {
	class Camera final : public engine::Singleton<Camera> {
		glm::vec3 origin_{};
		float     yaw_{};
		float     pitch_{};
		glm::vec3 forward_{0, 0, 1};
		glm::vec3 right_{1, 0, 0};
		glm::vec3 up_{0, 1, 0};

	public:
		void rotate(float yaw, float pitch);

		void translate(glm::vec3 translation);

		[[nodiscard]]
		glm::mat4 get_mat() const;
	};
};// namespace raytracing

#endif//  SRC_CAMERA_H_
