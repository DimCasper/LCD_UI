#include "stringstack.h"
#include <limits.h>

stringstack::stringstack(const size_t default_size)
{
    _size = 0;
    capacity = 1;
    //_sort_by = 0;
    sorted = false;
    if(default_size==0) container = NULL;
    container = (char **)malloc(default_size*sizeof(char*));
}

bool stringstack::add(const char *value)
{
    if(_size == capacity)
    {
        if(!increaseCapacity()){ return false; }
    }
    container[_size] = (char*)malloc(strlen(value)+1);
    strcpy(container[_size], value);
    _size++;
    sorted = false;
}

void stringstack::pop_back()
{
    free(container[_size-1]);
    _size--;
}

void stringstack::erase(uint16_t number)
{
    if(number>=_size) return ;
    container[number] = "";
    sorted = false;
}

void stringstack::clear()
{
    for(;_size>0;_size--)
    {
        free(container[_size-1]);
    }
    free(container);
    container = NULL;
    capacity = 0;
}

char* stringstack::get(uint16_t number)
{
    if(number>=_size) return '\0';
    return container[number];
}

uint16_t stringstack::size()
{
    return _size;
}

uint16_t stringstack::max_size()
{
    return capacity;
}

size_t stringstack::length(uint16_t number)
{
    if(number>=_size) return 0;
    return strlen(container[number]);
}

uint16_t stringstack::contain(const char target[])
{
    if(sorted)
    {
        for(uint16_t i,l=0,r=_size-1,_i;l!=r;)
        {
            i = (l+r)/2;
            _i = strcmp(container[i],target);
            if(_i==0) return i;
            else _i<0 ? (l = i-1):(r = i+1);
        }
    }
    else
    {
        for(uint16_t i=0;i<_size;i++)
        {
            if(strcmp(container[i],target)==0) return i;
        }
    }
    return NORESULT;
}

void stringstack::sort()
{
    _mergeSort(0,_size-1);
}

bool stringstack::mergeSort(int sort_by)
{
    if(!_mergeSort(0,_size-1)) return false;
    sorted = true;
    return true;
}

/*
 * Private method
 * include sort, about capacity
 */

bool stringstack::increaseCapacity()
{
    capacity++;
    char **temp = (char **)realloc(container, capacity*sizeof(char*));
    if(temp == NULL) return false;
    container = temp;
    return true;
}

#define MALLOC(p,s) \
    if(!((p)=(char**)malloc(s))) { \
        return false; \
    }

bool stringstack::_mergeSort(int front,int rear)
{
    if(front<rear)         //base case check
    {
        int q = (front+rear)/2;                     //divide
        if(!_mergeSort(front,q))    return false;  //left
        if(!_mergeSort(q+1  ,rear)) return false;  //right
        if(!merge(front,q,rear))    return false;
    }
    return true;
}

bool stringstack::merge(int front,int q,int rear)
{
    int left_length=q-front+1,right_length=rear-q;
    char **left,**right;
    MALLOC(left,(left_length+1)*sizeof(char*));MALLOC(right,(right_length+1)*sizeof(char*));
    left[left_length] = new char(CHAR_MAX);
    right[right_length] = new char(CHAR_MAX);
    int i,j,k;
    for(i=0;i<left_length ;i++) left[i]  = container[front+i];
    for(i=0;i<right_length;i++) right[i] = container[q+i+1];
    for(i=0,j=0,k=front;k<=rear;k++)
    {
        if(strcmp(left[i],right[j])<0)
        {
            container[k] = left[i++];
        }
        else
        {
            container[k] = right[j++];
        }
    }

    free(left);free(right);
    return true;
}

#undef MALLOC(p,s)
