// ---------------------------------------------------------------------
// $Id$
//
// Copyright (C) 2012 - 2013 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the deal.II distribution.
//
// ---------------------------------------------------------------------

#ifndef __deal2__config_h
#define __deal2__config_h


/**
 * Two macro names that we put at the top and bottom of all deal.II files
 * and that will be expanded to "namespace dealii {" and "}".
 */
#define DEAL_II_NAMESPACE_OPEN namespace dealii {
#define DEAL_II_NAMESPACE_CLOSE }


/***********************************************
 * Configured in setup_cached_variables.cmake: *
 ***********************************************/

/* #undef DEAL_II_WITH_64BIT_INDICES */
#ifdef DEAL_II_WITH_64BIT_INDICES
#  define DEAL_II_USE_LARGE_INDEX_TYPE
#endif


/**************************************
 * Configured in setup_deal_ii.cmake: *
 **************************************/

/* Defined to the full name of this package. */
#define DEAL_II_PACKAGE_NAME "deal.II"

/* Defined to the version of this package. */
#define DEAL_II_PACKAGE_VERSION "8.1.0"

/* Major version number of deal.II */
#define DEAL_II_VERSION_MAJOR 8
#define DEAL_II_MAJOR 8

/* Minor version number of deal.II */
#define DEAL_II_VERSION_MINOR 1
#define DEAL_II_MINOR 1


/********************************************
 * Configured in check_1_compiler_features: *
 ********************************************/

/* Defined if the compiler can use arithmetic operations on vectorized data types */
#define DEAL_II_COMPILER_USE_VECTOR_ARITHMETICS

/* Defined if vector iterators are just plain pointers */
/* #undef DEAL_II_VECTOR_ITERATOR_IS_POINTER */

/* Define if the compiler provides __builtin_expect */
#define HAVE_BUILTIN_EXPECT

/* Define if the compiler provides __verbose_terminate_handler */
#define HAVE_VERBOSE_TERMINATE

/* Define if deal.II is linked against a libc that provides stacktrace
 * debug information that can be printed out in the exception class
 * */
#define HAVE_GLIBC_STACKTRACE

/* Defined if the std c++ library provides a demangler conforming to the
 * GCC libstdc++ interface.
 */
#define HAVE_LIBSTDCXX_DEMANGLER

/* If already available, do not define at all. Otherwise, define to
 * __func__ if that is available. In all other cases, indicate that no
 * information about the present function is available for this compiler.
 */
/* #undef __PRETTY_FUNCTION__ */

/* If the compiler supports it, then this variable is defined to a string
 * that when written after a function name makes the compiler emit a warning
 * whenever this function is used somewhere that its use is deprecated.
 */
#define DEAL_II_DEPRECATED __attribute__((deprecated))


/***************************************
 * Configured in check_1_cpu_features: *
 ***************************************/

/* Defined if the system stores words with the most significant byte first */
/* #undef DEAL_II_WORDS_BIGENDIAN */

/* Equal to 0 in the generic case, equal to 1 if CPU compiled for supports
 * SSE2, equal to 2 if CPU compiled for supports AVX
 */
#define DEAL_II_COMPILER_VECTORIZATION_LEVEL 1


/***************************************
 * Configured in check_1_cxx_features: *
 ***************************************/

/* Defined if the compiler we use supports the C++2011 standard well enough
 * to allow using the standard library classes instead of the corresponding
 * BOOST classes.
 */
#define DEAL_II_USE_CXX11
#ifdef DEAL_II_USE_CXX11
# define DEAL_II_CAN_USE_CXX11
# define DEAL_II_CAN_USE_CXX1X
#endif

/* Defined if C++11 is enabled and the standard library supports
 * template<typename T> std::is_trivially_copyable<T>
 */
/* #undef DEAL_II_HAVE_CXX11_IS_TRIVIALLY_COPYABLE */

/* Defined if isnan is available */
#define HAVE_ISNAN

/* Defined if _isnan is available */
/* #undef HAVE_UNDERSCORE_ISNAN */

/* Defined if std::isfinite is available */
#define DEAL_II_HAVE_ISFINITE


/******************************************
 * Configured in check_1_system_features: *
 ******************************************/

