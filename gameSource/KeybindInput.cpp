#include "KeybindInput.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/game/game.h"
#include "minorGems/game/gameGraphics.h"
#include "minorGems/game/drawUtils.h"

#include "minorGems/game/game.h"


#include <string.h>
#include <ctype.h>


KeybindInput *KeybindInput::sListening = NULL;


KeybindInput::KeybindInput( Font *inFont, double inX, double inY, int inCharsWide, const char *inActionName )
        : TextField( inFont, inX, inY, inCharsWide, false, NULL, NULL, NULL ),
          mActionName( stringDuplicate( inActionName ) ) {

    mClearButton = new TextButton( inFont, mWide / 2 + 30, 0, "X" );
    mResetButton = new SpriteButton( "resetButton.tga", mWide / 2 + 80, 0, 0.1875);
    mResetButton->setSize( mHigh, mHigh );
    mResetButton->setPixelSize( inFont->getFontHeight() / 8 );

    mClearButton->setSize( mHigh, mHigh );
    mClearButton->setDisabledFontColor( 150/255.0f, 150/255.0f, 150/255.0f, 1 );

    mClearButton->addActionListener( this );
    mResetButton->addActionListener( this );

    addComponent( mClearButton );
    addComponent( mResetButton );

    setIgnoreArrowKeys( true );

    mRecord = KeybindManager::findAction ( mActionName );


    mResetEnabledSprite = loadSprite( "resetButton.tga" );
    mResetDisabledSprite = loadSprite( "resetButtonDisabled.tga" );

    setShowClearButton( false );
    refreshText();
    }


KeybindInput::~KeybindInput() {
    delete [] mActionName;
    freeSprite( mResetEnabledSprite );
    freeSprite( mResetDisabledSprite );
    delete mClearButton;
    delete mResetButton;
    }


const char *KeybindInput::getActionName() {
    return mActionName;
    }


KeybindInput *KeybindInput::getListening() {
    return sListening;
    }


void KeybindInput::refreshText() {
    char isNone = mRecord->key == 0 && mRecord->modifiers == KEYBIND_MOD_NONE;
    unsigned char defaultKey;
    int defaultMods;
    KeybindManager::parseKeyString( mRecord->defaultKeyStr, &defaultKey, &defaultMods );
    char isDefault = mRecord->key == defaultKey && mRecord->modifiers == defaultMods;
    mClearButton->setDisabled( isNone );
    mClearButton->setIgnoreEvents( isNone );
    mResetButton->setDisabled( isDefault );
    mResetButton->setIgnoreEvents( isDefault );

    if ( isDefault ) {
        mResetButton->setSprite( mResetDisabledSprite, false );
        }
    else {
        mResetButton->setSprite( mResetEnabledSprite, false );
        }

    if( mRecord->key == 0 && mRecord->modifiers == KEYBIND_MOD_NONE ) {
        if( mRecord->keyOnly ) setText( "[ ]" );
        else setText( "[NONE]" );
        return;
        }
    char *text = KeybindManager::buildKeyString( mActionName );
    setText( text );
    delete [] text;
    }


void KeybindInput::setShowClearButton( char inShow ) {
    mClearButton->setVisible( inShow );
    mClearButton->setIgnoreEvents( !inShow );
    if( !inShow ) mResetButton->setPosition( mWide / 2 + 30, 0 );
    }

void KeybindInput::setShowResetButton( char inShow ) {
    mResetButton->setVisible( inShow );
    mResetButton->setIgnoreEvents( !inShow );
    }


void KeybindInput::setWidth( double inWide ) {
    TextField::setWidth( inWide );
    mClearButton->setPosition( mWide / 2 + 30, 0 );
    if( mClearButton->isVisible() ) mResetButton->setPosition( mWide / 2 + 80, 0 );
    else mResetButton->setPosition( mWide / 2 + 30, 0 );
    }

void KeybindInput::focus() {
    sListening = this;
    TextField::focus();
    }


void KeybindInput::unfocus() {
    if( sListening == this ) sListening = NULL;
    refreshText();
    TextField::unfocus();
    }


void KeybindInput::pointerUp( float inX, float inY ) {
    if( isInside( inX, inY ) ) focus();
    else unfocus();
    }


