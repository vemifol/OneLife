/*
 * KeybindManager.cpp
 */

#include "KeybindManager.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/io/file/File.h"
#include "minorGems/io/file/Path.h"
#include "minorGems/io/file/Directory.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>


SimpleVector<KeybindRecord *> KeybindManager::sActions;
char KeybindManager::sInited = false;


void KeybindManager::init() {
    ensureDirectory();
    loadAll();
    sInited = true;
    }


void KeybindManager::deInit() {
    for( int i = 0; i < sActions.size(); i++ ) {
        KeybindRecord *r = sActions.getElementDirect( i );
        delete [] r->actionName;
        delete [] r->displayLabel;
        delete [] r->defaultKeyStr;
        delete r;
        }
    sActions.deleteAll();
    sInited = false;
    }


void KeybindManager::registerAction( const char *inActionName,
                                     const char *inDisplayLabel,
                                     const char *inDefaultKeyStr ) {
    KeybindRecord *r = new KeybindRecord;
    r->actionName    = stringDuplicate( inActionName );
    r->displayLabel  = stringDuplicate( inDisplayLabel );
    r->defaultKeyStr = stringDuplicate( inDefaultKeyStr );
    r->key           = 0;
    r->modifiers     = KEYBIND_MOD_NONE;
    r->posX          = 0;
    r->posY          = 0;
    sActions.push_back( r );
    }


void KeybindManager::loadAll() {
    for( int i = 0; i < sActions.size(); i++ ) {
        KeybindRecord *r = sActions.getElementDirect( i );

        char *path = buildFilePath( r->actionName );
        FILE *f = fopen( path, "r" );
        delete [] path;

        if( f != NULL ) {
            char buf[64];
            buf[0] = '\0';
            if( fgets( buf, sizeof( buf ), f ) != NULL ) {
                // strip trailing newline / carriage-return
                int len = strlen( buf );
                while( len > 0 &&
                       ( buf[len - 1] == '\n' || buf[len - 1] == '\r' ) ) {
                    buf[--len] = '\0';
                    }
                }
            fclose( f );

            unsigned char key = 0;
            int mods = KEYBIND_MOD_NONE;
            parseKeyString( buf, &key, &mods );
            r->key       = key;
            r->modifiers = mods;
            }
        else {
            // file not found — apply default
            unsigned char key = 0;
            int mods = KEYBIND_MOD_NONE;
            parseKeyString( r->defaultKeyStr, &key, &mods );
            r->key       = key;
            r->modifiers = mods;
            }
        }
    }


void KeybindManager::saveAction( const char *inActionName ) {
    KeybindRecord *r = findAction( inActionName );
    if( r == NULL ) {
        return;
        }

    char *path = buildFilePath( inActionName );
    FILE *f = fopen( path, "w" );
    delete [] path;

    if( f == NULL ) {
        return;
        }

    char *keyStr = getKeyString( inActionName );
    fprintf( f, "%s\n", keyStr );
    delete [] keyStr;

    fclose( f );
    }


void KeybindManager::setBinding( const char *inActionName,
                                  unsigned char inKey,
                                  int inModifiers ) {
    KeybindRecord *r = findAction( inActionName );
    if( r == NULL ) {
        return;
        }
    r->key       = inKey;
    r->modifiers = inModifiers;
    }


