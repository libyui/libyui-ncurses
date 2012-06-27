/*
  Copyright (C) 1970-2012 Novell, Inc
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

   File:       NCLogView.h

   Author:     Michael Andres <ma@suse.de>

/-*/

#ifndef NCLogView_h
#define NCLogView_h

#include <iosfwd>

#include <yui/YLogView.h>
#include "NCPadWidget.h"


class NCLogView : public YLogView, public NCPadWidget
{
private:

    friend std::ostream & operator<<( std::ostream & STREAM, const NCLogView & OBJ );

    NCLogView & operator=( const NCLogView & );
    NCLogView( const NCLogView & );


    NCtext text;

protected:

    virtual const char * location() const { return "NCLogView"; }

    virtual void wRedraw();
    virtual void wRecoded();

    virtual NCPad * CreatePad();
    virtual void    DrawPad();

public:

    NCLogView( YWidget * parent,
	       const std::string & label,
	       int visibleLines,
	       int maxLines );
    virtual ~NCLogView();

    virtual int preferredWidth();
    virtual int preferredHeight();

    virtual void setSize( int newWidth, int newHeight );

    virtual void setLabel( const std::string & nlabel );
    virtual void displayLogText( const std::string & ntext );

    virtual NCursesEvent wHandleInput( wint_t key );

    virtual void setEnabled( bool do_bv );

    virtual bool setKeyboardFocus()
    {
	if ( !grabFocus() )
	    return YWidget::setKeyboardFocus();

	return true;
    }
};


#endif // NCLogView_h
