/*+--------------------------------------------------------------+
  |            demonize.h  -  description                        |
  |                -------------------                           |
  | begin      : 08/01/2010 13.00                                |
  | copyright  : (C) 2010 xAppSoftware                           |
  | author     : Luigi D'Andrea                                  |
  | email      : gg1 ( at ) xappsoftware.com                     |
  | compiling  : gcc -o server server.c                          |
  |                                                              |
  | Latest version on http://www.xappsoftware.com                |
  +--------------------------------------------------------------+
  | demonize librarymay be redistributed and modified under      |
  | certain conditions. This software is distributed on an       |
  | "AS IS" basis WITHOUT WARRANTY OF ANY KIND, either express or|
  ! implied. See the file License.txt for details.               |
  +--------------------------------------------------------------+*/
#ifndef _DEMONIZE_H_
#define _DEMONIZE_H_

/*+--------------------------------------------------------------+
  | FUNCTION PROTOTYPES                                          |
  +--------------------------------------------------------------+*/
void demonize(char *arg);
void detachFromTerminal(void);

#endif