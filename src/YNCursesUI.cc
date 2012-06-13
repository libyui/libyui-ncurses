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

   File:       YNCursesUI.cc

   Author:     Michael Andres <ma@suse.de>

/-*/

#include "YNCursesUI.h"
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <langinfo.h>

#include <YUI.h>
#include <YEvent.h>
#include <YDialog.h>
#include <YCommandLine.h>
#include <YButtonBox.h>
#include <YMacro.h>

#define YUILogComponent "ncurses"
#include <yui/YUILog.h>


#define	 YUILogComponent "ncurses"
#include <YUILog.h>
#include "NCstring.h"
#include "NCWidgetFactory.h"
#include "NCOptionalWidgetFactory.h"
#include "NCPackageSelectorPluginStub.h"

extern string language2encoding( string lang );

YNCursesUI * YNCursesUI::_ui = 0;


YUI * createUI( bool withThreads )
{
    if ( ! YNCursesUI::ui() )
	new YNCursesUI( withThreads );

    return YNCursesUI::ui();
}


YNCursesUI::YNCursesUI( bool withThreads )
	: YUI( withThreads )
{
    yuiMilestone() << "Start YNCursesUI" << std::endl;
    _ui = this;

    if ( getenv( "LANG" ) != NULL )
    {
	string language = getenv( "LANG" );
	string encoding =  nl_langinfo( CODESET );

	// setlocale ( LC_ALL, "" ) is called in WMFInterpreter::WFMInterpreter;

	// Explicitly set LC_CTYPE so that it won't be changed if setenv( LANG ) is called elsewhere.
	// (it's not enough to call setlocale( LC_CTYPE, .. ), set env. variable LC_CTYPE!)

	string locale = setlocale( LC_CTYPE, NULL );
	setenv( "LC_CTYPE", locale.c_str(), 1 );

	yuiMilestone() << "setenv LC_CTYPE: " << locale << " encoding: " << encoding << std::endl;

	// The encoding of a terminal (xterm, konsole etc.) can never change; the encoding
	// of the "real" console is changed in setConsoleFont().
	NCstring::setTerminalEncoding( encoding );

	app()->setLanguage( language, encoding );
    }

    YButtonBoxMargins buttonBoxMargins;
    buttonBoxMargins.left    = 1;
    buttonBoxMargins.right   = 1;
    buttonBoxMargins.top     = 1;
    buttonBoxMargins.bottom  = 0;
    buttonBoxMargins.spacing = 1;
    buttonBoxMargins.helpButtonExtraSpacing = 3;
    YButtonBox::setDefaultMargins( buttonBoxMargins );

    try
    {
	NCurses::init();
    }
    catch ( NCursesError & err )
    {
	yuiMilestone() << err << std::endl;
	::endwin();
	abort();
    }

    topmostConstructorHasFinished();
}


YNCursesUI::~YNCursesUI()
{
    //delete left-over dialogs (if any)
    YDialog::deleteAllDialogs();
    yuiMilestone() << "Stop YNCursesUI" << std::endl;
}


YWidgetFactory *
YNCursesUI::createWidgetFactory()
{
    NCWidgetFactory * factory = new NCWidgetFactory();
    YUI_CHECK_NEW( factory );

    return factory;
}


YOptionalWidgetFactory *
YNCursesUI::createOptionalWidgetFactory()
{
    NCOptionalWidgetFactory * factory = new NCOptionalWidgetFactory();
    YUI_CHECK_NEW( factory );

    return factory;
}


YApplication *
YNCursesUI::createApplication()
{
    NCApplication * app = new NCApplication();
    YUI_CHECK_NEW( app );

    return app;
}


void YNCursesUI::idleLoop( int fd_ycp )
{

    int	   timeout = 5;

    struct timeval tv;
    fd_set fdset;
    int	   retval;

    do
    {
	tv.tv_sec  = timeout;
	tv.tv_usec = 0;

	FD_ZERO( &fdset );
	FD_SET( 0,	&fdset );
	FD_SET( fd_ycp, &fdset );

	retval = select( fd_ycp + 1, &fdset, 0, 0, &tv );

	if ( retval < 0 )
	{
	    if ( errno != EINTR )
		yuiError() << "idleLoop error in select() (" << errno << ')' << std::endl;
	}
	else if ( retval != 0 )
	{
	    //do not throw here, as current dialog may not necessarily exist yet
	    //if we have threads
	    YDialog *currentDialog = YDialog::currentDialog( false );

	    if ( currentDialog )
	    {
		NCDialog * ncd = static_cast<NCDialog *>( currentDialog );

		if ( ncd )
		{
		    extern NCBusyIndicator* NCBusyIndicatorObject;

		    if ( NCBusyIndicatorObject )
			NCBusyIndicatorObject->handler( 0 );

		    ncd->idleInput();
		}
	    }
	} // else no input within timeout sec.
    }
    while ( !FD_ISSET( fd_ycp, &fdset ) );
}


/**
 * Create the package selector plugin
 **/
NCPackageSelectorPluginStub * YNCursesUI::packageSelectorPlugin()
{
    static NCPackageSelectorPluginStub * plugin = 0;

    if ( ! plugin )
    {
	plugin = new NCPackageSelectorPluginStub();

	// This is a deliberate memory leak: If an application requires a
	// PackageSelector, it is a package selection application by
	// definition. In this case, the ncurses_pkg plugin is intentionally
	// kept open to avoid repeated start-up cost of the plugin and libzypp.
    }

    return plugin;
}


