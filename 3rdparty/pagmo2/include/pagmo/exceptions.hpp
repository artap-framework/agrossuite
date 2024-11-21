/* Copyright 2017-2021 PaGMO development team

This file is part of the PaGMO library.

The PaGMO library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The PaGMO library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the PaGMO library.  If not,
see https://www.gnu.org/licenses/. */

#ifndef PAGMO_EXCEPTIONS_HPP
#define PAGMO_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <pagmo/detail/visibility.hpp>
#include <pagmo/type_traits.hpp>

namespace pagmo
{
namespace detail
{

template <typename Exception>
struct ex_thrower {
    // Determine the type of the __LINE__ macro.
    using line_type = std::decay_t<decltype(__LINE__)>;
    explicit ex_thrower(const char *file, line_type line, const char *func) : m_file(file), m_line(line), m_func(func)
    {
    }
    template <typename... Args, enable_if_t<std::is_constructible<Exception, Args...>::value, int> = 0>
    [[noreturn]] void operator()(Args &&...args) const
    {
        throw Exception(std::forward<Args>(args)...);
    }
    template <typename Str, typename... Args,
              enable_if_t<conjunction<std::is_constructible<Exception, std::string, Args...>,
                                      disjunction<std::is_same<std::decay_t<Str>, std::string>,
                                                  std::is_same<std::decay_t<Str>, char *>,
                                                  std::is_same<std::decay_t<Str>, const char *>>>::value,
                          int> = 0>
    [[noreturn]] void operator()(Str &&desc, Args &&...args) const
    {
        std::string msg("\nfunction: ");
        msg += m_func;
        msg += "\nwhere: ";
        msg += m_file;
        msg += ", ";
        msg += std::to_string(m_line);
        msg += "\nwhat: ";
        msg += desc;
        msg += "\n";
        throw Exception(std::move(msg), std::forward<Args>(args)...);
    }
    const char *m_file;
    const line_type m_line;
    const char *m_func;
};

} // namespace detail

} // namespace pagmo

/// Exception-throwing macro.
/**
 * By default, this variadic macro will throw an exception of type \p exception_type, using the variadic
 * arguments for the construction of the exception object. The macro will check if the exception can be constructed
 * from the variadic arguments, and will produce a compilation error in case no suitable constructor is found.
 *
 * Additionally, given a set of variadic arguments <tt>[arg0,arg1,...]</tt>, and
 *
 * - if the first variadic argument \p arg0 is a string type (either C or C++),
 * - and if the exception can be constructed from the set of arguments <tt>[str,arg1,...]</tt>,
 *   where \p str is an instance of \p std::string,
 *
 * then the first argument \p arg0 is interpreted as the error message associated to the exception object, and it
 * will be decorated with information about the context in which the exception was thrown (file, line, function) before
 * being passed on for construction.
 *
 * Note that, in order to be fully standard-compliant, for use with exceptions that take no arguments on construction
 * the invocation must include a closing comma. E.g.,
 * @code{.unparsed}
 * pagmo_throw(std::bad_alloc);
 * @endcode
 * is not correct, whereas
 * @code{.unparsed}
 * pagmo_throw(std::bad_alloc,);
 * @endcode
 * is correct.
 */
#define pagmo_throw(exception_type, ...)                                                                               \
    pagmo::detail::ex_thrower<exception_type>(__FILE__, __LINE__, __func__)(__VA_ARGS__)

namespace pagmo
{

/// Exception for functionality which has not been implemented.
/**
 * This exception is used by pagmo::problem, pagmo::algorithm, etc. to signal that
 * optional methods in user-defined classes are not implemented.
 * This class inherits the constructors from \p std::runtime_error.
 */
struct PAGMO_DLL_PUBLIC_INLINE_CLASS not_implemented_error final : std::runtime_error {
    using std::runtime_error::runtime_error;
};
} // namespace pagmo

#endif
