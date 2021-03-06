#include <ui/UIClickable.h>


UIClickable::UIClickable()
{

}

UIClickable::~UIClickable()
{

}

void UIClickable::ClearCallbackFunctions()
{
	callbacks.clear();
}

void UIClickable::OnClick()
{
	// call all the functions on click
	for (cFunction* f : callbacks)
	{
		f->call();
	}
}

void UIClickable::OnClick(k2d::vf2d relative_click_pos)
{
	// Just a thing for stuff

	OnClick();
}
