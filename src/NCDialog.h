/*************************************************************************************************************

 Copyright (C) 2000 - 2010 Novell, Inc.   All Rights Reserved.

 This program is free software; you can redistribute it and/or modify it under the terms of the GNU General
 Public License as published by the Free Software Foundation; either version 2 of the License, or (at your
 option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
 implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License along with this program; if not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*************************************************************************************************************/



 /////////////////////////////////////////////////////////////////////////////////////////////////////////////
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////
 ////                                                                                                     ////
 ////                                                                                                     ////
 ////                                                                                                     ////
 ////   __/\\\\\\_____________/\\\__________/\\\________/\\\___/\\\________/\\\___/\\\\\\\\\\\_           ////
 ////    _\////\\\____________\/\\\_________\///\\\____/\\\/___\/\\\_______\/\\\__\/////\\\///__          ////
 ////     ____\/\\\______/\\\__\/\\\___________\///\\\/\\\/_____\/\\\_______\/\\\______\/\\\_____         ////
 ////      ____\/\\\_____\///___\/\\\_____________\///\\\/_______\/\\\_______\/\\\______\/\\\_____        ////
 ////       ____\/\\\______/\\\__\/\\\\\\\\\_________\/\\\________\/\\\_______\/\\\______\/\\\_____       ////
 ////        ____\/\\\_____\/\\\__\/\\\////\\\________\/\\\________\/\\\_______\/\\\______\/\\\_____      ////
 ////         ____\/\\\_____\/\\\__\/\\\__\/\\\________\/\\\________\//\\\______/\\\_______\/\\\_____     ////
 ////          __/\\\\\\\\\__\/\\\__\/\\\\\\\\\_________\/\\\_________\///\\\\\\\\\/_____/\\\\\\\\\\\_    ////
 ////           _\/////////___\///___\/////////__________\///____________\/////////______\///////////__   ////
 ////                                                                                                     ////
 ////                                                                                                     ////
 ////                 widget abstraction library providing Qt, GTK and ncurses frontends                  ////
 ////                                                                                                     ////
 ////                                   3 UIs for the price of one code                                   ////
 ////                                                                                                     ////
 ////                                      ***  NCurses plugin  ***                                       ////
 ////                                                                                                     ////
 ////                                                                                                     ////
 ////                                                                                                     ////
 ////                                                                              (C) SUSE Linux GmbH    ////
 ////                                                                                                     ////
 ////                                                              libYUI-AsciiArt (C) 2012 Björn Esser   ////
 ////                                                                                                     ////
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*-/

   File:       NCDialog.h

   Author:     Michael Andres <ma@suse.de>

/-*/

#ifndef NCDialog_h
#define NCDialog_h

#include <iosfwd>

#include "YDialog.h"
#include "NCWidget.h"
#include "NCPushButton.h"


class NCDialog;
class NCPopupInfo;


class NCDialog : public YDialog, public NCWidget
{
private:

    friend std::ostream & operator<<( std::ostream & STREAM, const NCDialog & OBJ );
    friend std::ostream & operator<<( std::ostream & STREAM, const NCDialog * OBJ );

    NCDialog & operator=( const NCDialog & );
    NCDialog( const NCDialog & );

    typedef tnode<NCWidget *> * ( tnode<NCWidget *>::* SeekDir )( const bool );

    NCWidget & GetNormal( NCWidget & startwith, SeekDir Direction );
    void       Activate( SeekDir Direction );

    void _init();
    void _init_size();

protected:

    virtual const char * location() const { return "NCDialog"; }

private:

    NCursesUserPanel<NCDialog> * pan;
    NCstyle::StyleSet		 mystyleset;
    const NCstyle::Style *	 dlgstyle;

    unsigned inMultiDraw_i;

    bool	    active;
    NCWidget *const wActive;

    NCursesEvent pendingEvent;
    YEvent::EventReason eventReason;

    NCPopupInfo *helpPopup;

    // wrapper for wHandle... calls in processInput()
    NCursesEvent getInputEvent( wint_t ch );
    NCursesEvent getHotkeyEvent( wint_t key );

    void grabActive( NCWidget * nactive );
    virtual void grabNotify( NCWidget * mgrab );
    virtual bool wantFocus( NCWidget & ngrab );

    virtual void wCreate( const wrect & newrect );
    virtual void wMoveTo( const wpos & newpos );
    virtual void wDelete();
    virtual void wRedraw();
    virtual void wRecoded();
    virtual void wUpdate( bool forced_br = false );
    void doUpdate() { wUpdate( true ); }

    NCWidget & GetNextNormal( NCWidget & startwith );
    NCWidget & GetPrevNormal( NCWidget & startwith );

    bool Activate( NCWidget & nactive );
    void Activate();
    void Deactivate();
    void ActivateNext();
    void ActivatePrev();

    bool ActivateByKey( int key );

    void processInput( int timeout_millisec );

    std::map<int, string> describeFunctionKeys();

    wint_t getinput();		// get the input (respect terminal encoding)

    bool flushTypeahead();

protected:

    wint_t getch( int timeout_millisec = -1 );

    virtual NCursesEvent wHandleInput( wint_t ch );
    virtual NCursesEvent wHandleHotkey( wint_t key );

    virtual void startMultipleChanges();
    virtual void doneMultipleChanges();

    /**
     * Internal open() method: Initialize what is left over to initialize after
     * all dialog children have been created.
     * YDialog::setInitialSize() is already called before this in
     * YDailog::open(), so don't call it here again (very expensive!).
     *
     * This function is called (exactly once during the life time of the
     * dialog) in YDialog::open().
     *
     * Implemented from YDialog.
     **/
    virtual void openInternal();

    /**
     * Wait for a user event.
     *
     * Implemented from YDialog.
     **/
    virtual YEvent * waitForEventInternal( int timeout_millisec );

    /**
     * Check if a user event is pending. If there is one, return it.
     * If there is none, do not wait for one - return 0.
     *
     * Implemented from YDialog.
     **/
    virtual YEvent * pollEventInternal();


public:

    NCDialog( YDialogType	dialogType,
	      YDialogColorMode	colorMode = YDialogNormalColor );
    virtual ~NCDialog();

    void showDialog();
    void closeDialog();

    void activate( const bool newactive );
    bool isActive() const { return active; }

    void idleInput();

    NCursesEvent userInput( int timeout_millisec = -1 );
    NCursesEvent pollInput();

    virtual int preferredWidth();
    virtual int preferredHeight();
    
    virtual void setSize( int newWidth, int newHeight );

    /**
     * Activate this dialog: Make sure that it is shown as the topmost dialog
     * of this application and that it can receive input.
     *
     * Implemented from YDialog.
     **/
    virtual void activate();

protected:

    enum NCDopts
    {
	DEFAULT = 0x00,
	POPUP	= 0x01,
	NOBOX	= 0x10
    };

    typedef unsigned NCDoptflag;

    NCDoptflag ncdopts;
    wpos       popedpos;
    bool       hshaddow;
    bool       vshaddow;

    NCDialog( YDialogType dialogType, const wpos at, const bool boxed = true );

    bool isPopup() const { return ( ncdopts & POPUP ); }

    bool isBoxed() const { return !( ncdopts & NOBOX ); }

    virtual void initDialog();

    virtual const NCstyle::Style & wStyle() const
    {
	return dlgstyle ? *dlgstyle : NCurses::style()[NCstyle::DefaultStyle];
    }

    virtual void setEnabled( bool do_bv ) {}

private:

    friend class NCurses;
    bool getInvisible();
    bool getVisible();
    void resizeEvent();
};


#endif // NCDialog_h
