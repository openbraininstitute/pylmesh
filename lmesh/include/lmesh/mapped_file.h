#pragma once

#ifdef _WIN32
  #include <windows.h>
#else
  #include <sys/mman.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h>
#endif

namespace pylmesh
{
struct MappedFile
{
    const char* data = nullptr;
    size_t      size = 0;

#ifdef _WIN32
    HANDLE hFile   = INVALID_HANDLE_VALUE;
    HANDLE hMap    = nullptr;
#else
    int    fd      = -1;
#endif

    bool open(const std::string& path)
    {
#ifdef _WIN32
        hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        LARGE_INTEGER li;
        GetFileSizeEx(hFile, &li);
        size = static_cast<size_t>(li.QuadPart);
        if (size == 0) return false;

        hMap = CreateFileMappingA(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (!hMap) return false;

        data = static_cast<const char*>(MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0));
        return data != nullptr;
#else
        fd = ::open(path.c_str(), O_RDONLY);
        if (fd < 0) return false;

        struct stat st;
        if (fstat(fd, &st) < 0) return false;

        size = static_cast<size_t>(st.st_size);
        if (size == 0) return false;

        data = static_cast<const char*>(
            mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));

        if (data == MAP_FAILED) { data = nullptr; return false; }

        // Tell the kernel we'll scan forward sequentially
        madvise(const_cast<char*>(data), size, MADV_SEQUENTIAL);
        return true;
#endif
    }

    ~MappedFile()
    {
#ifdef _WIN32
        if (data)                        UnmapViewOfFile(data);
        if (hMap  != nullptr)            CloseHandle(hMap);
        if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
#else
        if (data) munmap(const_cast<char*>(data), size);
        if (fd >= 0) ::close(fd);
#endif
    }

    // Non-copyable
    MappedFile() = default;
    MappedFile(const MappedFile&)            = delete;
    MappedFile& operator=(const MappedFile&) = delete;
};
}