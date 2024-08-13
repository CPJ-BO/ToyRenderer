#pragma once

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// spdlog引用导致的宏冲突
#undef near             // <minwindef.h>
#undef far
#undef CreateSemaphore  // <winbase.h>
#undef CopyFile
#undef UpdateResource
#undef CreateFile       // <fileapi.h>
#undef LoadString       // <libloaderapi.h>
