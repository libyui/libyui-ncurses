# LibYUI - The Ncurses (Text Mode) Frontend

[![Build Status](https://travis-ci.org/libyui/libyui-ncurses.svg?branch=master
)](https://travis-ci.org/libyui/libyui-ncurses)


[Libyui](https://github.com/libyui/libyui) is a widget abstraction library
providing Qt, GTK and Ncurses frontends. Originally it was developed for
[YaST](https://yast.github.io/) but it can be used in any independent project.

This part contains the text mode front end which displays the UI in a console
or a terminal emulator.

### Building

Libyui uses CMake, driven by a slightly complex set of
[CMakefiles](https://github.com/libyui/libyui/tree/master/buildtools). For
reproducible builds it is best to use the [libyui-rake](
https://github.com/libyui/libyui-rake) Ruby gem like the [Jenkins CI](
https://ci.opensuse.org/view/libyui/) jobs do.

It can be installed from [rubygems.org](https://rubygems.org/gems/libyui-rake/)
using this command (Ruby needs to be installed in the system):

```
gem install libyui-rake
```

Then to build the package run:

```
rake osc:build
```

