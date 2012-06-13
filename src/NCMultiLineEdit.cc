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

   File:       NCMultiLineEdit.cc

   Author:     Michael Andres <ma@suse.de>

/-*/

#define  YUILogComponent "ncurses"
#include <YUILog.h>
#include "NCMultiLineEdit.h"


NCMultiLineEdit::NCMultiLineEdit( YWidget * parent, const string & nlabel )
	: YMultiLineEdit( parent, nlabel )
	, NCPadWidget( parent )
{
    yuiDebug() << std::endl;
    defsze = wsze( 5, 5 ) + wsze( 0, 2 );
    setLabel( nlabel );
}


NCMultiLineEdit::~NCMultiLineEdit()
{
    yuiDebug() << std::endl;
}


int NCMultiLineEdit::preferredWidth()
{
    defsze.W = ( 5 > labelWidth() ? 5 : labelWidth() ) + 2;
    return wGetDefsze().W;
}


int NCMultiLineEdit::preferredHeight()
{
    return wGetDefsze().H;
    //return YMultiLineEdit::defaultVisibleLines();
}


void NCMultiLineEdit::setEnabled( bool do_bv )
{
    NCWidget::setEnabled( do_bv );
    YMultiLineEdit::setEnabled( do_bv );
}


void NCMultiLineEdit::setSize( int newwidth, int newheight )
{
    wRelocate( wpos( 0 ), wsze( newheight, newwidth ) );
}


void NCMultiLineEdit::setLabel( const string & nlabel )
{
    YMultiLineEdit::setLabel( nlabel );
    NCPadWidget::setLabel( NCstring( nlabel ) );
}


void NCMultiLineEdit::setValue( const string & ntext )
{
    DelPad();
    ctext = NCstring( ntext );
    Redraw();
}


string NCMultiLineEdit::value()
{
    if ( myPad() )
    {
	ctext = NCstring( myPad()->getText() );
    }

    return ctext.Str();
}


void NCMultiLineEdit::wRedraw()
{
    if ( !win )
	return;

    NCPadWidget::wRedraw();
}


NCursesEvent NCMultiLineEdit::wHandleInput( wint_t key )
{
    NCursesEvent ret;
    handleInput( key );

    if ( notify() )
	ret = NCursesEvent::ValueChanged;

    return ret;
}


NCPad * NCMultiLineEdit::CreatePad()
{
    wsze psze( defPadSze() );
    NCPad * npad = new NCTextPad( psze.H, psze.W, *this );
    npad->bkgd( listStyle().item.plain );

    return npad;
}


void NCMultiLineEdit::DrawPad()
{
    myPad()->setText( ctext );
}


void NCMultiLineEdit::setInputMaxLength( int numberOfChars )
{
    myPad()->setInputMaxLength( numberOfChars );
    YMultiLineEdit::setInputMaxLength( numberOfChars );
}
