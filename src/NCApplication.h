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

  File:	      NCApplication.h

  Author:     Gabriele Mohr <gs@suse.de>

/-*/

#ifndef NCApplication_h
#define NCApplication_h

#include "YApplication.h"


class NCApplication: public YApplication
{

protected:

    friend class YNCursesUI;

    /**
     * Constructor.
     *
     * Use YUI::app() to get the singleton for this class.
     **/
    NCApplication();

    /**
     * Destructor.
     **/
    virtual ~NCApplication();

public:

    /**
     * Set language and encoding for the locale environment ($LANG).
     *
     * 'language' is the ISO short code ("de_DE", "en_US", ...).
     *
     * 'encoding' an (optional) encoding ("utf8", ...) that will be appended if
     *	present.
     *
     * Reimplemented from YApplication.
     **/
    virtual void setLanguage( const string & language,
			      const string & encoding = string() );

    /**
     * Open a directory selection box and prompt the user for an existing
     * directory.
     *
     * 'startDir' is the initial directory that is displayed.
     *
     * 'headline' is an explanatory text for the directory selection box.
     * Graphical UIs may omit that if no window manager is running.
     *
     * Returns the selected directory name
     * or an empty string if the user canceled the operation.
     *
     * Implemented from YApplication.
     **/
    virtual string askForExistingDirectory( const string & startDir,
					    const string & headline );

    /**
     * Open a file selection box and prompt the user for an existing file.
     *
     * 'startWith' is the initial directory or file.
     *
     * 'filter' is one or more blank-separated file patterns, e.g.
     * "*.png *.jpg"
     *
     * 'headline' is an explanatory text for the file selection box.
     * Graphical UIs may omit that if no window manager is running.
     *
     * Returns the selected file name
     * or an empty string if the user canceled the operation.
     *
     * Implemented from YApplication.
     **/
    virtual string askForExistingFile( const string & startWith,
				       const string & filter,
				       const string & headline );

    /**
     * Open a file selection box and prompt the user for a file to save data
     * to.  Automatically asks for confirmation if the user selects an existing
     * file.
     *
     * 'startWith' is the initial directory or file.
     *
     * 'filter' is one or more blank-separated file patterns, e.g.
     * "*.png *.jpg"
     *
     * 'headline' is an explanatory text for the file selection box.
     * Graphical UIs may omit that if no window manager is running.
     *
     * Returns the selected file name
     * or an empty string if the user canceled the operation.
     *
     * Implemented from YApplication.
     **/
    virtual string askForSaveFileName( const string & startWith,
				       const string & filter,
				       const string & headline );

    /**
     * Beep.
     *
     * Reimplemented from YApplication.
     **/
    virtual void beep();

    /**
     * Redraw the screen.
     *
     * Reimplemented from YApplication.
     **/
    virtual void redrawScreen();

    /**
     * Initialize the (text) console keyboard.
     *
     * Reimplemented from YApplication.
     **/
    virtual void initConsoleKeyboard();

    /**
     * Set the (text) console font according to the current encoding etc.
     * See the setfont(8) command and the console HowTo for details.
     *
     * Reimplemented from YApplication.
     **/
    virtual void setConsoleFont( const string & console_magic,
				 const string & font,
				 const string & screen_map,
				 const string & unicode_map,
				 const string & language );

    /**
     * Run a shell command (typically an interactive program using NCurses)
     * in a terminal (window).
     *
     * Here in the NCurses UI, this shuts down the NCurses lib, runs the
     * command and then restores the status of the NCurses lib so that the next
     * instance of the NCurses lib from the started command doesn't interfere
     * with the NCurses UI's instance.
     *
     * Reimplemented from YApplication.
     **/
    virtual int runInTerminal( const string & command );


    // Display information and UI capabilities.
    //
    // All implemented from YApplication.

    virtual int	 displayWidth();
    virtual int	 displayHeight();
    virtual int	 displayDepth();
    virtual long displayColors();

    virtual int	 defaultWidth();
    virtual int	 defaultHeight();

    virtual bool isTextMode()			{ return true; }

    virtual bool hasImageSupport()		{ return false; }

    virtual bool hasIconSupport()		{ return false; }

    virtual bool hasAnimationSupport()		{ return false; }

    virtual bool hasFullUtf8Support();
    virtual bool richTextSupportsTable()	{ return false; }

    virtual bool leftHandedMouse()		{ return false; }
};


#endif // NCApplication_h
