#include "Core/ThreadPool.h"

#include <iostream>

namespace FDB::Core
{
	JobPool::JobPool(std::uint32_t threadCount)
	{
		m_threads.reserve(threadCount);
		for (std::uint32_t i = 0; i < threadCount; ++i)
		{
			m_threads.push_back(std::jthread([this](std::stop_token token) { WorkerLoop(token); }));
		}
	}

	JobPool::~JobPool()
	{
		for (auto& thread : m_threads)
		{
			thread.request_stop();
		}
		m_jobCv.notify_all();
		m_threads.clear();
		m_jobs.clear();
	}

	void JobPool::PushJob(job_t job)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			++m_jobCount;
			m_jobs.push_back(std::move(job));
		}
		m_jobCv.notify_one();
	}

	void JobPool::WaitIdle()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_idleCv.wait(lock, [this] { return m_jobCount == 0; });
	}

	void JobPool::WorkerLoop(std::stop_token token)
	{
		job_t task = nullptr;
		while (!token.stop_requested())
		{
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_jobCv.wait(lock, [this, token] { return !m_jobs.empty() || token.stop_requested(); });
				if (m_jobs.empty())
					break;

				task = std::move(m_jobs.front());
				m_jobs.pop_front();
			}
			task();
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				if (--m_jobCount == 0)
					m_idleCv.notify_all();
			}
		}
		std::cout << "Thread " << std::this_thread::get_id() << " exiting\n";
	}
}