/* Defined if you have the <sys/resource.h> header file */
#define HAVE_SYS_RESOURCE_H

/* Defined if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H

/* Defined if you have the <sys/times.h> header file. */
#define HAVE_SYS_TIMES_H

/* Defined if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H

/* Defined if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H

/* Defined if you have the "gethostname" function. */
#define HAVE_GETHOSTNAME

/* Defined if you have the "getpid' function. */
#define HAVE_GETPID

/* Defined if you have the "rand_r" function */
#define HAVE_RAND_R

/* Defined if you have the "times" function. */
#define HAVE_TIMES

/* Defined if you have the "jn" function. */
#define HAVE_JN

/* Defined if deal.II was configured on a native Windows platform. */
/* #undef DEAL_II_MSVC */

/* Disable a bunch of warnings for Microsoft Visual C++. */
#ifdef _MSC_VER
#  pragma warning( disable : 4244 ) /* implied downcasting from double to float */
#  pragma warning( disable : 4267 ) /* implied downcasting from size_t to unsigned int */
#  pragma warning( disable : 4996 ) /* unsafe functions, such as strcat and sprintf */
#  pragma warning( disable : 4355 ) /* 'this' : used in base member initializer list */
#  pragma warning( disable : 4661 ) /* no suitable definition provided for explicit template instantiation request */
#  pragma warning( disable : 4800 ) /* forcing value to bool 'true' or 'false' (performance warning) */
#  pragma warning( disable : 4146 ) /* unary minus operator applied to unsigned type, result still unsigned */
#  pragma warning( disable : 4667 ) /* no function template defined that matches forced instantiation */
#  pragma warning( disable : 4520 ) /* multiple default constructors specified */
#  pragma warning( disable : 4700 ) /* uninitialized local variable */
#  pragma warning( disable : 4789 ) /* destination of memory copy is too small */
#  pragma warning( disable : 4808 ) /* case 'value' is not a valid value for switch condition of type 'bool */
#endif /*_MSC_VER*/


/****************************************
 * Configured in check_3_compiler_bugs: *
 ****************************************/

/* Defined if we have to work around a bug with some compilers that will not
 * allow us to specify a fully specialized class of a template as a friend.
 * See the aclocal.m4 file in the top-level directory for a description of
 * this bug.
 * */
/* #undef DEAL_II_TEMPL_SPEC_FRIEND_BUG */

/* Defined if the compiler refuses to allow the explicit specialization of
 * static member arrays. For the exact failure mode, look at aclocal.m4 in the
 * top-level directory.
 */
/* #undef DEAL_II_MEMBER_ARRAY_SPECIALIZATION_BUG */

/* Defined if the compiler refuses to allow the explicit specialization of
 * static member variables.
 */
/* #undef DEAL_II_MEMBER_VAR_SPECIALIZATION_BUG */

/* Defined if the compiler does not honor the explicit keyword on template
 * constructors.
 */
/* #undef DEAL_II_EXPLICIT_CONSTRUCTOR_BUG */

/* Defined if the compiler has a bug in deducing the type of pointers to const
 * member functions.
 */
/* #undef DEAL_II_CONST_MEMBER_DEDUCTION_BUG */

/* Defined if the compiler gets an internal error compiling some code that
 * involves boost::bind
 */
#define DEAL_II_BOOST_BIND_COMPILER_BUG

/* Defined if the compiler incorrectly deduces a constexpr as not being a
 * constant integral expression under certain optimization (notably
 * gcc-4.8.1 on Windows and Mac)
 */
/* #undef DEAL_II_CONSTEXPR_BUG */


/*****************************************
 * Configured in configure_arpack.cmake: *
 *****************************************/

/* #undef DEAL_II_WITH_ARPACK */
#ifdef DEAL_II_WITH_ARPACK
#  define DEAL_II_USE_ARPACK
#endif

/*************************************************
 * Configured in configure_functionparser.cmake: *
 *************************************************/

#define DEAL_II_WITH_FUNCTIONPARSER
#ifdef DEAL_II_WITH_FUNCTIONPARSER
#  define HAVE_FUNCTIONPARSER
#endif

/***************************************
 * Configured in configure_hdf5.cmake: *
 ***************************************/

