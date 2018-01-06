#ifndef MYSTACK_H
#define MYSTACK_H

#include <arduino.h>
#include <limits.h>

#define NORESULT UINT_MAX

template<class stacktype>
class mystack
{
    public:
        mystack(const size_t default_size = 1);

        // content control
        bool add(const stacktype);
        void pop_back();
        stacktype* begin() { return container; };
        stacktype* end() { return container+_size-1; };
        stacktype get(uint16_t);
        stacktype& operator[] (size_t i) { return *(container+i); };
        const stacktype& operator[] (size_t i) const { return *(container+i); };
        void erase(uint16_t);
        void clear();

        // sort and search
        void sort(int (*)(const void*,const void*));
        bool mergeSort(int (*)(const void*,const void*));
        uint16_t contain(const stacktype);

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
        stacktype *container;
        bool increaseCapacity();
        bool _mergeSort(const int,const int);
        bool merge(const int,const int,const int);
        int (*_compare)(const void*,const void*);
};

// ==================================================

template<class stacktype>
mystack<stacktype>::mystack(const size_t default_size)
{
    _size = 0;
    capacity = 1;
    //_sort_by = 0;
    sorted = false;
    if(default_size==0) container = NULL;
    container = (stacktype *)malloc(default_size*sizeof(stacktype));
}

template<class stacktype>
bool mystack<stacktype>::add(const stacktype value)
{
    if(_size == capacity)
    {
        if(!increaseCapacity()){ return false; }
    }
    container[_size] = value;
    _size++;
    sorted = false;
}

template<class stacktype>
void mystack<stacktype>::pop_back()
{
    if(_size) _size--;
    if(!_size) this->clear();
}

template<class stacktype>
void mystack<stacktype>::erase(uint16_t number)
{
    if(number>=_size) return ;
    container[number] = "";
    sorted = false;
}

template<class stacktype>
void mystack<stacktype>::clear()
{
    free(container);
    container = NULL;
    capacity = 0;
    _size = 0;
}

template<class stacktype>
stacktype mystack<stacktype>::get(uint16_t number)
{
    return container[number];
}

template<class stacktype>
uint16_t mystack<stacktype>::size()
{
    return _size;
}

template<class stacktype>
uint16_t mystack<stacktype>::max_size()
{
    return capacity;
}

template<class stacktype>
uint16_t mystack<stacktype>::contain(const stacktype target)
{
    if(sorted)
    {
        for(uint16_t i,l=0,r=_size-1,_i;l!=r;)
        {
            i = (l+r)/2;
            _i = (*_compare)(container[i],target);
            if(_i==0) return i;
            else _i<0 ? (l = i-1):(r = i+1);
        }
    }
    else
    {
        for(uint16_t i=0;i<_size;i++)
        {
            if((*_compare)(container[i],target)==0) return i;
        }
    }
    return NORESULT;
}

template<class stacktype>
void mystack<stacktype>::sort(int (*cmpfun)(const void*,const void*))
{
    qsort(container,_size,sizeof(stacktype),cmpfun);
}

template<class stacktype>
bool mystack<stacktype>::mergeSort(int (*cmpfun)(const void*,const void*))
{
    _compare = cmpfun;
    if(!_mergeSort(0,_size-1)) return false;
    sorted = true;
    return true;
}

/**********************************
 * Private method
 * include sort, about capacity
 **********************************/

template<class stacktype>
bool mystack<stacktype>::increaseCapacity()
{
    capacity++;
    stacktype *temp = (stacktype *)realloc(container, capacity*sizeof(stacktype));
    if(temp == NULL) return false;
    container = temp;
    return true;
}

#define MALLOC(p,s) \
    if(!((p)=(stacktype*)malloc(s))) { \
        return false; \
    }

template<class stacktype>
bool mystack<stacktype>::_mergeSort(const int front,const int rear)
{
    if(front<rear)         //base case check
    {
        int q = (front+rear)/2;                     //divide
        if(!_mergeSort(front,q))    return false;  //left
        if(!_mergeSort(q+1  ,rear)) return false;  //right
        //Serial.println(F("merge"));
        if(!merge(front,q,rear))    return false;
    }
    return true;
}

template<class stacktype>
bool mystack<stacktype>::merge(const int front,const int q,const int rear)
{
    int left_length=q-front+1,right_length=rear-q;
    stacktype *left,*right;
    MALLOC(left,(left_length)*sizeof(stacktype));MALLOC(right,(right_length)*sizeof(stacktype));
    for(int i=0;i<left_length ;i++) left[i]  = container[front+i];
    for(int i=0;i<right_length;i++) right[i] = container[q+i+1];
    for(int i=0,j=0,k=front;k<=rear;k++)
    {
        if(j==right_length || i!=left_length && _compare((void*)(left+i),(void*)(right+j))<0)
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

#endif // MYLIST_H
