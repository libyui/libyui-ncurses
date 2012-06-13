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

   File:       NCPopupMenu.cc

   Author:     Michael Andres <ma@suse.de>

/-*/

#define  YUILogComponent "ncurses"
#include <YUILog.h>
#include "NCPopupMenu.h"

#include "NCTable.h"
#include "YMenuButton.h"


NCPopupMenu::NCPopupMenu( const wpos at, YItemIterator begin, YItemIterator end )
	: NCPopupTable( at )
	, itemsMap()
{
    vector<string> row( 2 );
    createList( row );

    for ( YItemIterator it = begin; it != end; ++it )
    {
	YMenuItem * item = dynamic_cast<YMenuItem *>( *it );
	YUI_CHECK_PTR( item );

	row[0] = item->label();
	row[1] = item->hasChildren() ? "..." : "";

	YTableItem *tableItem = new YTableItem( row[0], row[1] );
	yuiDebug() << "Add to map: TableItem: " << tableItem << " Menu item: " << item << std::endl;

	addItem( tableItem );
	itemsMap[tableItem] = item;
    }

    stripHotkeys();
}


NCPopupMenu::~NCPopupMenu()
{
    itemsMap.clear();
}


NCursesEvent NCPopupMenu::wHandleInput( wint_t ch )
{
    NCursesEvent ret;

    switch ( ch )
    {
	case KEY_RIGHT:
	    {
		yuiDebug() << "CurrentItem: " << getCurrentItem() << std::endl;
		YTableItem * tableItem = dynamic_cast<YTableItem *> ( getCurrentItemPointer() );

		if ( tableItem )
		{
		    YMenuItem * item = itemsMap[ tableItem ];

		    if ( item && item->hasChildren() )
			ret = NCursesEvent::button;
		}

		break;
	    }

	case KEY_LEFT:
	    ret = NCursesEvent::cancel;
	    ret.detail = NCursesEvent::CONTINUE;
	    break;

	default:
	    ret = NCPopup::wHandleInput( ch );
	    break;
    }

    return ret;
}


bool NCPopupMenu::postAgain()
{
    // dont mess up postevent.detail here
    bool again = false;
    int  selection = ( postevent == NCursesEvent::button ) ? getCurrentItem()
		     : -1;
    yuiDebug() << "Index: " << selection << std::endl;
    YTableItem * tableItem = dynamic_cast<YTableItem *>( getCurrentItemPointer() );

    YMenuItem * item = itemsMap[ tableItem ];

    if ( !item )
	return false;

    yuiMilestone() << "Menu item: " << item->label() << std::endl;

    if ( selection != -1 )
    {
	if ( item->hasChildren() )
	{
	    // post submenu
	    wpos at( ScreenPos() + wpos( selection, inparent.Sze.W - 1 ) );
	    NCPopupMenu * dialog = new NCPopupMenu( at,
						    item->childrenBegin(),
						    item->childrenEnd() );
	    YUI_CHECK_NEW( dialog );

	    again = ( dialog->post( &postevent ) == NCursesEvent::CONTINUE );

	    if ( !again )
		YDialog::deleteTopmostDialog();
	}
	else
	{
	    // store selection
	    //postevent.detail = menu.itemList()[selection]->getIndex();
	    postevent.detail = item->index();
	}
    }

    return again;
}

