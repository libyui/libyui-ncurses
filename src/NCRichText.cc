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

   File:       NCRichText.cc

   Author:     Michael Andres <ma@suse.de>

/-*/

#define	 YUILogComponent "ncurses"
#include <yui/YUILog.h>
#include "NCRichText.h"
#include "YNCursesUI.h"
#include "stringutil.h"
#include "stdutil.h"
#include <sstream>
#include <boost/algorithm/string.hpp>

#include <yui/YMenuItem.h>
#include <yui/YApplication.h>

using stdutil::form;


const unsigned NCRichText::listindent = 4;
const std::wstring	NCRichText::listleveltags( L"@*+o#-%$&" );//

const bool NCRichText::showLinkTarget = false;

std::map<std::wstring, std::wstring> NCRichText::_charentity;



const std::wstring NCRichText::entityLookup( const std::wstring & val_r )
{
    //strip leading '#', if any
    std::wstring::size_type hash = val_r.find( L"#", 0 );
    std::wstring ascii = L"";

    if ( hash != std::wstring::npos )
    {
	std::wstring s = val_r.substr( hash + 1 );
	wchar_t *endptr;
	//and try to convert to int (wcstol only knows "0x" for hex)
        boost::replace_all( s, "x", "0x" );

	long int c = std::wcstol( s.c_str(), &endptr, 0 );

	//conversion succeeded

	if ( s.c_str() != endptr )
	{
	    std::wostringstream ws;
	    ws << char( c );
	    ascii = ws.str();
	}
    }

#define REP(l,r) _charentity[l] = r
    if ( _charentity.empty() )
    {
	// initialize replacement for character entities. A value of NULL
	// means do not replace.
	std::wstring product;
	NCstring::RecodeToWchar( YUI::app()->productName(), "UTF-8", &product );

	REP( L"amp", L"&" );
	REP( L"gt", L">" );
	REP( L"lt", L"<" );
	REP( L"nbsp", L" " );
	REP( L"quot", L"\"" );
	REP( L"product", product );
    }

    std::map<std::wstring, std::wstring>::const_iterator it = _charentity.find( val_r );

    if ( it != _charentity.end() )
    {
	//known entity - already in the map
	return it->second;
    }
    else
    {
	if ( !ascii.empty() )
	{
	    //replace ascii code by character - e.g. #42 -> '*'
	    //and insert into map to remember it
	    REP( val_r, ascii );
	}
    }

    return ascii;

#undef REP
}



/**
 * Filter out the known &...; entities and return the text with entities
 * replaced
 **/
const std::wstring NCRichText::filterEntities( const std::wstring & text )
{
    std::wstring txt = text;
    // filter known '&..;'

    for ( std::wstring::size_type special = txt.find( L"&" );
	  special != std::wstring::npos;
	  special = txt.find( L"&", special + 1 ) )
    {
	std::wstring::size_type colon = txt.find( L";", special + 1 );

	if ( colon == std::wstring::npos )
	    break;  // no ';'  -> no need to continue

	const std::wstring repl = entityLookup( txt.substr( special + 1, colon - special - 1 ) );

	if ( !repl.empty()
	     || txt.substr( special + 1, colon - special - 1 ) == L"product" )	// always replace &product;
	{
	    txt.replace( special, colon - special + 1, repl );
	}
	else
	    yuiDebug() << "porn.bat" << std::endl;
    }

    return txt;
}


void NCRichText::Anchor::draw( NCPad & pad, const chtype attr, int color )
{
    unsigned l = sline;
    unsigned c = scol;

    while ( l < eline )
    {
	pad.move( l, c );
	pad.chgat( -1, attr, color );
	++l;
	c = 0;
    }

    pad.move( l, c );

    pad.chgat( ecol - c, attr, color );
}


NCRichText::NCRichText( YWidget * parent, const std::string & ntext,
			bool plainTextMode )
	: YRichText( parent, ntext, plainTextMode )
	, NCPadWidget( parent )
	, text( ntext )
	, plainText( plainTextMode )
	, textwidth( 0 )
	, cl( 0 )
	, cc( 0 )
	, cindent( 0 )
	, atbol( true )
	, preTag( false )
	, Tattr( 0 )
{
    // yuiDebug() << std::endl;
    activeLabelOnly = true;
    setValue( ntext );
}


NCRichText::~NCRichText()
{
    // yuiDebug() << std::endl;
}


