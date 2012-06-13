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

   File:       NCurses.cc

   Author:     Michael Andres <ma@suse.de>

/-*/

#include <unistd.h>
#include <string.h>	// strcmp(), strerror()

#include <cstdarg>
#include <fstream>
#include <list>
#include <set>

using std::ostream;
using std::list;
using std::set;

#include <Libyui_config.h>

#define  YUILogComponent "ncurses"
#include <YUILog.h>
#include "NCurses.h"
#include "NCDialog.h"
#include "NCi18n.h"

#include "stdutil.h"
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fnmatch.h>


using stdutil::vform;
using stdutil::form;

/*
  Textdomain "ncurses"
 */

NCurses * NCurses::myself = 0;
set<NCDialog*> NCurses::_knownDlgs;
const NCursesEvent NCursesEvent::Activated( NCursesEvent::button, YEvent::Activated );
const NCursesEvent NCursesEvent::SelectionChanged( NCursesEvent::button, YEvent::SelectionChanged );
const NCursesEvent NCursesEvent::ValueChanged( NCursesEvent::button, YEvent::ValueChanged );




#define CONVERR(n,p) \
    va_list ap;        \
    va_list ap1;       \
    va_start( ap, p ); \
    va_start( ap1, p );\
    errval_i = n;      \
    errmsg_t = vform( p, ap, ap1 ); \
    va_end( ap );      \
    va_end( ap1 )

NCursesError::NCursesError( const char * msg, ... )
	: errval_i( ERR )
{
    CONVERR( ERR, msg );
}

NCursesError::NCursesError( int val, const char * msg, ... )
	: errval_i( val )
{
    CONVERR( val, msg );
}

NCursesError & NCursesError::NCError( const char * msg, ... )
{
    CONVERR( ERR, msg );
    return *this;
}

NCursesError & NCursesError::NCError( int val, const char * msg, ... )
{
    CONVERR( val, msg );
    return *this;
}

#undef CONVERR


ostream & operator<<( ostream & STREAM, const NCursesError & OBJ )
{
    STREAM << form( "%s: (%d) %s"
		    , OBJ.location()
		    , OBJ.errval_i
		    , OBJ.errmsg_t.c_str() );
    return STREAM;
}


ostream & operator<<( ostream & STREAM, const NCursesEvent & OBJ )
{
#define ENUM_OUT(v) case NCursesEvent::v: return STREAM << "Ev::" << #v

    switch ( OBJ.type )
    {
	ENUM_OUT( none );
	ENUM_OUT( handled );
	ENUM_OUT( cancel );
	ENUM_OUT( button );
	ENUM_OUT( menu );
	ENUM_OUT( timeout );
	ENUM_OUT( key );
    }

#undef ENUM_OUT
    return STREAM << "Ev::unknown";
}



NCurses::NCurses()
	: theTerm( 0 )
	, title_w( 0 )
	, status_w( 0 )
	, styleset( 0 )
	, stdpan( 0 )
{
    const char * term = getenv( "TERM" );

    if ( term && *term )
	envTerm = term;
}



NCurses::~NCurses()
{
    yuiMilestone() << "Shutdown NCurses..." << std::endl;
    myself = 0;

    //restore env. variable - might have been changed by NCurses::init()
    setenv( "TERM", envTerm.c_str(), 1 );
    delete styleset;
    delete stdpan;

    if ( title_w )
	::delwin( title_w );

    if ( status_w )
	::delwin( status_w );

    ::endwin();

    if ( theTerm )
	::delscreen( theTerm );

    yuiMilestone() << "NCurses down" << std::endl;
}



WINDOW * NCurses::ripped_w_top = 0;
WINDOW * NCurses::ripped_w_bottom = 0;

int NCurses::ripinit_top( WINDOW * w, int c )
{
    ripped_w_top = w;
    return OK;
}


