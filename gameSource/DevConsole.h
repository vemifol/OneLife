void cprintf( const char *fmt, ... );
void openCustomConsole();
void closeCustomConsole();
char isCustomConsoleOpen();

// Returns one pending line of user input typed into the console, or NULL if
// none waiting.  Caller must delete[] the returned string.
char *getConsolePendingInput();