int NCRichText::preferredWidth()
{
    return wGetDefsze().W;
}


int NCRichText::preferredHeight()
{
    return wGetDefsze().H;
}


void NCRichText::setEnabled( bool do_bv )
{
    NCWidget::setEnabled( do_bv );
    YRichText::setEnabled( do_bv );
}


void NCRichText::setSize( int newwidth, int newheight )
{
    wRelocate( wpos( 0 ), wsze( newheight, newwidth ) );
}


void NCRichText::setLabel( const std::string & nlabel )
{
    // not implemented: YRichText::setLabel( nlabel );
    NCPadWidget::setLabel( NCstring( nlabel ) );
}


void NCRichText::setValue( const std::string & ntext )
{
    DelPad();
    text = NCstring( ntext );
    YRichText::setValue( ntext );
    Redraw();
}


void NCRichText::wRedraw()
{
    if ( !win )
	return;

    bool initial = ( !myPad() || !myPad()->Destwin() );

    if ( !( plainText || anchors.empty() ) )
	arm( armed );

    NCPadWidget::wRedraw();

    if ( initial && autoScrollDown() )
    {
	myPad()->ScrlTo( wpos( myPad()->maxy(), 0 ) );
    }

    return;
}


void NCRichText::wRecoded()
{
    DelPad();
    wRedraw();
}


void NCRichText::activateLink( const std::string & url )
{
    NCursesEvent event = NCursesEvent::menu;
    event.result = url;
    event.widget = this;
    YNCursesUI::ui()->sendEvent( event );
}


NCursesEvent NCRichText::wHandleInput( wint_t key )
{
    NCursesEvent ret;
    handleInput( key );

    if ( !( plainText || anchors.empty() ) )
    {
	switch ( key )
	{
	    case KEY_SPACE:
	    case KEY_RETURN:

		if ( armed != Anchor::unset )
		{
		    ret = NCursesEvent::menu;
		    std::string str;
		    NCstring::RecodeFromWchar( anchors[armed].target, "UTF-8", &str );
		    yuiMilestone() << "LINK: " << str << std::endl;
		    ret.result = str;
		    ret.selection = NULL;
		}

		break;
	}
    }
    return ret;
}


NCPad * NCRichText::CreatePad()
{
    wsze psze( defPadSze() );
    textwidth = psze.W;
    NCPad * npad = new NCPad( psze.H, textwidth, *this );
    return npad;
}


void NCRichText::DrawPad()
{
#if 0
    yuiDebug() << "Start: plain mode " << plainText << std::endl
               << "       padsize " << myPad()->size() << std::endl
               << "       text length " << text.str().size() << std::endl;
#endif

    myPad()->bkgdset( wStyle().richtext.plain );
    myPad()->clear();

    if ( plainText )
	DrawPlainPad();
    else
	DrawHTMLPad();

    // yuiDebug() << "Done" << std::endl;
}


void NCRichText::DrawPlainPad()
{
    NCtext ftext( text );
    // yuiDebug() << "ftext is " << wsze( ftext.Lines(), ftext.Columns() ) << std::endl;

    AdjustPad( wsze( ftext.Lines(), ftext.Columns() ) );

    cl = 0;

    for ( NCtext::const_iterator line = ftext.begin();
	  line != ftext.end(); ++line, ++cl )
    {
	myPad()->addwstr( cl, 0, ( *line ).str().c_str() );
    }
}

void NCRichText::PadPreTXT( const wchar_t * osch, const unsigned olen )
{
    std::wstring wtxt( osch, olen );

    // resolve the entities even in PRE (#71718)
    wtxt = filterEntities( wtxt );

    NCstring nctxt( wtxt );
    NCtext ftext( nctxt );

    // insert the text
    const wchar_t * sch = wtxt.data();

    while ( *sch )
    {
	myPad()->addwstr( sch, 1 );	// add one wide chararacter

	++sch;
    }
}

//
// DrawHTMLPad tools
//

inline void SkipToken( const wchar_t *& wch )
{
    do
    {
	++wch;
    }
    while ( *wch && *wch != L'>' );

    if ( *wch )
	++wch;
}


static std::wstring WStoken( L" \n\t\v\r\f" );


inline void SkipWS( const wchar_t *& wch )
{
    do
    {
	++wch;
    }
    while ( *wch && WStoken.find( *wch ) != std::wstring::npos );
}


