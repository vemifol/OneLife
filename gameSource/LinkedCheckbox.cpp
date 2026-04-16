#include "LinkedCheckbox.h"

#include <string.h>

#include "minorGems/game/game.h"
#include "minorGems/game/gameGraphics.h"
#include "minorGems/game/drawUtils.h"
#include "minorGems/util/stringUtils.h"


LinkedCheckbox::LinkedCheckbox( Font *inDisplayFont, double inX, double inY,
                                int inNumOptions,
                                Mode inMode,
                                TextAlignment inDefaultAlignment,
                                double inDrawScale )
        : PageComponent( inX, inY ),
          mFont( inDisplayFont ),
          mNumOptions( inNumOptions ),
          mSpaceX( 0 ),
          mSpaceY( 0 ),          
          mLabels( NULL ),
          mLabelAlignments( NULL ),
          mMode(inMode),
          mDefaultAlignment( inDefaultAlignment ) {

    mStartPos.x = inX;
    mStartPos.y = inY;

    mLabels = new char*[ mNumOptions ];
    mLabelAlignments = new TextAlignment[ mNumOptions ];
    mChecked = new char[ mNumOptions ];


    for( int i = 0; i < mNumOptions; i++ ) {
        mLabels[i] = NULL;
        mChecked[i] = false;
        mLabelAlignments[i] = mDefaultAlignment;
        int x = mStartPos.x + mSpaceX * i;
        int y = mStartPos.y + mSpaceY * i;
        CheckboxButton *newCheckbox = new CheckboxButton( x, y, inDrawScale );
        mCheckboxes.push_back( newCheckbox );
        newCheckbox->addActionListener( this );
        addComponent( newCheckbox );
        }

    if ( mMode == single ) { 
        mChecked[0] = true; 
        CheckboxButton *first = mCheckboxes.getElementDirect( 0 );
        first->setToggled( true );
    }

    }


LinkedCheckbox::~LinkedCheckbox() {
    for( int i = 0; i < mNumOptions; i++ ) {
        delete mCheckboxes.getElementDirect( i );

        if( mLabels[i] != NULL ) {
            delete [] mLabels[i];
            mLabels[i] = NULL;
            }
        }

    if( mLabels != NULL ) {
        delete [] mLabels;
        mLabels = NULL;
        }

    if( mLabelAlignments != NULL ) {
        delete [] mLabelAlignments;
        mLabelAlignments = NULL;
        }
    
    if ( mChecked != NULL ) { 
        delete [] mChecked;
        mChecked = NULL;
    }
    }


void LinkedCheckbox::setAutoSpacing( int inSpaceX, int inSpaceY ) {
    mSpaceX = inSpaceX;
    mSpaceY = inSpaceY;

    for( int i = 0; i < mNumOptions; i++ ) {
        CheckboxButton *cb = mCheckboxes.getElementDirect( i );
        cb->setPosition( mStartPos.x + mSpaceX * i, mStartPos.y + mSpaceY * i );
        }
    }


void LinkedCheckbox::setPosition( int inI, double inX, double inY ) {
    CheckboxButton *cb = mCheckboxes.getElementDirect( inI );
    cb->setPosition( inX, inY );
    }


void LinkedCheckbox::setStartPosition( double inX, double inY ) {
    mStartPos.x = inX;
    mStartPos.y = inY;

    for( int i = 0; i < mNumOptions; i++ ) {
        CheckboxButton *cb = mCheckboxes.getElementDirect( i );
        cb->setPosition( mStartPos.x + mSpaceX * i, mStartPos.y + mSpaceY * i );
        }
    }


void LinkedCheckbox::setLabel( int inI, const char *inLabel ) {
    if( mLabels[inI] != NULL ) {
        delete [] mLabels[inI];
        }
    mLabels[inI] = stringDuplicate( inLabel );
    }


void LinkedCheckbox::setLabelAlignment( int inI, TextAlignment inAlignment ) {
    mLabelAlignments[inI] = inAlignment;
    }


char LinkedCheckbox::isChecked( int inI ) {
    return mChecked[inI];
    }


void LinkedCheckbox::actionPerformed( GUIComponent *inTarget ) {
    for( int i = 0; i < mNumOptions; i++ ) {
        CheckboxButton *cb = mCheckboxes.getElementDirect( i );
        if( inTarget == cb ) {
            mChecked[i] = !mChecked[i];
            cb->setToggled( mChecked[i] );
            if (mMode == single) { cb->setIgnoreEvents( true ); }
            }
        else {
            cb->setIgnoreEvents( false );
            if ( mMode == single ) {
                cb->setToggled( false );
                cb->setIgnoreEvents( false );
                mChecked[i] = false;
                }
            }
        }
    fireActionPerformed( this );
    }


void LinkedCheckbox::draw() {
    for( int i = 0; i < mNumOptions; i++ ) {
        if( mLabels[i] == NULL ) continue;
        CheckboxButton *cb = mCheckboxes.getElementDirect( i );
        doublePair pos = cb->getPosition();
        pos.x -= 30;
        pos.y -= 2;
        doublePair shadowOffset = { -2, 2 };
        setDrawColor( 0, 0, 0, 1 );
        mFont->drawString( mLabels[i], add( pos, shadowOffset ), mLabelAlignments[i] );
        setDrawColor( 1, 1, 1, 1 );
        mFont->drawString( mLabels[i], pos, mLabelAlignments[i] );
        }
    }

int LinkedCheckbox::getNumOptions() {
    return mNumOptions;
}