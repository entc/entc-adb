/*
 * Copyright (c) 2010-2013 "Alexander Kalkhof" [email:entc@kalkhof.org]
 *
 * This file is part of the extension n' tools (entc-base) framework for C.
 *
 * entc-base is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * entc-base is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with entc-base.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENTC_SYSTEM_PLATFORM_H
#define ENTC_SYSTEM_PLATFORM_H 1

#if defined _WIN64 || defined _WIN32

#define ENTC_PLATFORM_WINDOWS 1

#elif defined __DOS__

#define ENTC_PLATFORM_DOS 1

#elif defined __linux__

#define ENTC_PLATFORM_LINUX 1

#elif defined __APPLE_CC__

#define ENTC_PLATFORM_DARWIN 1

#endif

#endif