static std::wstring WDtoken( L" <\n\t\v\r\f" ); // WS + TokenStart '<'


inline void SkipWord( const wchar_t *& wch )
{
    do
    {
	++wch;
    }
    while ( *wch && WDtoken.find( *wch ) == std::wstring::npos );
}

static std::wstring PREtoken( L"<\n\v\r\f" ); // line manipulations + TokenStart '<'


inline void SkipPreTXT( const wchar_t *& wch )
{
    do
    {
	++wch;
    }
    while ( *wch && PREtoken.find( *wch ) == std::wstring::npos );
}


//
// Calculate longest line of text in <pre> </pre> tags
// and adjust the pad accordingly
//
void NCRichText::AdjustPrePad( const wchar_t *osch )
{
    const wchar_t * wch = osch;
    std::wstring wstr( wch, 6 );

    do
    {
        ++wch;
	wstr.assign( wch, 6 );
    }
    while ( *wch && wstr != L"</pre>" );

    std::wstring wtxt( osch, wch - osch );

    // resolve the entities to get correct length for calculation of longest line
    wtxt = filterEntities( wtxt );

    // replace <br> by \n to get appropriate lines in NCtext
    boost::replace_all( wtxt, L"<br>", L"\n" );
    boost::replace_all( wtxt, L"<br/>", L"\n" );

    // yuiDebug() << "Text: " << wtxt << " initial length: " << wch - osch << std::endl;

    NCstring nctxt( wtxt );
    NCtext ftext( nctxt );

    std::list<NCstring>::const_iterator line;
    size_t llen = 0;		// longest line

    // iterate through NCtext
    for ( line = ftext.Text().begin(); line != ftext.Text().end(); ++line )
    {
	size_t tmp_len = 0;

        tmp_len = textWidth( (*line).str() );

	if ( tmp_len > llen )
	    llen = tmp_len;
    }
    // yuiDebug() << "Longest line: " << llen << std::endl;

    if ( llen > textwidth )
    {
	textwidth = llen;
	AdjustPad( wsze( cl + ftext.Lines(), llen ) );	// adjust pad to longest line
    }

}

void NCRichText::DrawHTMLPad()
{
    // yuiDebug() << "Start:" << std::endl;

    liststack = std::stack<int>();
    canchor = Anchor();
    anchors.clear();
    armed = Anchor::unset;

    cl = 0;
    cc = 0;
    cindent = 0;
    myPad()->move( cl, cc );
    atbol = true;

    const wchar_t * wch = ( wchar_t * )text.str().data();
    const wchar_t * swch = 0;

    while ( *wch )
    {
	switch ( *wch )
	{
	    case L' ':
	    case L'\t':
	    case L'\n':
	    case L'\v':
	    case L'\r':
            case L'\f':
		if ( ! preTag )
		{
		    SkipWS( wch );
		    PadWS();
		}
		else
		{
		    switch ( *wch )
		    {
			case L' ':	// add white space
			case L'\t':
			    myPad()->addwstr( wch, 1 );
			    break;

			case L'\n':
                        case L'\f':
			    PadNL();	// add new line
			    break;

			default:
			    yuiDebug() << "Ignoring " << *wch << std::endl;
		    }
		    ++wch;
		}

		break;

	    case L'<':
		swch = wch;
		SkipToken( wch );

		if ( PadTOKEN( swch, wch ) )
		    break;	// strip token
		else
		    wch = swch;		// reset and fall through

	    default:
		swch = wch;

		if ( !preTag )
		{
		    SkipWord( wch );
		    PadTXT( swch, wch - swch );
		}
		else
		{
		    SkipPreTXT( wch );
		    PadPreTXT( swch, wch - swch );
		}

		break;
	}
    }

    PadBOL();
    AdjustPad( wsze( cl, textwidth ) );

#if 0
    yuiDebug() << "Anchors: " << anchors.size() << std::endl;

    for ( unsigned i = 0; i < anchors.size(); ++i )
    {
	yuiDebug() << form( "  %2d: [%2d,%2d] -> [%2d,%2d]",
			    i,
			    anchors[i].sline, anchors[i].scol,
			    anchors[i].eline, anchors[i].ecol ) << std::endl;
    }
#endif
}


inline void NCRichText::PadNL()
{
    cc = cindent;

    if ( ++cl == ( unsigned )myPad()->height() )
    {
	AdjustPad( wsze( myPad()->height() + defPadSze().H, textwidth ) );
    }

    myPad()->move( cl, cc );

    atbol = true;
}


