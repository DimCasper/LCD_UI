#ifndef STRINGSTACK_H
#define STRINGSTACK_H

#include <arduino.h>

#define NORESULT 16777215

class stringstack
{
    public:
        stringstack(const size_t default_size = 1);

        // content control
        bool add(const char[]);
        char* get(uint16_t);
        void pop_back();
        void erase(uint16_t);
        void clear();
        char* operator[] (size_t i) { return *(container+i); };
        const char* operator[] (size_t i) const { return *(container+i); };

        // sort and search
        void sort();
        bool mergeSort(int sort_by=0);
        uint16_t contain(const char[]);

        // get info
        uint16_t size();
        uint16_t max_size();
        size_t length(uint16_t);

    protected:
    private:
        uint16_t _size;
        uint16_t capacity;
        //int _sort_by;
        bool sorted;
        char **container;
        bool increaseCapacity();
        bool _mergeSort(int,int);
        bool merge(int,int,int);
};

#endif // STRINGLIST_H
