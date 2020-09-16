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

   File:       NCTree.cc

   Author:     Michael Andres <ma@suse.de>

/-*/

#define	 YUILogComponent "ncurses"
#include <yui/YUILog.h>
#include "NCTree.h"
#include "YNCursesUI.h"

#include <yui/TreeItem.h>
#include <yui/YSelectionWidget.h>

using std::string;
using std::endl;


NCTree::NCTree( YWidget *      parent,
                const string & nlabel,
                bool           multiselection,
                bool           recursiveselection )
    : YTree( parent, nlabel, multiselection, recursiveselection )
    , NCPadWidget( parent )
    , _multiSelect( multiselection )
{
    // yuiDebug() << endl;

    if ( multiselection && recursiveselection )
	yuiDebug() << "NCTree recursive multi selection ON" << endl;
    else if ( multiselection )
	yuiDebug() << "NCTree multi selection ON" << endl;

    setLabel( nlabel );
}


NCTree::~NCTree()
{

}


inline const NCTreeLine *
NCTree::getTreeLine( unsigned idx ) const
{
    if ( myPad() )
	return dynamic_cast<const NCTreeLine *>( myPad()->GetLine( idx ) );
    else
	return 0;
}


inline NCTreeLine *
NCTree::modifyTreeLine( unsigned idx )
{
    if ( myPad() )
    {
	return dynamic_cast<NCTreeLine *>( myPad()->ModifyLine( idx ) );
    }

    return 0;
}


int NCTree::preferredWidth()
{
    wsze sze = wsze::max( defsze, wsze( 0, labelWidth() + 2 ) );
    return sze.W;
}


int NCTree::preferredHeight()
{
    wsze sze = wsze::max( defsze, wsze( 0, labelWidth() + 2 ) );
    return sze.H;
}


void NCTree::setEnabled( bool do_bv )
{
    NCWidget::setEnabled( do_bv );
    YWidget::setEnabled( do_bv );
}


void NCTree::setSize( int newwidth, int newheight )
{
    wRelocate( wpos( 0 ), wsze( newheight, newwidth ) );
}


YTreeItem * NCTree::getCurrentItem() const
{
    YTreeItem * yitem = 0;

    if ( myPad() && myPad()->GetCurrentLine() )
    {
	const NCTreeLine * cline = dynamic_cast<const NCTreeLine *>( myPad()->GetCurrentLine() );

	if ( cline )
	    yitem = cline->YItem();
    }

    // yuiDebug() << "-> " << ( yitem ? yitem->label().c_str() : "noitem" ) << endl;

    return yitem;
}


void NCTree::deselectAllItems()
{
    if ( _multiSelect )
    {
	YItemCollection selectedItems = YTree::selectedItems();

	for ( YItemConstIterator it = selectedItems.begin(); it != selectedItems.end(); ++it )
	{
	    selectItem( *it, false );
	}
    }

    YTree::deselectAllItems();
}


void NCTree::selectItem( YItem *item, bool selected )
{
    if ( !myPad() )
	return;

    YTreeItem * treeItem =  dynamic_cast<YTreeItem *>( item );
    YUI_CHECK_PTR( treeItem );
    YTreeItem *citem = getCurrentItem();

    //retrieve position of item
    int at = treeItem->index();

    NCTreeLine * cline = 0;	// current line
    NCTableCol * ccol = 0;	// current column

    if ( _multiSelect )
    {
	cline = modifyTreeLine( at );

	if ( cline )
	{
	    ccol = cline->GetCol(0);
	}
    }

    if ( !selected )
    {
	if ( !_multiSelect && ( treeItem == citem ) )
	{
	    YTree::deselectAllItems();
	}
	else
	{
	    YTree::selectItem( treeItem, false );

	    if ( ccol )
	    {
		ccol->SetLabel( NCstring( string( cline->Level() + 3, ' ' ) + "[ ] "
					  + item->label() ) );
	    }
	}
    }
    else
    {
	YTree::selectItem( treeItem, selected );

	if ( _multiSelect && ccol )
	{
	    ccol->SetLabel( NCstring( string( cline->Level() + 3, ' ' ) + "[x] "
				      + item->label() ) );
	}

	// Highlight the selected item and possibly expand the tree if it is in
	// a currently hidden branch

	myPad()->ShowItem( getTreeLine( at ) );
    }
}


void NCTree::selectItem( int index )
{
    YItem * item = YTree::itemAt( index );

    if ( item )
    {
	selectItem( item, true );
    }
    else
	YUI_THROW( YUIException( "Can't find selected item" ) );
}


void NCTree::setLabel( const string & nlabel )
{
    YTree::setLabel( nlabel );
    NCPadWidget::setLabel( NCstring( nlabel ) );
}


void NCTree::rebuildTree()
{
    DelPad();
    Redraw();
}


