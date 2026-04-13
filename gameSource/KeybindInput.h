/*
 * KeybindInput.h
 *
 * A TextField subclass that captures key+modifier combinations.
 *
 * When focused (clicked), it enters "listening" mode and waits for the
 * user to press a key combo.  The result is written to KeybindManager
 * and saved to disk immediately.
 *
 *   ESC              — cancel, keep existing binding
 *   Delete/Backspace — clear the binding (set to unbound)
 *   Any other key    — commit key + current modifiers as the new binding
 *
 * Fires actionPerformed when a binding is committed or cleared, so the
 * parent SettingsPage can react (e.g. show a conflict warning).
 */

#ifndef KEYBIND_INPUT_INCLUDED
#define KEYBIND_INPUT_INCLUDED

#include "TextField.h"
#include "KeybindManager.h"


class KeybindInput : public TextField {

    public:

        // inActionName is copied internally.
        // inX, inY are the centre position (PageComponent convention).
        KeybindInput( Font *inFont,
                      double inX, double inY,
                      int inCharsWide,
                      const char *inActionName );

        virtual ~KeybindInput();

        // Refresh the displayed text from the current binding in
        // KeybindManager.  Call this after any external binding change.
        void refreshDisplay();

        // Returns true while the widget is waiting for a key press.
        char isListening();

        // Returns the action name this widget controls.
        // Pointer is valid for the lifetime of this KeybindInput.
        const char *getActionName();

        // PageComponent / TextField overrides
        virtual void focus();
        virtual void unfocus();
        virtual void keyDown( unsigned char inASCII );
        virtual void specialKeyDown( int inKeyCode );
        virtual void pointerUp( float inX, float inY );
        virtual void draw();

    private:

        char *mActionName;
        char  mListening;
        void startListening();
        void stopListening();

    };


#endif
