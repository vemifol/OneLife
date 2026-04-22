#ifndef KEYBIND_MANAGER_INCLUDED
#define KEYBIND_MANAGER_INCLUDED

#include "minorGems/util/SimpleVector.h"

#define KEYBIND_MOD_NONE   0
#define KEYBIND_MOD_SHIFT  1 << 0
#define KEYBIND_MOD_CTRL   1 << 1
#define KEYBIND_MOD_ALT    1 << 2



enum KeybindType {
    DEFAULT_TYPE,
    MODIFIER_ONLY,
    KEY_ONLY
    };

struct KeybindRecord {
    char *actionName;
    char *displayLabel;
    unsigned char key;
    int modifiers;
    char *defaultKeyStr;
    int tags;
    char modifierOnly;
    char keyOnly;
    };

class KeybindManager {

    public:

        static void init();
        static void deInit();

        static void registerAction( const char *inActionName, const char *inDisplayLabel, const char *inDefaultKeyStr, int inTags = 0,
                                    KeybindType inType = DEFAULT_TYPE );
        static int getActionCount();
        static KeybindRecord *getAction( int inIndex );
        static KeybindRecord *findAction( const char *inActionName );

        static void loadAll();
        static void saveBinding( const char *inActionName );

        static void setBinding( const char *inActionName, unsigned char inKey, int inModifiers );
        static void clearBinding( const char *inActionName );
        static void resetBinding( const char *inActionName );

        static void parseKeyString( const char *inStr, unsigned char *outKey, int *outModifiers );
        static char *buildKeyString( const char *inActionName, char makeUppercase = false );

        static char isActive( const char *inActionName );
        static char isReleased( const char *inActionName );

        static void keyDown ( unsigned char inASCII );
        static void keyUp ( unsigned char inASCII );
        static void clearAllPressed();

        static void specialKeyDown ( int inKey );
        static void specialKeyUp ( int inKey );

        static char isShiftDown();
        static char isControlDown();
        static char isAltDown();
    private:

        static SimpleVector<KeybindRecord *> sActions;
        static char sInited;
        static char sPressed[256];
        static char sShiftDown;
        static char sControlDown;
        static char sAltDown;

        static void ensureDirectory();
        static char *buildFilePath( const char *inActionName );
        static char checkActive( const char *inActionName, char inStrict );
    };


#endif
