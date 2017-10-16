#include <iostream>

class SimpleType
{
private:
	int a { 1 };    // << wow!
	int b { 1 };    // << wow2!
	std::string name { "string" }; // wow3!

public:
	SimpleType() {
		std::cout << "SimpleType::ctor, {" << a << ", " << b << ", \"" << name << "\"}\n";
	}

	~SimpleType() { std::cout << "SimpleType::destructor\n"; }
};

int main()
{
	SimpleType simpleObj;
}