/* #undef DEAL_II_WITH_HDF5 */
#ifdef DEAL_II_WITH_HDF5
#  define DEAL_II_HAVE_HDF5
#endif

/*****************************************
 * Configured in configure_lapack.cmake: *
 *****************************************/

#define DEAL_II_WITH_LAPACK
#ifdef DEAL_II_WITH_LAPACK
#  define HAVE_LIBLAPACK

/* Defined if the corresponding BLAS or LAPACK function is available */
#define HAVE_DAXPY_
#define HAVE_DGEEVX_
#define HAVE_DGEEV_
#define HAVE_DGELSD_
#define HAVE_DGEMM_
#define HAVE_DGEMV_
#define HAVE_DGEQRF_
#define HAVE_DGESDD_
#define HAVE_DGESVD_
#define HAVE_DGETRF_
#define HAVE_DGETRI_
#define HAVE_DGETRS_
#define HAVE_DORGQR_
#define HAVE_DORMQR_
#define HAVE_DSTEV_
#define HAVE_DSYEVX_
#define HAVE_DSYGV_
#define HAVE_DSYGVX_
#define HAVE_DTRTRS_
#define HAVE_SAXPY_
#define HAVE_SGEEVX_
#define HAVE_SGEEV_
#define HAVE_SGELSD_
#define HAVE_SGEMM_
#define HAVE_SGEMV_
#define HAVE_SGEQRF_
#define HAVE_SGESDD_
#define HAVE_SGESVD_
#define HAVE_SGETRF_
#define HAVE_SGETRI_
#define HAVE_SGETRS_
#define HAVE_SORGQR_
#define HAVE_SORMQR_
#define HAVE_SSTEV_
#define HAVE_SSYEVX_
#define HAVE_SSYGV_
#define HAVE_SSYGVX_
#define HAVE_STRTRS_
#endif



/****************************************
 * Configured in configure_metis.cmake: *
 ****************************************/

/* #undef DEAL_II_WITH_METIS */
#ifdef DEAL_II_WITH_METIS
#  define DEAL_II_USE_METIS
#endif


/**************************************
 * Configured in configure_mpi.cmake: *
 **************************************/

/* #undef DEAL_II_WITH_MPI */
#ifdef DEAL_II_WITH_MPI
#  define DEAL_II_COMPILER_SUPPORTS_MPI
#endif


/*****************************************
 * Configured in configure_mumps.cmake:  *
 *****************************************/

/* #undef DEAL_II_WITH_MUMPS */
#ifdef DEAL_II_WITH_MUMPS
#  define DEAL_II_USE_MUMPS
#endif


/*****************************************
 * Configured in configure_netcdf.cmake: *
 *****************************************/

/* #undef DEAL_II_WITH_NETCDF */
#ifdef DEAL_II_WITH_NETCDF
#  define HAVE_LIBNETCDF
#endif


/****************************************
 * Configured in configure_p4est.cmake: *
 ****************************************/

/* #undef DEAL_II_WITH_P4EST */
#ifdef DEAL_II_WITH_P4EST
#  define DEAL_II_USE_P4EST

#  define DEAL_II_P4EST_VERSION_MAJOR 
#  define DEAL_II_P4EST_VERSION_MINOR 
#  define DEAL_II_P4EST_VERSION_SUBMINOR 
#  define DEAL_II_P4EST_VERSION_PATCH 

#  define DEAL_II_P4EST_VERSION_GTE(major,minor,subminor,patch) \
 ((DEAL_II_P4EST_VERSION_MAJOR * 1000000 + \
    DEAL_II_P4EST_VERSION_MINOR * 10000 + \
     DEAL_II_P4EST_VERSION_SUBMINOR * 100 + \
      DEAL_II_P4EST_VERSION_PATCH) \
    >=  \
    (major)*1000000 + (minor)*10000 + (subminor)*100 + (patch))
#else
  // p4est up to 0.3.4.1 didn't define P4EST_VERSION_*. since
  // we didn't supports anything before 0.3.4, we assume 0.3.4
  // This means that we can't use the new features in 0.3.4.1
