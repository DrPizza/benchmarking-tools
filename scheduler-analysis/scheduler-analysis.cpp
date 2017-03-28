#include "stdafx.h"

void scheduler_bounce() {
	using namespace std::chrono_literals;

	constexpr size_t thread_count = 1;
	std::vector<std::thread> threads;
	threads.reserve(thread_count);

	SYSTEM_INFO si = { 0 };
	::GetSystemInfo(&si);

	std::vector<std::vector<size_t> > usage_histograms{ thread_count, std::vector<size_t> (static_cast<size_t>(si.dwNumberOfProcessors), 0ULL) };
	std::vector<size_t> thread_hop_counters(thread_count);

	std::vector<size_t> garbage(thread_count); // just used to have some writes from the thread that won't be optimized out
	std::atomic<bool> clean_up = false;
	for(size_t i = 0; i < thread_count; ++i) {
		threads.push_back(std::thread([&](size_t num) {
			// since the system may be quite busy to start, let's make sure the threads are spread out to start with at the very least
			DWORD_PTR mask = 1ULL << (num * 2ULL);
			::SetThreadAffinityMask(::GetCurrentThread(), mask);
			std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
			start = std::chrono::high_resolution_clock::now();
			do {
				++garbage[num];
				end = std::chrono::high_resolution_clock::now();
			} while(end - start < 10s);

			//::SetThreadIdealProcessor(::GetCurrentThread(), static_cast<DWORD>(num * 2ULL));

			DWORD preferred_processor = ::GetCurrentProcessorNumber();
			DWORD previous_processor = preferred_processor;
			// now remove the mask and let's see where we get put
			mask = ~0ULL;
			::SetThreadAffinityMask(::GetCurrentThread(), mask);

			// bump priority to try to prevent getting displaced on a busy system
			//::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

			while(!clean_up) {
				++garbage[num];

				DWORD latest_processor = ::GetCurrentProcessorNumber();
				if(latest_processor != previous_processor) {
					++thread_hop_counters[num];
					previous_processor = latest_processor;
				}
				++usage_histograms[num][latest_processor];
			}
		}, i));
	}
	std::this_thread::sleep_for(60s);
	clean_up = true;
	for(std::thread& t : threads) {
		t.join();
	}

	for(size_t i = 0; i < thread_count; ++i) {
		std::cout << "thread " << i << " jumped " << thread_hop_counters[i] << " times. ";
		size_t total_runs = std::accumulate(std::begin(usage_histograms[i]), std::end(usage_histograms[i]), 0ULL);
		for(size_t j = 0; j < si.dwNumberOfProcessors; ++j) {
			std::cout << std::fixed << std::setprecision(2) << static_cast<double>(usage_histograms[i][j]) / static_cast<double>(total_runs) << " ";
		}
		std::cout << std::endl;
	}
}

typedef struct _PROCESSOR_POWER_INFORMATION {
	ULONG Number;
	ULONG MaxMhz;
	ULONG CurrentMhz;
	ULONG MhzLimit;
	ULONG MaxIdleState;
	ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

int main() {
	std::array<int, 4> cpu = { 0 };
	__cpuid(cpu.data(), 0x8000'0000);
	if(cpu[0] >= 0x8000'0004) {
		union {
			std::array<char, 48> brand;
			std::array<std::array<int, 4>, 3> registers;
		} data;
		__cpuid(data.registers[0].data(), 0x8000'0002);
		__cpuid(data.registers[1].data(), 0x8000'0003);
		__cpuid(data.registers[2].data(), 0x8000'0004);
		std::cout << data.brand.data() << std::endl;
	}

	SYSTEM_INFO si = { 0 };
	::GetSystemInfo(&si);

	std::unique_ptr<PROCESSOR_POWER_INFORMATION[]> ppi{ new PROCESSOR_POWER_INFORMATION[si.dwNumberOfProcessors] };

	::CallNtPowerInformation(ProcessorInformation, nullptr, 0, ppi.get(), sizeof(PROCESSOR_POWER_INFORMATION) * si.dwNumberOfProcessors);
	std::cout << "Maximum frequency: " << ppi[0].MaxMhz << " MHz" << std::endl;

	scheduler_bounce();
}
