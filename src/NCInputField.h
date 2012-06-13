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

   File:       NCInputField.h

   Author:     Michael Andres <ma@suse.de>

/-*/

#ifndef NCInputField_h
#define NCInputField_h

#include <iosfwd>

#include "YInputField.h"
#include "NCWidget.h"


class NCInputField : public YInputField, public NCWidget
{

    friend std::ostream & operator<<( std::ostream & STREAM, const NCInputField & OBJ );

    NCInputField & operator=( const NCInputField & );
    NCInputField( const NCInputField & );

public:

    enum FTYPE
    {
	PLAIN,
	NUMBER
    };

private:

    bool     passwd;
    NClabel  label;
    wstring   buffer;

    NCursesWindow * lwin;
    NCursesWindow * twin;

    unsigned maxFldLength;
    unsigned maxInputLength;

    unsigned fldstart;
    unsigned fldlength;
    unsigned curpos;

    FTYPE    fldtype;
    NCstring validChars;

    bool     returnOnReturn_b;

    void setDefsze();
    void tUpdate();

    bool     bufferFull() const;
    unsigned maxCursor() const;

    // specifies how much characters can be inserted. -1 for unlimited input
    int InputMaxLength;

protected:

    virtual const char * location() const { return "NCInputField"; }

    virtual void wCreate( const wrect & newrect );
    virtual void wDelete();

    virtual void wRedraw();

    bool validKey( wint_t key ) const;

public:

    NCInputField( YWidget * parent,
		  const string & label,
		  bool passwordMode = false,
		  unsigned maxInput = 0,
		  unsigned maxFld   = 0
		);
    virtual ~NCInputField();

    void setFldtype( FTYPE t )		 { fldtype = t; }

    void setReturnOnReturn( bool on_br ) { returnOnReturn_b = on_br; }

    virtual int preferredWidth();
    virtual int preferredHeight();

    virtual void setSize( int newWidth, int newHeight );

    virtual void setLabel( const string & nlabel );

    virtual void setValue( const std::string & ntext );
    virtual string value();

    virtual void setValidChars( const string & validchars );

    virtual NCursesEvent wHandleInput( wint_t key );

    virtual void setEnabled( bool do_bv );

    virtual bool setKeyboardFocus()
    {
	if ( !grabFocus() )
	    return YWidget::setKeyboardFocus();

	return true;
    }

    // limits  the input to numberOfChars characters and truncates the text
    // if appropriate
    void setInputMaxLength( int numberOfChars );

    void setCurPos( unsigned pos ) { curpos = pos; }
};


#endif // NCInputField_h
