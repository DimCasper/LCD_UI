#ifndef PATH_H
#define PATH_H

#include "stringstack.h"

class path : public stringstack
{
    public:
        char* fullpath();
        bool fullpath(char *,size_t);

    protected:
    private:
};

#endif // PATH_H
