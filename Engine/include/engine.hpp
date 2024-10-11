#pragma once

#include <tuple>
#include <vector>
#include <chrono>
#include <algorithm>

#include "graphic_manager/fps_counter_renderer.hpp"

struct IManager
{
	virtual bool Update(float time) = 0;
	virtual ~IManager() = 0 {};
};

template <typename... Extensions>
class Engine
{
private:
	std::tuple<std::shared_ptr<Extensions>...> m_extensions;
	std::vector<std::shared_ptr<IManager>> m_managers;
	const std::chrono::time_point<std::chrono::steady_clock> time_start_point;

	std::atomic<bool> m_is_active;

public:
	Engine(std::tuple<std::shared_ptr<Extensions>...> extensions) : 
		time_start_point(std::chrono::steady_clock::now()),
		m_extensions(extensions)
	{
		m_is_active.store(true);
	};

	template<typename T>
	std::shared_ptr<T> AddManager()
	{
		auto manager = std::make_shared<T>(m_extensions, GetCurrentTimeStamp());
		m_managers.push_back(manager);
		return manager;
	}

	float GetCurrentTimeStamp() const
	{
		return std::chrono::duration<float>(std::chrono::steady_clock::now() - time_start_point).count();
	}

	bool IsActive() const
	{
		return m_is_active.load();
	}

	bool Update()
	{
		float time = GetCurrentTimeStamp();
		bool NextUpdate = true;
		std::for_each(m_managers.begin(), m_managers.end(), [&NextUpdate, time](auto& obj) {
			NextUpdate &= obj->Update(time);
			});
		std::apply([&NextUpdate, time](auto&... extension) {
			((NextUpdate &= extension->Update(time)) && ...);
			}, m_extensions);

		m_is_active.store(NextUpdate);

		return NextUpdate;
	}
};
