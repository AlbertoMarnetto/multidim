#ifndef MULTIDIM_CONTAINER_METRICS_H
#define MULTIDIM_CONTAINER_METRICS_H

#include <iterator> // begin, end
#include <string> // DefaultCustomScalar, to_string
#include <vector> // computeContainerGeometry
#include <type_traits> // is_lvalue_reference, enable_if etc.

// Since this is intended as a general-purpose module,
// I prefer not to #include any Boost library

namespace multidim {

/** \defgroup basic_templates Basic templates
 * \brief Helper templates extending the `<type_traits> library
 */

/** \defgroup utility_functions Functions
 * \brief Utility functions for multilevel containers
 */

/** \defgroup custom_scalars Implementations for "CustomScalarTrait"
 * \brief Implementations for the "CustomScalarTrait" template argument of
 * the library main classes and functions.
 * Each implementation indicates which classes should be considered scalars,
 * even if they are containers.
 */

using std::begin;
using std::end;

constexpr size_t NO_VALUE = static_cast<size_t>(-1);

// **************************************************************************
// size
// **************************************************************************
/** \fn auto size(const Container& container) -> size_t
 * \ingroup basic_templates
 * \brief Returns the size of `container`
 * \param container A container (is is only required to support `begin()` and `end()`)
 */
template<typename Container>
static auto _size(const Container& container, long) -> decltype(begin(container), end(container), size_t()) {
    return std::distance(begin(container), end(container));
}
template<typename Container>
static auto _size(const Container& container, int) -> decltype(container.size(), size_t()) {
    return container.size();
}

// Entry point
template<typename Container>
static auto size(const Container& container) -> size_t {
    return _size(container, 0);
}

// **************************************************************************
// Trivial tag classes
// **************************************************************************
/** \brief Trivial tag class to indicate the seek direction of an iterator
 * \ingroup basic_templates
 */
struct Forward {};
/** \brief Trivial tag class to indicate the seek direction of an iterator
 * \ingroup basic_templates
 */
struct Backward {};

//***************************************************************************
// IsRange
//***************************************************************************
/**
 * \brief Provides the member constant `value`, which is
 * `true` if `begin(T)` and `end(T)` return the same type,
 * which must be an iterator class
 * `false` otherwise
 * \ingroup basic_templates
 * \param T (typename)
 * \note Here a "Range" is defined as any class supporting begin()
*/

template <typename T>
struct IsRange {
    template <typename T1 = T>
    static constexpr auto _isRange(int)
    -> decltype(
        begin(std::declval<T1&>()),
        end(std::declval<T1&>()),
        std::declval<typename std::iterator_traits<decltype(begin(std::declval<T1&>()))>::value_type>(),
        bool()
    ) {
        return std::is_same<decltype(begin(std::declval<T1&>())), decltype(end(std::declval<T1&>()))>::value;
    }

    static constexpr auto _isRange(long) -> decltype(bool()) {
        return false;
    }

    static constexpr bool value = _isRange(0);
};

//***************************************************************************
// IteratorType
//***************************************************************************
/**
 * \brief Provides the member typedef `type`
 * as the type of `begin(T)` and `end(T)`
 * \ingroup basic_templates
 * \param T (typename)
 */
template <typename T, bool = IsRange<T>::value>
struct IteratorType;

// Only defined if T supports `begin`
template <typename T>
struct IteratorType<T, true> {
    using type = decltype(begin(std::declval<T&>()));
};

//***************************************************************************
// PointsToRange
//***************************************************************************
/**
 * \brief Provides the member constant `value`, which is
 * `true` if `Iterator` is an iterator whose pointee is a range,
 * `false` otherwise
 * \ingroup basic_templates
 * \param Iterator (typename)
*/

template <typename Iterator>
struct PointsToRange {
    template <typename Iterator1 = Iterator>
    static constexpr auto _pointsToRange(int)
    -> decltype(
        std::declval<typename std::iterator_traits<Iterator1>::reference_type>(),
        bool()
    ) {
        return IsRange<Iterator1>::value;
    }

