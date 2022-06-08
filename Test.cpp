#include "pch.h"
#include "Test.h"

Test::Test()
{
	testInt = 15;
	testChar = 20;
}

Test::~Test()
{
	testInt = -1;
	testChar = -1;
}