int NCurses::ripinit_bottom( WINDOW * w, int c )
{
    ripped_w_bottom = w;
    return OK;
}


void NCurses::init()
{
    yuiMilestone() << "Launch NCurses..."
#ifdef VERSION
    << "(ui-ncurses-" << VERSION << ")"
#endif
    << std::endl;
    yuiMilestone() << "TERM=" << envTerm << std::endl;

    signal( SIGINT, SIG_IGN );	// ignore Ctrl C

    //rip off the top line

    if ( title_line() && ::ripoffline( 1, ripinit_top ) != OK )
	throw NCursesError( "ripoffline() failed" );

    //and bottom line (-1 means 1st from the bottom)
    if ( ::ripoffline( -1, ripinit_bottom ) != OK )
	throw NCursesError( "ripoffline() failed" );

    yuiMilestone() << "isatty(stdin)" << ( isatty( 0 ) ? "yes" : "no" ) << std::endl;

    if ( isatty( 0 ) )
    {
	char * mytty = ttyname( 0 );

	if ( mytty )
	{
	    yuiMilestone() << "mytty: " << mytty << std::endl;
	    FILE * fdi = fopen( mytty, "r" );

	    if ( !fdi )
	    {
		yuiError() << "fdi: (" << errno << ") " << strerror( errno ) << std::endl;
	    }

	    FILE * fdo = fopen( mytty, "w" );

	    if ( !fdo )
	    {
		yuiError() << "fdo: (" << errno << ") " << strerror( errno ) << std::endl;
	    }

	    if ( fdi && fdo )
	    {
		theTerm = newterm( 0, fdo, fdi );

		//initialization failed

		if ( theTerm == NULL )
		{
		    //bug #235954: workaround missing terminfos in inst-sys
		    //let's close the first term
		    ::endwin();

		    string fallbackTerm = "";
		    //try generic xterm for xterm-like terminals, otherwise use vt100

		    if ( ! fnmatch( "xterm*", envTerm.c_str(), 0 ) )
			fallbackTerm = "xterm";
		    else
			fallbackTerm = "vt100";

		    yuiWarning() << "newterm() failed, using generic " << fallbackTerm << " as a fallback" << std::endl;

		    //overwrite environment variable
		    setenv( "TERM", fallbackTerm.c_str(), 1 );

		    //.. and try again
		    theTerm = newterm( 0, fdo, fdi );

		    if ( theTerm == NULL )
			throw NCursesError( "fallback newterm() failed" );
		}

		if ( set_term( theTerm ) == NULL )
		    throw NCursesError( "set_term() failed" );

		myTerm = mytty;
	    }
	}
    }

    //duplicate stdout and stderr before redirecting them to log
    //so that they can be regenerated before system() call
    stdout_save = dup( 1 );
    stderr_save = dup( 2 );

    RedirectToLog();

    if ( !theTerm )
    {
	yuiMilestone() << "no term so fall back to initscr" << std::endl;

	if ( ::initscr() == NULL )
	    throw NCursesError( "initscr() failed" );
    }

    yuiMilestone() << "have color = " << ::has_colors()  << std::endl;

    if ( want_colors() && ::has_colors() )
    {
	if ( ::start_color() != OK )
	    throw NCursesError( "start_color() failed" );

	NCattribute::init_colors();
    }

    if ( title_line() )
    {
	if ( !ripped_w_top )
	    throw NCursesError( "ripinit_top() failed" );

	title_w = ripped_w_top;
    }

    if ( !ripped_w_bottom )
	throw NCursesError( "ripinit_bottom() failed" );

    status_w = ripped_w_bottom;

    setup_screen();

    yuiMilestone() << form( "screen size %d x %d\n", lines(), cols() );

    myself = this;
    styleset = new NCstyle( envTerm );
    stdpan   = new NCursesPanel();
    stdpan->bkgd( style()( NCstyle::AppText ) );

    if ( title_line() )
	init_title();

    init_screen();
    yuiMilestone() << "NCurses ready" << std::endl;
}



