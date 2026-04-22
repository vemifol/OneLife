#include "KeybindGroup.h"

#include "DevConsole.h"
KeybindGroup::KeybindGroup( Font *inFont, double inX, double inY,
                            char showReset, char showClear )
        : PageComponent ( inX, inY ),
          mShowReset ( showReset ),
          mShowClear ( showClear ),
          mFont( inFont ) {

    mCharWidth = mFont->getFontHeight(); // Font is square anyways...
    mBorderWide = mCharWidth * 0.25;
    mTotalWidth =  mCharWidth + 2 * mBorderWide; 
    
    mResetEnabledSprite = loadSprite( "resetButton.tga" );
    mResetDisabledSprite = loadSprite( "resetButtonDisabled.tga" );

    mClearButton = NULL;
    mResetButton = NULL;

    if( mShowClear ) {
        mClearButton = new TextButton( inFont, 0, 0, "X" );
        mClearButton->setSize( mTotalWidth, mTotalWidth );
        mClearButton->setDisabledFontColor( 150/255.0f, 150/255.0f, 150/255.0f, 1 );
        mClearButton->addActionListener( this );
        addComponent( mClearButton );
        }

    if( mShowReset ) {
        mResetButton = new SpriteButton( "resetButton.tga", -185 + 52 + 5, -40 + 5, 0.1875 );
        mResetButton->setSize( mTotalWidth, mTotalWidth );
        mResetButton->setPixelSize( inFont->getFontHeight() / 8 );
        mResetButton->addActionListener( this );
        addComponent( mResetButton );
        refreshResetButton();
        }

    }


KeybindGroup::~KeybindGroup() {
    freeSprite( mResetEnabledSprite );
    freeSprite( mResetDisabledSprite );
    }


void KeybindGroup::addInput( KeybindInput *inInput, int inX, int inY) {
    mInputs.push_back( inInput );
    inInput->setPosition( inX, inY );
    inInput->setShowClearButton( false );
    inInput->setShowResetButton( false );
    inInput->setLabelText( NULL );
    }


void KeybindGroup::addLabel( const char *inLabelText, doublePair inLabelPos, TextAlignment inAlign ) {
    }


void KeybindGroup::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == mResetButton ) {
        for( int i = 0; i < mInputs.size(); i++ ) {
            KeybindManager::resetBinding( mInputs.getElementDirect( i )->getActionName() );
            mInputs.getElementDirect( i )->refreshText();
            }
        }
    if( inTarget == mClearButton ) {
        for( int i = 0; i < mInputs.size(); i++ ) {
            KeybindManager::clearBinding( mInputs.getElementDirect( i )->getActionName() );
            mInputs.getElementDirect( i )->refreshText();
            }
        }
    else {
        refreshResetButton();
        }
    }

void KeybindGroup::refreshResetButton() {
    char isDefault = true;
    unsigned char defaultKey;
    int defaultMods;
    
    for ( int i = 0; i < mInputs.size(); i++ ) {
        KeybindInput *inp = mInputs.getElementDirect( i );
        KeybindRecord *r = KeybindManager::findAction( inp->getActionName() );
        KeybindManager::parseKeyString( r->defaultKeyStr, &defaultKey, &defaultMods );
        if ( r->key != defaultKey || r->modifiers != defaultMods ) isDefault = false;
    }
    mResetButton->setIgnoreEvents( isDefault );
    mResetButton->setDisabled( isDefault );
    if ( isDefault ) {
        mResetButton->setSprite( mResetDisabledSprite, false );
    }
    else {
        mResetButton->setSprite( mResetEnabledSprite, false );
    }
    
}