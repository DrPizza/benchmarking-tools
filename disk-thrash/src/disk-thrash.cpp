#include "stdafx.h"

#include "docopt.h"

namespace fs = std::experimental::filesystem;

DWORD get_sector_size(const fs::path& path) {
	fs::path drive = fs::absolute(path).root_path();

	DWORD sectors_per_cluster = 0ul;
	DWORD bytes_per_sector = 0ul;
	DWORD free_clusters = 0ul;
	DWORD total_clusters = 0ul;
	::GetDiskFreeSpaceW(drive.c_str(), &sectors_per_cluster, &bytes_per_sector, &free_clusters, &total_clusters);
	return bytes_per_sector;
}

unsigned __int64 align_down(unsigned __int64 value, unsigned __int64 alignment) {
	if(alignment == 0 || 0 != (alignment & (alignment - 1))) {
		return value;
	}

	return value & ~(alignment - 1ui64);
}

template<typename T>
T greatest_common_divisor(T a, T b) {
	if(a == 0) {
		return b;
	}
	if(b == 0) {
		return a;
	}
	T shift = 0;
	for(; 0 == ((a | b) & 1); ++shift) {
		a >>= 1;
		b >>= 1;
	}
	while(0 == (a & 1)) {
		a >>= 1;
	}
	do {
		while(0 == (b & 1)) {
			b >>= 1;
		}
		if(a > b) {
			std::swap(a, b);
		}
		b -= a;
	} while(b != 0);

	return a << shift;
}

template<typename T>
T least_common_multiple(T a, T b) {
	return (a * b) / greatest_common_divisor(a, b);
}

static const char usage_message[] =
R"(disk-thrash.

	Usage:
		disk-thrash [--target <path>] [--progress <bytes>]
		disk-thrash --help
		disk-thrash --version

	Options:
		--target <path>       Path to create test file [default: C:\]
		--progress <bytes>    Number of bytes already written [default: 0]
		--help                Show this text
		--version             Show the version
)";

int main(int argc, char* argv[])
{
	std::map<std::string, docopt::value> args = docopt::docopt(usage_message, { argv + 1, argv + argc }, true, "disk-thrash 0.1");

	const std::string& progress_arg = std::get<std::string>(args["--progress"]);
	char* end;
	static const unsigned __int64 initial_progress = std::strtoull(progress_arg.c_str(), &end, 10);

	fs::path target = std::get<std::string>(args["--target"]);

	ULARGE_INTEGER free_bytes = { 0 };
	::GetDiskFreeSpaceExW(target.wstring().c_str(), &free_bytes, nullptr, nullptr);

	target /= "space_filler";

	HANDLE space_filler = ::CreateFileW(target.wstring().c_str(), GENERIC_ALL, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_DELETE_ON_CLOSE, nullptr);

	DWORD sector_size = get_sector_size(target);
	constexpr size_t buffer_size = 1024 * 1024;
	free_bytes.QuadPart = align_down(free_bytes.QuadPart, least_common_multiple(static_cast<size_t>(sector_size), buffer_size));

	LARGE_INTEGER distance = { 0 };
	distance.QuadPart = static_cast<__int64>(free_bytes.QuadPart);
	::SetFilePointerEx(space_filler, distance, nullptr, FILE_BEGIN);
	::SetEndOfFile(space_filler);
	LARGE_INTEGER zero = { 0 };
	::SetFilePointerEx(space_filler, zero, nullptr, FILE_BEGIN);
	
	std::unique_ptr<unsigned char[]> even_buffer{ new unsigned char[buffer_size] };
	std::unique_ptr<unsigned char[]> odd_buffer { new unsigned char[buffer_size] };

	for(size_t i = 0; i < buffer_size; ++i) {
		even_buffer[i] = static_cast<unsigned char>(  i & 0xff );
		odd_buffer [i] = static_cast<unsigned char>(~(i & 0xff));
	}

	std::unique_ptr<unsigned char[]> read_buffer{ new unsigned char[buffer_size] };
	
	const size_t blocks_to_write = free_bytes.QuadPart / buffer_size;
	for(size_t iterations = 0; ; ++iterations) {
		unsigned char* buffer = (iterations % 2) ? odd_buffer.get() : even_buffer.get();
		for(size_t i = 0; i < blocks_to_write; ++i) {
			DWORD bytes_written = 0ul;
			if(FALSE == ::WriteFile(space_filler, buffer, buffer_size, &bytes_written, nullptr)
			|| bytes_written != buffer_size) {
				std::cout << "write error detected at offset " << (i * buffer_size) << std::endl;
			}
		}
		::SetFilePointerEx(space_filler, zero, nullptr, FILE_BEGIN);
		for(size_t i = 0; i < blocks_to_write; ++i) {
			DWORD bytes_read = 0ul;
			if(FALSE == ::ReadFile(space_filler, read_buffer.get(), buffer_size, &bytes_read, nullptr)
			|| bytes_read != buffer_size) {
				std::cout << "read error detected at offset " << (i * buffer_size) << std::endl;
			}
			if(0 != std::memcmp(read_buffer.get(), buffer, buffer_size)) {
				std::cout << "read mismatch detected at offset " << (i * buffer_size) << std::endl;
			}
		}
		::SetFilePointerEx(space_filler, zero, nullptr, FILE_BEGIN);
		std::cout << "Written " << (initial_progress + (iterations + 1) * (blocks_to_write * buffer_size)) << std::endl;
	}

	::CloseHandle(space_filler);
    return 0;
}

