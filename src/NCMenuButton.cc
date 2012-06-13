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

   File:       NCMenuButton.cc

   Author:     Michael Andres <ma@suse.de>

/-*/

#define  YUILogComponent "ncurses"
#include <YUILog.h>
#include "NCurses.h"
#include "NCMenuButton.h"
#include "NCPopupMenu.h"


NCMenuButton::NCMenuButton( YWidget * parent,
			    string nlabel )
	: YMenuButton( parent, nlabel )
	, NCWidget( parent )
{
    yuiDebug() << std::endl;
    setLabel( nlabel );
    hotlabel = &label;
}


NCMenuButton::~NCMenuButton()
{
    yuiDebug() << std::endl;
}


int NCMenuButton::preferredWidth()
{
    return wGetDefsze().W;
}


int NCMenuButton::preferredHeight()
{
    return wGetDefsze().H;
}


void NCMenuButton::setEnabled( bool do_bv )
{
    NCWidget::setEnabled( do_bv );
    YMenuButton::setEnabled( do_bv );
}


void NCMenuButton::setSize( int newwidth, int newheight )
{
    wRelocate( wpos( 0 ), wsze( newheight, newwidth ) );
}


NCursesEvent NCMenuButton::wHandleInput( wint_t key )
{
    NCursesEvent ret;

    switch ( key )
    {
	case KEY_HOTKEY:
	case KEY_SPACE:
	case KEY_RETURN:
	case KEY_DOWN:
	    ret = postMenu();
	    break;
    }

    return ret;
}


void NCMenuButton::setLabel( const string & nlabel )
{
    label = NCstring( nlabel );
    label.stripHotkey();
    defsze = wsze( label.height(), label.width() + 3 );
    YMenuButton::setLabel( nlabel );
    Redraw();
}


void NCMenuButton::wRedraw()
{
    if ( !win )
	return;

    const NCstyle::StWidget & style( widgetStyle() );

    win->bkgdset( style.plain );

    if ( label.height() > 1 )
    {
	win->box( wrect( 0, win->size() - wsze( 0, 1 ) ) );
    }

    win->printw( 0, 0, "[" );

    win->printw( 0, win->maxx(), "]" );

    label.drawAt( *win, style, wpos( 0, 1 ), wsze( -1, win->width() - 3 ),
		  NC::CENTER );

    win->bkgdset( style.scrl );
    win->vline( 0, win->maxx() - 1, win->height(), ' ' );
    haveUtf8() ?
    win->add_wch( 0, win->maxx() - 1, WACS_DARROW )
    : win->addch( 0, win->maxx() - 1, ACS_DARROW );
}


void NCMenuButton::rebuildMenuTree()
{
    // NOP
}


NCursesEvent NCMenuButton::postMenu()
{
    wpos at( ScreenPos() + wpos( win->height(), 0 ) );
    NCPopupMenu * dialog = new NCPopupMenu( at,
					    itemsBegin(),
					    itemsEnd() );
    YUI_CHECK_NEW( dialog );

    int selection = dialog->post();

    if ( selection < 0 )
    {
	YDialog::deleteTopmostDialog();
	return NCursesEvent::none;
    }

    NCursesEvent ret = NCursesEvent::menu;
    ret.selection = findMenuItem( selection );
    YDialog::deleteTopmostDialog();

    return ret;
}


