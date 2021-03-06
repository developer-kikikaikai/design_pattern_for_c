# What's design_pattern_for_c?
  Libraries package for Linux C language to show some design patterns.

  Some C systems uses class design as them. And sometimes, this code is different from an image of object-oriented programming.

  Because "*C system shows efficient design patterns with Goodness of C language*".

  In other words, *C system mix a strong point of design patterns and C language (not just an imitation of patterns)*. 

  This package may helps you to use some design patterns with Goodness of C language.

# How to use
  Download:
  Please download from [here](https://github.com/developer-kikikaikai/design_pattern_for_c/releases).

  Before build:
  1. Please install gcc, autoconf libtool and pkg-config
     Debian: apt install autoconf libtool pkg-config
     Redhat: yum install gcc autoconf libtool 

  2. Please install libevent if you want to use plugin of libevent.
     Debian: apt install libevent-dev
     Redhat: yum install libev libevent-devel (include libevent)

     If you don't want to use it, please add configure option --disable-threadpool-libevent

  3. Please install libev if you want to use plugin of libev. (Please care this plugin libevent_if_libev is GPL License).
     Debian: apt install libev4
     Redhat: yum install libev libev-devel

     If you don't want to use it, please add configure option --disable-threadpool-libev    

  Build:
  1. ./configure
     (or ./configure --disable-threadpool-libev --disable-threadpool-libevent)
  2. make
  3. sudo make install

  If you failed to make, as "[aclocal.m4]" error, please call ./autogen.sh before calling configure.

  Headers and libraries are installed in prefix dir (default: /usr/local/)

  Option:
  - You can choose to disable package libraries with --without-XXX option.
    Ex. ./configure --without-builder

 - I checked Debian=> Ubuntu 18.04, Redhat=>CentOS 7.5.1804

# Latest Version 
 Stable: V1.11
 Please see [here](https://github.com/developer-kikikaikai/design_pattern_for_c/releases).

 Current: master code
 Please see [here](https://github.com/developer-kikikaikai/design_pattern_for_c)

# Support libraries

- Builder
- Singleton (contained in Flyweight)
- Flyweight
- Chain of Responsibility 
- Observer(Publish-Subscribe) 
- State 
- Threadpool (with File disctriptor)
- Memorypool
- Prototype
- Mement

Almost of other pattern will create sample.

# Detail: all design pattern libraries

Related to Create:

  |name            |support| exist sample  |  other             |
  |:---------------|:-----:|:-------------:|:------------------:|
  |Builder         |  Yes  | see builder   |                    |
  |Singleton       |  Yes  | see flyweight | wrap in Flyweight  |
  |Prototype       |  Yes  | see prototype |                    |
  |Abstract Factory|  No   | create future |                    |
  |Factory Method  |  No   | create future |                    |

Related to Construct:

  |name            |support| exist sample    |  other                                     |
  |:---------------|:-----:|:---------------:|:------------------------------------------:|
  |Flyweight       |  Yes  | see flyweight   |                                            |
  |Adapter         |  No   | create future   |                                            |
  |Bridge          |  No   | create future   |                                            |
  |Composite       |  No   |   No            | Please use a tree or a list as dp_list.h   |
  |Decorator       |  No   | create future   |                                            |
  |Facade          |  No   | create future   |                                            |
  |Proxy           |  No   | see other repos | Switch by library/dlopen/LD_PRELOAD        |

Related to Behavior:

  |name                        |support| exist sample              |  other                        |
  |:---------------------------|:-----:|:-------------------------:|:-----------------------------:|
  |Chain of Responsibility     |  Yes  | see ChainOfResponsibility |                               |
  |Observer(Publish-Subscribe) |  Yes  | see publisher <br> or combination_sample/publisher_with_fd ||
  |State                       |  Yes  | see state                 | State and StateMachine lib    |
  |Command                     |  No   | see other repos           |                               |
  |Mediator                    |  No   | see other repos           |                               |
  |Interpreter                 |  No   | No                        | There is no skill             |
  |Iterator                    |  No   | No                        | Please use list as dp_list.h  |
  |Memento                     |  Yes  | see mement                |                               |
  |Strategy                    |  No   | create future             |                               |
  |Template Method             |  No   | Not yet                   |                               |
  |Visitor                     |  No   | create future             |                               |

Related to Multi thread:

  |name                        |support| exist sample              |  other                        |
  |:---------------------------|:-----:|:-------------------------:|:-----------------------------:|
  |Threadpool                  |  Yes  | see threadpool            |                               |

Other:

  |name                        |support| exist sample              |  other                        |
  |:---------------------------|:-----:|:-------------------------:|:-----------------------------:|
  |Memorypool                  |  Yes  | see memorypool            | Also it can use as Prototype  |

  If you want to see detail, please see [here](https://qiita.com/developer-kikikaikai/items/8e7858c130c8ae8df488) (Japanese website) 

  Sample without support is in [here](https://github.com/developer-kikikaikai/design_patter_for_c_appendix).

# Detail of libraries code

If you want to see detail of code, please see [doxgen file in here](https://developer-kikikaikai.github.io/design_pattern_for_c_doc/docs/)

# License
  Almost of this software is released under the MIT License, see [LICENSE file in this package](https://github.com/developer-kikikaikai/design_pattern_for_c/blob/master/LICENSE).
  Only libevent_if_libev plugin is released under the GNU General Public License ("GPL") version 2 or any later version. Please see LICENSE.GPL

# Sample case

I try to modify [lighttpd](https://github.com/lighttpd/lighttpd1.4) (which is light memory/cpu/speed httpd server) for using this package.
And I success to do following:

- Multi-thread connection by using threadpool and state machine
- Faster access of memory by using memorypool
- Reuse same header types by using flyweight
- Register some settings and clone it by using prototype.
- Communication between main and connections by observer.

Other libraries is not better to use for it, so I only write test code for other.

This sample repository is [here](https://github.com/developer-kikikaikai/lighttpd_multithread) .
(lighttpd_multithread is only for sample, please care it.)