YEvent * YNCursesUI::runPkgSelection( YWidget * selector )
{
    YEvent * event = 0;

    YDialog *dialog = YDialog::currentDialog();
    NCPackageSelectorPluginStub * plugin = packageSelectorPlugin();

    if ( !dialog )
    {
	yuiError() << "ERROR package selection: No dialog rexisting." << std::endl;
	return 0;
    }

    if ( !selector )
    {
	yuiError() << "ERROR package selection: No package selector existing." << std::endl;
	return 0;
    }

    // debug: dump the widget tree
    dialog->dumpDialogWidgetTree();

    if ( plugin )
    {
	event = plugin->runPkgSelection( dialog, selector );
    }

    return event;
}


void YNCursesUI::init_title()
{
    // Fetch command line args
    YCommandLine cmdline;

    //
    // Retrieve program name from command line
    //

    string progName = YUILog::basename( cmdline[0] );

    if ( progName == "y2base" )
    {
	progName = "YaST2";

	// Special case for YaST2: argv[1] is the module name -
	// this is what we want to display in the window title
	//
	// '/usr/lib/whatever/y2base' 'module_name' 'selected_ui'
	// (e.g. 'y2base' 'lan' 'ncurses') -> we need 'lan'

	if ( cmdline.size() > 1 )
	    progName += " - " + cmdline[1];
    }

    if ( progName.find( "lt-" ) == 0 )	// progName starts with "lt-"
    {
	// Remove leading "lt-" from libtool-generated binaries
	progName = progName.substr( sizeof( "lt-" ) - 1 );
    }


    //
    // Retrieve host name (if set)
    //

    string hostName;

    char hostNameBuffer[ 256 ];

    if ( gethostname( hostNameBuffer, sizeof( hostNameBuffer ) - 1 ) != -1 )
    {
	// gethostname() might have messed up - yet another POSIX standard that
	// transfers the burden of doing things right to the application
	// programmer: Possibly no null byte

	hostNameBuffer[ sizeof( hostNameBuffer ) -1 ] = '\0';
	hostName = hostNameBuffer;
    }

    if ( hostName == "(none)" )
	hostName = "";

    //
    // Build and set window title
    //

    string windowTitle = progName;

    if ( ! hostName.empty() )
	windowTitle += " @ " + hostName;

    NCurses::SetTitle( windowTitle );
}


bool YNCursesUI::want_colors()
{
    if ( getenv( "Y2NCURSES_BW" ) != NULL )
    {
	yuiMilestone() << "Y2NCURSES_BW is set - won't use colors" << std::endl;
	return false;
    }

    return true;
}


/**
 * Set the console font, encoding etc.
 * This is called from Console.ycp.
 * The terminal encoding must be set correctly.
 *
 * This doesn't belong here, but it is so utterly entangled with member
 * variables that are not exported at all (sic!) that it's not really feasible
 * to extract the relevant parts.
 **/
void YNCursesUI::setConsoleFont( const string & console_magic,
				 const string & font,
				 const string & screen_map,
				 const string & unicode_map,
				 const string & lang )
{
    string cmd( "setfont" );
    cmd += " -C " + myTerm;
    cmd += " " + font;

    if ( !screen_map.empty() )
	cmd += " -m " + screen_map;

    if ( !unicode_map.empty() )
	cmd += " -u " + unicode_map;

    yuiMilestone() << cmd << std::endl;

    int ret = system(( cmd + " >/dev/null 2>&1" ).c_str() );

    // setfont returns error if called e.g. on a xterm -> return
    if ( ret )
    {
	yuiError() << cmd.c_str() << " returned " << ret << std::endl;
	Refresh();
	return;
    }

    // go on in case of a "real" console
    cmd = "(echo -en \"\\033";

    if ( console_magic.length() )
	cmd += console_magic;
    else
	cmd += "(B";

    cmd += "\" >" + myTerm + ")";

    yuiMilestone() << cmd << std::endl;

    ret = system(( cmd + " >/dev/null 2>&1" ).c_str() );

    if ( ret )
    {
	yuiError() << cmd.c_str() << " returned " << ret << std::endl;
    }

    // set terminal encoding for console
    // (setConsoleFont() in Console.ycp has passed the encoding as last
    // argument but this encoding was not correct; now Console.ycp passes the
    // language) if the encoding is NOT UTF-8 set the console encoding
    // according to the language

    if ( NCstring::terminalEncoding() != "UTF-8" )
    {
	string language = lang;
	string::size_type pos = language.find( '.' );

	if ( pos != string::npos )
	{
	    language.erase( pos );
	}

	pos = language.find( '_' );

	if ( pos != string::npos )
	{
	    language.erase( pos );
	}

	string code = language2encoding( language );

	yuiMilestone() << "setConsoleFont( ENCODING:  " << code << " )" << std::endl;

	if ( NCstring::setTerminalEncoding( code ) )
	{
	    Redraw();
	}
	else
	{
	    Refresh();
	}
    }
    else
    {
	Refresh();
    }
}

