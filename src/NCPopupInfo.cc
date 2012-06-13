/****************************************************************************
|
| Copyright (c) [2002-2011] Novell, Inc.
| All Rights Reserved.
|
| This program is free software; you can redistribute it and/or
| modify it under the terms of version 2 of the GNU General Public License as
| published by the Free Software Foundation.
|
| This program is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
| GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with this program; if not, contact Novell, Inc.
|
| To contact Novell about this file by physical or electronic mail,
| you may find current contact information at www.novell.com
|
|***************************************************************************/

/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       NCPopupInfo.cc

   Author:     Gabriele Strattner <gs@suse.de>

/-*/

#define  YUILogComponent "ncurses"
#include <YUILog.h>
#include "NCPopupInfo.h"

#include "NCTree.h"
#include "YMenuButton.h"
#include "YDialog.h"
#include "NCLayoutBox.h"
#include "NCSpacing.h"


namespace
{
    const string idOk( "ok" );
    const string idCancel( "cancel" );
}


NCPopupInfo::NCPopupInfo( const wpos at,
			  const string & headline,
			  const string & text,
			  string okButtonLabel,
			  string cancelButtonLabel )
	: NCPopup( at, false )
	, helpText( 0 )
	, okButton( 0 )
	, cancelButton( 0 )
	, hDim( 50 )
	, vDim( 20 )
	, visible( false )
{
    createLayout( headline, text, okButtonLabel, cancelButtonLabel );
}


NCPopupInfo::~NCPopupInfo()
{
}


void NCPopupInfo::createLayout( const string & headline,
				const string & text,
				string okButtonLabel,
				string cancelButtonLabel )
{
    // the vertical split is the (only) child of the dialog
    NCLayoutBox * split = new NCLayoutBox( this, YD_VERT );

    // add the headline
    new NCLabel( split, headline, true, false ); // isHeading = true

    // add the rich text widget
    helpText = new NCRichText( split, text );

    NCLayoutBox * hSplit = new NCLayoutBox( split, YD_HORIZ );

    if ( okButtonLabel != "" && cancelButtonLabel != "" )
    {
	new NCSpacing( hSplit, YD_HORIZ, true, 0.4 ); // stretchable = true
    }

    if ( okButtonLabel != "" )
    {
	// add the OK button
	okButton = new NCPushButton( hSplit, okButtonLabel );
	okButton->setFunctionKey( 10 );
    }

    if ( cancelButtonLabel != "" )
    {
	new NCSpacing( hSplit, YD_HORIZ, true, 0.4 );

	// add the Cancel button
	cancelButton = new NCPushButton( hSplit, cancelButtonLabel );
	cancelButton->setFunctionKey( 9 );

	new NCSpacing( hSplit, YD_HORIZ, true, 0.4 );
    }

    //If we don't have cancel button and have single ok button instead
    //let's focus it by default (#397393)
    if ( cancelButtonLabel == "" && okButton )
	focusOkButton();

    //the same with missing ok button and single cancel button
    if ( okButtonLabel == "" && cancelButton )
	focusCancelButton();
}


NCursesEvent & NCPopupInfo::showInfoPopup( )
{
    postevent = NCursesEvent();

    do
    {
	popupDialog( );
    }
    while ( postAgain() );

    popdownDialog();

    return postevent;
}


void NCPopupInfo::popup()
{
    initDialog();
    showDialog();
    activate( true );
    visible = true;
}


void NCPopupInfo::popdown()
{
    activate( false );
    closeDialog();
    visible = false;
}


int NCPopupInfo::preferredWidth()
{
    int horDim = hDim;

    if ( hDim >= NCurses::cols() )
	horDim = NCurses::cols() - 10;

    return horDim;
}


int NCPopupInfo::preferredHeight()
{
    int vertDim = vDim;

    if ( vDim >= NCurses::lines() )
	vertDim = NCurses::lines() - 5;

    return vertDim;
}


NCursesEvent
NCPopupInfo::wHandleInput( wint_t ch )
{
    if ( ch == 27 ) // ESC
	return NCursesEvent::cancel;

    if ( ch == KEY_RETURN )
	return NCursesEvent::button;

    return NCDialog::wHandleInput( ch );
}


bool NCPopupInfo::postAgain()
{
    if ( ! postevent.widget )
	return false;

    if ( okButton && cancelButton )
    {
	if ( postevent.widget == cancelButton )
	{
	    yuiMilestone() << "Cancel button pressed" << std::endl;
	    // close the dialog
	    postevent = NCursesEvent::cancel;
	}

	// else - nothing to do (postevent is already set)
    }

    if ( postevent == NCursesEvent::button || postevent == NCursesEvent::cancel )
    {
	// return false means: close the popup dialog
	return false;
    }

    return true;
}


