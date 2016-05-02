#include "Camera.h"

#include "GLFW/glfw3.h"

#include "glm/ext.hpp"

Camera::Camera(float aspect)
{
	view = glm::lookAt(vec3(0,0,0), vec3(0,0,1), vec3(0, 1, 0));
	world = glm::inverse(view);
	proj = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 1000.0f);
	view_proj = proj * view;
}

void Camera::updateViewProj()
{
	view_proj = proj * view;
}

void Camera::setPerspective(float fov, float aspect, float near, float far)
{
	proj = glm::perspective(fov, aspect, near, far);
	updateViewProj();
}

void Camera::setLookAt(vec3 pos, vec3 center, vec3 up)
{
	view = glm::lookAt(pos, center, up);
	world = glm::inverse(view);
	updateViewProj();
}

void Camera::setPosition(vec3 pos)
{
	world[3] = vec4(pos, 1);
	view = glm::inverse(world);
	updateViewProj();
}

vec3 Camera::pickAgainstPlane(float x, float y, vec4 plane)
{
    float nxPos = x / 1280.0f; //replace these with your screen width and height
    float nyPos = y / 720.0f;

    float sxPos = nxPos - 0.5f;
    float syPos = nyPos - 0.5f;

    float fxPos = sxPos * 2;
    float fyPos = syPos * -2;

    mat4 inv_viewproj = glm::inverse(view_proj); //view_proj is the memeber variable

    vec4 mouse_pos(fxPos, fyPos, 1, 1);
    vec4 world_pos = inv_viewproj * mouse_pos;

    world_pos /= world_pos.w;

    vec3 cam_pos = world[3].xyz(); //world is the member variable
    vec3 dir = world_pos.xyz() - cam_pos;

    float t = -(glm::dot(cam_pos, plane.xyz()) + plane.w)
        / (glm::dot(dir, plane.xyz()));

    vec3 result = cam_pos + dir * t;

    return result;
}

FlyCamera::FlyCamera(float aspect, float new_speed) : Camera(aspect)
{
	this->m_speed = new_speed;
	m_clicked_down = false;
	yaw = 0;
	pitch = 0;
}

void FlyCamera::update(float dt)
{
	GLFWwindow* curr_window = glfwGetCurrentContext();

	float speed = m_speed;

	if (glfwGetKey(curr_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		speed *= 3.0f;
	}

	if (glfwGetKey(curr_window, GLFW_KEY_W) == GLFW_PRESS)
	{
		world[3] -= world[2] * speed * dt;
	}
	if (glfwGetKey(curr_window, GLFW_KEY_S) == GLFW_PRESS)
	{
		world[3] += world[2] * speed * dt;
	}
	if (glfwGetKey(curr_window, GLFW_KEY_A) == GLFW_PRESS)
	{
		world[3] -= world[0] * speed * dt;
	}
	if (glfwGetKey(curr_window, GLFW_KEY_D) == GLFW_PRESS)
	{
		world[3] += world[0] * speed * dt;
	}
	if (glfwGetKey(curr_window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		world[3] -= world[1] * speed * dt;
	}
	if (glfwGetKey(curr_window, GLFW_KEY_E) == GLFW_PRESS)
	{
		world[3] += world[1] * speed * dt;
	}
	

	if (glfwGetMouseButton(curr_window, 1) == GLFW_PRESS)
	{
		double x_delta, y_delta;



		glfwGetCursorPos(curr_window, &x_delta, &y_delta);

		glfwSetCursorPos(curr_window, 1280.f / 2.f, 720.f / 2.f);

		double post_set_x, post_set_y;

		glfwGetCursorPos(curr_window, &post_set_x, &post_set_y);
		if (post_set_x == 1280.0f / 2.0f && post_set_y == 720.0f / 2.0f)
		{
			if (m_clicked_down)
			{
				x_delta -= (1280.f / 2.f);
				y_delta -= (720.f / 2.f);

				x_delta /= (1280.f / 2.f);
				y_delta /= (720.f / 2.f);

				x_delta *= -sensitivity;
				y_delta *= -sensitivity;

				yaw += (float)x_delta;
				pitch += (float)y_delta;

				if (pitch >= glm::radians(89.f))
				{
					pitch = glm::radians(89.f);
				}
				if (pitch <= glm::radians(-89.f))
				{
					pitch = glm::radians(-89.f);
				}

					mat4 pitch_mat = glm::rotate((pitch), vec3(1, 0, 0));
					mat4 yaw_mat = glm::rotate((yaw), vec3(0, 1, 0));

					mat4 transform = yaw_mat * pitch_mat;

					transform[3] = world[3];
					world = transform;
					view = glm::inverse(world);
					view_proj = proj * view;
			}
			m_clicked_down = true;
		}
	}
	else
	{
		m_clicked_down = false;
	}

	view = glm::inverse(world);
	updateViewProj();
}
