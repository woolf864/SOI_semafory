#define P(s) ( s ).wait();
#define V(s) ( s ).post();

struct pstate 
{ 
    bool state; 
    boost::interprocess::interprocess_semaphore sem;
    pstate(bool st, int se) : sem(se),
                              state(st) {}
};

class element
{
  private:
    int val;
    element *next;
    element *prev;
  public:
    element(int v);
    element(element *p, element *n, int v);
    ~element();
    void wepnij(element *p, element *n);
    void wypnij();
    void set(int v);
    int get();

    friend class kolejka;
};

class kolejka
{
  private:
    element *head;
    int num;
    boost::interprocess::interprocess_semaphore *mutex;

  public:
    kolejka();
    ~kolejka();
    void push(int v);
    int pop();
    int count();
};

/*void P(boost::interprocess::interprocess_semaphore &sem);
void V(boost::interprocess::interprocess_semaphore &sem);
void P(boost::interprocess::interprocess_semaphore *sem);
void V(boost::interprocess::interprocess_semaphore *sem);*/
