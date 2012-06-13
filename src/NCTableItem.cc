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

   File:       NCTableItem.cc

   Author:     Michael Andres <ma@suse.de>

/-*/

#define  YUILogComponent "ncurses"
#include <YUILog.h>
#include "NCTableItem.h"
#include "stringutil.h"
#include "stdutil.h"

using stdutil::form;


NCTableCol::NCTableCol( const NCstring & l, const STYLE & st )
	: label( l )
	, style( st )
{
}


NCTableCol::~NCTableCol()
{
}


chtype NCTableCol::setBkgd( NCursesWindow & w,
			    NCTableStyle & tableStyle,
			    NCTableLine::STATE linestate,
			    STYLE colstyle ) const
{
    chtype bkgdstyle = tableStyle.getBG( linestate, colstyle );

    if ( bkgdstyle != NCTableStyle::currentBG )
	w.bkgdset( bkgdstyle );
    else
	bkgdstyle = w.getbkgd();

    return bkgdstyle;
}


void NCTableCol::DrawAt( NCursesWindow & w, const wrect at,
			 NCTableStyle & tableStyle,
			 NCTableLine::STATE linestate,
			 unsigned colidx ) const
{
    chtype bg  = setBkgd( w, tableStyle, linestate, style );
    chtype hbg = tableStyle.hotBG( linestate, colidx );

    if ( hbg == NCTableStyle::currentBG )
	hbg = bg;

    label.drawAt( w, bg, hbg, at, tableStyle.ColAdjust( colidx ) );
}


std::ostream & operator<<( std::ostream & STREAM, const NCTableCol & OBJ )
{
    return STREAM << OBJ.label;
}





NCTableLine::NCTableLine( unsigned cols, int idx, const unsigned s )
	: Items( cols, ( NCTableCol* )0 )
	, state( s )
	, index( idx )
	, vstate( S_HIDDEN )
{
}


NCTableLine::NCTableLine( vector<NCTableCol*> & nItems, int idx, const unsigned s )
	: Items( nItems )
	, state( s )
	, index( idx )
	, vstate( S_HIDDEN )
{
}


void NCTableLine::setOrigItem( YTableItem *it )
{
    yitem = it;
    yitem->setData( this ) ;
}


NCTableLine::~NCTableLine()
{
    ClearLine();
}


void NCTableLine::assertCol( unsigned idx )
{
    if ( idx >= Cols() )
	SetCols( idx + 1 );
}


void NCTableLine::SetCols( unsigned idx )
{
    if ( idx == Cols() )
	return;

    if ( idx < Cols() )
    {
	for ( unsigned i = idx; i < Cols(); ++i )
	{
	    delete Items[i];
	}
    }

    Items.resize( idx, 0 );
}


void NCTableLine::stripHotkeys()
{
    for ( unsigned i = 0; i < Cols(); ++i )
    {
	if ( Items[i] )
	    Items[i]->stripHotkey();
    }
}



void NCTableLine::SetCols( vector<NCTableCol*> & nItems )
{
    SetCols( 0 );
    Items = nItems;
}


void NCTableLine::AddCol( unsigned idx, NCTableCol * item )
{
    assertCol( idx );
    delete Items[idx];
    Items[idx] = item;
}


void NCTableLine::DelCol( unsigned idx )
{
    if ( idx < Cols() )
    {
	delete Items[idx];
	Items[idx] = 0;
    }
}


NCTableCol * NCTableLine::GetCol( unsigned idx )
{
    if ( idx < Cols() )
	return Items[idx];

    return 0;
}


void NCTableLine::UpdateFormat( NCTableStyle & tableStyle )
{
    tableStyle.AssertMinCols( Cols() );

    for ( unsigned c = 0; c < Cols(); ++c )
    {
	if ( !Items[c] )
	    continue;

	tableStyle.MinColWidth( c, Items[c]->Size().W );
    }
}


