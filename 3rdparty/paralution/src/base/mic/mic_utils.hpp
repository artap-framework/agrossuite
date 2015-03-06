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


#ifndef PARALUTION_MIC_MIC_UTILS_HPP_
#define PARALUTION_MIC_MIC_UTILS_HPP_

#define MIC_ALLOC   alloc_if(1)
#define MIC_FREE    free_if(1)
#define MIC_RETAIN  free_if(0)
#define MIC_REUSE   alloc_if(0)

namespace paralution {

template <typename ValueType>
void copy_to_mic(const int mic_dev, const ValueType *src, ValueType *dst, const int size);

template <typename ValueType>
void copy_to_host(const int mic_dev, const ValueType *src, ValueType *dst, const int size);

template <typename ValueType>
void copy_mic_mic(const int mic_dev, const ValueType *src, ValueType *dst, const int size);


};


#endif // PARALUTION_MIC_MIC_UTILS_HPP_
