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

   File:       NCFileSelection.h

   Author:     Gabriele Strattner <gs@suse.de>

/-*/

#ifndef NCFileSelection_h
#define NCFileSelection_h

#include <iosfwd>

#include "NCPadWidget.h"
#include "NCTablePad.h"
#include "NCTable.h"

#include <map>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/errno.h>


struct NCFileInfo
{
    /**
     * Constructor from a stat buffer (i.e. based on an lstat64() call).
     **/
    NCFileInfo( string	fileName,
		struct stat64	* statInfo,
		bool link	= false );

    NCFileInfo();

    ~NCFileInfo() {};

    // Data members.

    string		_name;		// the file name (without path!)
    string		_realName;	// actual file name
    string		_tag;		// short label
    string		_perm;		// permission string
    string		_user;		// user name
    string		_group;		// group name
    dev_t		_device;	// device this object resides on
    mode_t		_mode;		// file permissions + object type
    nlink_t		_links;		// number of links
    off64_t		_size;		// size in bytes
    time_t		_mtime;		// modification time

    bool isDir()  { return (( S_ISDIR( _mode ) ) ? true : false ); }

    bool isLink() { return (( S_ISLNK( _mode ) ) ? true : false ); }

    bool isFile() { return (( S_ISREG( _mode ) ) ? true : false ); }
};


/**
 * This class is used for the first column of the file table.
 * Contains the file data.
 **/
class NCFileSelectionTag : public YTableCell
{

private:

    NCFileInfo	* fileInfo;

public:

    NCFileSelectionTag( NCFileInfo 	* info );

    ~NCFileSelectionTag();

    NCFileInfo	* getFileInfo() const		{ return fileInfo; }
};


/**
 * The class which provides methods to handle a list of files or directories.
 **/
class NCFileSelection : public NCTable
{
public:
    enum NCFileSelectionType
    {
	T_Overview,
	T_Detailed,
	T_Unknown
    };

private:

    NCFileSelection & operator=( const NCFileSelection & );
    NCFileSelection( const NCFileSelection & );

    // returns the first column of line with 'index' (the tag)
    NCFileSelectionTag * getTag( const int & index );


protected:

    string startDir;
    string currentDir;
    NCFileSelectionType tableType;	// T_Overview or T_Detailed

    void	setCurrentDir( );
    string	getCurrentLine( );

    NCursesEvent handleKeyEvents( wint_t key );

public:

    /**
     * Constructor
     */
    NCFileSelection( YWidget * parent,
		     YTableHeader * tableHeader,
		     NCFileSelectionType type,
		     const string & iniDir );

    virtual ~NCFileSelection();

    /**
     * Get the file info.
     * index: The list index
     * return: fileInfo Information about the file (directory)
     */
    NCFileInfo * getFileInfo( int index );

    /**
     * Set the type of the table widget
     * type:  Possible values: NCFileSelection::T_Overview, NCFileSelection::T_Detailed
     */
    void setTableType( NCFileSelectionType type ) { tableType = type; };

    virtual void addLine( const vector<string> & elements,
			  NCFileInfo * fileInfo );

    /**
     * Get number of lines ( list entries )
     */
    unsigned int getNumLines( ) { return myPad()->Lines(); }

    /**
     * Draws the file list (has to be called after the loop with
     * addLine() calls)
     */
    void drawList( ) { return DrawPad(); }

    /**
     * Clears the package list
     */
    virtual void deleteAllItems();

    /**
     * Fills the header of the table
     */
    virtual void fillHeader() = 0;

    /**
     * Creates a line in the package table.
     */
    virtual bool createListEntry( NCFileInfo * fileInfo ) = 0;

    /**
     * Get the current directory
     * return: The currently selected directory
     */
    string getCurrentDir() { return currentDir; }

    /**
     * Fill the list of diretcories or files
     * Returns 'true' on success.
     */
    virtual bool fillList( ) = 0;

    /**
     * Set the start directory
     */
    void setStartDir( const string & start )
    {
	currentDir = start;
	startDir = start;
    }

};


class NCFileTable : public NCFileSelection
{
private:

    list<string> pattern;	// files must match this pattern
    string currentFile;		// currently selected file

public:

    /**
     * Constructor
     */
    NCFileTable( YWidget * parent,
		 YTableHeader * tableHeader,
		 NCFileSelectionType type,
		 const string & filter,
		 const string & iniDir );

    virtual ~NCFileTable() {}

    void setCurrentFile( const string & file )
    {
	currentFile = file;
    }

    bool filterMatch( const string & fileName );

    string getCurrentFile() { return currentFile; }

    virtual void fillHeader();

    virtual bool createListEntry( NCFileInfo * fileInfo );

    /**
     * Fill the list of files
     * Returns 'true' on success.
     */
    virtual bool fillList( );

    virtual NCursesEvent wHandleInput( wint_t key );
};


class NCDirectoryTable : public NCFileSelection
{
public:
    NCDirectoryTable( YWidget * parent,
		      YTableHeader * tableHeader,
		      NCFileSelectionType type,
		      const string & iniDir );

    virtual ~NCDirectoryTable() {}

    virtual void fillHeader();

    virtual bool createListEntry( NCFileInfo * fileInfo );

    /**
     * Fill the list of directories.
     * Returns 'true' on success.
     */
    virtual bool fillList( );

    virtual NCursesEvent wHandleInput( wint_t key );
};



#endif // NCFileSelection_h
