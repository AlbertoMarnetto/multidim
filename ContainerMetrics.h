#ifndef MULTIDIM_CONTAINER_METRICS_H
#define MULTIDIM_CONTAINER_METRICS_H

#include <iterator> // begin, end
#include <string> // DefaultCustomScalar, to_string
#include <vector> // computeContainerGeometry
#include <type_traits> // is_lvalue_reference, enable_if etc.

// Since this is intended as a general-purpose module,
// I prefer not to #include any Boost library

namespace multidim {
/** \addtogroup multidim Multidim library
 */

/** \defgroup basic_templates Basic templates
 * \ingroup multidim
 * \brief Helper templates extending the `<type_traits> library
 */

/** \defgroup container_metrics ContainerMetrics
 * \ingroup multidim
 * \brief Helper templates extending the `<type_traits>` library
 */

using std::begin;
using std::end;

//***************************************************************************
// isFirstElementOf
//***************************************************************************
/** \brief Returns `true` is `element` is the first element of `container`
 * \ingroup basic_templates
 * \param element
 * \param container
 */
template <typename T, typename Container>
inline bool isFirstElementOf(const T& element, const Container& container) {
    using std::begin;
    return &element == &*begin(container);
}

// **************************************************************************
// size
// **************************************************************************
/** \fn auto size(const T& t) -> size_t
 * \ingroup basic_templates
 * \brief Returns the size of `t`
 * \param c A container
 */
template<typename T>
static auto _size(const T& t, long) -> decltype(begin(t), end(t), size_t()) {
    return std::distance(begin(t), end(t));
}
template<typename T>
static auto _size(const T& t, int) -> decltype(t.size(), size_t()) {
    return t.size();
}

// Entry point
template<typename T>
static auto size(const T& t) -> size_t {
    return _size(t, 0);
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
 * `true` if `begin(T)` and `end(T)` are iterators of the same type
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

// Only defined if T is a range
template <typename T>
struct IteratorType<T, true> {
    using type = decltype(begin(std::declval<T&>()));
};

//***************************************************************************
// PointsToRange
//***************************************************************************
/**
 * \brief Provides the member constant `value`, which is
 * `true` if `T` is an iterator whose pointee is a range,
 * `false` otherwise
 * \ingroup basic_templates
 * \param T (typename)
*/

template <typename T>
struct PointsToRange {
    template <typename T1 = T>
    static constexpr auto _pointsToRange(int)
    -> decltype(
        std::declval<typename std::iterator_traits<T1>::reference_type>(),
        bool()
    ) {
        return IsRange<T1>::value;
    }

    static constexpr auto _pointsToRange(long) -> decltype(bool()) {
        return false;
    }

    static constexpr bool value = _pointsToRange(0);
};

//***************************************************************************
// CustomScalarTrait
//***************************************************************************
/** \defgroup custom_scalar Implementations for"CustomScalarTrait"
 * \brief Implementations for the "CustomScalarTrait" template argument of
 * the library main classes and functions.
 * These trait template indicates which classes should be considered scalars,
 * even if they are containers.
 */

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
 * \ingroup custom_scalar
 * \param T (typename, as template parameter)
 */
template<typename T>
struct NoCustomScalars{static constexpr bool isCustomScalar = false; /*for any T*/};


// **************************************************************************
// IsScalar
/** \brief Provides the member constant `value`, which is
 * `true` if `T` is a scalar (i.e. if it is not a container,
 * or if it defined as custom scalar by the trait)
 * \ingroup container_metrics
 * \param T (typename, as template parameter)
 * \note a variant accepting a concrete instance of type `T` is provided for easier use at runtime
 */
// **************************************************************************
template <template<typename> class CustomScalarTrait = NoCustomScalars, typename T = void>
struct IsScalar {
    static constexpr bool value = !IsRange<T>::value || CustomScalarTrait<T>::isCustomScalar;
};

// **************************************************************************
// IteratorScalarType
// **************************************************************************
/** \brief Provides the member typedef `type` as the reference type
 * of the scalar value in a (possibly nested) iterator,
 * e.g. `vector<set<int>>::iterator` --> `int&`
 * \ingroup container_metrics
 * \param CustomScalarTrait (trait template)
 * \param Iterator (typename)
 */
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename Iterator = void,
    bool = IsScalar<CustomScalarTrait, typename std::iterator_traits<Iterator>::value_type>::value
>
struct IteratorScalarType;

// Specialization for Iterator pointing to scalar: just dereference
template <template<typename> class CustomScalarTrait, typename Iterator>
struct IteratorScalarType<CustomScalarTrait, Iterator, true> {
    using reference = typename std::iterator_traits<Iterator>::reference;
    using type = typename std::remove_reference<reference>::type;
};

// Specialization for Iterator pointing to a not scalar: recurse to next level
template <template<typename> class CustomScalarTrait, typename Iterator>
struct IteratorScalarType<CustomScalarTrait, Iterator, false> {
    using reference =
        typename IteratorScalarType<
            CustomScalarTrait,
            decltype(begin(
                std::declval<typename std::iterator_traits<Iterator>::reference>()
            ))
        >::reference;
    using type = typename std::remove_reference<reference>::type;
};

//***************************************************************************
// Dimensionality
//***************************************************************************
/** \brief Provides the `size_t` member `value` as the dimensionality,
 * i.e. the number of levels in a multilevel range
 * \ingroup container_metrics
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

// Function version
/** \brief Returns the dimensionality,
 * i.e. the number of levels in a multilevel range (or 0 if the argument is a custom scalar)
 * \ingroup container_metrics
 * \param CustomScalarTrait (trait template)
 * \param argument : the object whose dimensionality will be returned
 */
template <template<typename> class CustomScalarTrait = NoCustomScalars, typename T = void>
static constexpr size_t dimensionality(const T& arg) {
    return Dimensionality<CustomScalarTrait, const T&>::value;
}

// **************************************************************************
// bounds
// **************************************************************************
/** \brief Determines the bounds of an object of an object, i.e. its maximum
 * size in all its subdimensions
 * \ingroup container_metrics
 */

template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename T = void,
    typename std::enable_if<(Dimensionality<CustomScalarTrait, T>::value == 1), long>::type = 0xBEEF
>
std::vector<size_t> bounds(const T& argument) {
    return std::vector<size_t>{size(argument),};
}

template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename T = void,
    typename std::enable_if<(Dimensionality<CustomScalarTrait, T>::value >= 2), int>::type = 0xBEEF
>
std::vector<size_t> bounds(const T& argument) {
    constexpr auto dimensionalityT = Dimensionality<CustomScalarTrait, T>::value;

    std::vector<size_t> containerBounds(dimensionalityT);

    // The outermost bound is obviously the size of the container
    containerBounds[0] = size(argument);

    // The other bounds are computed from the subcontainers info
    for (const auto& subContainer : argument) {
        const auto& subContainerBounds = bounds<CustomScalarTrait>(subContainer);

        if (subContainerBounds.size() != dimensionalityT-1) {
            throw std::runtime_error("bounds : encountered Container with strange dimensionality");
        }

        // Set tentative bounds in the subdimensions as bounds of the first subcontainer
        if (isFirstElementOf(subContainer, argument)) {
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

// **************************************************************************
// scalarSize
// **************************************************************************
/** \brief Returns the total number of scalar elements in a
 * \ingroup container_metrics
 */

// Specialization for scalar T
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename T = void,
    typename std::enable_if<IsScalar<CustomScalarTrait, T>::value, bool>::type = true
>
size_t scalarSize(const T& argument) {
    return 1;
}

// Specialization for non-scalar T
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename T = void,
    typename std::enable_if<!IsScalar<CustomScalarTrait, T>::value, int>::type = 0xDEADBEEF
>
size_t scalarSize(const T& argument) {
    constexpr auto dimensionalityT = dimensionality<CustomScalarTrait>(argument);

    if (dimensionalityT == 1) return size(argument);

    // assert(dimensionalityT >= 2);
    size_t result = 0;

    size_t containerScalarSize = 0;
    for (const auto& subContainer : argument) {
        result += scalarSize<CustomScalarTrait>(subContainer);
    }

    return result;
}


} // namespace multidim

#endif // MULTIDIM_CONTAINER_METRICS_H