char KeybindManager::isPressed( const char *inActionName,
                                 unsigned char inASCII,
                                 char inShift,
                                 char inCtrl,
                                 char inAlt,
                                 char inCaps ) {
    KeybindRecord *r = findAction( inActionName );
    if( r == NULL || r->key == 0 ) {
        return false;
        }

    // check modifier requirements
    char needShift = ( r->modifiers & KEYBIND_MOD_SHIFT ) != 0;
    char needCtrl  = ( r->modifiers & KEYBIND_MOD_CTRL  ) != 0;
    char needAlt   = ( r->modifiers & KEYBIND_MOD_ALT   ) != 0;
    char needCaps  = ( r->modifiers & KEYBIND_MOD_CAPS  ) != 0;

    if( needShift != ( inShift != 0 ) ) return false;
    if( needCtrl  != ( inCtrl  != 0 ) ) return false;
    if( needAlt   != ( inAlt   != 0 ) ) return false;
    if( needCaps  != ( inCaps  != 0 ) ) return false;

    unsigned char k = r->key;

    // FOR/BAK virtual keys — exact match only, no case folding
    if( k == 30 || k == 31 ) {
        return ( inASCII == k );
        }

    // ctrl-codes: ctrl+A arrives as ASCII 1, ctrl+B as 2, etc.
    // This mirrors the legacy isCharKey() c+64 == toupper(key) check.
    if( inCtrl && inASCII > 0 && inASCII < 27 ) {
        unsigned char ctrlBase = 'a' + inASCII - 1;
        return ( ctrlBase == (unsigned char)tolower( k ) );
        }

    return ( inASCII == k ||
             inASCII == (unsigned char)toupper( k ) ||
             inASCII == (unsigned char)tolower( k ) );
    }


int KeybindManager::getActionCount() {
    return sActions.size();
    }


KeybindRecord *KeybindManager::getAction( int inIndex ) {
    if( inIndex < 0 || inIndex >= sActions.size() ) {
        return NULL;
        }
    return sActions.getElementDirect( inIndex );
    }


KeybindRecord *KeybindManager::findAction( const char *inActionName ) {
    for( int i = 0; i < sActions.size(); i++ ) {
        KeybindRecord *r = sActions.getElementDirect( i );
        if( strcmp( r->actionName, inActionName ) == 0 ) {
            return r;
            }
        }
    return NULL;
    }


void KeybindManager::buildDisplayString( unsigned char inKey,
                                          int inModifiers,
                                          char *outBuf ) {
    outBuf[0] = '\0';

    if( inKey == 0 ) {
        strcpy( outBuf, "[NONE]" );
        return;
        }

    // canonical order: Ctrl, Shift, Alt, CAPS
    if( inModifiers & KEYBIND_MOD_CTRL  ) strcat( outBuf, "Ctrl+" );
    if( inModifiers & KEYBIND_MOD_SHIFT ) strcat( outBuf, "Shift+" );
    if( inModifiers & KEYBIND_MOD_ALT   ) strcat( outBuf, "Alt+" );
    if( inModifiers & KEYBIND_MOD_CAPS  ) strcat( outBuf, "CAPS+" );

    if( inKey == 30 ) {
        strcat( outBuf, "FOR" );
        }
    else if( inKey == 31 ) {
        strcat( outBuf, "BACK" );
        }
    else if( inKey == ' ' ) {
        strcat( outBuf, "Space" );
        }
    else {
        int len = strlen( outBuf );
        outBuf[len]     = (char)toupper( inKey );
        outBuf[len + 1] = '\0';
        }
    }


char KeybindManager::parseKeyString( const char *inStr,
                                      unsigned char *outKey,
                                      int *outModifiers ) {
    *outKey       = 0;
    *outModifiers = KEYBIND_MOD_NONE;

    if( inStr == NULL || inStr[0] == '\0' ) {
        return false;
        }

    // work on a lowercase copy
    char *copy = stringDuplicate( inStr );
    for( int i = 0; copy[i] != '\0'; i++ ) {
        copy[i] = (char)tolower( (unsigned char)copy[i] );
        }

    int numTokens = 0;
    char **tokens = split( copy, "+", &numTokens );
    delete [] copy;

    unsigned char foundKey = 0;
    int foundMods = KEYBIND_MOD_NONE;

    for( int i = 0; i < numTokens; i++ ) {
        char *tok = tokens[i];

        if( strcmp( tok, "ctrl" ) == 0 ) {
            foundMods |= KEYBIND_MOD_CTRL;
            }
        else if( strcmp( tok, "shift" ) == 0 ) {
            foundMods |= KEYBIND_MOD_SHIFT;
            }
        else if( strcmp( tok, "alt" ) == 0 ) {
            foundMods |= KEYBIND_MOD_ALT;
            }
        else if( strcmp( tok, "caps" ) == 0 ) {
            foundMods |= KEYBIND_MOD_CAPS;
            }
        else if( strcmp( tok, "for" ) == 0 ) {
            foundKey = 30;
            }
        else if( strcmp( tok, "bak" ) == 0 ) {
            foundKey = 31;
            }
        else if( strcmp( tok, "space" ) == 0 ) {
            foundKey = ' ';
            }
        else if( strcmp( tok, "none" ) == 0 ) {
            // explicitly unbound — leave foundKey as 0
            }
        else if( tok[0] != '\0' ) {
            foundKey = (unsigned char)tok[0];
            }

        delete [] tok;
        }
    delete [] tokens;

    if( foundKey == 0 ) {
        return false;
        }

    *outKey       = foundKey;
    *outModifiers = foundMods;
    return true;
    }


