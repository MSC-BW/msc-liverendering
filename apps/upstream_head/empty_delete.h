#pragma once
//http://stackoverflow.com/questions/15534146/how-to-wrap-a-raw-pointer-into-a-shared-ptr-and-prevent-shared-ptr-from-deleting
// usage: std::shared_ptr<ClassName> d(new ClassName(), empty_delete<ClassName>());
template <typename T>
struct empty_delete
{
	empty_delete()
	{
	}

	template <typename U>
	empty_delete(const empty_delete<U>&,
		typename std::enable_if<
			std::is_convertible<U*, T*>::value
		>::type* = nullptr)
	{
	}

	void operator()(T* const) const
	{
		// do nothing
	}
};