    static constexpr auto _pointsToRange(long) -> decltype(bool()) {
        return false;
    }

    static constexpr bool value = _pointsToRange(0);
};

//***************************************************************************
// CustomScalarTrait
//***************************************************************************
/** \brief Defines `std::string` and `std::wstring` as scalar types.
 * \ingroup custom_scalars
 * \param T (typename, as template parameter)
 */
template<typename T>
struct StringsAsScalars {
    static constexpr bool isCustomScalar =
           std::is_same<typename std::decay<T>::type, std::string>::value
        || std::is_same<typename std::decay<T>::type, std::wstring>::value
    ;
};

/** \brief Defines no scalar types.
 * \ingroup custom_scalars
 * \param T (typename, as template parameter)
 */
template<typename T>
struct NoCustomScalars{static constexpr bool isCustomScalar = false; /*for any T*/};


// **************************************************************************
// IsScalar
/** \brief Provides the member constant `value`, which is
 * `true` if `T` is a scalar (i.e. if it is not a container,
 * or if it defined as custom scalar by the trait)
 * \ingroup basic_templates
 * \param T (typename, as template parameter)
 */
// **************************************************************************
template <template<typename> class CustomScalarTrait = NoCustomScalars, typename T = void>
struct IsScalar {
    static constexpr bool value = !IsRange<T>::value || CustomScalarTrait<T>::isCustomScalar;
};

// **************************************************************************
// IteratorScalarType
// **************************************************************************
/** \brief Provides the member typedef `type` and `reference`
 * as the value and reference scalar type
 * in a (possibly nested) iterator,
 * e.g. `vector<set<int>>::iterator` --> `type:int, reference = int&`
 * \ingroup basic_templates
 * \param CustomScalarTrait (trait template)
 * \param Iterator (typename)
 */
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename Iterator = void,
    bool = IsScalar<CustomScalarTrait, typename std::iterator_traits<Iterator>::value_type>::value
>
struct IteratorScalarType;

// Specialization for Iterator pointing to scalar
template <template<typename> class CustomScalarTrait, typename Iterator>
struct IteratorScalarType<CustomScalarTrait, Iterator, true> {
    using reference = typename std::iterator_traits<Iterator>::reference;
    using type = typename std::iterator_traits<Iterator>::value_type;
};

// Specialization for Iterator pointing to a not scalar: recurse to next level
template <template<typename> class CustomScalarTrait, typename Iterator>
struct IteratorScalarType<CustomScalarTrait, Iterator, false> {
    using MyScalarType =
        IteratorScalarType<
            CustomScalarTrait,
            decltype(begin(
                std::declval<typename std::iterator_traits<Iterator>::reference>()
            ))
        >;

