// GKCli.cpp
#include "GKCli.h"

#include <iostream> // Standard C++ IO
#include <string>   // For std::string and std::getline
#include <csignal>  // For signal handling
#include <termios.h> // Provides terminal control functions, such as disabling canonical mode and echoing
#include <unistd.h> // Provides access to the POSIX operating system API, such as read and write functions
#include <wchar.h> // Provides wide character handling functions, such as wcswidth for calculating display width
#include <cstdlib> // For std::system
#include <sstream> // For std::ostringstream
#include <cstdio> // For popen and pclose

// Function to count characters in a string
bool CountChar(int& tAns,const wxString& tStr) {
    tAns=tStr.length();
    return true;
};

// Function to count bytes in a string
bool CountBytes(int& tAns,wxString& tStr,int tCharCount) {
    tAns=0;
    if(tCharCount == tStr.length()) {
        tAns = tStr.ToStdString().length(); // Corrected return value   
        return true; // Successfully counted the specified number of characters
    };
    if(tCharCount == 0 || tStr.length() < tCharCount) return false;
    char* tpEnd=(char*)(&(tStr.ToStdWstring()[tCharCount]));
    char* tpStart=(char*)(&(tStr.ToStdWstring()[0]));
    tAns=tpEnd-tpStart;
    return true;;
  }

// Function to calculate the display width of a string
int CountWidth(wxString& tStr,int tCharCount=-1) {
    if(tCharCount == -1) {
        tCharCount = tStr.length();
    }
    if(tCharCount == 0) return 0;
    return wcswidth(tStr.ToStdWstring().c_str(),tCharCount);
  };

// Constructor for the GKCli class
GKCli::GKCli() : historyIndex(-1), cursorPosition(0),currentInput(L"") { // Initialize historyIndex
    // Initialize parser and executor (default constructors used)
}

// Static method to handle Ctrl + C signal
void SignalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nInterrupt signal (Ctrl + C) received. Exiting..." << std::endl;
        std::exit(0);
    }
}

// Get the current display width of the input
int GKCli::GetCurWcWidth() {
    return CountWidth(currentInput,cursorPosition);
}

// Move the cursor to the correct position
void GKCli::MoveCursor() {
    int tWcWidth = GetCurWcWidth();
    std::cout << "\033[" << tWcWidth +3 << "G" << std::flush; // plus 3 for the prompt "> "
    return ;
}

// Handle escape sequences for arrow keys and other special keys
void GKCli::HandleEscapeSequence(char seq[2]) {
    if (seq[0] == '[') {
        if (seq[1] == 'A') { // Up arrow
            if (historyIndex > 0) {
                historyIndex--;
                currentInput = commandHistory[historyIndex].ToStdString();
                std::cout << "\r> \033[K" << currentInput << std::flush;
                cursorPosition=currentInput.length();
                MoveCursor();
            }
        } else if (seq[1] == 'B') { // Down arrow
            if (historyIndex < (int)commandHistory.size() - 1) {
                historyIndex++;
                currentInput = commandHistory[historyIndex].ToStdString();
                std::cout << "\r> \033[K" << currentInput << std::flush;
                cursorPosition=currentInput.length();
                MoveCursor();
            } else if (historyIndex == (int)commandHistory.size() - 1) {
                historyIndex++;
                currentInput = "";
                std::cout << "\r> \033[K" << std::flush;
                cursorPosition = 0;
            }
        } else if (seq[1] == 'D') { // Left arrow
            if (cursorPosition > 0) {
                cursorPosition--;
                MoveCursor();
            }
        } else if (seq[1] == 'C') { // Right arrow
            int tCount=0;
            CountChar(tCount,currentInput);
            if (cursorPosition < tCount) {
                cursorPosition++;
                MoveCursor();
            }
        } else if (seq[1] == '3') { // Delete key
            if (read(STDIN_FILENO, &seq[0], 1) == 1 && seq[0] == '~') {
                int tCount=0;
                tCount = currentInput.length();
                if (cursorPosition < tCount) {
                    currentInput.erase(cursorPosition, 1);
                    if(cursorPosition == tCount) {
                        cursorPosition--;
                    }
                    std::cout << "\r> " << currentInput << "\033[K" << std::flush;
                    MoveCursor();
                }
            }
        }
    }
}

// Handle backspace key
void GKCli::HandleBackspace() {
    if (!currentInput.IsEmpty() && cursorPosition > 0) {
        currentInput.erase(cursorPosition - 1, 1);
        cursorPosition--;
        std::cout << "\r> " << currentInput << "\033[K" << std::flush;
        MoveCursor();
    }
}

// Handle character input
void GKCli::HandleCharacterInput(const wxString& utf8Char) {
    if(utf8Char.length() == 0) return;
    currentInput.insert(cursorPosition, utf8Char);
    cursorPosition += utf8Char.length();
    std::cout << "\r> " << currentInput << "\033[K" << std::flush;
    MoveCursor();
}

// Process user input
void GKCli::ProcessInput(wxString& line) {
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == 10) { // Enter key
            std::cout << std::endl;
            line = currentInput;
            break;
        } else if (c == 27) { // Escape sequence
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1) {
                HandleEscapeSequence(seq);
            }
        } else if (c == 127) { // Backspace
            HandleBackspace();
        } else {
            std::string utf8Char(1, c);
            if ((c & 0x80) != 0) { // Multi-byte UTF-8 character
                int additionalBytes = 0;
                if ((c & 0xE0) == 0xC0) additionalBytes = 1; // 2-byte character
                else if ((c & 0xF0) == 0xE0) additionalBytes = 2; // 3-byte character
                else if ((c & 0xF8) == 0xF0) additionalBytes = 3; // 4-byte character

                for (int i = 0; i < additionalBytes; ++i) {
                    if (read(STDIN_FILENO, &c, 1) == 1) {
                        utf8Char += c;
                    }
                }
            }
            HandleCharacterInput(wxString::FromUTF8(utf8Char.c_str()));
        }
    }
}

// Main loop for the CLI
void GKCli::Run() {
    std::signal(SIGINT, SignalHandler);

    std::cout << "Welcome to GKCli (C++/wxWidgets)!" << std::endl;
    std::cout << "This is a simple command line interface." << std::endl;
    std::cout << "You can use arrow keys to navigate command history." << std::endl;
    std::cout << "Fully support UTF8 multibytes characters." << std::endl;
    std::cout << "[[ Note: Run this program in a Mac terminal. VS Code terminal isn't fully support ! ]] " << std::endl;
    std::cout << "Type \"exit\" to exit." << std::endl;

    wxString line;
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (true) {
        std::cout << "> ";
        std::cout.flush();

        currentInput.clear();
        cursorPosition = 0;
        ProcessInput(line);

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        wxString userInput(line);

        if (!userInput.IsEmpty()) {
            commandHistory.push_back(userInput);
            historyIndex = commandHistory.size();
        }

        if(userInput.IsEmpty() == false) std::cout << "You have input \""<< userInput.c_str() << "\"." << std::endl; 

        userInput.Trim();
        userInput.LowerCase();

        if (userInput == "exit" || userInput == "quit") {
            std::cout << "Exiting..." << std::endl;
            break;
            }
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }

    std::cout << "GKCli finished." << std::endl;
    return ;
};
