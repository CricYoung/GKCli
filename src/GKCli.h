// GKCli.h
#ifndef YccGKCliH
#define YccGKCliH

#include <vector> 
#include <wx/string.h> // wxString

class GKCli {
private:
    std::vector<wxString> commandHistory; // Stores the history of commands entered by the user
    int historyIndex; // Index for navigating the command history
    int cursorPosition; // Cursor position in the current input string
    wxString currentInput; // Stores the current input string

    void HandleEscapeSequence(char seq[2]); // Handles escape sequences for special keys
    void HandleBackspace(); // Handles the backspace key
    void HandleCharacterInput(const wxString& utf8Char); // Handles character input, including UTF-8 characters
    void ProcessInput(wxString& line); // Processes user input
    int GetCurWcWidth(); // Gets the display width of the current input string
    void MoveCursor(); // Moves the cursor to the correct position
public:
    GKCli(); // Constructor for the GKCli class
    void Run(); // Starts the main CLI loop
};

#endif // YccGKCliH
