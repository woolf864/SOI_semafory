#include <iostream>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include "kolejka.hpp"

element::element(int v = 0)
{
    val = v;
    next = this;
    prev = this;
}
element::element(element *p, element *n, int v = 0)
{
    val = v;
    prev = p;
    next = n;    
    next->prev = this;
    prev->next = this;
}
element::~element()
{
    wypnij();
}
void element::wepnij(element *p, element *n)
{
    prev = p;
    next = n;
    prev->next = this;
    next->prev = this;
}
void element::wypnij()
{
    next->prev = prev;
    prev->next = next;
    prev = NULL;
    next = NULL;
}
void element::set(int v)
{
    val = v;
}
int element::get()
{
    return val;
}




kolejka::kolejka()
{
    head = new element();
    mutex = new boost::interprocess::interprocess_semaphore(1);
}
kolejka::~kolejka()
{
    while(head->next != head)
        delete head->next;
    delete head;
    delete mutex;
}
void kolejka::push(int v)
{
    //P(*mutex);
    {
        new element(head,head->next,v);
    }
    //V(*mutex);
}
int kolejka::pop()
{
    int w;
    //P(*mutex);
    {
        if ( head->prev != head )
        {
            w = head->prev->val;
            delete head->prev;
        }
        else
        {
            w = -1;
        }
    }
    //V(*mutex);
    return w;
}
int kolejka::count()
{
    int i = 0;
    element *tmp;
    //P(*mutex);
    {
        //std::cout << "count in\n";
        tmp = head->next;
        while(tmp != head)
        {  
            ++i;
            tmp = tmp->next;
        }
        //std::cout << "jest juz: " << i << "\n";
        //std::cout << "count out\n";

    }
    //V(*mutex);
    return i;
}

/*void P(boost::interprocess::interprocess_semaphore &sem)
{
    sem.wait();
}
void V(boost::interprocess::interprocess_semaphore &sem)
{
    sem.post();
}

void P(boost::interprocess::interprocess_semaphore *sem)
{
    sem->wait();
}
void V(boost::interprocess::interprocess_semaphore *sem)
{
    sem->post();
}*/

