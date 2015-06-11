#include "IOHelper.h"

int max(int i, int j)
{
    return i > j ? i : j;
}

int readn(int fildes, void *buffer, size_t nbyte)
{
    int sucess = -1;
    int offset;

    offset = 0;
    while(nbyte > 0)
    {
        sucess = read(fildes, (char *)buffer + offset, nbyte);
        if (sucess == -1)
        {
            return -1;
        }
        if (sucess == 0)
        {
            break;
        }

        offset += sucess;
        nbyte -= sucess;
    }

    return offset;
}

int writen(int fildes, const void *buffer, size_t nbyte)
{
    int sucess = -1;
    int offset;

    offset = 0;

    while(nbyte > 0)
    {
        sucess = write(fildes, (const char *)buffer + offset, nbyte);
        if (sucess == -1)
        {
            return -1;
        }
        if (sucess == 0)
        {
            break;
        }

        offset += sucess;
        nbyte -=sucess;
    }

    return offset;
}

