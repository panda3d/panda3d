/*
 * 
 * This file is part of ARToolKit.
 * 
 * ARToolKit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * ARToolKit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ARToolKit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */

#ifndef AR_VRML_H
#define AR_VRML_H

#ifdef __cplusplus
extern "C" {
#endif

int arVrmlLoadFile(const char *file);
int arVrmlFree( int id );
int arVrmlDraw( int id );
int arVrmlTimerUpdate( void );
int arVrmlSetInternalLight( int flag );

#ifdef __cplusplus
}
#endif

#endif