void NCurses::setup_screen()
{
    ::cbreak();
    ::noecho();
    ::keypad( ::stdscr, true );
    ::meta( ::stdscr, true );
    ::leaveok( ::stdscr, true );
    ::curs_set( 0 );

    ::define_key( "\e[Z",   KEY_BTAB );
    ::define_key( "\e\t",   KEY_BTAB );
    ::define_key( "\030\t", KEY_BTAB );
}



void NCurses::init_title()
{
    ::wbkgd( title_w, style()( NCstyle::AppTitle ) );
    ::wnoutrefresh( title_w );
    ::wbkgd( status_w, style()( NCstyle::AppTitle ) );
    ::wnoutrefresh( status_w );
}



void NCurses::init_screen()
{
    bool redefine = false;

    char *value = getenv( "Y2NCPSEUDO" );

    // The 9.0 workaround for missing ACS chars (bug #30512) is not necessary
    // any longer (a patch is provided for ncurses-5.4).

    // Redefine ACS chars if Y2NCPSEUDO is set to "1" (just in case of ...)

    if ( value != NULL && strcmp( value, "1" ) == 0 )
    {
	redefine = true;
    }

    if ( redefine )
    {
	chtype cch = 0;

	NCattribute::setChar( cch, '+' );
	ACS_ULCORNER = cch;
	ACS_LLCORNER = cch;
	ACS_URCORNER = cch;
	ACS_LRCORNER = cch;
	ACS_LTEE = cch;
	ACS_RTEE = cch;
	ACS_BTEE = cch;
	ACS_TTEE = cch;
	ACS_PLUS = cch;

	NCattribute::setChar( cch, '|' );
	ACS_VLINE = cch;

	NCattribute::setChar( cch, '-' );
	ACS_HLINE = cch;

	NCattribute::setChar( cch, '#' );
	ACS_DIAMOND = cch;
	ACS_CKBOARD = cch;
	ACS_BOARD = cch;

	NCattribute::setChar( cch, '<' );
	ACS_LARROW = cch;

	NCattribute::setChar( cch, '>' );
	ACS_RARROW = cch;

	NCattribute::setChar( cch, 'v' );
	ACS_DARROW = cch;

	NCattribute::setChar( cch, '^' );
	ACS_UARROW = cch;
    }
}


const NCstyle & NCurses::style()
{
    return *myself->styleset;
}


void NCurses::Update()
{
    if ( myself && myself->initialized() )
    {
	//myself->stdpan->refresh();
	myself->stdpan->redraw();
    }
}


void NCurses::Refresh()
{
    if ( myself && myself->initialized() )
    {
	yuiMilestone() << "start refresh ..." << std::endl;
	SetTitle( myself->title_t );
	SetStatusLine( myself->status_line );
	::clearok( ::stdscr, true );
	myself->stdpan->refresh();
	yuiMilestone() << "done refresh ..." << std::endl;
    }
}


void NCurses::Redraw()
{
    if ( myself && myself->initialized() )
    {
	yuiMilestone() << "start redraw ..." << std::endl;

	// initialize all dialogs rewdraw
	PANEL * pan = ::panel_above( NULL );

	while ( pan )
	{
	    NCDialog * dlg = NCursesUserPanel<NCDialog>::UserDataOf( *pan );

	    if ( dlg )
	    {
		dlg->Recoded();
	    }

	    pan = ::panel_above( pan );
	}

	// TBD: initialize all dialogs rewdraw
	Refresh();

	yuiMilestone() << "done redraw ..." << std::endl;
    }
}


