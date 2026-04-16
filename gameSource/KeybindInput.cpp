/*
 * KeybindInput.cpp
 */

#include "KeybindInput.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/game/game.h"
#include "minorGems/game/gameGraphics.h"
#include "minorGems/game/drawUtils.h"
#include "minorGems/graphics/openGL/KeyboardHandlerGL.h"

#include <string.h>
#include <ctype.h>


KeybindInput::KeybindInput( Font *inFont,
                             double inX, double inY,
                             int inCharsWide,
                             const char *inActionName )
        : TextField( inFont, inX, inY, inCharsWide,
                     false,   // forceCaps — buildDisplayString handles case
                     NULL,    // no built-in label; SettingsPage sets it
                     NULL,    // allowedChars — we intercept all input
                     NULL ),  // forbiddenChars
          mActionName( stringDuplicate( inActionName ) ),
          mListening( false ) {
    setIgnoreArrowKeys( true );
    refreshDisplay();
    }


KeybindInput::~KeybindInput() {
    delete [] mActionName;
    }


void KeybindInput::refreshDisplay() {
    KeybindRecord *r = KeybindManager::findAction( mActionName );
    if( r == NULL || r->key == 0 ) {
        setText( "[NONE]" );
        return;
        }
    char buf[KEYBIND_DISPLAY_MAX];
    KeybindManager::buildDisplayString( r->key, r->modifiers, buf );
    setText( buf );
    }


char KeybindInput::isListening() {
    return mListening;
    }


const char *KeybindInput::getActionName() {
    return mActionName;
    }


void KeybindInput::focus() {
    TextField::focus();
    startListening();
    }


void KeybindInput::unfocus() {
    stopListening();
    TextField::unfocus();
    }


void KeybindInput::startListening() {
    mListening = true;
    }


void KeybindInput::stopListening() {
    if( !mListening ) {
        return;
        }
    mListening = false;
    refreshDisplay();
    }


void KeybindInput::pointerUp( float inX, float inY ) {
    if( isInside( inX, inY ) ) {
        focus();
        }
    else {
        unfocus();
        }
    }


void KeybindInput::keyDown( unsigned char inASCII ) {
    if( !mFocused || !mListening ) {
        return;
        }

    // ESC — cancel without changing anything
    if( inASCII == 27 ) {
        unfocus();
        return;
        }

    // Delete (127) or Backspace (8) — clear the binding
    if( inASCII == 127 || inASCII == 8 ) {
        KeybindManager::setBinding( mActionName, 0, KEYBIND_MOD_NONE );
        KeybindManager::saveAction( mActionName );
        stopListening();
        TextField::unfocus();
        fireActionPerformed( this );
        return;
        }

    // Build modifier flags from current physical key state
    int mods = KEYBIND_MOD_NONE;
    if( isShiftKeyDown() ) mods |= KEYBIND_MOD_SHIFT;
    if( isControlKeyDown() ) mods |= KEYBIND_MOD_CTRL;
    if( isAltKeyDown() ) mods |= KEYBIND_MOD_ALT;
    if( isCapsLockDown() ) mods |= KEYBIND_MOD_CAPS;

    // Recover base key from ctrl-code.
    // ctrl+A arrives as ASCII 1, ctrl+B as 2, ..., ctrl+Z as 26.
    // ASCII 30 = forward mouse button, ASCII 31 = back mouse button —
    // treat them as primary keys directly, no ctrl-code conversion.
    unsigned char baseKey;
    if( inASCII == 30 || inASCII == 31 ) {
        baseKey = inASCII;
        }
    else if( inASCII > 0 && inASCII < 27 ) {
        baseKey = 'a' + inASCII - 1;
        mods |= KEYBIND_MOD_CTRL;
        }
    else {
        baseKey = (unsigned char)tolower( inASCII );
        }

    KeybindManager::setBinding( mActionName, baseKey, mods );
    KeybindManager::saveAction( mActionName );
    stopListening();
    TextField::unfocus();
    fireActionPerformed( this );
    }


void KeybindInput::specialKeyDown( int inKeyCode ) {
    // Special keys (F-keys, arrow keys) are not supported as primary keys
    // in this iteration.  We only cancel listening to avoid getting stuck.
    // ESC arrives as ASCII 27 in keyDown(), not here.
    if( !mFocused || !mListening ) {
        return;
        }
    // Do nothing — user must press a normal key or ESC to exit.
    }


void KeybindInput::draw() {
    // Draw border — white when listening, grey otherwise
    if( mListening ) {
        setDrawColor( 1, 1, 1, 1 );
        }
    else {
        setDrawColor( 0.5, 0.5, 0.5, 1 );
        }

    drawRect( - mWide / 2, - mHigh / 2,
              mWide / 2, mHigh / 2 );

    double pixWidth = mCharWidth / 8;

    double rectStartX = - mWide / 2 + pixWidth;
    double rectStartY = - mHigh / 2 + pixWidth;
    double rectEndX = mWide / 2 - pixWidth;
    double rectEndY = mHigh / 2 - pixWidth;

    setDrawColor( 0.25, 0.25, 0.25, 1 );
    drawRect( rectStartX, rectStartY, rectEndX, rectEndY );

    // Draw label to the left
    if( mLabelText != NULL ) {
        double xPos = mWide / 2 + mBorderWide;
        doublePair labelPos = { xPos, 0 };
        setDrawColor( 1, 1, 1, 1 );
        mFont->drawString( mLabelText, labelPos, alignLeft );
        }

    // Draw text — left-aligned from border, no scroll, no caret, no fade
    setDrawColor( 1, 1, 1, 1 );
    doublePair textPos = { - mWide / 2 + mBorderWide, 0 };
    mFont->drawString( mText, textPos, alignLeft );

    // Inactive overlay
    if( !mActive ) {
        setDrawColor( 0, 0, 0, 0.5 );
        drawRect( - mWide / 2, - mHigh / 2,
                    mWide / 2,   mHigh / 2 );
        }
    }
