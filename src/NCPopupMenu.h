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

   File:       NCPopupMenu.h

   Author:     Michael Andres <ma@suse.de>

/-*/

#ifndef NCPopupMenu_h
#define NCPopupMenu_h

#include <iosfwd>
#include <map>

#include "NCPopupTable.h"
#include "NCMenuButton.h"


class NCPopupMenu : public NCPopupTable
{
private:

    NCPopupMenu & operator=( const NCPopupMenu & );
    NCPopupMenu( const NCPopupMenu & );

    std::map<YTableItem *, YMenuItem *> itemsMap;

protected:

    virtual NCursesEvent wHandleInput( wint_t ch );
    virtual bool postAgain();

public:

    NCPopupMenu( const wpos at,
		 YItemIterator begin,
		 YItemIterator end );

    virtual ~NCPopupMenu();

};



#endif // NCPopupMenu_h
