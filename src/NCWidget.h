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

   File:       NCWidget.h

   Author:     Michael Andres <ma@suse.de>

/-*/

#ifndef NCWidget_h
#define NCWidget_h


#include <iosfwd>

#include "NCurses.h"
#include "tnode.h"

class NCursesWindow;
class NClabel;


#define DLOC location() << ' '

#define YWIDGET_MAGIC	42


class NCWidget : public tnode<NCWidget*>, protected NCursesError
{
private:

    friend std::ostream & operator<<( std::ostream & STREAM, const NCWidget & OBJ );
    friend std::ostream & operator<<( std::ostream & STREAM, const NCWidget * OBJ );

    NCWidget & operator=( const NCWidget & );
    NCWidget( const NCWidget & );


    /**
     * Make this widget invalid. This operation cannot be reversed.
     */
    void invalidate() { magic = 0; }

    /**
     * This object is only valid if this magic number is YWIDGET_MAGIC.
     */
    int magic;

protected:

    virtual const char * location() const { return "NCWidget"; }

    virtual void PreDisconnect();
    virtual void PostDisconnect();
    virtual void PreReparent();
    virtual void PostReparent();

    NCursesWindow * ParentWin();

    NCWidget *const grabedBy;
    bool grabFocus();
    virtual void grabNotify( NCWidget * ) {}

    virtual bool wantFocus( NCWidget & ngrab ) { return false; }

public:

    void grabSet( NCWidget * ngrab )
    {
	if ( grabedBy && grabedBy != ngrab )
	    grabedBy->grabNotify( this );

	const_cast<NCWidget *&>( grabedBy ) = ngrab;
    }

    void grabRelease( NCWidget * ograb )
    {
	if ( grabedBy && grabedBy != ograb )
	    grabedBy->grabNotify( this );

	const_cast<NCWidget *&>( grabedBy ) = 0;
    }

protected:

    NCursesWindow * win;
    wsze	    defsze;
    wrect	    framedim;
    wrect	    inparent;
    bool	    noUpdates;
    bool	    skipNoDimWin;

    void wMoveChildTo( NCWidget & child, const wpos & newpos );
    void wRelocate( const wrect & newrect );
    void wRelocate( const wpos & newpos, const wsze & newsze )
    {
	wRelocate( wrect( newpos, newsze ) );
    }

    virtual void wCreate( const wrect & newrect );
    virtual void wMoveTo( const wpos & newpos );
    virtual void wDelete();
    virtual void wUpdate( bool forced_br = false );

    wpos ScreenPos() const;

    NC::WState wstate;

    virtual void wRedraw();
    virtual void wRecoded();

    NClabel * hotlabel;

public:

    NCWidget( NCWidget * myparent );
    NCWidget( YWidget * parent = 0 );
    virtual ~NCWidget();

    bool isValid() const	{ return magic == YWIDGET_MAGIC; }

    bool winExist() const	{ return win != ( NCursesWindow * )0; }

    virtual const NCstyle::Style & wStyle() const
    {
	if ( Parent() )
	    return Top().Value()->wStyle();

	return NCurses::style()[NCstyle::DefaultStyle];
    }

    const NCstyle::StWidget & widgetStyle( bool nonactive = false ) const
	{ return wStyle().getWidget( GetState(), nonactive ); }

    const NCstyle::StWidget & frameStyle()  const
	{ return wStyle().getFrame( GetState() ); }

    const NCstyle::StList   & listStyle()   const
	{ return wStyle().getList( GetState() ); }

    wsze wGetDefsze() const { return defsze; }

    wrect wGetSize() const { return inparent; }

    void Update();
    void Redraw( const bool sub = false );
    void Recoded();

    NC::WState GetState() const { return wstate; }

    void SetState( const NC::WState newstate, const bool force = false );

    /**
     * Pure virtual to make sure every widget implements it. Necessary to make
     * sure that UI requests via YWidget::setEnabled perform, and behave the
     * same way as direct calls to NCWidget::setEnabled.
     **/
    virtual void setEnabled( bool do_bv ) = 0;

    virtual bool HasHotkey( int key );
    virtual bool HasFunctionHotkey( int key ) const;

    virtual NCursesEvent wHandleHotkey( wint_t key );
    virtual NCursesEvent wHandleInput( wint_t key );

    void DumpOn( std::ostream & str, std::string prfx ) const;
};


#include "NCstring.h"
#include "NCtext.h"


#endif // NCWidget_h
