// { autofold
// not_null playground
// bfilipek.com


#include <iostream>
#include <string_view>
#include <string>
#include <memory>
// }

#define GSL_THROW_ON_CONTRACT_VIOLATION
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


struct TestParams { };
void ReportError(const std::string& str) { }

void RunApp(gsl::not_null<App *> pApp)
{
	pApp->Run();
	pApp->Shutdown();
}

void DiagnoseApp(gsl::not_null<App *> pApp)
{
	pApp->Diagnose();
}

void TestAppCheck(App* pApp, TestParams* pParams)
{
	if (pApp && pParams)
	{
		// ...
	}
	else
		ReportError("null input params");
}

void TestApp(gsl::not_null<App *> pApp, gsl::not_null<TestParams *> pParams)
{
	// input pointers are valid
}

int main()
{
	auto myApp = std::make_unique<App>("Poker");
	auto myParams = std::make_unique<TestParams>();
	
	// older way:
	TestAppCheck(myApp.get(), myParams.get());
	
	// with catch:
	try
	{
	    // reset the pointer
	    myApp.reset(nullptr);
	    
		TestApp(myApp.get(), myParams.get());
		RunApp(myApp.get());
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << "\n";
		ReportError("null input params");
	}
	
	std::cout << "Finished...\n";
}