NCPad * NCTree::CreatePad()
{
    wsze    psze( defPadSze() );
    NCPad * npad = new NCTreePad( psze.H, psze.W, *this );
    npad->bkgd( listStyle().item.plain );
    return npad;
}


void NCTree::CreateTreeLines( NCTreeLine * parentLine,
                              NCTreePad  * pad,
                              YItem      * item )
{
    // Set the item index explicitely: It is set to -1 by default which makes
    // selecting items painful.

    item->setIndex( idx++ );

    YTreeItem * treeItem = dynamic_cast<YTreeItem *>( item );
    YUI_CHECK_PTR( treeItem );

    NCTreeLine * line = new NCTreeLine( parentLine, treeItem, _multiSelect );
    pad->Append( line );

    if (item->selected())
    {
        // Retrieve the position of the item
        int at = treeItem->index();

        NCTreeLine * cline = 0;     // current line
        NCTableCol * ccol = 0;      // current column
        if ( _multiSelect )
        {
            cline = modifyTreeLine( at );

            if ( cline )
                ccol = cline->GetCol(0);

            if ( ccol )
            {
                ccol->SetLabel( NCstring( string( cline->Level() + 3, ' ' ) + "[x] "
                                          + item->label() ) );
            }
        }

        // Highlight the selected item and expand the tree if it is in a
        // currently hidden branch

        pad->ShowItem( getTreeLine( at ) );
    }

    // Create TreeLines for the children of this item

    for ( YItemIterator it = item->childrenBegin();  it < item->childrenEnd(); ++it )
    {
	CreateTreeLines( line, pad, *it );
    }
}


YTreeItem * NCTree::currentItem()
{
    return getCurrentItem();
}


void NCTree::DrawPad()
{
    if ( !myPad() )
    {
	yuiWarning() << "PadWidget not yet created" << endl;
	return;
    }

    idx = 0;

    // Iterate over the toplevel items

    for ( YItemIterator it = itemsBegin(); it < itemsEnd(); ++it )
    {
        // Create a TreeLine for this item.
        // This will recurse into children if there are any.

	CreateTreeLines( 0, myPad(), *it );
    }

    idx = 0;
    NCPadWidget::DrawPad();
}


NCursesEvent NCTree::wHandleInput( wint_t key )
{
    NCursesEvent ret = NCursesEvent::none;
    YTreeItem * oldCurrentItem = getCurrentItem();

    bool handled = handleInput( key ); // NCTreePad::handleInput()
    const YItem * currentItem = getCurrentItem();

    if ( !currentItem )
	return ret;

    if ( _multiSelect )
    {
	if ( ! handled )
	{
	    switch ( key )
	    {
		// KEY_SPACE is handled in NCTreeLine::handleInput
		case KEY_RETURN:

		    if ( currentItem->selected() )
			selectItem( const_cast<YItem *>(currentItem), false );
		    else
			selectItem( const_cast<YItem *>(currentItem), true );

		    if ( notify() )
		    {
			return NCursesEvent::ValueChanged;
		    }
		    break;
	    }
	}
    }
    else
    {
	if ( ! handled )
	{
	    switch ( key )
	    {
		// KEY_SPACE is handled in NCTreeLine::handleInput
		case KEY_RETURN:

		    if ( notify() )
		    {
			return NCursesEvent::Activated;
		    }
		    break;
	    }
	}

	YTree::selectItem( const_cast<YItem *>( currentItem ), true );
    }

    if ( notify() && immediateMode() && ( oldCurrentItem != currentItem ) )
	    ret = NCursesEvent::SelectionChanged;

    // yuiDebug() << "Notify: " << ( notify() ? "true" : "false" )
    // << " Return event: " << ret.reason << endl;

    return ret;
}


void NCTree::activate()
{
    // Send an activation event for this widget

    NCursesEvent event = NCursesEvent::Activated;
    event.widget = this;
    YNCursesUI::ui()->sendEvent(event);
}


void NCTree::deleteAllItems()
{
    YTree::deleteAllItems();
    myPad()->ClearTable();
}


//
// ----------------------------------------------------------------------
//


NCTreeLine::NCTreeLine( NCTreeLine * parentLine,
                        YTreeItem  * item,
                        bool         multiSelection )
    : NCTableLine( 0 )
    , _yitem( item )
    , _level( parentLine ? parentLine->Level() + 1 : 0 )
    , _parent( parentLine )
    , _nsibling( 0 )
    , _fchild( 0 )
    , _prefix( 0 )
    , _multiSelect( multiSelection )
{
    if ( _parent )
    {
        if ( _parent->_fchild )
        {
            NCTreeLine * sibling = _parent->_fchild;

            for ( ; sibling->_nsibling; sibling = sibling->_nsibling )
                ;

            sibling->_nsibling = this;
        }
        else
        {
            _parent->_fchild = this;
        }

        if ( !_parent->_yitem->isOpen() )
        {
            SetState( S_HIDDEN );
        }
    }

    if ( !_multiSelect )
    {
        Append( new NCTableCol( NCstring( string( prefixLen(), ' ' )
					  + _yitem->label() ) ) );
    }
    else
    {
        Append( new NCTableCol( NCstring( string( prefixLen(), ' ' ) + "[ ] "
                                          + _yitem->label() ) ) );
    }
}


