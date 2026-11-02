#ifndef FDBCORETHREADPOOLF
#define FDBCORETHREADPOOLF
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

//TODO remove this include
#include <GLM/vec4.hpp>
inline thread_local glm::vec4 threadColor;

struct MAPTHREAD
{
	static std::unordered_map<std::thread::id, glm::vec4> threadColorMap;
};
inline std::unordered_map<std::thread::id, glm::vec4> MAPTHREAD::threadColorMap = {};

namespace FDB::Core
{
	using job_t = std::function<void()>;
	struct JobPool
	{
	public:
		JobPool(std::uint32_t threadCount = std::thread::hardware_concurrency());
		JobPool(const JobPool&) = delete;
		JobPool& operator=(const JobPool&) = delete;
		JobPool& operator=(JobPool&& other) noexcept = delete;
		JobPool(JobPool&& other) noexcept = delete;
		~JobPool();
	public:
		void PushJob(job_t&& job);
		void WaitIdle();
	private:
		void WorkerLoop(std::stop_token token);

	private:
		std::vector<std::jthread> m_threads;
		std::deque<job_t> m_jobs;
		std::atomic<std::uint64_t> m_jobCount = 0;
		std::condition_variable m_jobCv, m_idleCv;
		std::mutex m_mutex;

	};

	using jobPool_t = JobPool;
}

#endif