    using reference = typename MyScalarType::reference;
    using type = typename MyScalarType::type;
};

//***************************************************************************
// Dimensionality
//***************************************************************************
/** \brief Provides the `size_t` member `value` as the dimensionality of a type
 * \ingroup basic_templates
 * \param CustomScalarTrait (trait template)
 * \param T (typename)
 */
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename T = void,
    bool = IsScalar<CustomScalarTrait, T>::value
>
struct Dimensionality;

// Specialization for scalar T
template <template<typename> class CustomScalarTrait, typename T>
struct Dimensionality<CustomScalarTrait, T, true> {
    static constexpr size_t value = 0;
};

// Specialization for non-scalar T
template <template<typename> class CustomScalarTrait, typename T>
struct Dimensionality<CustomScalarTrait, T, false> {
    static constexpr size_t value = 1
        + Dimensionality<
            CustomScalarTrait,
            typename std::iterator_traits<typename IteratorType<T>::type>::reference
        >::value;
};

// Version for ranges
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename Iterator = void
>
struct DimensionalityRange {
    static constexpr size_t value = 1+Dimensionality<CustomScalarTrait, typename std::iterator_traits<Iterator>::reference>::value;
};


// Function versions
/** \brief Returns the dimensionality of the type of the argument
 * (or 0 if such type is a scalar)
 * \ingroup utility_functions
 * \param CustomScalarTrait (trait template)
 * \param argument the object whose dimensionality will be returned
 */
template <template<typename> class CustomScalarTrait = NoCustomScalars, typename T = void>
constexpr size_t dimensionality(T const& arg) {
    return Dimensionality<CustomScalarTrait, T const&>::value;
}

/** \brief Returns the dimensionality of the type of the range
 * \ingroup utility_functions
 * \param CustomScalarTrait (trait template)
 * \param first, last the range whose dimensionality will be returned
 */
template <template<typename> class CustomScalarTrait = NoCustomScalars, typename Iterator = void>
constexpr size_t dimensionality(Iterator const& first, Iterator const& last) {
    return DimensionalityRange<CustomScalarTrait, Iterator>::value;
}

// **************************************************************************
// bounds
// **************************************************************************
/** \fn bounds(T const& argument);
 * \brief Returns a `vector<size_t>` containing the bounds of an object,
 * i.e. its maximum size in all its subdimensions (or an empty vector
 * is the argument is a scalar)
 * \ingroup utility_functions
 * \param CustomScalarTrait (trait template)
 * \param argument the object whose bounds will be returned
 */

/** \fn bounds(Iterator const first, Iterator const last, size_t precomputedDistance = NO_VALUE);
 * \brief Returns a `vector<size_t>` containing the bounds of a range,
 * i.e. its maximum size in all its subdimensions
 * \ingroup utility_functions
 * \param CustomScalarTrait (trait template)
 * \param first, last  the range whose bounds will be returned
 * \param precomputedDistance (optional) the value of (last - first)
 */

// Version for scalars
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename T = void,
    typename std::enable_if<(true == IsScalar<CustomScalarTrait, T>::value), long>::type = 0xBEEF
>
std::vector<size_t> bounds(T const& argument) {
    return std::vector<size_t> {};
}

// Forward-declare version for containers
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename T = void,
    typename std::enable_if<(false == IsScalar<CustomScalarTrait, T>::value), int>::type = 0xBEEF
>
std::vector<size_t> bounds(const T& argument);


// Version for monolevel ranges
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename Iterator = void,
    typename std::enable_if<
        (true == IsScalar<CustomScalarTrait, typename std::iterator_traits<Iterator>::value_type>::value),
        long
    >::type = 0xBEEF
>
std::vector<size_t> bounds(Iterator const first, Iterator const last, size_t precomputedDistance = NO_VALUE) {
    if (precomputedDistance != NO_VALUE) {
        return std::vector<size_t>{precomputedDistance,};
    } else {
        return std::vector<size_t>{static_cast<size_t>(std::distance(first, last)),};
    }
}

// Version for multilevel ranges
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename Iterator = void,
    typename std::enable_if<
        (false == IsScalar<CustomScalarTrait, typename std::iterator_traits<Iterator>::value_type>::value),
        int
    >::type = 0xBEEF
>
std::vector<size_t> bounds(Iterator const first, Iterator const last, size_t precomputedDistance = NO_VALUE) {
    constexpr auto rangeDimensionality = dimensionality<CustomScalarTrait>(first, last);

    std::vector<size_t> containerBounds(rangeDimensionality);

    // The outermost bound is the size of the range
    if (precomputedDistance != NO_VALUE) {
        containerBounds[0] = precomputedDistance;
    } else {
        containerBounds[0] = std::distance(first, last);
    }

    // The other bounds are computed from the subcontainers info
    for (auto it = first; it != last; ++it) {
        const auto& subContainer = *it;
        const auto& subContainerBounds = bounds<CustomScalarTrait>(subContainer);

        if (subContainerBounds.size() != rangeDimensionality-1) {
            throw std::runtime_error("bounds : encountered Container with strange dimensionality");
        }

        // Set tentative bounds in the subdimensions as bounds of the first subcontainer
        if (it == first) {
            std::copy(begin(subContainerBounds), end(subContainerBounds), begin(containerBounds)+1);
            continue;
        }

        // Then adapt it by looking the other subcontainers
        for (size_t dimension = 0; dimension < subContainerBounds.size(); ++dimension) {
            // if one of the other subcontainers is larger, enlarge the bounding box and set the "jagged" flag
            if (containerBounds[dimension+1] < subContainerBounds[dimension]) {
                containerBounds[dimension+1] = subContainerBounds[dimension];
            }
        }
    }

    return containerBounds;
}

// Version for containers
template <
    template<typename> class CustomScalarTrait,
    typename T,
    typename std::enable_if<(false == IsScalar<CustomScalarTrait, T>::value), int>::type
>
std::vector<size_t> bounds(T const& argument) {
    return bounds<CustomScalarTrait>(begin(argument), end(argument), size(argument));
}

// **************************************************************************
// scalarSize
// **************************************************************************
/** \fn scalarSize(T const& argument);
 * \brief Determines the number of scalar elements contained in the argument
 * (or 1 is the argument is a scalar)
 * \ingroup utility_functions
 * \param CustomScalarTrait (trait template)
 * \param argument the object whose number of scalar elements will be returned
 */

/** \fn scalarSize(Iterator const first, Iterator const last, size_t precomputedDistance = NO_VALUE);
 * \brief Determines the bounds of a range, i.e. its maximum
 * size in all its subdimensions
 * \ingroup utility_functions
 * \param CustomScalarTrait (trait template)
 * \param first, last the range whose number of scalar elements will be returned
 * \param precomputedDistance (optional) the value of (last - first)
 */

// Version for scalars
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename T = void,
    typename std::enable_if<(true == IsScalar<CustomScalarTrait, T>::value), long>::type = true
>
constexpr size_t scalarSize(T const& argument) {
    return 1;
}

// Forward-declare version for containers
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename T = void,
    typename std::enable_if<(false == IsScalar<CustomScalarTrait, T>::value), int>::type = 0xBEEF
>
size_t scalarSize(const T& argument);


// Version for monolevel range
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename Iterator = void,
    typename std::enable_if<
        (true == IsScalar<CustomScalarTrait, typename std::iterator_traits<Iterator>::value_type>::value),
        long
    >::type = 0xBEEF
>
size_t scalarSize(Iterator const first, Iterator const last, size_t precomputedDistance = NO_VALUE) {
    if (precomputedDistance != NO_VALUE) {
        return precomputedDistance;
    } else {
        return static_cast<size_t>(std::distance(first, last));
    }
}

// Version for multilevel range
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename Iterator = void,
    typename std::enable_if<
        (false == IsScalar<CustomScalarTrait, typename std::iterator_traits<Iterator>::value_type>::value),
        int
    >::type = 0xBEEF
>
size_t scalarSize(Iterator const first, Iterator const last, size_t precomputedDistance = NO_VALUE) {
    size_t result = 0;

    size_t rangeScalarSize = 0;
    for (auto it = first; it != last; ++it) {
        result += scalarSize<CustomScalarTrait>(*it);
    }

    return result;
}

// Version for containers
template <
    template<typename> class CustomScalarTrait,
    typename T,
    typename std::enable_if<(false == IsScalar<CustomScalarTrait, T>::value), int>::type
>
size_t scalarSize(T const& argument) {
    return scalarSize<CustomScalarTrait>(begin(argument), end(argument), size(argument));
}


} // namespace multidim

#endif // MULTIDIM_CONTAINER_METRICS_H