inline void NCRichText::PadBOL()
{
    if ( !atbol )
	PadNL();
}


inline void NCRichText::PadWS( bool tab )
{
    if ( atbol )
	return; // no WS at beginning of line

    if ( cc == textwidth )
    {
	PadNL();
    }
    else
    {
	myPad()->addwstr( L" " );
	++cc;
    }
}


inline void NCRichText::PadTXT( const wchar_t * osch, const unsigned olen )
{
    std::wstring txt( osch, olen );

    txt = filterEntities( txt );

    size_t	len = textWidth( txt );

    if ( !atbol && cc + len > textwidth )
	PadNL();

    // insert the text
    const wchar_t * sch = txt.data();

    while ( *sch )
    {
	myPad()->addwstr( sch, 1 );	// add one wide chararacter
	cc += wcwidth( *sch );
	atbol = false;	// at begin of line = false

	if ( cc >= textwidth )
	{
	    PadNL();	// add a new line
	}

	sch++;
    }
}

/**
 * Get the number of columns needed to print a 'std::wstring'. Only printable characters
 * are taken into account because otherwise 'wcwidth' would return -1 (e.g. for '\n').
 * Tabs are calculated with tabsize().
 * Attention: only use textWidth() to calculate space, not for iterating through a text
 * or to get the length of a text (real text length includes new lines).
 */
size_t NCRichText::textWidth( std::wstring wstr )
{
    size_t len = 0;
    std::wstring::const_iterator wstr_it;	// iterator for std::wstring

    for ( wstr_it = wstr.begin(); wstr_it != wstr.end() ; ++wstr_it )
    {
	// check whether char is printable
	if ( iswprint( *wstr_it ) )
	{
	    len += wcwidth( *wstr_it );
	}
	else if ( *wstr_it == '\t' )
	{
	    len += myPad()->tabsize();
	}
    }

    return len;
}


/**
 * Set character attributes (e.g. color, font face...)
 **/
inline void NCRichText::PadSetAttr()
{
    const NCstyle::StRichtext & style( wStyle().richtext );
    chtype nbg = style.plain;

    if ( Tattr & T_ANC )
    {
	nbg = style.link;
    }
    else if ( Tattr & T_HEAD )
    {
	nbg = style.title;
    }
    else
    {
	switch ( Tattr & Tfontmask )
	{
	    case T_BOLD:
		nbg = style.B;
		break;

	    case T_IT:
		nbg = style.I;
		break;

	    case T_TT:
		nbg = style.T;
		break;

	    case T_BOLD|T_IT:
		nbg = style.BI;
		break;

	    case T_BOLD|T_TT:
		nbg = style.BT;
		break;

	    case T_IT|T_TT:
		nbg = style.IT;
		break;

	    case T_BOLD|T_IT|T_TT:
		nbg = style.BIT;
		break;
	}
    }

    myPad()->bkgdset( nbg );
}


void NCRichText::PadSetLevel()
{
    cindent = listindent * liststack.size();

    if ( cindent > textwidth / 2 )
	cindent = textwidth / 2;

    if ( atbol )
    {
	cc = cindent;
	myPad()->move( cl, cc );
    }
}


void NCRichText::PadChangeLevel( bool down, int tag )
{
    if ( down )
    {
	if ( liststack.size() )
	    liststack.pop();
    }
    else
    {
	liststack.push( tag );
    }

    PadSetLevel();
}


void NCRichText::openAnchor( std::wstring args )
{
    canchor.open( cl, cc );

    const wchar_t * ch = ( wchar_t * )args.data();
    const wchar_t * lookupstr = L"href = ";
    const wchar_t * lookup = lookupstr;

    for ( ; *ch && *lookup; ++ch )
    {
	wchar_t c = towlower( *ch );

	switch ( c )
	{
	    case L'\t':
	    case L' ':

		if ( *lookup != L' ' )
		    lookup = lookupstr;

		break;

	    default:
		if ( *lookup == L' ' )
		{
		    ++lookup;

		    if ( !*lookup )
		    {
			// ch is the 1st char after lookupstr
			--ch; // end of loop will ++ch
			break;
		    }
		}

		if ( c == *lookup )
		    ++lookup;
		else
		    lookup = lookupstr;

		break;
	}
    }

    if ( !*lookup )
    {
	const wchar_t * delim = ( *ch == L'"' ) ? L"\"" : L" \t";
	args = ( *ch == L'"' ) ? ++ch : ch;

	std::wstring::size_type end = args.find_first_of( delim );

	if ( end != std::wstring::npos )
	    args.erase( end );

	canchor.target = args;
    }
    else
    {
	yuiError() << "No value for 'HREF=' in anchor '" << args << "'" << std::endl;
    }
}