NCTreeLine::~NCTreeLine()
{
    delete [] _prefix;
}


bool NCTreeLine::isVisible() const
{
    return !_parent || ( !isHidden() && _parent->isVisible() );
}


bool NCTreeLine::ChangeToVisible()
{
    if ( isVisible() )
        return false;   // no status change

    if ( _parent )
    {
        // Make sure the parent is visible.
        // This recurses upwards until the toplevel.

        _parent->ChangeToVisible();


        // Make sure this line and all siblings on this level are visible

        for ( NCTreeLine * sibling = _parent->_fchild; sibling; sibling = sibling->_nsibling )
        {
            sibling->ClearState( S_HIDDEN );
            sibling->YItem()->setOpen( true );
        }
    }
    else // No parent (i.e. this is a toplevel item)
    {
        // Make sure this item is visible

        ClearState( S_HIDDEN );
        _yitem->setOpen( true );
    }

    return true; // status change (the line was invisible before)
}


unsigned NCTreeLine::Hotspot( unsigned & at ) const
{
    at = Level();
    return 6;
}

#define EVENT_HANDLED           1
#define EVENT_NOT_HANDLED       0

int NCTreeLine::handleInput( wint_t key )
{
    switch ( key )
    {
        case KEY_IC:    // "Insert" key
        case '+':
            // Open this branch

            if ( ! _fchild->isVisible() )
                toggleOpenClosedState();

            return EVENT_HANDLED;


        case KEY_DC:    // "Delete" key
        case '-':
            // Close this branch

            if ( _fchild->isVisible() )
                toggleOpenClosedState();

            return EVENT_HANDLED;


        case KEY_SPACE: // Toggle open/closed state of this branch
            //	case KEY_RETURN: see bug 67350

            toggleOpenClosedState();
            return EVENT_HANDLED;


        default:
            return EVENT_NOT_HANDLED;   // propagate this event up to the widget parent
    }
}


void NCTreeLine::toggleOpenClosedState()
{
    if ( ! _fchild )
        return;

    if ( _fchild->isVisible() )
    {
        _yitem->setOpen( false );
        yuiDebug() << "Closing item " << _yitem->label() << endl;

        for ( NCTreeLine * child = _fchild; child; child = child->_nsibling )
            child->SetState( S_HIDDEN );
    }
    else
    {
        _yitem->setOpen( true );
        yuiDebug() << "Opening item " << _yitem->label() << endl;

        for ( NCTreeLine * child = _fchild; child; child = child->_nsibling )
            child->ClearState( S_HIDDEN );
    }
}


void NCTreeLine::DrawAt( NCursesWindow & w,
                         const wrect     at,
                         NCTableStyle  & tableStyle,
                         bool            active ) const
{

    NCTableLine::DrawAt( w, at, tableStyle, active );

    if ( !isSpecial() )
        w.bkgdset( tableStyle.getBG( vstate, NCTableCol::SEPARATOR ) );

    if ( ! _prefix )
    {
        _prefix = new chtype[ prefixLen() ];
        chtype * tagend = &_prefix[ prefixLen()-1 ];
        *tagend-- = ACS_HLINE;
        *tagend-- = _fchild ? ACS_TTEE : ACS_HLINE;

        if ( _parent )
        {
            *tagend-- = _nsibling ? ACS_LTEE : ACS_LLCORNER;

            for ( NCTreeLine * p = _parent; p; p = p->_parent )
            {
                *tagend-- = p->_nsibling ? ACS_VLINE : ( ' '&A_CHARTEXT );
            }
        }
        else
        {
            *tagend-- = ACS_HLINE;
        }
    }

    w.move( at.Pos.L, at.Pos.C );

    unsigned i = 0;

    for ( ; i < prefixLen(); ++i )
        w.addch( _prefix[i] );

    w.move( at.Pos.L, at.Pos.C + prefixLen() - 2 );

    if ( _fchild && !isSpecial() )
    {
        w.bkgdset( tableStyle.highlightBG( vstate,
                                           NCTableCol::HINT,
                                           NCTableCol::SEPARATOR ) );
    }

    if ( _fchild && !_fchild->isVisible() )
        w.addch( '+' );
    else
        w.addch( _prefix[ prefixLen() - 2 ] );
}