void NCurses::SetTitle( const string & str )
{
    if ( myself && myself->title_w )
    {
	myself->title_t = str;
	::wbkgd( myself->title_w, myself->style()( NCstyle::AppTitle ) );
	::wclear( myself->title_w );

	yuiMilestone() << "Draw title called" << std::endl;

#if 0
	setTextdomain( "ncurses" );
	// part of title (headline) of the textmode yast
	NCstring helpF1( _( "Press F1 for Help" ) );
	NCtext textF1( helpF1 );

	int s = myself->title_w->_maxx - textF1.Columns();

	if ( NCstring::terminalEncoding() != "UTF-8" )
	{
	    string out;
	    NCstring::RecodeFromWchar( helpF1.str(), NCstring::terminalEncoding(), &out );
	    ::mvwaddstr( myself->title_w, 0, s, out.c_str() );
	}
	else
	{
	    ::mvwaddwstr( myself->title_w, 0, s, ( wchar_t * )helpF1.str().c_str() );
	}

#endif

	::mvwaddstr( myself->title_w, 0, 1, myself->title_t.c_str() );
	::wnoutrefresh( myself->title_w );
    }

}

void NCurses::SetStatusLine( std::map <int, string> fkeys )
{

    if ( myself && myself->status_w )
    {
	myself->status_line = fkeys;
	::wbkgd( myself->status_w, myself->style()( NCstyle::AppTitle ) );
	::werase( myself->status_w );

	char key[10];
	char value[100];

	std::map<int, string>::iterator it;

	for ( it = fkeys.begin(); it != fkeys.end(); ++it )
	{
	    sprintf( key, " F%d ", ( *it ).first );
	    //reverse F-key to make it more visible
	    ::wattron( myself->status_w, A_REVERSE );
	    ::waddstr( myself->status_w, key );
	    ::wattroff( myself->status_w, A_REVERSE );

	    sprintf( value, "%s ", ( *it ).second.c_str() );
	    ::waddstr( myself->status_w, value );
	}

	::wnoutrefresh( myself->status_w );
    }
}



void NCurses::drawTitle()
{
    if ( myself && myself->title_w )
    {
	SetTitle( myself->title_t );
    }
}



void NCurses::RememberDlg( NCDialog * dlg_r )
{
    if ( dlg_r )
    {
	_knownDlgs.insert( dlg_r );
    }
}



void NCurses::ForgetDlg( NCDialog * dlg_r )
{
    if ( dlg_r )
    {
	_knownDlgs.erase( dlg_r );
    }
}



void NCurses::RedirectToLog()
{
    string log = "/dev/null";	// this used to be get_log_filename()

    yuiMilestone() << "isatty(stderr)" << ( isatty( 2 ) ? "yes" : "no" ) << std::endl;

    if ( isatty( 2 ) && theTerm )
    {
	// redirect stderr to log
	close( 2 );
	open( log.c_str(), O_APPEND | O_CREAT, 0666 );
    }

    yuiMilestone() << "isatty(stdout)" << ( isatty( 1 ) ? "yes" : "no" ) << std::endl;

    if ( isatty( 1 ) && theTerm )
    {
	// redirect stdout to log
	close( 1 );
	open( log.c_str(), O_APPEND | O_CREAT, 0666 );
    }
}