void NCRichText::closeAnchor()
{
    canchor.close( cl, cc );

    if ( canchor.valid() )
	anchors.push_back( canchor );

    canchor = Anchor();
}


// expect "<[/]value>"
bool NCRichText::PadTOKEN( const wchar_t * sch, const wchar_t *& ech )
{
    // "<[/]value>"
    if ( *sch++ != L'<' || *( ech - 1 ) != L'>' )
	return false;

    // "[/]value>"
    bool endtag = ( *sch == L'/' );

    if ( endtag )
	++sch;

    // "value>"
    if ( ech - sch <= 1 )
	return false;

    std::wstring value( sch, ech - 1 - sch );

    std::wstring args;

    std::wstring::size_type argstart = value.find_first_of( L" \t\n" );

    if ( argstart != std::wstring::npos )
    {
	args = value.substr( argstart );
	value.erase( argstart );
    }

    for ( unsigned i = 0; i < value.length(); ++i )
    {
	if ( isupper( value[i] ) )
	{
	    value[i] = static_cast<char>( tolower( value[i] ) );
	}
    }

    int leveltag = 0;

    int headinglevel = 0;

    TOKEN token = T_UNKNOWN;

    switch ( value.length() )
    {
	case 1:

	    if      ( value[0] == 'b' )		token = T_BOLD;
	    else if ( value[0] == 'i' )		token = T_IT;
	    else if ( value[0] == 'p' )		token = T_PAR;
	    else if ( value[0] == 'a' )		token = T_ANC;
	    else if ( value[0] == 'u' )		token = T_BOLD;

	    break;

	case 2:
	    if      ( value == L"br" )		token = T_BR;
	    else if ( value == L"em" )		token = T_IT;
	    else if ( value == L"h1" )		{ token = T_HEAD; headinglevel = 1; }
	    else if ( value == L"h2" )		{ token = T_HEAD; headinglevel = 2; }
	    else if ( value == L"h3" )		{ token = T_HEAD; headinglevel = 3; }
	    else if ( value == L"hr" )		token = T_IGNORE;
	    else if ( value == L"li" )		token = T_LI;
	    else if ( value == L"ol" )		{ token = T_LEVEL; leveltag = 1; }
	    else if ( value == L"qt" )		token = T_IGNORE;
	    else if ( value == L"tt" )		token = T_TT;
	    else if ( value == L"ul" )		{ token = T_LEVEL; leveltag = 0; }

	    break;

	case 3:

	    if      ( value == L"big" )		token = T_IGNORE;
	    else if ( value == L"pre" )		token = T_PLAIN;
            // <br> and <hr> are the only non-pair tags currently supported.
            // We treat bellow these two special cases in order to work as
            // users expect. This issue was described at
            // https://github.com/libyui/libyui-ncurses/issues/33
            else if ( value == L"br/" )		token = T_BR;
	    else if ( value == L"hr/" )		token = T_IGNORE;

	    break;

	case 4:
	    if      ( value == L"bold" )	token = T_BOLD;
	    else if ( value == L"code" )	token = T_TT;
	    else if ( value == L"font" )	token = T_IGNORE;

	    break;

	case 5:
	    if      ( value == L"large" )	token = T_IGNORE;
	    else if ( value == L"small" )	token = T_IGNORE;

	    break;

	case 6:
	    if      ( value == L"center" )	token = T_PAR;
	    else if ( value == L"strong" )	token = T_BOLD;

	    break;

	case 10:
	    if ( value == L"blockquote" )	token = T_PAR;

	    break;

	default:
	    token = T_UNKNOWN;

	    break;
    }

    if ( token == T_UNKNOWN )
    {
	yuiDebug() << "T_UNKNOWN :" << value << ":" << args << ":" << std::endl;
	// see bug #67319
        //  return false;
	return true;
    }

    if ( token == T_IGNORE )
	return true;

    switch ( token )
    {
	case T_LEVEL:
	    PadChangeLevel( endtag, leveltag );
	    PadBOL();
	    // add new line after end of the list
            // (only at the very end)
	    if ( endtag && !cindent )
		PadNL();

	    break;

	case T_BR:
	    PadNL();

	    break;

	case T_HEAD:
	    if ( endtag )
		Tattr &= ~token;
	    else
		Tattr |= token;

	    PadSetAttr();
	    PadBOL();

	    if ( headinglevel && endtag )
		PadNL();

	    break;

	case T_PAR:
	    PadBOL();

	    if ( !cindent )
	    {
		if ( endtag )
		    // add new line after closing tag (FaTE 3124)
		    PadNL();
	    }

	    break;

	case T_LI:
	    PadSetLevel();
	    PadBOL();

	    if ( !endtag )
	    {
		std::wstring tag;

		if ( liststack.empty() )
		{
		    tag = std::wstring( listindent, L' ' );
		}
		else
		{
		    wchar_t buf[16];

		    if ( liststack.top() )
		    {
			swprintf( buf, 15, L"%2ld. ", liststack.top()++ );
		    }
		    else
		    {
			swprintf( buf, 15, L" %lc  ", listleveltags[liststack.size()%listleveltags.size()] );
		    }

		    tag = buf;
		}

		// outsent list tag:
		cc = ( tag.size() < cc ? cc - tag.size() : 0 );

		myPad()->move( cl, cc );

		PadTXT( tag.c_str(), tag.size() );

		atbol = true;
	    }

	    break;

	case T_PLAIN:

	    if ( !endtag )
	    {
		preTag = true;	// display text preserving newlines and spaces
		AdjustPrePad( ech );
	    }
	    else
	    {
		preTag = false;
		PadNL();	 // add new line (text may continue after </pre>)
	    }

	    break;

	case T_ANC:

	    if ( endtag )
	    {
		closeAnchor();
	    }
	    else
	    {
		openAnchor( args );
	    }

	    // fall through

	case T_BOLD:
	case T_IT:
	case T_TT:
	    if ( endtag )
		Tattr &= ~token;
	    else
		Tattr |= token;

	    PadSetAttr();

	    break;

	case T_IGNORE:
	case T_UNKNOWN:
	    break;
    }

    return true;
}


