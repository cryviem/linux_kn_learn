#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include <map>
#include "choen_main.h"

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
    std::cout << "-r [-nw]            To read, -nw for non blocking mode " << std::endl;
    std::cout << "-w [-nw] <string>   To write, -nw for non blocking mode " << std::endl;
    std::cout << "-io_setv            IOCTL set by value " << std::endl;
    std::cout << "-io_setp            IOCTL set by pointer " << std::endl;
    std::cout << "-io_getp            IOCTL get by pointer " << std::endl;
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
    close(fd);
} 

static void read_device_nblk(const char* dev_name)
{
    int fd = open(dev_name, (O_RDONLY|O_NONBLOCK));
    if (fd < 0)
    {
        std::cout << "Fail to open fail, ret " << fd << std::endl;
        return;
    }
    char rbuf[MAX_BUFF];
    memset(rbuf, 0, MAX_BUFF);
    bool is_completed = false;
    do
    {
        ssize_t ret = read(fd, rbuf, MAX_BUFF);
        std::cout << "read return " << ret << std::endl;
        if (ret >= 0)
        {
            std::cout << "Read " << ret << " bytes, > " << rbuf << std::endl;
            is_completed = true;
        }
        else 
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::cout << "Read blocked, should wait for data available" << std::endl;
                pollfd poll_pf;
                poll_pf.fd = fd;
                poll_pf.events = POLLIN;
                int r = poll(&poll_pf, 1, -1);
                if (r > 0 && (poll_pf.revents & POLLIN))
                {
                    std::cout << "Read ready, go ahead and get data" << std::endl;
                }
                else
                {
                    std::cout << "Something wrong" << std::endl;
                    is_completed = true;
                }
            }
            else
            {
                std::cout << "Something wrong, ret " << ret << ", errno " << errno << std::endl;
                is_completed = true;
            }
        }
    } while (!is_completed);
    
    close(fd);
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
    close(fd);
} 

static void write_device_nblk(const char* dev_name, char* buf, int len)
{
    if (buf == nullptr || len < 0)
    {
        std::cout << "Bad param!" << std::endl;
        return;
    }

    int fd = open(dev_name, (O_WRONLY|O_NONBLOCK));
    if (fd < 0)
    {
        std::cout << "Fail to open fail, ret " << fd << std::endl;
        return;
    }

    bool is_completed = false;
    do
    {
        ssize_t ret = write(fd, buf, len);
        if (ret > 0)
        {
            std::cout << "Write success "<< ret << "/" << len << " bytes" << std::endl;
            is_completed = true;
        }
        else 
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::cout << "Write blocked, should wait for slot available" << std::endl;
                pollfd poll_pf;
                poll_pf.fd = fd;
                poll_pf.events = POLLOUT;
                int r = poll(&poll_pf, 1, -1);
                if (r > 0 && (poll_pf.revents & POLLOUT))
                {
                    std::cout << "Write ready, go ahead and set data" << std::endl;
                }
                else
                {
                    std::cout << "Something wrong" << std::endl;
                    is_completed = true;
                }
            }
            else
            {
                std::cout << "Something wrong, ret " << ret << ", errno " << errno << std::endl;
                is_completed = true;
            }
    
        }
    } while (!is_completed);

    close(fd);
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
    close(fd);
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
        if (argc == 3 && 0 == std::string(argv[2]).compare("-nw"))
        {
            /*non blocking mode*/
            read_device_nblk(DEVICE);
        }
        else
        {
            read_device(DEVICE);
        }
        
        break;

        case CMD_WRITE:
        if (argc == 4 && 0 == std::string(argv[2]).compare("-nw"))
        {
            /*non blocking mode*/
            std::string w_str = argv[3];
            write_device_nblk(DEVICE, (char*)w_str.data(), w_str.length());
        }
        else if (argc == 3)
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