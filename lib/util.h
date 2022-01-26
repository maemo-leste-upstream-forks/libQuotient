/******************************************************************************
 * Copyright (C) 2016 Kitsune Ral <kitsune-ral@users.sf.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#pragma once

#include <QtCore/QLatin1String>
#include <QtCore/QHashFunctions>

#include <functional>
#include <memory>
#include <unordered_map>
#include <optional>

// Along the lines of Q_DISABLE_COPY - the upstream version comes in Qt 5.13
#define DISABLE_MOVE(_ClassName)               \
    _ClassName(_ClassName&&) Q_DECL_EQ_DELETE; \
    _ClassName& operator=(_ClassName&&) Q_DECL_EQ_DELETE;

namespace Quotient {
/// An equivalent of std::hash for QTypes to enable std::unordered_map<QType, ...>
template <typename T>
struct HashQ {
    size_t operator()(const T& s) const Q_DECL_NOEXCEPT
    {
        return qHash(s, uint(qGlobalQHashSeed()));
    }
};
/// A wrapper around std::unordered_map compatible with types that have qHash
template <typename KeyT, typename ValT>
using UnorderedMap = std::unordered_map<KeyT, ValT, HashQ<KeyT>>;

constexpr auto none = std::nullopt;

/** `std::optional` with tweaks
 *
 * The tweaks are:
 * - streamlined assignment (operator=)/emplace()ment of values that can be
 *   used to implicitly construct the underlying type, including
 *   direct-list-initialisation, e.g.:
 *   \code
 *   struct S { int a; char b; }
 *   Omittable<S> o;
 *   o = { 1, 'a' }; // std::optional would require o = S { 1, 'a' }
 *   \endcode
 * - entirely deleted value(). The technical reason is that Xcode 10 doesn't
 *   have it; but besides that, value_or() or (after explicit checking)
 *   `operator*()`/`operator->()` are better alternatives within Quotient
 *   that doesn't practice throwing exceptions (as doesn't most of Qt).
 * - disabled non-const lvalue operator*() and operator->(), as it's too easy
 *   to inadvertently cause a value change through them.
 * - edit() to provide a safe and explicit lvalue accessor instead of those
 *   above. Requires the underlying type to be default-constructible.
 *   Allows chained initialisation of nested Omittables:
 *   \code
 *   struct Inner { int member = 10; Omittable<int> innermost; };
 *   struct Outer { int anotherMember = 10; Omittable<Inner> inner; };
 *   Omittable<Outer> o; // = { 10, std::nullopt };
 *   o.edit().inner.edit().innermost.emplace(42);
 *   \endcode
 * - merge() - a soft version of operator= that only overwrites its first
 *   operand with the second one if the second one is not empty.
 */
template <typename T>
class Omittable : public std::optional<T> {
public:
    using base_type = std::optional<T>;
    using value_type = std::decay_t<T>;

    using std::optional<T>::optional;

    // Overload emplace() and operator=() to allow passing braced-init-lists
    // (the standard emplace() does direct-initialisation but
    // not direct-list-initialisation).
    using base_type::operator=;
    Omittable& operator=(const value_type& v)
    {
        base_type::operator=(v);
        return *this;
    }
    Omittable& operator=(value_type&& v)
    {
        base_type::operator=(v);
        return *this;
    }
    using base_type::emplace;
    T& emplace(const T& val) { return base_type::emplace(val); }
    T& emplace(T&& val) { return base_type::emplace(std::move(val)); }

    // use value_or() or check (with operator! or has_value) before accessing
    // with operator-> or operator*
    // The technical reason is that Xcode 10 has incomplete std::optional
    // that has no value(); but using value() may also mean that you rely
    // on the optional throwing an exception (which is not assumed practice
    // throughout Quotient) or that you spend unnecessary CPU cycles on
    // an extraneous has_value() check.
    value_type& value() = delete;
    const value_type& value() const = delete;
    value_type& edit()
    {
        return this->has_value() ? base_type::operator*() : this->emplace();
    }

    [[deprecated("Use '!o' or '!o.has_value()' instead of 'o.omitted()'")]]
    bool omitted() const
    {
        return !this->has_value();
    }

    /// Merge the value from another Omittable
    /// \return true if \p other is not omitted and the value of
    ///         the current Omittable was different (or omitted);
    ///         in other words, if the current Omittable has changed;
    ///         false otherwise
    template <typename T1>
    auto merge(const Omittable<T1>& other)
        -> std::enable_if_t<std::is_convertible<T1, T>::value, bool>
    {
        if (!other || (this->has_value() && **this == *other))
            return false;
        *this = other;
        return true;
    }

    // Hide non-const lvalue operator-> and operator* as these are
    // a bit too surprising: value() & doesn't lazy-create an object;
    // and it's too easy to inadvertently change the underlying value.

    const value_type* operator->() const& { return base_type::operator->(); }
    value_type* operator->() && { return base_type::operator->(); }
    const value_type& operator*() const& { return base_type::operator*(); }
    value_type& operator*() && { return base_type::operator*(); }
};

namespace _impl {
    template <typename AlwaysVoid, typename>
    struct fn_traits;
}

