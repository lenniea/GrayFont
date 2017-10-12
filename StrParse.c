/**
 * @file
 * @brief	Implementation file for the StrParse utility module
 *
 * C Implementation file for the StrParse utility module which parses 
 * text strings as numeric values
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

#include <windows.h>
#include <ctype.h>

#include "StrParse.h"

/**
 *	Convert Text Character into digit (0-25).
 * @param	ch ('0'-'9', 'A'-'Z', 'a'-'z')
 * @return	digit (0-25) or -1 if invalid digit
 */
int Char2Digit(TCHAR ch)
{
	int digit;
	ch = toupper(ch);

	if (ch >= '0' && ch <= '9')
		digit = ch - '0';
	else if (ch >= 'A' && ch <= 'F')
		digit = 10 + ch - 'A';
	else
		digit = -1;
	return digit;
}

/**
 *	Convert Text String into number.
 * @param	str - array of Text Characters
 * @param	base - numeric base (1-26)
 * @return	positive number or -1 if error
 */
int Str2Int(const TCHAR* str, unsigned int base)
{
	int n = 0;
	TCHAR ch;

	/* Range check paramters */
	if ((str == NULL) || (base >= 26))
		return -1;

	for (ch = *str; ch != '\0'; ch = *(++str))
	{
		int digit = Char2Digit(ch);
		if (digit < 0 || (unsigned) digit >= base)
			return -1;

		n = (n * base) + digit;
	}
	return n;
}

/**
 *	Convert Decimal String into number.
 * @param	str - array of Text Characters
 * @return	positive number or -1 if error
 */
int Dec2Int(const TCHAR* str)
{
	return Str2Int(str, 10U);
}

/**
 *	Convert Hexacecimal String into number.
 * @param	str - array of Text Characters
 * @return	positive number or -1 if error
 */
int Hex2Int(const TCHAR* str)
{
	return Str2Int(str, 16U);
}
