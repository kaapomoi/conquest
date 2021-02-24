#pragma once

#include <core/Engine.h>
#include <util/cFunction.h>

//typedef void (*callback_function)();

class UIClickable
{
public:
	UIClickable();
	virtual ~UIClickable();

	virtual void ClearCallbackFunctions();

	virtual void OnClick();
	virtual void OnClick(k2d::vf2d relative_click_pos);


	template<class T>
	void AddCallbackFunction(T* obj, void (T::* mem_fkt)()) {
		cFunction* m_func = new TemplatedFunction<T>(obj, mem_fkt);
		callbacks.push_back(m_func);
	}

private:
	std::vector<cFunction*> callbacks;

};