#  define DEAL_II_P4EST_VERSION_GTE(major,minor,subminor,patch) \
  ((0 * 1000000 + \
    3 * 10000 + \
    4 * 100 + \
    0) \
    >=  \
    (major)*1000000 + (minor)*10000 + (subminor)*100 + (patch))

#endif


/****************************************
 * Configured in configure_petsc.cmake: *
 ****************************************/

/* #undef DEAL_II_WITH_PETSC */
#ifdef DEAL_II_WITH_PETSC
#  define DEAL_II_USE_PETSC
#endif


/*
 * Note: The following definitions will be set in petscconf.h and
 *       petscversion.h, so we don't repeat them here.
 *
 *  PETSC_VERSION_MAJOR
 *  PETSC_VERSION_MINOR
 *  PETSC_VERSION_SUBMINOR
 *  PETSC_VERSION_PATCH
 *  PETSC_VERSION_RELEASE
 *  PETSC_USE_COMPLEX
 */

/**
 * These macros are defined to make testing for PETSc versions within
 * the deal.II main code as simple as possible. In brief they are used
 * like this: (i) DEAL_II_PETSC_VERSION_LT is used to advance the
 * PETScWrappers to newer versions of PETSc while preserving backward
 * compatibility; and (ii) DEAL_II_PETSC_VERSION_GTE is used to add
 * functionality to the PETScWrappers that does not exist in previous
 * versions of PETSc.  Examples of usage can be found in
 * lac/source/petsc_matrix_base.h.  Note: SLEPcWrappers do not need
 * their own analogical macros, since SLEPc and PETSc must have
 * identical version numbers anyways.
 */
#define DEAL_II_PETSC_VERSION_LT(major,minor,subminor) \
  ((PETSC_VERSION_MAJOR * 10000 + \
    PETSC_VERSION_MINOR * 100 + \
    PETSC_VERSION_SUBMINOR) \
    <  \
    (major)*10000 + (minor)*100 + (subminor))

#define DEAL_II_PETSC_VERSION_GTE(major,minor,subminor) \
  ((PETSC_VERSION_MAJOR * 10000 + \
    PETSC_VERSION_MINOR * 100 + \
    PETSC_VERSION_SUBMINOR) \
    >=  \
    (major)*10000 + (minor)*100 + (subminor))

/****************************************
 * Configured in configure_slepc.cmake: *
 ****************************************/

/* #undef DEAL_II_WITH_SLEPC */
#ifdef DEAL_II_WITH_SLEPC
#  define DEAL_II_USE_SLEPC
#endif


/********************************************
 * Configured in configure_1_threads.cmake: *
 ********************************************/

#define DEAL_II_WITH_THREADS
#ifdef DEAL_II_WITH_THREADS
#  define DEAL_II_USE_MT
#endif

/**
 * Defined if multi-threading is to be achieved by using the POSIX functions
 */
#define DEAL_II_USE_MT_POSIX

/* Defined if POSIX is supported but not the newer POSIX barrier functions.
 * Barriers will then not work in the library, but the other threading
 * functionality is available.
 */
/* #undef DEAL_II_USE_MT_POSIX_NO_BARRIERS */

/**
 * Depending on the use of threads, we will have to make some variables
 * volatile. We do this here in a very old-fashioned C-style, but still
 * convenient way.
 */
#ifdef DEAL_II_WITH_THREADS
#  define DEAL_VOLATILE volatile
#else
#  define DEAL_VOLATILE
#endif


/*******************************************
 * Configured in configure_trilinos.cmake: *
 *******************************************/

/* #undef DEAL_II_WITH_TRILINOS */
#ifdef DEAL_II_WITH_TRILINOS
#  define DEAL_II_USE_TRILINOS
#endif


/******************************************
 * Configured in configure_umfpack.cmake: *
 ******************************************/

#define DEAL_II_WITH_UMFPACK
#ifdef DEAL_II_WITH_UMFPACK
#  define HAVE_LIBUMFPACK
#endif


/***************************************
 * Configured in configure_zlib.cmake: *
 ***************************************/

#define DEAL_II_WITH_ZLIB
#ifdef DEAL_II_WITH_ZLIB
#  define HAVE_LIBZ
#endif


#include <deal.II/base/numbers.h>
#include <deal.II/base/types.h>

#endif

