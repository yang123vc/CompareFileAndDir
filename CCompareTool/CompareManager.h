#pragma once
class CompareManagerImpl;
class CompareManager
{
public:
	static CompareManager * Instance();
	CompareManager(void);

private:
	~CompareManager(void);
	CompareManagerImpl * m_p;
};

