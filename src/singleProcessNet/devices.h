/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DEVICES_H
#define __DEVICES_H

#include <cstddef> // NULL

class device {
public:
	device (char * m_name, device * m_next = NULL) 
	{
		name = m_name; next = m_next;
	}
	char * name;
	device * next;
};

/**
 * This function can return null, if no good interface is found
 * The function avoids loopback interface and down/not running interfaces
 */
device * get_default_devices();

#endif
