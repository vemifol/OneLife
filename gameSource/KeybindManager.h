/*
 * KeybindManager.h
 *
 * Centralised keybind storage and query system.
 *
 * Each action is registered with a name, display label, and default key
 * string (e.g. "w", "ctrl+shift+w").  Bindings are stored as individual
 * .ini files under settings/keybinds/<actionName>.ini so players can edit
 * them by hand if they wish.
 *
 * A "full keybind" is one primary ASCII key plus up to three modifier
 * flags: SHIFT, CTRL, ALT.
 */

#ifndef KEYBIND_MANAGER_INCLUDED
#define KEYBIND_MANAGER_INCLUDED

#include "minorGems/util/SimpleVector.h"


// Modifier bit flags — OR these together for combos.
#define KEYBIND_MOD_NONE   0x00
#define KEYBIND_MOD_SHIFT  0x01
#define KEYBIND_MOD_CTRL   0x02
#define KEYBIND_MOD_ALT    0x04
#define KEYBIND_MOD_CAPS   0x08

// Largest possible display string: "Ctrl+Shift+Alt+CAPS+FOR" = 23 chars + null.
#define KEYBIND_DISPLAY_MAX 32


struct KeybindRecord {
    char          *actionName;    // e.g. "moveUp"       -- owned by manager
    char          *displayLabel;  // e.g. "MOVE UP"      -- owned by manager
    unsigned char  key;           // lowercase ASCII key, 0 = unbound
    int            modifiers;     // OR of KEYBIND_MOD_* bits
    char          *defaultKeyStr; // e.g. "w"            -- owned by manager
    double         posX;
    double         posY;
    };


class KeybindManager {

    public:

        // Call once at startup, after all registerAction calls.
        // Creates settings/keybinds/ directory if it does not yet exist,
        // then loads all bindings from disk.
        static void init();

        // Call once at shutdown to free all internal memory.
        static void deInit();

        // Register a named action before calling init().
        // inActionName and inDisplayLabel are copied internally.
        // inDefaultKeyStr format: "w", "ctrl+w", "ctrl+shift+w", "space".
        static void registerAction( const char *inActionName,
                                    const char *inDisplayLabel,
                                    const char *inDefaultKeyStr );

        // Reload all bindings from disk (e.g. when settings page is opened).
        static void loadAll();

        // Write one action's current binding to its .ini file.
        static void saveAction( const char *inActionName );

        // Change a binding in memory.  Call saveAction afterwards to persist.
        // inKey = 0 means "unbound".
        static void setBinding( const char *inActionName,
                                unsigned char inKey,
                                int inModifiers );

        // Returns true when inASCII + modifier state matches the named action.
        // Handles ctrl-code offset (ctrl+W arrives as ASCII 23) the same way
        // the legacy isCharKey() did, so existing behaviour is preserved.
        // inCaps defaults to 0 for callers that do not track caps lock.
        static char isPressed( const char *inActionName,
                               unsigned char inASCII,
                               char inShift,
                               char inCtrl,
                               char inAlt,
                               char inCaps = 0 );

        // Number of registered actions.
        static int getActionCount();

        // Returns pointer to internal record — do NOT free.
        // Returns NULL if inIndex is out of range.
        static KeybindRecord *getAction( int inIndex );

        // Returns pointer to internal record — do NOT free.
        // Returns NULL if no action with that name exists.
        static KeybindRecord *findAction( const char *inActionName );

        // Writes a human-readable display string (e.g. "Ctrl+Shift+W") into
        // outBuf.  outBuf must be at least KEYBIND_DISPLAY_MAX bytes.
        // Writes "[NONE]" if inKey == 0.
        static void buildDisplayString( unsigned char inKey,
                                        int inModifiers,
                                        char *outBuf );

        // Parses an ini-format string (e.g. "ctrl+shift+w") into a key and
        // modifier flags.  Tokens are case-insensitive.  "space" maps to ' '.
        // Returns false if the string is empty, "none", or otherwise invalid,
        // in which case *outKey is set to 0 and *outModifiers to NONE.
        static char parseKeyString( const char *inStr,
                                    unsigned char *outKey,
                                    int *outModifiers );

        // Returns a heap-allocated ini-format string for the named action
        // (e.g. "ctrl+shift+w" or "" if unbound).  Caller must delete[].
        static char *getKeyString( const char *inActionName );

        // Returns the displayLabel of the first OTHER action that already
        // uses inKey + inModifiers, or NULL if none.
        // Returned pointer is internal storage — do NOT free.
        static const char *findConflict( const char *inExcludeActionName,
                                         unsigned char inKey,
                                         int inModifiers );

        // Modifier-blind key match — used by keyUp to release movement flags
        // regardless of which modifier combo is currently held.
        // Returns true if inASCII matches the base key of the named action
        // (checks plain char, uppercase, and ctrl-code), ignoring modifiers.
        static char baseKeyMatches( const char *inActionName,
                                    unsigned char inASCII );

    private:

        static SimpleVector<KeybindRecord *> sActions;
        static char sInited;

        // Creates settings/keybinds/ if it does not exist.
        static void ensureDirectory();

        // Returns a heap-allocated full file path for an action's .ini file.
        // Caller must delete[].
        static char *buildFilePath( const char *inActionName );

    };


#endif
