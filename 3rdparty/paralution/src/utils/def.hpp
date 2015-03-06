// **************************************************************************
//
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2015  PARALUTION Labs UG (haftungsbeschränkt) & Co. KG
//                        Am Hasensprung 6, 76571 Gaggenau
//                        Handelsregister: Amtsgericht Mannheim, HRA 706051
//                        Vertreten durch:
//                        PARALUTION Labs Verwaltungs UG (haftungsbeschränkt)
//                        Am Hasensprung 6, 76571 Gaggenau
//                        Handelsregister: Amtsgericht Mannheim, HRB 721277
//                        Geschäftsführer: Dimitar Lukarski, Nico Trost
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// **************************************************************************



// PARALUTION version 1.0.0 


#ifndef PARALUTION_UTILS_DEF_HPP_
#define PARALUTION_UTILS_DEF_HPP_

// Uncomment to define verbose level
#define VERBOSE_LEVEL 2


// Uncomment for debug mode
// #define DEBUG_MODE

// Uncomment to log all msg to file
// #define LOG_FILE

// Uncomment to disable the assert()s
//#define ASSERT_OFF

// Uncomment to disable automatic object tracking
// #define OBJ_TRACKING_OFF




// ******************
// ******************
// Do not edit below! 
// ******************
// ******************

#ifdef ASSERT_OFF

#define assert(a) ;

#else

#include <assert.h>

#endif


#ifdef DEBUG_MODE

#define assert_dbg(a) assert(a)

#else

#define assert_dbg(a) ;

#endif

#if defined(SUPPORT_CUDA) || defined(SUPPORT_OCL) || defined(SUPPORT_MIC)

#undef SUPPORT_COMPLEX

#else

#define SUPPORT_COMPLEX

#endif


#endif // PARALUTION_UTILS_DEF_HPP_
