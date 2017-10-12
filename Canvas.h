/**
 * @file
 * @brief	Header file for the CANVAS abstraction
 *
 * Header file for the CANVAS abstraction.  This is simply an HDC on
 * Windows but a structure for embedded targets.
 *
 * @author  Lennie Araki
 * @version 1.0
 *
 * @section LICENSE
 * Copyright © 2010. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 */
 
#ifndef __CANVAS_H__
#define __CANVAS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
	#include <windows.h>
	typedef HDC CANVAS;
	typedef HWND WINDOW;
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CANVAS_H__ */
