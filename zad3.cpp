#include <iostream>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "kolejka.hpp"

using namespace std;

//boost::random::uniform_int_distribution<> gen(1, 2500);
boost::mt19937 randGen(std::time(NULL) + getpid());
boost::uniform_int<> num1_2500(100,1500);
boost::variate_generator<boost::mt19937, boost::uniform_int<> > gen(randGen, num1_2500);
 
kolejka k1;
kolejka k2;

bool write_debug;
bool write_debug_more;

boost::interprocess::interprocess_semaphore mutex2(1);

boost::interprocess::interprocess_semaphore m_in1(1), m_in2(1);
boost::interprocess::interprocess_semaphore m_a1(10), m_b1(0), m_b2(0);


int i_in1 = 0, i_in2 = 0;

int db2 = 0;

void A1(int init);
void A2(int init);
void B1();
void B2();

int i = 0;
bool end = false;
bool end2 = false;


void A1(int init)
{
    int i = init;
    while(!end){
        if(i == INT_MAX)
            return;
        if(write_debug_more)
        {
            mutex2.wait();
            cout << "(A1)" << endl;
            mutex2.post();
        }
        P(m_a1);
        {
            if(end) break;

            P(m_in1);

            if(i_in1 >= 10)
            {
                V(m_in1);
                //P(m_a1);
            }
            else
            {
                if(write_debug)
                {
                    mutex2.wait();
                    cout << "(A1) w kolejce 1 jest \"" << i_in1 << "\" elementow; dodaje \"" << i << "\"" << endl;
                    mutex2.post();
                }
                i_in1++;
                k1.push(i++);
                //assert(i_in1 == k1.count());
                if(i_in1 >= 6)
                    V(m_b1);
                V(m_in1);
            }
        }
    }
}
void A2(int init)
{
    int i = init;
    while(!end)
    {
        if(i == INT_MAX)
            return;
        if(write_debug_more)
        {
            mutex2.wait();
            cout << "(A2)" << endl;
            mutex2.post();
        }
        P(m_in2);
        if(i_in2 >= 10)  
        {
            V(m_in2);
            if(write_debug)
            {
                mutex2.wait();
                cout << "(A2) is dead" << endl;
                mutex2.post();
            }
            return;
        }
        P(m_in1);
        if(write_debug)
        {
            mutex2.wait();
            cout << "(A2) dodaje " << i << " do kolejki 1; w kolejkach jest jest " << i_in1 << " / " << i_in2 << " elementow" << endl;
            mutex2.post();
        }
        i_in1++;
        k1.push(i++);
        //assert(i_in1 == k1.count());
        if(i_in1 >= 6)
            V(m_b1);
        V(m_in1);
        V(m_in2);
    }
}
void B1()
{
    int wyn;
    bool dec = false;
    while(!end)
    {
        P(m_b1);
        if(end) break;
        {
            if(write_debug_more)
            {
                mutex2.wait();
                cout << "(B1)" << endl;
                mutex2.post();
            }
            P(m_in2);
            P(m_in1);
            //if(i_in2 < 1000)
            {
                wyn = k1.pop();
                k2.push( wyn );
                if(write_debug)
                {
                    mutex2.wait();
                    cout << "(B1) przenosi " << wyn << " z kolejki 1 do 2;" << endl;
                    mutex2.post();
                }
                i_in1--;
                // assert(i_in1 == k1.count());
                if(i_in1 < 10)
                {
                    V(m_a1);
                }
                i_in2++;
                //assert(i_in2 == k2.count());
                //cout << "debug(1): " << i_in2 << endl;
                if(i_in2 >= 15)
                {
                    db2++;
                    //cout << "debug(2): " << db2 << endl;
                    V(m_b2);
                    //assert((k2.count() - db2) == 15);
                }
                V(m_in1);
                V(m_in2);
            }
        }
    }
}
void B2()
{
    while(!end2)
    {
        P(m_b2);
        if(end2) break;
        if(write_debug_more)
        {
            mutex2.wait();
            cout << "(B2)" << endl;
            mutex2.post();
        }
        P(m_in2);
        if(write_debug)
        {
            mutex2.wait();
            cout << "(B2) w kolejce 2 jest " << i_in2 << " elementow; usowam: " << k2.pop() << endl;
            mutex2.post();
        }
        else
        {
            k2.pop();
        }
        //k2.pop();
        i_in2--;
        db2--;
        assert((k2.count() - db2) == 14);
        // assert(i_in2 == k2.count());
        V(m_in2);
    }
}

int main(int argc, char *argv[])
{
    int work_for;
    if(argc > 1)
        work_for = atoi(argv[1]);
    else
        work_for = 1000;
    cout << "czas wykonania " << work_for << endl;
    if(argc > 2)
        write_debug = argv[2][0] == '1';
    if(argc > 3)
        write_debug_more = argv[3][0] == '1';
            
    boost::thread threadB1(&B1);
    boost::thread threadB21(&B2);
    //boost::thread threadB22(&B2);
    //boost::thread threadB23(&B2);
	boost::thread threadA1(&A1,(int)0);//(INT_MAX-1));
    boost::thread threadA2(&A2,(int)0);//(INT_MAX-1));
    boost::posix_time::millisec wait(work_for);
    boost::this_thread::sleep(wait);

    end = true;

    mutex2.wait();
    cout << "joining" << endl;
    mutex2.post();

    V(m_a1);
    V(m_b1);
	threadA1.join();
	threadA2.join();
	threadB1.join();

    boost::posix_time::millisec wait_b2(50);
    boost::this_thread::sleep(wait_b2);
    end2 = true;
    V(m_b2);
    //V(m_b2);
    //V(m_b2);

	threadB21.join();
	//threadB22.join();
	//threadB23.join();
    
    cout << "w k1: " << k1.count() << " / " << i_in1 << " / " << i_in1 - k1.count() << endl;
    cout << "last in k1: " << k1.pop() << endl;
    cout << "w k2: " << k2.count() << " / " << i_in2 << " / " << i_in2 - k2.count() << endl;
    cout << "do usuniecia w k2: " << db2 << endl;
    cout << "last in k2: " << k2.pop() << endl;

	return 0;
}