void NCTableLine::DrawAt( NCursesWindow & w, const wrect at,
			  NCTableStyle & tableStyle,
			  bool active ) const
{
    vstate = S_HIDDEN;

    if ( isVisible() )
    {
	if ( isDisabeled() )
	    vstate = S_DISABELED;
	else
	    vstate = active ? S_ACTIVE : S_NORMAL;
    }

    w.bkgdset( tableStyle.getBG( vstate ) );

    for ( int l = 0; l < at.Sze.H; ++l )
    {
	w.move( at.Pos.L + l, at.Pos.C );
	w.clrtoeol();
    }

    DrawItems( w, at, tableStyle, active );
}


void NCTableLine::DrawItems( NCursesWindow & w, const wrect at,
			     NCTableStyle & tableStyle,
			     bool active ) const
{
    if ( !( at.Sze > wsze( 0 ) ) )
	return;

    wrect    lRect( at );

    unsigned destWidth;

    for ( unsigned c = 0; c < Cols(); ++c )
    {

	if ( c && tableStyle.ColSepwidth() )
	{
	    // draw centered
	    destWidth = tableStyle.ColSepwidth() / 2;

	    if ( destWidth < ( unsigned )lRect.Sze.W )
	    {
		w.bkgdset( tableStyle.getBG( vstate, NCTableCol::SEPARATOR ) );
		w.vline( lRect.Pos.L, lRect.Pos.C + destWidth,
			 lRect.Sze.H, tableStyle.ColSepchar() );
		// skip over
		destWidth = tableStyle.ColSepwidth();

		if (( unsigned )lRect.Sze.W <= destWidth )
		    break;

		lRect.Pos.C += destWidth;

		lRect.Sze.W -= destWidth;
	    }
	}

	destWidth = tableStyle.ColWidth( c );

	wrect cRect( lRect );
	// adjust remaining linespace
	lRect.Pos.C += destWidth;
	lRect.Sze.W -= destWidth;
	// adjust destinated width

	if ( lRect.Sze.W < 0 )
	    cRect.Sze.W = destWidth + lRect.Sze.W;
	else
	    cRect.Sze.W = destWidth;

	// draw item
	if ( Items[c] )
	{
	    Items[c]->DrawAt( w, cRect, tableStyle, vstate, c );
	}
    }
}


std::ostream & operator<<( std::ostream & STREAM, const NCTableLine & OBJ )
{
    STREAM << "Line: cols " << OBJ.Cols() << std::endl;

    for ( unsigned idx = 0; idx < OBJ.Cols(); ++idx )
    {
	STREAM << "  " << idx << " ";
	const NCTableCol * ci = OBJ.GetCol( idx );

	if ( ci )
	    STREAM << *ci;
	else
	    STREAM << "NO_ITEM";

	STREAM << std::endl;
    }

    return STREAM;
}






void NCTableHead::DrawAt( NCursesWindow & w, const wrect at,
			  NCTableStyle & tableStyle,
			  bool active ) const
{
    vstate = S_HEADLINE;

    w.bkgdset( tableStyle.getBG( vstate ) );

    for ( int l = 0; l < at.Sze.H; ++l )
    {
	w.move( at.Pos.L + l, at.Pos.C );
	w.clrtoeol();
    }

    DrawItems( w, at, tableStyle, active );
}






NCTableStyle::NCTableStyle( const NCWidget & p )
	: headline( 0 )
	, colWidth( 0 )
	, colAdjust( 0 )
	, parw( p )
	, colSepwidth( 1 )
	, colSepchar( ACS_VLINE )
	, hotCol(( unsigned ) - 1 )
{
}


