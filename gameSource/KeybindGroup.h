#ifndef KEYBIND_GROUP_INCLUDED
#define KEYBIND_GROUP_INCLUDED

#include "minorGems/game/Font.h"
#include "minorGems/ui/event/ActionListener.h"
#include "KeybindManager.h"
#include "KeybindInput.h"
#include "PageComponent.h"

class KeybindGroup : public PageComponent, public ActionListener  {
    
    public:
        KeybindGroup( Font *inFont, double inX, double inY, 
                      char showReset = true, char showClear = false );

        virtual ~KeybindGroup();

        void addInput ( KeybindInput *inInput, int inX, int inY);

        void addLabel( const char *inLabelText, doublePair inLabelPos, TextAlignment inAlign );

        virtual void actionPerformed( GUIComponent *inTarget );
    protected:
        char mShowReset;
        char mShowClear;

        Font *mFont;

        double mCharWidth;
        double mBorderWide;
        double mTotalWidth;

        SimpleVector<KeybindInput *> mInputs;

        TextButton *mClearButton;
        SpriteButton *mResetButton;
        
        void refreshResetButton();

        SpriteHandle mResetEnabledSprite;
        SpriteHandle mResetDisabledSprite;

    };

#endif