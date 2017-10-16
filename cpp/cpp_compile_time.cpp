// not_null playground
// bfilipek.com

#include <iostream>
#include <string_view>
#include <string>
#include <memory>


#include "gsl/gsl"

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
	{
		gsl::not_null<App *> myApp = new App("Poker");

		// we can delete it, but cannot assign null
		delete myApp;
		myApp = nullptr;
	}

	{
		// cannot invoke such function, contract violation
		RunApp(nullptr);
	}

	{
		gsl::not_null<App *> myApp = nullptr;
	}
}