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

   File:       NCLogView.cc

   Author:     Michael Andres <ma@suse.de>

/-*/

#define  YUILogComponent "ncurses"
#include <YUILog.h>
#include "NCLogView.h"


NCLogView::NCLogView( YWidget * parent,
		      const string & nlabel,
		      int visibleLines,
		      int maxLines )
	: YLogView( parent, nlabel, visibleLines, maxLines )
	, NCPadWidget( parent )
{
    yuiDebug() << std::endl;
    defsze = wsze( visibleLines, 5 ) + 2;
    setLabel( nlabel );
}


NCLogView::~NCLogView()
{
    yuiDebug() << std::endl;
}


int NCLogView::preferredWidth()
{
    defsze.W = ( 5 > labelWidth() ? 5 : labelWidth() ) + 2;
    return wGetDefsze().W;
}


int NCLogView::preferredHeight()
{
    return wGetDefsze().H;
}


void NCLogView::setEnabled( bool do_bv )
{
    NCWidget::setEnabled( do_bv );
    YLogView::setEnabled( do_bv );
}


void NCLogView::setSize( int newwidth, int newheight )
{
    wRelocate( wpos( 0 ), wsze( newheight, newwidth ) );
}


void NCLogView::setLabel( const string & nlabel )
{
    YLogView::setLabel( nlabel );
    NCPadWidget::setLabel( NCstring( nlabel ) );
}


void NCLogView::displayLogText( const string & ntext )
{
    DelPad();
    text = NCtext( NCstring( ntext ), Columns() );
    Redraw();
}


void NCLogView::wRedraw()
{
    if ( !win )
	return;

    bool initial = ( !myPad() || !myPad()->Destwin() );

    if ( myPad() )
	myPad()->bkgd( listStyle().item.plain );

    NCPadWidget::wRedraw();

    if ( initial )
	myPad()->ScrlTo( wpos( text.Lines(), 0 ) );
}


void NCLogView::wRecoded()
{
    DelPad();
    wRedraw();
}


NCursesEvent NCLogView::wHandleInput( wint_t key )
{
    handleInput( key );
    return NCursesEvent::none;
}


NCPad * NCLogView::CreatePad()
{
    wsze psze( defPadSze() );
    NCPad * npad = new NCPad( psze.H, psze.W, *this );
    npad->bkgd( listStyle().item.plain );
    return npad;
}


void NCLogView::DrawPad()
{
    // maximal value for lines is 32000!
    unsigned int maxLines = 20000;
    unsigned int skipLines = 0;
    unsigned int lines = text.Lines();
    unsigned int cl = 0;

    if ( lines > maxLines )
    {
	skipLines = lines - maxLines;
	lines = maxLines;
    }

    AdjustPad( wsze( lines, Columns() ) );

    for ( NCtext::const_iterator line = text.begin(); line != text.end(); ++line )
    {
	if ( skipLines == 0 )
	{
	    myPad()->move( cl++, 0 );
	    wstring cline = ( *line ).str();
	    myPad()->addwstr( cline.c_str() );
	}
	else
	{
	    skipLines--;
	}
    }
}

