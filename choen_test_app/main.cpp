#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include <map>
#include "choen.h"

#define DEVICE              "/dev/choen0"
#define MAX_BUFF            256

typedef enum {
    CMD_READ,
    CMD_WRITE,
    CMD_IOCTL_HELLO,
    CMD_IOCTL_SET_VALUE,
    CMD_IOCTL_SET_PTR,
    CMD_IOCTL_GET_PTR,
    CMD_INVALID
} cmd_t;

static std::map<std::string, cmd_t> cmd_table =
{
    {"-r", CMD_READ},
    {"-w", CMD_WRITE},
    {"-io", CMD_IOCTL_HELLO},
    {"-io_setv", CMD_IOCTL_SET_VALUE},
    {"-io_setp", CMD_IOCTL_SET_PTR},
    {"-io_getp", CMD_IOCTL_GET_PTR},
};

static void print_help()
{
    std::cout << "--CHOEN TEST APP------------- " << std::endl;
    std::cout << "-r            To read " << std::endl;
    std::cout << "-w <string>   To write " << std::endl;
    std::cout << "----------------------------- " << std::endl;
}

static void read_device(const char* dev_name)
{
    int fd = open(dev_name, O_RDONLY);
    if (fd < 0)
    {
        std::cout << "Fail to open fail, ret " << fd << std::endl;
        return;
    }
    char rbuf[MAX_BUFF];
    memset(rbuf, 0, MAX_BUFF);
    ssize_t ret = read(fd, rbuf, MAX_BUFF);
    if (ret > 0)
    {
        std::cout << "Read " << ret << " byte, > " << rbuf << std::endl;
    }
} 

static void write_device(const char* dev_name, char* buf, int len)
{
    if (buf == nullptr || len < 0)
    {
        std::cout << "Bad param!" << std::endl;
        return;
    }

    int fd = open(dev_name, O_WRONLY);
    if (fd < 0)
    {
        std::cout << "Fail to open fail, ret " << fd << std::endl;
        return;
    }

    ssize_t ret = write(fd, buf, len);
    if (ret == len)
    {
        std::cout << "Write OK" << std::endl;
    }
} 

static void ioctl_device(const char* dev_name, unsigned int cmd, unsigned long arg)
{
    int fd = open(dev_name, O_RDWR);
    if (fd < 0)
    {
        std::cout << "Fail to open fail, ret " << fd << std::endl;
        return;
    }

    int ret = ioctl(fd, cmd, arg);
    if (ret < 0)
    {
        std::cout << "Fail ioctl, ret " << ret << std::endl;
    }
}

int main(int argc, char* argv[])
{
    cmd_t cmd = CMD_INVALID;

    std::cout << "Hello cruel world" << std::endl;

    if (argc < 2)
    {
        std::cout << "No actions ..." << std::endl;
        print_help();
        return -1;
    }

    auto it = cmd_table.find(argv[1]);
    if (it != cmd_table.end())
    {
        cmd = it->second;
    }

    switch(cmd)
    {
        case CMD_READ:
        read_device(DEVICE);
        break;

        case CMD_WRITE:
        if (argc == 3)
        {
            std::string w_str = argv[2];
            write_device(DEVICE, (char*)w_str.data(), w_str.length());
        }
        else
        {
            std::cout << "Nothing to write ..." << std::endl;
        }
        break;

        case CMD_IOCTL_HELLO:
        ioctl_device(DEVICE, IOCTL_CMD_HELLO, 0);
        break;

        case CMD_IOCTL_SET_VALUE:
        if (argc == 3)
        {
            int l_value = std::stoi(argv[2]);
            ioctl_device(DEVICE, IOCTL_CMD_SET_VALUE, l_value);
        }
        else
        {
            std::cout << "Nothing to set ..." << std::endl;
        }
        break;

        case CMD_IOCTL_SET_PTR:
        if (argc == 3)
        {
            int l_value = std::stoi(argv[2]);
            ioctl_device(DEVICE, IOCTL_CMD_SET_PTR, (unsigned long)&l_value);
        }
        else
        {
            std::cout << "Nothing to set ..." << std::endl;
        }
        break;

        case CMD_IOCTL_GET_PTR:
        {
            int l_value = 0;
            ioctl_device(DEVICE, IOCTL_CMD_GET_PTR, (unsigned long)&l_value);
            std::cout << "CMD_IOCTL_GET_PTR " << l_value << std::endl;
        }
        break;

        default:
        print_help();
        break;
    }
    
    return 0;
}