bool NCTableStyle::SetStyleFrom( const vector<NCstring> & head )
{
    unsigned ncols = head.size();

    headline.ClearLine();
    headline.SetCols( ncols );

    colWidth.clear();
    colAdjust.clear();
    AssertMinCols( ncols );

    bool hasContent = false;

    for ( unsigned i = 0; i < head.size(); ++i )
    {
	const wstring & entry( head[i].str() );
	bool strip = false;

	if ( entry.length() )
	{
	    switch ( entry[0] )
	    {
		case 'R':
		    strip = true;
		    colAdjust[i] = NC::RIGHT;
		    break;

		case 'C':
		    strip = true;
		    colAdjust[i] = NC::CENTER;
		    break;

		case 'L':
		    strip = true;
		    colAdjust[i] = NC::LEFT;
		    break;

		default:
		    yuiWarning() << "No style char [LRC] at beginning of '" << entry << "'" << std::endl;
		    break;
	    }
	}

	NCstring coltxt = strip ? entry.substr( 1 ) : entry;
	headline.AddCol( i, new NCTableCol( coltxt ) );

	if ( ! hasContent && coltxt.str().length() )
	    hasContent = true;
    }

    return hasContent;
}


chtype NCTableStyle::highlightBG( const NCTableLine::STATE lstate,
				  const NCTableCol::STYLE  cstyle,
				  const NCTableCol::STYLE  dstyle ) const
{
    return getBG( lstate, cstyle );
    // unused:

    if ( lstate == NCTableLine::S_ACTIVE
	 &&
	 parw.GetState() == NC::WSactive )
	return getBG( lstate, cstyle );

    return getBG( lstate, dstyle );
}


chtype NCTableStyle::getBG( const NCTableLine::STATE lstate,
			    const NCTableCol::STYLE  cstyle ) const
{
    switch ( lstate )
    {
	case NCTableLine::S_NORMAL:

	    switch ( cstyle )
	    {
		case NCTableCol::PLAIN:
		    return listStyle().item.plain;

		case NCTableCol::DATA:
		    return listStyle().item.data;

		case NCTableCol::ACTIVEDATA:
		    return listStyle().item.plain;

		case NCTableCol::HINT:
		    return listStyle().item.hint;

		case NCTableCol::SEPARATOR:
		    return listStyle().item.plain;

		case NCTableCol::NONE:
		    return currentBG;
	    }
	    break;

	    
	case NCTableLine::S_ACTIVE:

	    switch ( cstyle )
	    {
		case NCTableCol::PLAIN:
		    return listStyle().selected.plain;

		case NCTableCol::DATA:
		    return listStyle().selected.data;

		case NCTableCol::ACTIVEDATA:
		    return listStyle().selected.data;

		case NCTableCol::HINT:
		    return listStyle().selected.hint;

		case NCTableCol::SEPARATOR:
		    return listStyle().selected.plain;

		case NCTableCol::NONE:
		    return currentBG;
	    }
	    break;

	case NCTableLine::S_DISABELED:

	    switch ( cstyle )
	    {
		case NCTableCol::PLAIN:
		    return parw.wStyle().disabledList.item.plain;

		case NCTableCol::DATA:
		    return parw.wStyle().disabledList.item.data;

		case NCTableCol::ACTIVEDATA:
		    return parw.wStyle().disabledList.item.plain;

		case NCTableCol::HINT:
		    return parw.wStyle().disabledList.item.hint;

		case NCTableCol::SEPARATOR:
		    return listStyle().item.plain;

		case NCTableCol::NONE:
		    return currentBG;
	    }
	    break;

	    
	case NCTableLine::S_HEADLINE:
	    return listStyle().title;
	    break;

	case NCTableLine::S_HIDDEN:
	    return currentBG;
	    break;
    }

    return currentBG;
}


std::ostream & operator<<( std::ostream & STREAM, const NCTableStyle & OBJ )
{
    STREAM << form( "cols %d, sep %d (%lx)\n",
		    OBJ.Cols(), OBJ.ColSepwidth(), OBJ.ColSepchar() );

    for ( unsigned i = 0; i < OBJ.Cols(); ++i )
    {
	STREAM << form( "%2d %d(%3d) ", i, OBJ.ColAdjust( i ), OBJ.ColWidth( i ) );

	if ( OBJ.Headline().GetCol( i ) )
	    STREAM << OBJ.Headline().GetCol( i )->Label();

	STREAM << std::endl;
    }

    return STREAM;
}

