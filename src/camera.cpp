#include "camera.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

namespace raytracing {
	void Camera::rotate(float yaw, float pitch) {
		yaw_ += yaw;
		pitch_ += pitch;

		if (yaw_ > 360.f) {
			yaw_ -= 360.f;
		} else if (yaw_ < -360.f) {
			yaw += 360.f;
		}

		pitch_ = std::max(pitch_, -90.f);
		pitch_ = std::min(pitch_, 90.f);

		glm::mat4 yaw_mat{glm::rotate(glm::mat4{1.f}, glm::radians(yaw_), glm::vec3{0, 1, 0})};
		forward_ = glm::inverse(yaw_mat) * glm::vec4{0, 0, 1, 1};
		right_   = glm::cross(up_, forward_);
		forward_ = glm::normalize(forward_);
	}

	void Camera::translate(glm::vec3 translation) {
		origin_ += forward_ * translation.z;
		origin_ += right_ * translation.x;
		origin_ += up_ * translation.y;
	}

	glm::mat4 Camera::get_mat() const {
		auto rot_mat{glm::rotate(glm::mat4{1.f}, glm::radians(pitch_), glm::vec3{1, 0, 0})};
		rot_mat = glm::rotate(rot_mat, glm::radians(yaw_), glm::vec3{0, 1, 0});
		auto const trans_mat{glm::translate(glm::mat4{1.f}, origin_)};

		return rot_mat * trans_mat;
	}
}// namespace raytracing
