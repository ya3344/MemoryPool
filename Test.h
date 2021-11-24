#pragma once
class Test
{
public:
	Test();
	~Test();

public:
	void IncreaseA() { ++ a;}
private:
	int a = 1;
	short b = 2;
	__int16 c = 3;
};

