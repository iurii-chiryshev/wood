#ifndef SINGLETON_H
#define SINGLETON_H

namespace core{

template <class T>
class Singleton {
public:
    static T& instance() {
        static T instance;
        return instance;
    }

private:
    Singleton();
    ~Singleton();
    Singleton(const Singleton &);
    Singleton& operator=(const Singleton &);
};

}


#endif // SINGLETON_H
