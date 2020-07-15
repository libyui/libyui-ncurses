/*
  Copyright (C) 2000-2012 Novell, Inc
  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) version 3.0 of the License. This library
  is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
  License for more details. You should have received a copy of the GNU
  Lesser General Public License along with this library; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
  Floor, Boston, MA 02110-1301 USA
*/


/*-/

   File:       NCIntField.cc

   Author:     Michael Andres <ma@suse.de>

/-*/

#define  YUILogComponent "ncurses"
#include <yui/YUILog.h>
#include "NCurses.h"
#include "NCIntField.h"
#include "NCPopupTextEntry.h"
#include "stringutil.h"
#include "stdutil.h"

using stdutil::numstring;

const unsigned NCIntField::taglen = 2; // "^v"


NCIntField::NCIntField( YWidget * parent,
			const std::string & nlabel,
			int minV, int maxV,
			int initialV )
    : YIntField( parent, nlabel,
		 minV <= maxV ? minV : maxV,
		 maxV >= minV ? maxV : minV )
    , NCWidget( parent )
    , lwin( 0 )
    , twin( 0 )
    , cvalue( initialV )
    , vlen( 0 )
    , vstart( 0 )
{
    // yuiDebug() << std::endl;
    vlen = numstring( minValue() ).length();
    unsigned tmpval = numstring( maxValue() ).length();

    if ( tmpval > vlen )
	vlen = tmpval;

    setLabel( nlabel );
    hotlabel = &label;
    setValue( initialV );
}


NCIntField::~NCIntField()
{
    delete lwin;
    delete twin;
    // yuiDebug() << std::endl;
}


int NCIntField::preferredWidth()
{
    return wGetDefsze().W;
}


int NCIntField::preferredHeight()
{
    return wGetDefsze().H;
}


void NCIntField::setEnabled( bool do_bv )
{
    NCWidget::setEnabled( do_bv );
    YIntField::setEnabled( do_bv );
}


void NCIntField::setSize( int newwidth, int newheight )
{
    wRelocate( wpos( 0 ), wsze( newheight, newwidth ) );
}


void NCIntField::setDefsze()
{
    unsigned cols = vlen + taglen;
    defsze = wsze( label.height() + 1,
		   label.width() < cols ? cols : label.width() );
}


void NCIntField::wCreate( const wrect & newrect )
{
    NCWidget::wCreate( newrect );

    if ( !win )
	return;

    wrect lrect( 0, wsze::min( newrect.Sze,
			       wsze( label.height(), newrect.Sze.W ) ) );

    wrect trect( 0, wsze( 1, newrect.Sze.W ) );

    if ( lrect.Sze.H == newrect.Sze.H )
	lrect.Sze.H -= 1;

    trect.Pos.L = lrect.Sze.H > 0 ? lrect.Sze.H : 0;

    lwin = new NCursesWindow( *win,
			      lrect.Sze.H, lrect.Sze.W,
			      lrect.Pos.L, lrect.Pos.C,
			      'r' );

    twin = new NCursesWindow( *win,
			      trect.Sze.H, trect.Sze.W,
			      trect.Pos.L, trect.Pos.C,
			      'r' );

    //vstart = ( vlen + 2 < ( unsigned )trect.Sze.W ) ? label.width() - vlen - 2 : 0;
    vstart = 0;
    // vstart is calculated from label width only if value length (+ tags) is smaller
    // than window width AND smaller than label width, otherwise start with 0
    // (bug #488757)
    if ( vlen + 2 < ( unsigned )trect.Sze.W && vlen + 2 < label.width() )
    {
	vstart = label.width() - vlen - 2;
    }
}


void NCIntField::wDelete()
{
    delete lwin;
    delete twin;
    lwin = 0;
    twin = 0;
    NCWidget::wDelete();
    vstart = 0;
}


void NCIntField::setLabel( const std::string & nlabel )
{
    label = NCstring( nlabel );
    label.stripHotkey();
    setDefsze();
    YIntField::setLabel( nlabel );
    Redraw();
}