void NCRichText::arm( unsigned i )
{
    if ( !myPad() )
    {
	armed = i;
	return;
    }

    // yuiDebug() << i << " (" << armed << ")" << std::endl;

    if ( i == armed )
    {
	if ( armed != Anchor::unset )
	{
	    // just redraw
	    anchors[armed].draw( *myPad(), wStyle().richtext.getArmed( GetState() ), 0 );
	    myPad()->update();
	}

	return;
    }

    if ( armed != Anchor::unset )
    {
	anchors[armed].draw( *myPad(), wStyle().richtext.link, ( int ) wStyle().richtext.visitedlink );
	armed = Anchor::unset;
    }

    if ( i != Anchor::unset )
    {
	armed = i;
	anchors[armed].draw( *myPad(), wStyle().richtext.getArmed( GetState() ), 0 );
    }

    if ( showLinkTarget )
    {
	if ( armed != Anchor::unset )
	    NCPadWidget::setLabel( NCstring( anchors[armed].target ) );
	else
	    NCPadWidget::setLabel( NCstring() );
    }
    else
    {
	myPad()->update();
    }
}


void NCRichText::HScroll( unsigned total, unsigned visible, unsigned start )
{
    NCPadWidget::HScroll( total, visible, start );
    // no hyperlink handling needed, because Richtext does not HScroll
}


void NCRichText::VScroll( unsigned total, unsigned visible, unsigned start )
{
    NCPadWidget::VScroll( total, visible, start );

    if ( plainText || anchors.empty() )
	return; // <-- no links to check

    // Take care of hyperlinks: Check whether an armed link is visible.
    // If not arm the first visible link on page or none.
    vScrollFirstvisible	 = start;

    vScrollNextinvisible = start + visible;

    if ( armed != Anchor::unset )
    {
	if ( anchors[armed].within( vScrollFirstvisible, vScrollNextinvisible ) )
	    return; // <-- armed link is vissble
	else
	    disarm();
    }

    for ( unsigned i = 0; i < anchors.size(); ++i )
    {
	if ( anchors[i].within( vScrollFirstvisible, vScrollNextinvisible ) )
	{
	    arm( i );
	    break;
	}
    }
}