/// Determine traits of an arbitrary function/lambda/functor
/*!
 * Doesn't work with generic lambdas and function objects that have
 * operator() overloaded.
 * \sa
 * https://stackoverflow.com/questions/7943525/is-it-possible-to-figure-out-the-parameter-type-and-return-type-of-a-lambda#7943765
 */
template <typename T>
struct function_traits
    : public _impl::fn_traits<void, std::remove_reference_t<T>> {};

// Specialisation for a function
template <typename ReturnT, typename... ArgTs>
struct function_traits<ReturnT(ArgTs...)> {
    using return_type = ReturnT;
    using arg_types = std::tuple<ArgTs...>;
    // Doesn't (and there's no plan to make it) work for "classic"
    // member functions (i.e. outside of functors).
    // See also the comment for wrap_in_function() below
    using function_type = std::function<ReturnT(ArgTs...)>;
};

namespace _impl {
    // Specialisation for function objects with (non-overloaded) operator()
    // (this includes non-generic lambdas)
    template <typename T>
    struct fn_traits<decltype(void(&T::operator())), T>
        : public fn_traits<void, decltype(&T::operator())> {};

    // Specialisation for a member function
    template <typename ReturnT, typename ClassT, typename... ArgTs>
    struct fn_traits<void, ReturnT (ClassT::*)(ArgTs...)>
        : function_traits<ReturnT(ArgTs...)> {};

    // Specialisation for a const member function
    template <typename ReturnT, typename ClassT, typename... ArgTs>
    struct fn_traits<void, ReturnT (ClassT::*)(ArgTs...) const>
        : function_traits<ReturnT(ArgTs...)> {};
} // namespace _impl

template <typename FnT>
using fn_return_t = typename function_traits<FnT>::return_type;

template <typename FnT, int ArgN = 0>
using fn_arg_t =
    std::tuple_element_t<ArgN, typename function_traits<FnT>::arg_types>;

// TODO: get rid of it as soon as Apple Clang gets proper deduction guides
//       for std::function<>
//       ...or consider using QtPrivate magic used by QObject::connect()
//       since wrap_in_function() is actually made for qt_connection_util.h
//       ...for inspiration, also check a possible std::not_fn implementation at
//       https://en.cppreference.com/w/cpp/utility/functional/not_fn
template <typename FnT>
inline auto wrap_in_function(FnT&& f)
{
    return typename function_traits<FnT>::function_type(std::forward<FnT>(f));
}

inline auto operator"" _ls(const char* s, std::size_t size)
{
    return QLatin1String(s, int(size));
}

/** An abstraction over a pair of iterators
 * This is a very basic range type over a container with iterators that
 * are at least ForwardIterators. Inspired by Ranges TS.
 */
template <typename ArrayT>
class Range {
    // Looking forward to C++20 ranges
    using iterator = typename ArrayT::iterator;
    using const_iterator = typename ArrayT::const_iterator;
    using size_type = typename ArrayT::size_type;

public:
    constexpr Range(ArrayT& arr) : from(std::begin(arr)), to(std::end(arr)) {}
    constexpr Range(iterator from, iterator to) : from(from), to(to) {}

    constexpr size_type size() const
    {
        Q_ASSERT(std::distance(from, to) >= 0);
        return size_type(std::distance(from, to));
    }
    constexpr bool empty() const { return from == to; }
    constexpr const_iterator begin() const { return from; }
    constexpr const_iterator end() const { return to; }
    constexpr iterator begin() { return from; }
    constexpr iterator end() { return to; }

private:
    iterator from;
    iterator to;
};

/** A replica of std::find_first_of that returns a pair of iterators
 *
 * Convenient for cases when you need to know which particular "first of"
 * [sFirst, sLast) has been found in [first, last).
 */
template <typename InputIt, typename ForwardIt, typename Pred>
inline std::pair<InputIt, ForwardIt> findFirstOf(InputIt first, InputIt last,
                                                    ForwardIt sFirst,
                                                    ForwardIt sLast, Pred pred)
{
    for (; first != last; ++first)
        for (auto it = sFirst; it != sLast; ++it)
            if (pred(*first, *it))
                return std::make_pair(first, it);

    return std::make_pair(last, sLast);
}

/** Convert what looks like a URL or a Matrix ID to an HTML hyperlink */
void linkifyUrls(QString& htmlEscapedText);

/** Sanitize the text before showing in HTML
 *
 * This does toHtmlEscaped() and removes Unicode BiDi marks.
 */
QString sanitized(const QString& plainText);

/** Pretty-print plain text into HTML
 *
 * This includes HTML escaping of <,>,",& and calling linkifyUrls()
 */
QString prettyPrint(const QString& plainText);

/** Return a path to cache directory after making sure that it exists
 *
 * The returned path has a trailing slash, clients don't need to append it.
 * \param dir path to cache directory relative to the standard cache path
 */
QString cacheLocation(const QString& dirName);

/** Hue color component of based of the hash of the string.
 *
 * The implementation is based on XEP-0392:
 * https://xmpp.org/extensions/xep-0392.html
 * Naming and range are the same as QColor's hueF method:
 * https://doc.qt.io/qt-5/qcolor.html#integer-vs-floating-point-precision
 */
qreal stringToHueF(const QString& s);

/** Extract the serverpart from MXID */
QString serverPart(const QString& mxId);
} // namespace Quotient
