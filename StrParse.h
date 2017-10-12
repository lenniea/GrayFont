/**
 * @file
 * @brief	Header file for the StrParse utility module
 *
 * Header file for the StrParse utility module which parses text strings 
 * as numeric values
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

#ifndef __STR_PARSE_H__
#define __STR_PARSE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <tchar.h>

int Char2Digit(TCHAR ch);
int Str2Int(const TCHAR* str, unsigned int base);
int Dec2Int(const TCHAR* str);
int Hex2Int(const TCHAR* str);

#ifdef __cplusplus
}
#endif

#endif /* __STR_PARSE_H__ */
