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

  File:		NCApplication.cc

  Authors:	Gabriele Mohr <gs@suse.de>
		Stefan Hundhammer <sh@suse.de>

/-*/

#include <ncursesw/curses.h>

#define  YUILogComponent "ncurses"
#include <YUILog.h>
#include "NCurses.h"
#include "YNCursesUI.h"
#include "NCApplication.h"
#include "NCAskForDirectory.h"
#include "NCAskForFile.h"


NCApplication::NCApplication()
{

}


NCApplication::~NCApplication()
{

}

void
NCApplication::setLanguage( const string & language,
			    const string & encoding )
{
    // Intentionally NOT calling
    //	  YApplication::setLanguage( language, encoding );
    // This would implicitly overwrite LC_CTYPE which might result in encoding bugs.

    setlocale( LC_NUMERIC, "C" );	// always format numbers with "."
    NCurses::Refresh();

    yuiDebug() << "Language: " << language << " Encoding: " << (( encoding != "" ) ? encoding : "NOT SET" ) << std::endl;

}


string
NCApplication::askForSaveFileName( const string & startDir,
				   const string & filter,
				   const string & headline )
{
    NCAskForSaveFileName * filePopup = new NCAskForSaveFileName( wpos( 1, 1 ), startDir, filter, headline );
    YUI_CHECK_NEW( filePopup );

    NCursesEvent retEvent = filePopup->showDirPopup( );
    YDialog::deleteTopmostDialog();

    yuiMilestone() << "Returning: " <<	retEvent.result << std::endl;
    return retEvent.result;
}


string
NCApplication::askForExistingFile( const string & startDir,
				   const string & filter,
				   const string & headline )
{
    NCAskForExistingFile * filePopup = new NCAskForExistingFile( wpos( 1, 1 ), startDir, filter, headline );
    YUI_CHECK_NEW( filePopup );

    NCursesEvent retEvent = filePopup->showDirPopup( );
    YDialog::deleteTopmostDialog();

    yuiMilestone() << "Returning: " <<	retEvent.result << std::endl;
    return retEvent.result;
}


string
NCApplication::askForExistingDirectory( const string & startDir,
					const string & headline )
{
    NCAskForExistingDirectory * dirPopup = new NCAskForExistingDirectory( wpos( 1, 1 ), startDir, headline );
    YUI_CHECK_NEW( dirPopup );

    NCursesEvent retEvent = dirPopup->showDirPopup( );
    YDialog::deleteTopmostDialog();

    yuiMilestone() << "Returning: " <<	retEvent.result << std::endl;
    return retEvent.result;
}


void
NCApplication::beep()
{
    ::beep();
}


void NCApplication::redrawScreen()
{
    YNCursesUI::ui()->Refresh();
}


void
NCApplication::initConsoleKeyboard()
{
    /*
     * Following code breaks the console keyboard e.g. for czech language during
     * installation (bnc #433016). According to bnc #367801 comment #18/#19 the
     * line isn't needed at all.
     * "dumpkeys | loadkeys -C "$KBD_TTY" --unicode" has been also removed from kbd
     * initscript. If dumpkeys has to be called for any reason it definitely needs
     * the codepage argument, otherwise it cannot work.
     */
#if 0
    string cmd = "/bin/dumpkeys | /bin/loadkeys --unicode";

    if ( NCstring::terminalEncoding() == "UTF-8" )
    {
	int ret = system(( cmd + " >/dev/null 2>&1" ).c_str() );

	if ( ret != 0 )
	{
	    yuiError() << "ERROR: /bin/dumpkeys | /bin/loadkeys --unicode returned: " << ret << std::endl;
	}
    }
#endif
}


void
NCApplication::setConsoleFont( const string & console_magic,
			       const string & font,
			       const string & screen_map,
			       const string & unicode_map,
			       const string & language )
{
    /**
     * Moving that code from YNCursesUI to this class turned out to be
     * impossible (or at least a lot more work than it's worth) that I finally
     * gave it up.
     *
     * - sh@suse.de 2008-02-06
     **/
    YNCursesUI::ui()->setConsoleFont( console_magic,
				      font,
				      screen_map,
				      unicode_map,
				      language );
}


int
NCApplication::runInTerminal( const string & cmd )
{
    int ret;

    // Save tty modes and end ncurses mode temporarily
    ::def_prog_mode();
    ::endwin();

    // Regenerate saved stdout and stderr, so that app called
    // via system() can use them and draw something to the terminal
    dup2( YNCursesUI::ui()->stdout_save, 1 );
    dup2( YNCursesUI::ui()->stderr_save, 2 );

    // Call external program
    ret = system( cmd.c_str() );

    if ( ret != 0 )
    {
	yuiError() << cmd << " returned:" << ret << std::endl;
    }

    // Redirect stdout and stderr to y2log again
    YNCursesUI::ui()->RedirectToLog();

    // Resume tty modes and refresh the screen
    ::reset_prog_mode();

    ::refresh();

    return ret;
}


int
NCApplication::displayWidth()
{
    return ::COLS;	// exported from ncurses.h
}


int
NCApplication::displayHeight()
{
    return ::LINES;	// exported from ncurses.h
}


int
NCApplication::displayDepth()
{
    return -1;
}


long
NCApplication::displayColors()
{
    return NCattribute::colors();
}


int
NCApplication::defaultWidth()
{
    return ::COLS;	// exported from ncurses.h
}


int
NCApplication::defaultHeight()
{
    return ::LINES;	// exported from ncurses.h
}


bool
NCApplication::hasFullUtf8Support()
{
    return ( NCstring::terminalEncoding() == "UTF-8" );
}
