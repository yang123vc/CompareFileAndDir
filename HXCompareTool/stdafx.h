#include <QtWidgets>


template<typename T>
inline void SafeDeletePoint(T * p)
{
	if (p)
	{
		delete p;
		p = NULL;
	}
}