#ifndef KEYBIND_INPUT_INCLUDED
#define KEYBIND_INPUT_INCLUDED

#include "TextField.h"
#include "KeybindManager.h"
#include "TextButton.h"
#include "SpriteButton.h"


class KeybindInput : public TextField, public ActionListener {

    public:
        KeybindInput( Font *inFont, double inX, double inY, int inCharsWide, const char *inActionName );

        virtual ~KeybindInput();

        const char *getActionName();
        static KeybindInput *getListening();

        void refreshText();
        void setShowClearButton( char inShow );
        void setShowResetButton( char inShow );

        virtual void setWidth( double inWide );

        virtual void focus();
        virtual void unfocus();

        virtual void pointerUp( float inX, float inY );
        virtual void keyDown( unsigned char inASCII );
        virtual void specialKeyDown( int inKeyCode );

        virtual void actionPerformed( GUIComponent *inTarget );

        virtual void draw();

    private:

        static KeybindInput *sListening;

        char *mActionName;
        TextButton *mClearButton;
        SpriteButton *mResetButton;

        SpriteHandle mResetEnabledSprite;
        SpriteHandle mResetDisabledSprite;
        KeybindRecord *mRecord;

    };


#endif