void NCurses::ResizeEvent()
{
    if ( myself && myself->initialized() )
    {
	yuiMilestone() << "start resize to " << NCurses::lines() << 'x' << NCurses::cols() << "..." << std::endl;

	// remember stack of visible dialogs.
	// don't hide on the fly, as it will mess up stacking order.
	list<NCDialog*> dlgStack;

	for ( PANEL * pan = ::panel_above( NULL ); pan; pan = ::panel_above( pan ) )
	{
	    NCDialog * dlg = NCursesUserPanel<NCDialog>::UserDataOf( *pan );

	    if ( dlg )
	    {
		dlgStack.push_back( dlg );
	    }
	}

	// hide all visible dialogs.
	for ( list<NCDialog*>::iterator it = dlgStack.begin(); it != dlgStack.end(); ++it )
	{
	    ( *it )->getInvisible();
	}

	drawTitle();
	Update();

	// relayout all dialogs

	for ( set<NCDialog*>::iterator it = _knownDlgs.begin(); it != _knownDlgs.end(); ++it )
	{
	    ( *it )->resizeEvent();
	}

	// recreate stack of visible dialogs
	for ( list<NCDialog*>::iterator it = dlgStack.begin(); it != dlgStack.end(); ++it )
	{
	    ( *it )->getVisible();
	}

	Update();

	//FIXME: remove this once libncurses is upgraded to 20080105 patchlevel
	//after the resize, status line window needs to be moved to the new pos.
	::mvwin( myself->status_w, NCurses::lines(), 0 );
	SetStatusLine( myself->status_line );
	//update the screen
	::touchwin( myself->status_w );
	::doupdate();

	yuiMilestone() << "done resize ..." << std::endl;
    }
}



void NCurses::ScreenShot( const string & name )
{
    if ( !myself )
	return;

    //ofstream out( name.c_str(), ios::out|ios::app );
    ostream & out( yuiMilestone() );

    int curscrlines = myself->title_line() ? lines() + 1 : lines();

    for ( int l = 0; l < curscrlines; ++l )
    {
	for ( int c = 0; c < cols(); ++c )
	{

	    chtype al = ::mvwinch( ::curscr, l, c ) & ( A_ALTCHARSET | A_CHARTEXT );

	    if ( al & A_ALTCHARSET )
	    {
		if ( al == ACS_ULCORNER
		     || al == ACS_LLCORNER
		     || al == ACS_URCORNER
		     || al == ACS_LRCORNER
		     || al == ACS_LTEE
		     || al == ACS_RTEE
		     || al == ACS_BTEE
		     || al == ACS_TTEE
		     || al == ACS_PLUS )
		{
		    out << '+';
		}
		else if ( al == ACS_HLINE )
		{
		    out << '-';
		}
		else if ( al == ACS_VLINE )
		{
		    out << '|';
		}
		else if ( al == ACS_DIAMOND
			  || al == ACS_CKBOARD
			  || al == ACS_BOARD )
		{
		    out << '#';
		}
		else if ( al == ACS_LARROW )
		{
		    out << '<';
		}
		else if ( al == ACS_RARROW )
		{
		    out << '>';
		}
		else if ( al == ACS_DARROW )
		{
		    out << 'v';
		}
		else if ( al == ACS_UARROW )
		{
		    out << '^';
		}
		else
		{
		    out << ( char )( al & A_CHARTEXT );
		}
	    }
	    else
	    {
		out << ( char )( al & A_CHARTEXT );
	    }

	}

	out << std::endl;
    }
}


ostream & operator<<( ostream & STREAM, const NCurses & OBJ )
{
    STREAM << form( "NC - %d x %d - colors %d - pairs %d\n"
		    , OBJ.lines(), OBJ.cols()
		    , NCattribute::colors(), NCattribute::color_pairs() );

    WINDOW * cw = ::stdscr;
    STREAM << form( "NC - rootw %p", cw );

    if ( cw )
	STREAM << form( " - (%2hd,%2hd)%2hdx%2hd - {%p - (%2d,%2d)}\n"
			, cw->_begy, cw->_begx
			, cw->_maxy, cw->_maxx
			, cw->_parent
			, cw->_pary, cw->_parx
		      );
    else
	STREAM << std::endl;

    cw = OBJ.title_w;

    STREAM << form( "NC - title %p", cw );

    if ( cw )
	STREAM << form( " - (%2hd,%2hd)%2hdx%2hd - {%p - (%2d,%2d)}\n"
			, cw->_begy, cw->_begx
			, cw->_maxy, cw->_maxx
			, cw->_parent
			, cw->_pary, cw->_parx
		      );
    else
	STREAM << std::endl;

    return STREAM;
}