bool NCRichText::handleInput( wint_t key )
{
    if ( plainText || anchors.empty() )
    {
	return NCPadWidget::handleInput( key );
    }

    // take care of hyperlinks
    bool handled = true;

    switch ( key )
    {
	case KEY_LEFT:
	    // jump to previous link; scroll up if none
	    {
		unsigned newarmed = Anchor::unset;

		if ( armed == Anchor::unset )
		{
		    // look for an anchor above current page
		    for ( unsigned i = anchors.size(); i; )
		    {
			--i;

			if ( anchors[i].eline < vScrollFirstvisible )
			{
			    newarmed = i;
			    break;
			}
		    }
		}
		else if ( armed > 0 )
		{
		    newarmed = armed - 1;
		}

		if ( newarmed == Anchor::unset )
		{
		    handled = NCPadWidget::handleInput( KEY_UP );
		}
		else
		{
		    if ( !anchors[newarmed].within( vScrollFirstvisible, vScrollNextinvisible ) )
			myPad()->ScrlLine( anchors[newarmed].sline );

		    arm( newarmed );
		}
	    }

	    break;

	case KEY_RIGHT:
	    // jump to next link; scroll down if none
	    {
		unsigned newarmed = Anchor::unset;

		if ( armed == Anchor::unset )
		{
		    // look for an anchor below current page
		    for ( unsigned i = 0; i < anchors.size(); ++i )
		    {
			if ( anchors[i].sline >= vScrollNextinvisible )
			{
			    newarmed = i;
			    break;
			}
		    }
		}
		else if ( armed + 1 < anchors.size() )
		{
		    newarmed = armed + 1;
		}

		if ( newarmed == Anchor::unset )
		{
		    handled = NCPadWidget::handleInput( KEY_DOWN );
		}
		else
		{
		    if ( !anchors[newarmed].within( vScrollFirstvisible, vScrollNextinvisible ) )
			myPad()->ScrlLine( anchors[newarmed].sline );

		    arm( newarmed );
		}
	    }

	    break;

	case KEY_UP:
	    // arm previous visible link; scroll up if none

	    if ( armed != Anchor::unset
		 && armed > 0
		 && anchors[armed-1].within( vScrollFirstvisible, vScrollNextinvisible ) )
	    {
		arm( armed - 1 );
	    }
	    else
	    {
		handled = NCPadWidget::handleInput( key );
	    }

	    break;

	case KEY_DOWN:
	    // arm next visible link; scroll down if none

	    if ( armed != Anchor::unset
		 && armed + 1 < anchors.size()
		 && anchors[armed+1].within( vScrollFirstvisible, vScrollNextinvisible ) )
	    {
		arm( armed + 1 );
	    }
	    else
	    {
		handled = NCPadWidget::handleInput( key );
	    }

	    break;

	default:
	    handled = NCPadWidget::handleInput( key );
    };

    return handled;
}


std::string NCRichText::vScrollValue() const
{
    const NCPad* mypad = myPad();

    if ( !mypad )
	return "";

    return std::to_string( mypad->CurPos().L );
}


void NCRichText::setVScrollValue( const std::string & newValue )
{
    NCPad* mypad = myPad();

    if ( !mypad || newValue.empty() )
	return;

    if ( newValue == "minimum" )
	mypad->ScrlLine( 0 );
    else if ( newValue == "maximum" )
	mypad->ScrlLine( mypad->maxy() );
    else
    {
	try
	{
	    mypad->ScrlLine( std::stoi( newValue ) );
	}
	catch (...)
	{
	    yuiError() << "failed to set vertical scroll value '" << newValue << "'" << endl;
	}
    }
}


std::string NCRichText::hScrollValue() const
{
    const NCPad* mypad = myPad();

    if ( !mypad )
	return "";

    return std::to_string( mypad->CurPos().C );
}


void NCRichText::setHScrollValue( const std::string & newValue )
{
    NCPad* mypad = myPad();

    if ( !mypad || newValue.empty() )
	return;

    if ( newValue == "minimum" )
	mypad->ScrlCol( 0 );
    else if ( newValue == "maximum" )
	mypad->ScrlCol( mypad->maxx() );
    else
    {
	try
	{
	    mypad->ScrlCol( std::stoi( newValue ) );
	}
	catch (...)
	{
	    yuiError() << "failed to set horizontal scroll value '" << newValue << "'" << endl;
	}
    }
}
