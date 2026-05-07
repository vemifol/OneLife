#ifndef LINKED_CHECKBOX_INCLUDED
#define LINKED_CHECKBOX_INCLUDED

#include "PageComponent.h"
#include "CheckboxButton.h"

#include "minorGems/game/Font.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/ui/event/ActionListenerList.h"
#include "minorGems/ui/event/ActionListener.h"


class LinkedCheckbox : public PageComponent, public ActionListenerList,
                       public ActionListener {

    public:
        enum Mode { single, multi };
        LinkedCheckbox( Font *inDisplayFont, double inX, double inY,
                        int inNumOptions,
                        Mode inMode = single,
                        TextAlignment inDefaultAlignment = alignRight,
                        double inDrawScale = 1.0 );

        virtual ~LinkedCheckbox();

        void setAutoSpacing( int inSpaceX, int inSpaceY );
        void setPosition( int inI, double inX, double inY );
        void setStartPosition( double inX, double inY );
        void setLabel( int inI, const char *inLabel );
        void setLabelAlignment( int inI, TextAlignment inAlignment );
        char isChecked(int inI);

        virtual void actionPerformed( GUIComponent *inTarget );
        virtual void draw();
        
        int getNumOptions();

        SimpleVector<CheckboxButton*> mCheckboxes;


    protected:
        Font *mFont;
        doublePair mStartPos;
        int mNumOptions;
        int mSpaceX;
        int mSpaceY;
        char *mChecked;
        char **mLabels;
        TextAlignment *mLabelAlignments;
        Mode mMode;
        TextAlignment mDefaultAlignment;
    };


#endif
