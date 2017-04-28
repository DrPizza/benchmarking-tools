#pragma once

#include "targetver.h"

#pragma warning(disable: 4514) // warning C4514: '%s' unreferenced inline function has been removed
#pragma warning(disable: 4582) // warning C4582: '%s': constructor is not implicitly called
#pragma warning(disable: 4583) // warning C4582: '%s': destructor is not implicitly called
#pragma warning(disable: 4623) // warning C4623: '%s' default constructor was implicitly defined as deleted
#pragma warning(disable: 4625) // warning C4625: '%s' copy constructor was implicitly defined as deleted
#pragma warning(disable: 4626) // warning C4626: '%s' assignment operator was implicitly defined as deleted
#pragma warning(disable: 4668) // warning C4668: '%s' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning(disable: 4710) // warning C4710: '%s': function not inlined
#pragma warning(disable: 4711) // warning C4711: '%s': selected for automatic inline expansion
#pragma warning(disable: 4820) // warning C4820: '%s': '%d' bytes padding added after data member '%s'

#define STRICT
#define NOMINMAX

#include <Windows.h>

#if !defined(_STL_EXTRA_DISABLED_WARNINGS)
#define _STL_EXTRA_DISABLED_WARNINGS 4061 4365 4571 4623 4625 4626 4710 4774 4820 4987 5026 5027
#endif

#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>
#include <iomanip>
#include <array>
#include <numeric>
#include <condition_variable>
#include <mutex>
#include <locale>
#include <codecvt>
#include <filesystem>
#include <variant>

#include <intrin.h>

#pragma warning(disable: 4324) // warning C4234: structure was padded due to alignment specifier
#pragma warning(disable: 4625) // warning C4625: '%s': copy constructor was implicitly defined as deleted
#pragma warning(disable: 4626) // warning C4626: '%s': assignment operator was implicitly defined as deleted
#pragma warning(disable: 4820) // warning C4820: '%s': '%d' bytes padding added after data member '%s'
#pragma warning(disable: 5026) // warning C5026: '%s': move constructor was implicitly defined as deleted
#pragma warning(disable: 5027) // warning C5027: '%s': move assignment operator was implicitly defined as deleted
