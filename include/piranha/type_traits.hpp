// Copyright 2019 Francesco Biscani (bluescarni@gmail.com)
//
// This file is part of the piranha library.
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef PIRANHA_TYPE_TRAITS_HPP
#define PIRANHA_TYPE_TRAITS_HPP

#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <mp++/detail/type_traits.hpp>

#include <piranha/config.hpp>

namespace piranha
{

// Lift the detection idiom from mp++.
using ::mppp::detected_t;
using ::mppp::is_detected;

template <template <class...> class Op, class... Args>
inline constexpr bool is_detected_v = is_detected<Op, Args...>::value;

// Handy alias.
template <typename T>
using remove_cvref_t = ::std::remove_cv_t<::std::remove_reference_t<T>>;

#if defined(PIRANHA_HAVE_CONCEPTS)

template <typename T, typename U>
PIRANHA_CONCEPT_DECL Same = ::std::is_same_v<T, U>;

#endif

// Detect if T and U, after the removal of reference and cv qualifiers, are the same type.
template <typename T, typename U>
using is_same_cvref = ::std::is_same<remove_cvref_t<T>, remove_cvref_t<U>>;

template <typename T, typename U>
inline constexpr bool is_same_cvref_v = is_same_cvref<T, U>::value;

#if defined(PIRANHA_HAVE_CONCEPTS)

template <typename T, typename U>
PIRANHA_CONCEPT_DECL SameCvref = is_same_cvref_v<T, U>;

#endif

// Detect C++ integral types, including GCC-style 128bit integers.
template <typename T>
using is_integral = ::std::disjunction<::std::is_integral<T>
#if defined(PIRANHA_HAVE_GCC_INT128)
                                       ,
                                       ::std::is_same<::std::remove_cv_t<T>, __int128_t>,
                                       ::std::is_same<::std::remove_cv_t<T>, __uint128_t>
#endif
                                       >;

template <typename T>
inline constexpr bool is_integral_v = is_integral<T>::value;

#if defined(PIRANHA_HAVE_CONCEPTS)

template <typename T>
PIRANHA_CONCEPT_DECL Integral = is_integral_v<T>;

#endif

#if defined(PIRANHA_HAVE_CONCEPTS)

// Concept for detecting C++ FP types.
template <typename T>
PIRANHA_CONCEPT_DECL FloatingPoint = ::std::is_floating_point_v<T>;

#endif

// Detect C++ arithmetic types, including GCC-style 128bit integers.
template <typename T>
using is_arithmetic = ::std::disjunction<is_integral<T>, ::std::is_floating_point<T>>;

template <typename T>
inline constexpr bool is_arithmetic_v = is_arithmetic<T>::value;

#if defined(PIRANHA_HAVE_CONCEPTS)

template <typename T>
PIRANHA_CONCEPT_DECL Arithmetic = is_arithmetic_v<T>;

#endif

#if defined(PIRANHA_HAVE_CONCEPTS)

// Concept for detecting const-qualified types.
template <typename T>
PIRANHA_CONCEPT_DECL Const = ::std::is_const_v<T>;

#endif

// Detect (possibly cv-qualified) signed types.
// Supports also 128bit integers.
template <typename T>
using is_signed = ::std::disjunction<::std::is_signed<T>
#if defined(PIRANHA_HAVE_GCC_INT128)
                                     ,
                                     ::std::is_same<::std::remove_cv_t<T>, __int128_t>
#endif
                                     >;

template <typename T>
inline constexpr bool is_signed_v = is_signed<T>::value;

#if defined(PIRANHA_HAVE_CONCEPTS)

template <typename T>
PIRANHA_CONCEPT_DECL Signed = is_signed_v<T>;

#endif

namespace detail
{

template <typename T, typename = void>
struct make_unsigned_impl : ::std::make_unsigned<T> {
    // NOTE: std::make_unsigned requires integrals but refuses bool:
    // https://en.cppreference.com/w/cpp/types/make_unsigned
    static_assert(!::std::is_same_v<bool, ::std::remove_cv_t<T>>,
                  "make_unsigned_t does not accept bool as input type.");
    static_assert(::std::is_integral_v<::std::remove_cv_t<T>> || ::std::is_enum_v<::std::remove_cv_t<T>>,
                  "make_unsigned_t works only on integrals or enumeration types.");
};

#if defined(PIRANHA_HAVE_GCC_INT128)

// NOTE: make_unsigned is supposed to preserve cv qualifiers, hence the non-trivial implementation.
template <typename T>
struct make_unsigned_impl<T,
                          ::std::enable_if_t<::std::disjunction_v<::std::is_same<::std::remove_cv_t<T>, __uint128_t>,
                                                                  ::std::is_same<::std::remove_cv_t<T>, __int128_t>>>> {
    using tmp_type = ::std::conditional_t<::std::is_const_v<T>, const __uint128_t, __uint128_t>;
    using type = ::std::conditional_t<::std::is_volatile_v<T>, volatile tmp_type, tmp_type>;
};

#endif

} // namespace detail

// Compute the corresponding unsigned type. Works on 128bit integers too.
template <typename T>
using make_unsigned_t = typename detail::make_unsigned_impl<T>::type;

#if defined(PIRANHA_HAVE_CONCEPTS)

template <typename T>
PIRANHA_CONCEPT_DECL DefaultConstructible = ::std::is_default_constructible_v<T>;

#endif

// Detect if type can be returned from a function.
// NOTE: constructability implies destructability:
// https://cplusplus.github.io/LWG/issue2116
template <typename T>
using is_returnable = ::std::disjunction<::std::is_same<::std::remove_cv_t<T>, void>, ::std::is_copy_constructible<T>,
                                         ::std::is_move_constructible<T>>;

template <typename T>
inline constexpr bool is_returnable_v = is_returnable<T>::value;

#if defined(PIRANHA_HAVE_CONCEPTS)

template <typename T>
PIRANHA_CONCEPT_DECL Returnable = is_returnable_v<T>;

#endif

namespace detail
{

// Small wrapper to fetch the min/max values of a builtin numerical type. Works on 128bit integrals as well.
template <typename T>
inline constexpr auto limits_minmax = ::std::tuple{::std::numeric_limits<T>::min(), ::std::numeric_limits<T>::max()};

#if defined(PIRANHA_HAVE_GCC_INT128)

inline constexpr auto max_int128_t = (((__int128_t(1) << 126) - 1) << 1) + 1;

template <>
inline constexpr auto limits_minmax<__int128_t> = ::std::tuple{-max_int128_t - 1, max_int128_t};

template <>
inline constexpr auto limits_minmax<__uint128_t> = ::std::tuple{__uint128_t(0), ~__uint128_t(0)};

#endif

// NOTE: std::remove_pointer_t removes the top level qualifiers of the pointer as well:
// http://en.cppreference.com/w/cpp/types/remove_pointer
// After removal of pointer, we could still have a type which is cv-qualified. Thus,
// we remove cv-qualifications after pointer removal.
template <typename T>
using is_char_pointer
    = ::std::conjunction<::std::is_pointer<T>, ::std::is_same<::std::remove_cv_t<::std::remove_pointer_t<T>>, char>>;

} // namespace detail

// Detect string-like types. As usual, cv qualifiers are ignored.
template <typename T>
using is_string_like = ::std::disjunction<
    // Is it std::string?
    ::std::is_same<::std::remove_cv_t<T>, ::std::string>,
    // Is it a char pointer?
    detail::is_char_pointer<T>,
    // Is it an array of chars?
    // NOTE: std::remove_cv_t does remove cv qualifiers from arrays.
    ::std::conjunction<::std::is_array<::std::remove_cv_t<T>>,
                       ::std::is_same<::std::remove_extent_t<::std::remove_cv_t<T>>, char>>,
    // Is it a string view?
    ::std::is_same<::std::remove_cv_t<T>, ::std::string_view>>;

template <typename T>
inline constexpr bool is_string_like_v = is_string_like<T>::value;

#if defined(PIRANHA_HAVE_CONCEPTS)

template <typename T>
PIRANHA_CONCEPT_DECL StringLike = is_string_like_v<T>;

#endif

namespace detail
{

template <typename T, typename U>
using add_t = decltype(::std::declval<T>() + ::std::declval<U>());

}

template <typename T, typename U = T>
using is_addable = ::std::conjunction<is_detected<detail::add_t, T, U>, is_detected<detail::add_t, U, T>,
                                      ::std::is_same<detected_t<detail::add_t, T, U>, detected_t<detail::add_t, U, T>>>;

template <typename T, typename U = T>
inline constexpr bool is_addable_v = is_addable<T, U>::value;

#if defined(PIRANHA_HAVE_CONCEPTS)

template <typename T, typename U = T>
PIRANHA_CONCEPT_DECL Addable = requires(T &&x, U &&y)
{
    ::std::forward<T>(x) + ::std::forward<U>(y);
    ::std::forward<U>(y) + ::std::forward<T>(x);
    requires Same<decltype(::std::forward<T>(x) + ::std::forward<U>(y)),
                  decltype(::std::forward<U>(y) + ::std::forward<T>(x))>;
};

#endif

namespace detail
{

template <typename T>
using preinc_t = decltype(++::std::declval<T>());

}

// Pre-incrementable type-trait.
template <typename T>
using is_pre_incrementable = is_detected<detail::preinc_t, T>;

template <typename T>
inline constexpr bool is_pre_incrementable_v = is_pre_incrementable<T>::value;

#if defined(PIRANHA_HAVE_CONCEPTS)

template <typename T>
PIRANHA_CONCEPT_DECL PreIncrementable = requires(T &&x)
{
    ++::std::forward<T>(x);
};

#endif

namespace detail
{

template <typename T, typename U>
using eq_t = decltype(::std::declval<T>() == ::std::declval<U>());

template <typename T, typename U>
using ineq_t = decltype(::std::declval<T>() != ::std::declval<U>());

} // namespace detail

// Equality-comparable type trait.
// NOTE: if the expressions above for eq/ineq return a type which is not bool,
// the decltype() will also check that the returned type is destructible.
template <typename T, typename U = T>
using is_equality_comparable = ::std::conjunction<::std::is_convertible<detected_t<detail::eq_t, T, U>, bool>,
                                                  ::std::is_convertible<detected_t<detail::eq_t, U, T>, bool>,
                                                  ::std::is_convertible<detected_t<detail::ineq_t, T, U>, bool>,
                                                  ::std::is_convertible<detected_t<detail::ineq_t, U, T>, bool>>;

template <typename T, typename U = T>
inline constexpr bool is_equality_comparable_v = is_equality_comparable<T, U>::value;

#if defined(PIRANHA_HAVE_CONCEPTS)

template <typename T, typename U = T>
PIRANHA_CONCEPT_DECL EqualityComparable = requires(T &&x, U &&y)
{
    {
        ::std::forward<T>(x) == ::std::forward<U>(y)
    }
    ->bool;
    {
        ::std::forward<U>(y) == ::std::forward<T>(x)
    }
    ->bool;
    {
        ::std::forward<T>(x) != ::std::forward<U>(y)
    }
    ->bool;
    {
        ::std::forward<U>(y) != ::std::forward<T>(x)
    }
    ->bool;
};

#endif

namespace detail
{

// Helpers for the detection of the typedefs in std::iterator_traits.
// Use a macro (yuck) to reduce typing.
#define PIRANHA_DECLARE_IT_TRAITS_TYPE(type)                                                                           \
    template <typename T>                                                                                              \
    using it_traits_##type = typename ::std::iterator_traits<T>::type;

PIRANHA_DECLARE_IT_TRAITS_TYPE(difference_type)
PIRANHA_DECLARE_IT_TRAITS_TYPE(value_type)
PIRANHA_DECLARE_IT_TRAITS_TYPE(pointer)
PIRANHA_DECLARE_IT_TRAITS_TYPE(reference)
PIRANHA_DECLARE_IT_TRAITS_TYPE(iterator_category)

#undef PIRANHA_DECLARE_IT_TRAITS_TYPE

// Detect the availability of std::iterator_traits on type It.
template <typename It>
using has_iterator_traits
    = ::std::conjunction<is_detected<it_traits_reference, It>, is_detected<it_traits_value_type, It>,
                         is_detected<it_traits_pointer, It>, is_detected<it_traits_difference_type, It>,
                         is_detected<it_traits_iterator_category, It>>;

// All standard iterator tags packed in a tuple.
inline constexpr ::std::tuple<::std::input_iterator_tag, ::std::output_iterator_tag, ::std::forward_iterator_tag,
                              ::std::bidirectional_iterator_tag, ::std::random_access_iterator_tag>
    all_it_tags;

// Type resulting from the dereferencing operation.
template <typename T>
using deref_t = decltype(*::std::declval<T>());

// Check if the type T derives from one of the standard iterator tags.
// NOTE: MSVC has issues with the pattern below, adopt another implementation.
#if defined(_MSC_VER)

// NOTE: default empty for hard error (the default implementation is unused).
template <typename, typename>
struct derives_from_it_tag_impl {
};

template <typename T, typename... Args>
struct derives_from_it_tag_impl<T, ::std::tuple<Args...>> : ::std::disjunction<::std::is_base_of<Args, T>...> {
    static_assert(sizeof...(Args) > 0u, "Invalid parameter pack.");
};

template <typename T>
using derives_from_it_tag = derives_from_it_tag_impl<T, ::std::remove_const_t<decltype(all_it_tags)>>;

#else

template <typename T>
struct derives_from_it_tag {
    static constexpr bool value
        = ::std::apply([](auto... tag) { return (... || ::std::is_base_of_v<decltype(tag), T>); }, all_it_tags);
};

#endif

} // namespace detail

// Detect iterator types.
template <typename T>
using is_iterator = ::std::conjunction<
    // Copy constr/ass, destructible.
    ::std::is_copy_constructible<T>, ::std::is_copy_assignable<T>, ::std::is_destructible<T>,
    // Lvalue swappable.
    ::std::is_swappable_with<::std::add_lvalue_reference_t<T>, ::std::add_lvalue_reference_t<T>>,
    // Valid std::iterator_traits.
    detail::has_iterator_traits<T>,
    // Lvalue dereferenceable.
    is_detected<detail::deref_t, ::std::add_lvalue_reference_t<T>>,
    // Lvalue preincrementable, returning T &.
    ::std::is_same<detected_t<detail::preinc_t, ::std::add_lvalue_reference_t<T>>, ::std::add_lvalue_reference_t<T>>,
    // Add a check that the iterator category is one of the standard ones
    // or at least derives from it. This allows Boost.iterator iterators
    // (which have their own tags) to satisfy this type trait.
    detail::derives_from_it_tag<detected_t<detail::it_traits_iterator_category, T>>>;

template <typename T>
inline constexpr bool is_iterator_v = is_iterator<T>::value;

#if defined(PIRANHA_HAVE_CONCEPTS)

template <typename T>
PIRANHA_CONCEPT_DECL Iterator = is_iterator_v<T>;

#endif

namespace detail
{

// The purpose of these bits is to check whether U correctly implements the arrow operator.
// A correct implementation will return a pointer, after potentially calling
// the operator recursively as many times as needed. See:
// http://stackoverflow.com/questions/10677804/how-arrow-operator-overloading-works-internally-in-c

// The expression x->m is either:
// - equivalent to (*x).m, if x is a pointer, or
// - equivalent to (x.operator->())->m otherwise. That is, if operator->()
//   returns a pointer, then the member "m" of the pointee is returned,
//   otherwise there's a recursion to call again operator->() on the returned
//   value.
// This type trait will extract the final pointer type whose pointee type
// contains the "m" member.
template <typename, typename = void>
struct arrow_operator_type {
};

// Handy alias.
template <typename T>
using arrow_operator_t = typename arrow_operator_type<T>::type;

// If T is a pointer (after ref removal), we don't need to do anything: the final pointer type
// will be T itself (unreffed).
template <typename T>
struct arrow_operator_type<T, ::std::enable_if_t<::std::is_pointer_v<::std::remove_reference_t<T>>>> {
    using type = ::std::remove_reference_t<T>;
};

// Type resulting from the invocation of the member function operator->().
template <typename T>
using mem_arrow_op_t = decltype(::std::declval<T>().operator->());

// T is not a pointer, it is a class whose operator->() returns some type U.
// We call again arrow_operator_type on that U: if that leads eventually to a pointer
// (possibly by calling this specialisation recursively) then we define that pointer
// as the internal "type" member, otherwise we will SFINAE out.
template <typename T>
struct arrow_operator_type<T, ::std::enable_if_t<is_detected_v<arrow_operator_t, mem_arrow_op_t<T>>>> {
    using type = arrow_operator_t<mem_arrow_op_t<T>>;
};

} // namespace detail

} // namespace piranha

#endif