void NCIntField::setValueInternal( int newValue )
{
    // checking newValue is done by YIntField
    // -> no checks required
    cvalue = newValue;
    tUpdate();
}


bool NCIntField::Increment( bool bigstep )
{
    unsigned dist = maxValue() - cvalue;

    if ( !dist )
	return false;

    unsigned step = bigstep ? 10 : 1;

    if ( step < dist )
	setValue( cvalue + step );
    else
	setValue( maxValue() );

    return true;
}


bool NCIntField::Decrement( bool bigstep )
{
    unsigned dist = cvalue - minValue();

    if ( !dist )
	return false;

    unsigned step = bigstep ? 10 : 1;

    if ( step < dist )
	setValue( cvalue - step );
    else
	setValue( minValue() );

    return true;
}


void NCIntField::wRedraw()
{
    if ( !win )
	return;

    // label
    const NCstyle::StWidget & style( widgetStyle( true ) );

    lwin->bkgd( style.plain );

    lwin->clear();

    label.drawAt( *lwin, style );

    tUpdate();
}


void NCIntField::tUpdate()
{
    if ( !win )
	return;

    const NCstyle::StWidget & style( widgetStyle() );

    twin->bkgd( widgetStyle( true ).plain );

    twin->bkgdset( style.data );

    twin->printw( 0, vstart, " %*d ", vlen, cvalue );

    twin->bkgdset( style.scrl );

    twin->addch( 0, vstart,
		 ( cvalue != minValue() ? ACS_DARROW : ' ' ) );

    twin->addch( 0, vstart + vlen + 1,
		 ( cvalue != maxValue() ? ACS_UARROW : ' ' ) );
}


NCursesEvent NCIntField::wHandleInput( wint_t key )
{
    NCursesEvent ret;
    bool   beep   = false;
    int    ovlue  = cvalue;

    switch ( key )
    {
	case KEY_UP:
	    beep = !Increment();
	    break;

	case KEY_DOWN:
	    beep = !Decrement();
	    break;

	case KEY_PPAGE:
	    beep = !Increment( true );
	    break;

	case KEY_NPAGE:
	    beep = !Decrement( true );
	    break;

	case KEY_HOME:

	    if ( cvalue != maxValue() )
		setValue( maxValue() );
	    else
		beep = true;
	    break;

	case KEY_END:
	    if ( cvalue != minValue() )
		setValue( minValue() );
	    else
		beep = true;
	    break;

	case L'0':
	case L'1':
	case L'2':
	case L'3':
	case L'4':
	case L'5':
	case L'6':
	case L'7':
	case L'8':
	case L'9':
	case L'-':
	    enterPopup( key );
	    break;

	case L'+':
	    enterPopup();
	    break;

	case KEY_HOTKEY:
	    break;

	default:
	    beep = true;
	    break;
    }

    if ( beep )
	::beep();

    if ( notify() && ovlue != cvalue )
	ret = NCursesEvent::ValueChanged;

    return ret;
}


int NCIntField::enterPopup( wchar_t first )
{
    std::wstring wch( &first );
    std::string utf8;

    wpos at( ScreenPos() + wpos( win->maxy() - 1, vstart + 1 ) );
    std::string label( std::string( "[" ) + numstring( minValue() )
		  + "," + numstring( maxValue() ) + "]" );

    std::string text( 1, ( char )first );
    NCPopupTextEntry * dialog = new NCPopupTextEntry( at, label, text, vlen, 0,
						      NCInputField::NUMBER );
    YUI_CHECK_NEW( dialog );

    while ( dialog->post() != -1 )
    {
	int nval = atoi( dialog->value().c_str() );

	if ( nval < minValue() )
	{
	    dialog->setValue( numstring( minValue() ) );
	}
	else if ( maxValue() < nval )
	{
	    dialog->setValue( numstring( maxValue() ) );
	}
	else
	{
	    setValue( nval );
	    break;
	}

	::beep();
    }

    YDialog::deleteTopmostDialog();

    return 0;
}
