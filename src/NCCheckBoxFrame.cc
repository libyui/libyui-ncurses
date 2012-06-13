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

   File:       NCCheckBoxFrame.cc

   Author:     Michael Andres <ma@suse.de>

/-*/

#define  YUILogComponent "ncurses"
#include <YUILog.h>
#include "NCurses.h"
#include "NCCheckBoxFrame.h"


NCCheckBoxFrame::NCCheckBoxFrame( YWidget * parent, const string & nlabel,
				  bool checked )
    : YCheckBoxFrame( parent, nlabel, checked )
    , NCWidget( parent )
{
    yuiDebug() << std::endl;
    wstate = NC::WSnormal;
    framedim.Pos = wpos( 1 );
    framedim.Sze = wsze( 2 );

    setLabel( YCheckBoxFrame::label() );
    hotlabel = &label;

    if ( invertAutoEnable() )
	setValue( !checked );
    else
	setValue( checked );

    // setEnabled() is called in wRedraw()
}


NCCheckBoxFrame::~NCCheckBoxFrame()
{
    yuiDebug() << std::endl;
}


int NCCheckBoxFrame::preferredWidth()
{
    defsze.W = hasChildren() ? firstChild()->preferredWidth() : 0;

    if ( label.width() > ( unsigned )defsze.W )
	defsze.W = label.width();

    defsze.W += framedim.Sze.W + 4;	// add space for checkbox

    return defsze.W;
}


int NCCheckBoxFrame::preferredHeight()
{
    defsze.H  = hasChildren() ? firstChild()->preferredHeight() : 0;
    defsze.H += framedim.Sze.H;

    return defsze.H;
}


void NCCheckBoxFrame::setSize( int newwidth, int newheight )
{
    wsze csze( newheight, newwidth );
    wRelocate( wpos( 0 ), csze );
    csze = wsze::max( 0, csze - framedim.Sze );

    if ( hasChildren() )
	firstChild()->setSize( csze.W, csze.H );
}


void NCCheckBoxFrame::setLabel( const string & nlabel )
{
    YCheckBoxFrame::setLabel( nlabel );

    label = NCstring( YCheckBoxFrame::label() );
    label.stripHotkey();

    Redraw();
}


void NCCheckBoxFrame::setEnabled( bool do_bv )
{
    YWidget::setEnabled( do_bv );

    for ( tnode<NCWidget*> * c = this->Next();
	  c && c->IsDescendantOf( this );
	  c = c->Next() )
    {
	if ( c->Value()->GetState() != NC::WSdumb )
	{
	    c->Value()->setEnabled( do_bv );
	    // explicitely set the state (needed for first run - bug #268352)
	    c->Value()->SetState( do_bv ? NC::WSnormal : NC::WSdisabeled, true );
	}
    }
}


bool NCCheckBoxFrame::gotBuddy()
{
    if ( !label.hasHotkey() )
	return false;

    for ( tnode<NCWidget*> * c = this->Next();
	  c && c->IsDescendantOf( this );
	  c = c->Next() )
    {
	if ( c->Value()->GetState() != NC::WSdumb )
	    return true;
    }

    return false;
}


void NCCheckBoxFrame::wRedraw()
{
    if ( !win )
	return;

    chtype bg = wStyle().dumb.text;
    win->bkgd( bg );
    win->box();

    if ( gotBuddy() )
	label.drawAt( *win, widgetStyle(), wpos( 0, 5 ),
		      wsze( 1, win->width() - 2 ), NC::TOPLEFT, false );
    else
	label.drawAt( *win, bg, bg, wpos( 0, 5 ),
		      wsze( 1, win->width() - 2 ), NC::TOPLEFT, false );

    const NCstyle::StWidget & style( widgetStyle() );

    win->bkgdset( style.plain );
    win->printw( 0, 1, "[ ] " );

    if ( !invertAutoEnable() )
    {
	if ( getValue() )
	    win->printw( 0, 2, "%c", 'x' );
	else
	    win->printw( 0, 2, "%c", ' ' );
    }
    else
    {
	if ( getValue() )
	    win->printw( 0, 2, "%c", ' ' );
	else
	    win->printw( 0, 2, "%c", 'x' );
    }

    setEnabled( getValue() );
}


NCursesEvent NCCheckBoxFrame::wHandleInput( wint_t key )
{
    NCursesEvent ret = NCursesEvent::handled;

    if ( key == KEY_SPACE ||
	 key == KEY_RETURN ||
	 key == KEY_HOTKEY )
    {
	if ( getValue() == true )		// enabled
	{
	    setValue( false );
	}
	else
	{
	    setValue( true );
	}

	//No need to call Redraw() here, it is already done
	//in setValue

	if ( notify() )
	    ret = NCursesEvent::ValueChanged;
	else
	    setEnabled( getValue() );
    }

    return ret;
}


bool NCCheckBoxFrame::setKeyboardFocus()
{
    if ( !grabFocus() )
	return YWidget::setKeyboardFocus();

    return true;
}


bool NCCheckBoxFrame::value()
{
    return getValue();
}