void KeybindInput::keyDown( unsigned char inASCII ) {
    if( !mFocused ) return;

    if( inASCII == 8 ) {
        KeybindManager::clearBinding( mActionName );
        unfocus();
        fireActionPerformed( this );
        return;
        }

    if( mRecord->modifierOnly ) return;

    int mods = KEYBIND_MOD_NONE;
    if( isShiftKeyDown() ) mods |= KEYBIND_MOD_SHIFT;
    if( isControlKeyDown() ) mods |= KEYBIND_MOD_CTRL;
    if( isAltKeyDown() ) mods |= KEYBIND_MOD_ALT;

    if( inASCII == 13 && !isControlKeyDown() ) { // re route enter to ASCII 28 for now. Checks for ctrl to avoid conflict with ctrl+m
        KeybindManager::setBinding( mActionName, 28, mRecord->keyOnly ? KEYBIND_MOD_NONE : mods );
        KeybindManager::saveBinding( mActionName );
        unfocus();
        fireActionPerformed( this );
        return;
        }

    unsigned char baseKey;
    if( inASCII > 0 && inASCII < 27 ) {
        baseKey = 'a' + inASCII - 1;
        if( !mRecord->keyOnly ) mods |= KEYBIND_MOD_CTRL;
        }
    else {
        baseKey = (unsigned char)tolower( inASCII );
        }

    if( mRecord->keyOnly ) mods = KEYBIND_MOD_NONE;

    KeybindManager::setBinding( mActionName, baseKey, mods );
    KeybindManager::saveBinding( mActionName );
    unfocus();
    fireActionPerformed( this );
    }


void KeybindInput::specialKeyDown( int inKeyCode ) {
    if( !mFocused || !mRecord->modifierOnly ) return;

    int mods = KEYBIND_MOD_NONE;
    if( isShiftKeyDown() ) mods |= KEYBIND_MOD_SHIFT;
    if( isControlKeyDown() ) mods |= KEYBIND_MOD_CTRL;
    if( isAltKeyDown() ) mods |= KEYBIND_MOD_ALT;
    if( mods == KEYBIND_MOD_NONE ) return;

    KeybindManager::setBinding( mActionName, 0, mods );
    KeybindManager::saveBinding( mActionName );
    unfocus();
    fireActionPerformed( this );
    }


void KeybindInput::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == mClearButton ) {
        KeybindManager::clearBinding( mActionName );
        refreshText();
        fireActionPerformed( this );
        }
    else if( inTarget == mResetButton ) {
        KeybindManager::resetBinding( mActionName );
        refreshText();
        fireActionPerformed( this );
        }
    }


void KeybindInput::draw() {

    if( mFocused ) setDrawColor( 1, 1, 1, 1 );
    else {
        if( mHover ) setDrawColor( 0.75, 0.75, 0.75, 1 );
        else setDrawColor( 0.5, 0.5, 0.5, 1 );
        }

    drawRect( -mWide / 2, -mHigh / 2, mWide / 2, mHigh / 2 );

    double pixWidth = mCharWidth / 8;

    double rectStartX = -mWide / 2 + pixWidth;
    double rectStartY = -mHigh / 2 + pixWidth;
    double rectEndX = mWide / 2 - pixWidth;
    double rectEndY = mHigh / 2 - pixWidth;

    setDrawColor( 0.25, 0.25, 0.25, 1 );
    drawRect( rectStartX, rectStartY, rectEndX, rectEndY );

    if( mLabelText != NULL ) {
        double xPos = -mWide / 2 - mBorderWide;
        doublePair labelPos = { xPos, 0 };
        doublePair shadowOffset = { -2, 2 };
        setDrawColor( 0, 0, 0, 1 );
        mFont->drawString( mLabelText, add( labelPos, shadowOffset ), alignRight );
        setDrawColor( 1, 1, 1, 1 );
        mFont->drawString( mLabelText, labelPos, alignRight );
        }

    setDrawColor( 1, 1, 1, 1 );
    doublePair bindTextPos = { 0, 0 };
    mFont->drawString( mText, bindTextPos, alignCenter );
    }
