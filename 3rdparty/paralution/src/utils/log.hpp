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


#ifndef PARALUTION_UTILS_LOG_HPP_
#define PARALUTION_UTILS_LOG_HPP_

#include "def.hpp"
#include "../base/backend_manager.hpp"

#include <iostream>
#include <stdlib.h>

namespace paralution {

void _paralution_open_log_file(void);
void _paralution_close_log_file(void);

}

// Do not edit
#ifdef DEBUG_MODE

#undef VERBOSE_LEVEL
#define VERBOSE_LEVEL 10

#endif


// Do not edit
#ifdef LOG_FILE

#define LOG_STREAM *(_get_backend_descriptor()->log_file)

#else

#define LOG_STREAM std::cout

#endif

// LOG INFO
#define LOG_INFO(stream) {            \
    LOG_STREAM << stream << std::endl; \
  }


// LOG ERROR
#define FATAL_ERROR(file, line) {                                \
  LOG_INFO("Fatal error - the program will be terminated ");     \
  LOG_INFO("File: " << file << "; line: " << line);              \
  exit(1);                                                       \
}


// LOG VERBOSE
#ifdef VERBOSE_LEVEL

#define LOG_VERBOSE_INFO(level, stream) {                       \
    if (level <= VERBOSE_LEVEL)                                 \
      LOG_STREAM << stream << std::endl;                         \
  }

#else

#define LOG_VERBOSE_INFO(level, stream) ;

#endif


// LOG DEBUG
#ifdef DEBUG_MODE

#define LOG_DEBUG(obj, fct, stream) {                            \
    LOG_STREAM << "# Obj addr: " << obj                           \
              << "; fct: " << fct                                \
              << " " << stream << std::endl;             \
  }

#else

#define LOG_DEBUG(obj, fct, stream) ;

#endif


#endif // PARALUTION_UTILS_LOG_HPP_
