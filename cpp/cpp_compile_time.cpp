// { autofold
// not_null playground
// bfilipek.com


#include <iostream>
#include <string_view>
#include <string>
#include <memory>
// }

#include "gsl/gsl"

// { autofold
class App
{
public:
	App(const std::string& str) : m_name(str) { }

	void Run() { std::cout << "Running " << m_name << "\n"; }
	void Shutdown() { std::cout << "App " << m_name << " is closing...\n"; }
	void Diagnose() { std::cout << "Diagnosing...\n"; }

private:
	std::string m_name;
};
// }


void RunApp(gsl::not_null<App *> pApp)
{
	pApp->Run();
	pApp->Shutdown();
}

void DiagnoseApp(gsl::not_null<App *> pApp)
{
	pApp->Diagnose();
}

int main()
{
    // first case: deleting and marking as null:
	{
		gsl::not_null<App *> myApp = new App("Poker");

		// we can delete it, but cannot assign null
		delete myApp;
		//myApp = nullptr;
	}

    // second case: breaking the contract
	{
		// cannot invoke such function, contract violation
		//RunApp(nullptr);
	}

    // assigning a null on initilization
	{
		//gsl::not_null<App *> myApp = nullptr;
	}
	
	std::cout << "Finished...\n";
}