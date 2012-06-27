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

   File:       NCattribute.h

   Author:     Michael Andres <ma@suse.de>

/-*/

#ifndef NCattribute_h
#define NCattribute_h

#include <iosfwd>
#include <string>
#include <vector>

#include "ncursesw.h"


class NCattribute
{

    NCattribute & operator=( const NCattribute & );
    NCattribute( const NCattribute & );

public:

    enum NCAttribute
    {
	NCAdebug = 0	// for debugging and testing only
	, NCATitlewin	  // title line
	// WIDGETS
	, NCAWdumb	  // dumb widget
	, NCAWdisabeled   // disabeled widget
	// normal widget
	, NCAWnormal	  // default
	, NCAWlnormal	  // label
	, NCAWfnormal	  // frame
	, NCAWdnormal	  // data
	, NCAWhnormal	  // hint
	, NCAWsnormal	  // scroll hint
	// active widget
	, NCAWactive	  // default
	, NCAWlactive	  // label
	, NCAWfactive	  // frame
	, NCAWdactive	  // data
	, NCAWhactive	  // hint
	, NCAWsactive	  // scroll hint
	// DIALOG FRAME
	, NCADnormal	  // normal
	, NCADactive	  // active
	// COMMON ATTRIBUTES
	, NCACheadline	  // headlines
	, NCACcursor	  // cursor
	// RICHTEXT ATTRIBUTES
	, NCARTred
	, NCARTgreen
	, NCARTblue
	, NCARTyellow
	, NCARTmagenta
	, NCARTcyan
	// LAST ENTRY:
	, NCATTRIBUTE
    };

    enum NCAttrSet
    {
	ATTRDEF = 0
	, ATTRWARN
	, ATTRINFO
	, ATTRPOPUP
	// LAST ENTRY:
	, NCATTRSET
    };

protected:

    NCAttrSet		    defattrset;
    std::vector<vector<chtype> > attribset;

    virtual void _init();

    NCattribute( const bool init )
	: defattrset( ATTRDEF )
	, attribset(( unsigned )NCATTRSET, std::vector<chtype>(( unsigned )NCATTRIBUTE, A_NORMAL ) )
    {
	if ( init )
	    _init();
    }

public:

    NCattribute()
	: defattrset( ATTRDEF )
	, attribset(( unsigned )NCATTRSET, std::vector<chtype>(( unsigned )NCATTRIBUTE, A_NORMAL ) )
    {
	_init();
    }

    virtual ~NCattribute() {}

    chtype GetAttrib( const NCAttribute attr ) const
    {
	return attribset[defattrset][attr];
    }

    chtype GetAttrib( const NCAttribute attr, const NCAttrSet attrset ) const
    {
	return attribset[attrset][attr];
    }
};



class NCattrcolor : public NCattribute
{

protected:

    virtual void _init();

    bool scanFile( std::vector<chtype> & attribs );
    void scanLine( std::vector<chtype> & attribs, const std::string & line );
    void defInitSet( std::vector<chtype> & attribs, short f, short b );

public:

    NCattrcolor()
	    : NCattribute( false )
    {
	_init();
    }

    virtual ~NCattrcolor() {}
};


#endif // NCattribute_h
