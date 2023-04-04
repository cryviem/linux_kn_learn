#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>


#define DEVICE              "/dev/choen0"
#define MAX_BUFF            256

static volatile bool is_io_ready = false;
static volatile bool is_terminate = false;

static void signal_handler(int signum)
{
    char temp[100];
    sprintf(temp, "\nsignal_handler > %d \n", signum);
    write(STDOUT_FILENO, temp, strlen(temp));
    if (signum == SIGIO)
    {
        is_io_ready = true;
    }
    else if (signum == SIGINT)
    {
        is_terminate = true;
    }
} 

static int read_device(const char* dev_name)
{
    int ret = -1;
    int fd = open(dev_name, O_RDONLY|O_NONBLOCK);
    if (fd < 0)
    {
        std::cout << "Fail to open fail, ret " << fd << std::endl;
        return ret;
    }
    char rbuf[MAX_BUFF];
    memset(rbuf, 0, MAX_BUFF);
    ssize_t count = read(fd, rbuf, MAX_BUFF);
    if (count > 0)
    {
        std::cout << "Read " << count << " byte, > " << rbuf << std::endl;
        ret = 0;
    }
    else
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            std::cout << "Nothing to read..." << std::endl;
            ret = 1;
        }
        else
        {
            std::cout << "Read fail, ret " << count << " errno " << errno << std::endl;
        }
    }
    close(fd);
    return ret;
}

static void read_device_all()
{
    while (0 == read_device(DEVICE));
}

int main(int argc, char* argv[])
{
    struct sigaction sig_pf;
    sig_pf.sa_handler = signal_handler;
    sigaction(SIGIO, &sig_pf, NULL);
    sigaction(SIGINT, &sig_pf, NULL);

    int fd = open(DEVICE, O_RDONLY);
    if (fd < 0)
    {
        std::cout << "Fail to open fail, ret " << fd << std::endl;
        return 1;
    }

    if (-1 == fcntl(fd, F_SETOWN, getpid()))
    {
        std::cout << "Fail fcntl->F_SETOWN " << std::endl;
        is_terminate = true;
    }

    int outflags = fcntl(fd, F_GETFL);
    if (-1 == outflags)
    {
        std::cout << "Fail fcntl->F_GETFL " << std::endl;
        is_terminate = true;
    }
    
    if (-1 == fcntl(fd, F_SETFL, (outflags | FASYNC)))
    {
        std::cout << "Fail fcntl->F_SETFL " << std::endl;
        is_terminate = true;        
    }

    /* flush all pending data */
    read_device_all();
    while (!is_terminate)
    {
        std::cout << "I'm alive, PID: " << getpid() << ", is_io_ready: " << is_io_ready << std::endl;

        if (is_io_ready > 0)
        {
            read_device_all();
            is_io_ready = false;
        }
        sleep(2);
    }
    std::cout << "Bye cruel world ..."  << std::endl;
    close(fd);
    return 0;
}