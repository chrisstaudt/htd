#include "simple_graph_reader.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstddef>
#include <limits>
#include <stdexcept>

SimpleGraph read_pace_graph_mmap(const std::string & file_name)
{
    int fd = open(file_name.c_str(), O_RDONLY);

    if (fd == -1)
    {
        throw std::runtime_error("Could not open file");
    }

    struct stat sb;

    if (fstat(fd, &sb) == -1)
    {
        close(fd);
        throw std::runtime_error("Could not stat file");
    }

    std::size_t file_size = static_cast<std::size_t>(sb.st_size);

    const char * data = static_cast<const char *>(mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0));

    if (data == MAP_FAILED)
    {
        close(fd);
        throw std::runtime_error("Could not mmap file");
    }

    SimpleGraph result;
    result.node_count = 0;
    result.edge_count = 0;

    const char * ptr = data;
    const char * end = data + file_size;

    try
    {
        while (ptr < end)
        {
            if (*ptr == 'c')
            {
                while (ptr < end && *ptr != '\n')
                {
                    ++ptr;
                }

                if (ptr < end)
                {
                    ++ptr;
                }

                continue;
            }

            if (*ptr == '\n' || *ptr == '\r')
            {
                ++ptr;
                continue;
            }

            break;
        }

        if (ptr >= end || *ptr != 'p')
        {
            throw std::runtime_error("Invalid header");
        }

        while (ptr < end && *ptr != ' ')
        {
            ++ptr;
        }

        if (ptr < end)
        {
            ++ptr;
        }

        while (ptr < end && *ptr != ' ')
        {
            ++ptr;
        }

        if (ptr < end)
        {
            ++ptr;
        }

        while (ptr < end && *ptr >= '0' && *ptr <= '9')
        {
            result.node_count = result.node_count * 10 + (*ptr++ - '0');
        }

        while (ptr < end && (*ptr == ' ' || *ptr == '\t'))
        {
            ++ptr;
        }

        while (ptr < end && *ptr >= '0' && *ptr <= '9')
        {
            result.edge_count = result.edge_count * 10 + (*ptr++ - '0');
        }

        while (ptr < end && *ptr != '\n')
        {
            ++ptr;
        }

        if (ptr < end)
        {
            ++ptr;
        }

        result.edges.reserve(static_cast<std::size_t>(result.edge_count) * 2);

        while (ptr < end)
        {
            if (*ptr == 'c' || *ptr == '\n' || *ptr == '\r')
            {
                while (ptr < end && *ptr != '\n')
                {
                    ++ptr;
                }

                if (ptr < end)
                {
                    ++ptr;
                }

                continue;
            }

            int tail = 0;
            bool negative = false;

            if (*ptr == '-')
            {
                negative = true;
                ++ptr;
            }

            while (ptr < end && *ptr >= '0' && *ptr <= '9')
            {
                tail = tail * 10 + (*ptr++ - '0');
            }

            if (negative)
            {
                tail = -tail;
            }

            while (ptr < end && (*ptr == ' ' || *ptr == '\t'))
            {
                ++ptr;
            }

            int head = 0;
            negative = false;

            if (*ptr == '-')
            {
                negative = true;
                ++ptr;
            }

            while (ptr < end && *ptr >= '0' && *ptr <= '9')
            {
                head = head * 10 + (*ptr++ - '0');
            }

            if (negative)
            {
                head = -head;
            }

            while (ptr < end && *ptr != '\n')
            {
                ++ptr;
            }

            if (ptr < end)
            {
                ++ptr;
            }

            if (tail > 0 && head > 0 &&
                tail <= std::numeric_limits<uint32_t>::max() &&
                head <= std::numeric_limits<uint32_t>::max())
            {
                result.edges.push_back(static_cast<uint32_t>(tail));
                result.edges.push_back(static_cast<uint32_t>(head));
            }
        }

        munmap(const_cast<char *>(data), file_size);
        close(fd);

        return result;
    }
    catch (...)
    {
        munmap(const_cast<char *>(data), file_size);
        close(fd);

        throw;
    }
}