char *KeybindManager::getKeyString( const char *inActionName ) {
    KeybindRecord *r = findAction( inActionName );
    if( r == NULL || r->key == 0 ) {
        return stringDuplicate( "" );
        }

    char buf[64];
    buf[0] = '\0';

    // canonical order: ctrl, shift, alt, caps (lowercase for .ini)
    if( r->modifiers & KEYBIND_MOD_CTRL  ) strcat( buf, "ctrl+" );
    if( r->modifiers & KEYBIND_MOD_SHIFT ) strcat( buf, "shift+" );
    if( r->modifiers & KEYBIND_MOD_ALT   ) strcat( buf, "alt+" );
    if( r->modifiers & KEYBIND_MOD_CAPS  ) strcat( buf, "caps+" );

    if( r->key == 30 ) {
        strcat( buf, "for" );
        }
    else if( r->key == 31 ) {
        strcat( buf, "bak" );
        }
    else if( r->key == ' ' ) {
        strcat( buf, "space" );
        }
    else {
        int len = strlen( buf );
        buf[len]     = (char)tolower( r->key );
        buf[len + 1] = '\0';
        }

    return stringDuplicate( buf );
    }


const char *KeybindManager::findConflict( const char *inExcludeActionName,
                                           unsigned char inKey,
                                           int inModifiers ) {
    if( inKey == 0 ) {
        return NULL;
        }

    for( int i = 0; i < sActions.size(); i++ ) {
        KeybindRecord *r = sActions.getElementDirect( i );
        if( strcmp( r->actionName, inExcludeActionName ) == 0 ) {
            continue;
            }
        if( r->key == inKey && r->modifiers == inModifiers ) {
            return r->displayLabel;
            }
        }
    return NULL;
    }

char KeybindManager::baseKeyMatches( const char *inActionName,
                                      unsigned char inASCII ) {
    KeybindRecord *r = findAction( inActionName );
    if( r == NULL || r->key == 0 ) {
        return false;
        }

    unsigned char k = r->key;

    // ctrl-code: ctrl+A = ASCII 1, ctrl+B = 2, etc.
    if( inASCII > 0 && inASCII < 27 ) {
        return ( (unsigned char)( 'a' + inASCII - 1 ) == (unsigned char)tolower( k ) );
        }

    return ( inASCII == k ||
             inASCII == (unsigned char)toupper( k ) ||
             inASCII == (unsigned char)tolower( k ) );
    }


void KeybindManager::ensureDirectory() {
    char **pathSteps = new char*[1];
    pathSteps[0] = stringDuplicate( "settings" );

    File *keybindsDir = new File( new Path( pathSteps, 1, false ), "keybinds" );

    delete [] pathSteps[0];
    delete [] pathSteps;

    if( !keybindsDir->exists() ) {
        Directory::makeDirectory( keybindsDir );
        }

    delete keybindsDir;
    }


char *KeybindManager::buildFilePath( const char *inActionName ) {
    char **pathSteps = new char*[2];
    pathSteps[0] = stringDuplicate( "settings" );
    pathSteps[1] = stringDuplicate( "keybinds" );

    char *iniName = autoSprintf( "%s.ini", inActionName );
    File *f = new File( new Path( pathSteps, 2, false ), iniName );

    delete [] iniName;
    delete [] pathSteps[0];
    delete [] pathSteps[1];
    delete [] pathSteps;

    char *fullPath = f->getFullFileName();
    delete f;
    return fullPath;
    }
