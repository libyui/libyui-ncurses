##### PROJECTINFO for CMake

SET( BASELIB		"yui" )		# don't change this

##### MAKE ALL NEEDED CHANGES HERE #####

SET( SUBDIRS		src )
SET( PLUGINNAME		"ncurses" )
SET( LIB_DEPS		Curses )
SET( INTERNAL_DEPS	Libyui )
SET( LIB_LINKER		ncurses ncursesw panelw )
SET( URL		"http://github.com/libyui/" )
SET( SUMMARY		"Libyui - Character Based User Interface" )
SET( DESCRIPTION	"This package contains the character based (ncurses) user interface\ncomponent for libYUI.\n" )
