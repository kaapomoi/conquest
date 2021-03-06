#pragma once

class cFunction {
public:
    virtual void call() = 0;
};


template<typename T>
class TemplatedFunction : public cFunction {
public:
    void (T::* m_fkt)();
    T* m_obj;

    TemplatedFunction(T* obj, void (T::* fkt)()) :
        m_fkt(fkt), m_obj(obj)
    {
    
    }

    void call() {
        (m_obj->*m_fkt)();
    }
};

