#include "GKCli.h"
#include <wx/app.h> // Include wxWidgets application infrastructure (even for console apps)
#include <wx/cmdline.h> // Include wxWidgets command line processing

// To use certain wxWidgets features (e.g., wxString) in a console application,
// it is best to define a wxAppConsole
class MyApp : public wxAppConsole
{
public:
    
    virtual bool OnInit() wxOVERRIDE; // Override the OnInit method for initialization
    virtual int OnRun() wxOVERRIDE; // Override the OnRun method for the main application logic
};

wxIMPLEMENT_APP_CONSOLE(MyApp); // Inform wxWidgets about the application class

bool MyApp::OnInit()
{
    // Perform wxWidgets-related initialization here (if needed)
    // For example, handling command line arguments
    // wxCmdLineParser parser(argc, argv);
    // ... setup parser ...
    // parser.Parse();

    // For this simple CLI, OnInit may not need to do much
    if (!wxAppConsole::OnInit())
        return false;

    return true; // Initialization successful
}

int MyApp::OnRun()
{
    // Create and execute the CLI application
    GKCli cliApp; // Create an instance of the GKCli class
    cliApp.Run(); // Start the CLI loop

    // OnRun should return an exit code
    return 0; // Exit the program
}