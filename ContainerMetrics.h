#ifndef MULTIDIM_CONTAINER_METRICS_H
#define MULTIDIM_CONTAINER_METRICS_H

#include <iterator> // begin, end
#include <string> // DefaultCustomScalar, to_string
#include <vector> // computeContainerGeometry
#include <type_traits> // is_lvalue_reference, enable_if etc.

#include "Basics.h"

// Since this is intended as a general-purpose module,
// I prefer not to #include any Boost library

namespace multidim {
/** \addtogroup multidim Multidim library
 */

/** \addtogroup container_metrics ContainerMetrics
 * \brief Helper templates extending the `<type_traits> library
 */

// **************************************************************************
// ContainerMetrics
// **************************************************************************
/** \brief Offers a series of static functions to deal with multidimensional containers
 * \ingroup container_metrics
 * \param IsCustomScalar (template parameter): templated `struct` taking a typename as
 * template parameter and providing a bool member `value` indicating if such type is to be considered
 * a scalar type even if it offers a `begin()` function
 */
template <template<typename> class IsCustomScalar> struct ContainerMetrics {
    // **************************************************************************
    // isContainer
    /** \brief Returns `true` if T is a container (i.e. if it has a ContainedType
     * and `IsCustomScalar<T>::value == false`)
     * \ingroup container_metrics
     * \param T (typename, as template parameter)
     * \note a variant accepting a concrete instance of type `T` is provided for easier use at runtime
     */
    // **************************************************************************
    template <typename T>
    static constexpr bool isContainer() {
        return hasContainedType<T>() && !(IsCustomScalar<T>::value);
    }
    template <typename T>
    static constexpr bool isContainer(T&&) {
        return isContainer<T>();
    }

    // **************************************************************************
    // ScalarReferenceType
    // **************************************************************************
    /** \brief Provides the member typedef `type` as the reference type
     * of the scalar value in a (possibly nested) container,
     * e.g. `vector<set<int>>` --> `int&`
     * \ingroup container_metrics
     * \param T (typename, as template parameter)
     */
    template <typename T, bool = ContainerMetrics::isContainer<T>()>
    struct ScalarReferenceType {
        // If T is not a container, it is the scalar type we looked for
        using type = typename std::add_lvalue_reference<T>::type;
    };

    template <typename T>
    struct ScalarReferenceType<T, true> {
        // If T is a container, call ScalarReferenceType on the contained type
        using type = typename ScalarReferenceType<typename ContainedType<T>::type>::type;
    };

    // **************************************************************************
    // ScalarValueType
    // **************************************************************************
    /** \brief Provides the member typedef `type` as the value type
     * of the scalar value in a (possibly nested) container,
     * e.g. `vector<set<int>>` --> `int`
     * \ingroup container_metrics
     * \param T (typename, as template parameter)
     */

    template <typename T>
    struct ScalarValueType {
        // If T is a container, call ScalarReferenceType on the contained type
        using type = typename std::remove_reference<typename ScalarReferenceType<T>::type>::type;
    };



    // **************************************************************************
    // dimensionality
    // **************************************************************************
    /** \fn constexpr size_t dimensionality()
     * \brief Returns the dimensionality of the container,
     * i.e. how many times we must dereference to get to its ScalarReferenceType
     * \ingroup container_metrics
     * \param T (typename, as template parameter)
     * \note a variant accepting a concrete instance of type `T` is provided for easier use at runtime
     */

    template <typename T, bool = ContainerMetrics::isContainer<T>()>
    struct Dimensionality {
        static constexpr size_t value = 0;
    };
    template <typename T>
    struct Dimensionality<T, true> {
        static constexpr size_t value = 1+Dimensionality<typename ContainedType<T>::type>::value;
    };

    // Entry point
    template<typename T>
    static constexpr size_t dimensionality() {
        return Dimensionality<const T&>::value;
    }
    template<typename T>
    static constexpr size_t dimensionality(const T&) {
        return Dimensionality<const T&>::value;
    }


    // **************************************************************************
    // computeContainerGeometry
    // **************************************************************************
    /** \brief Determines information about bounds and number of scalar elements
     * of a (possibly multidimensional) container
     * \ingroup container_metrics
     */
    struct ContainerGeometry {
        /** \brief  maximum value that the i-th index can assume in an expression like
         * `container[i1][i2][i3][...]);`
         * The dimensionality of the container is given by "bounds.size()" */
        std::vector<size_t> bounds;

        /** \brief  The number of elements of scalar type stored into a container.
         * \note If `numberOfElements` is equal to the product of the elements of `bounds`,
         * the container can be 1:1 mapped with a multidimensional array
         * Otherwise, at least an index tuple within the bounding sizes is not valid,
         * i.e. the container is jagged */
        size_t numberOfScalarElements;
    };

    template<typename Container>
    static auto _computeContainerGeometry(const Container& container, long) -> ContainerGeometry {
        // Specialization for non-containers (needed since the main overload invokes begin(container))
        // Must be left as first case, as the main overload calls it
        return ContainerGeometry{{}, 1};
    }

    template<typename Container>
    static auto _computeContainerGeometry(const Container& container, int) -> decltype(*begin(container), ContainerGeometry()) {
        if (dimensionality<Container>(container) == 0) return ContainerGeometry{{}, 1};
        if (dimensionality<Container>(container) == 1) return ContainerGeometry{{size(container),}, size(container)};

        ContainerGeometry containerGeometry{std::vector<size_t>(dimensionality(container)), 0};

        // The outermost bounding size is obviously the size of the container
        containerGeometry.bounds[0] = size(container);

        // The other bounds are computed from the subcontainers info
        for (const auto& subContainer : container) {
            const auto& subContainerGeometry = _computeContainerGeometry(subContainer, 0);  // http://herbsutter.com/2008/01/01/gotw-88-a-candidate-for-the-most-important-const/

            if (subContainerGeometry.bounds.size() != containerGeometry.bounds.size()-1) {
                throw std::runtime_error("computeContainerGeometry : encountered Container with strange dimensionality "
                    + std::to_string(subContainerGeometry.bounds.size()) + std::to_string(containerGeometry.bounds.size()-1)
                                         );
            }

            containerGeometry.numberOfScalarElements += subContainerGeometry.numberOfScalarElements;

            // Set tentative bounds in the subdimensions as bounds of the first subcontainer
            if (isFirstElementOf(subContainer, container)) {
                std::copy(begin(subContainerGeometry.bounds), end(subContainerGeometry.bounds), 1+begin(containerGeometry.bounds));
                continue;
            }

            // Then adapt it by looking the other subcontainers
            for (size_t dimension = 0; dimension < subContainerGeometry.bounds.size(); ++dimension) {
                // if one of the other subcontainers is larger, enlarge the bounding box and set the "jagged" flag
                if (containerGeometry.bounds[dimension+1] < subContainerGeometry.bounds[dimension]) {
                    containerGeometry.bounds[dimension+1] = subContainerGeometry.bounds[dimension];
                }
            }
        }

        return containerGeometry;
    }

    /** \brief Returns the ContainerGeometry associated to a container
     * \ingroup container_metrics
     * \param container The container
     */
    template<typename Container>
    static auto computeContainerGeometry(const Container& container) -> ContainerGeometry {
        return _computeContainerGeometry<Container>(container, 0);
    }
};

//***************************************************************************
// Trait classes
//***************************************************************************
/** \defgroup custom_scalars "Custom scalar" traits
 * \brief Trait classes indicating which types should be considered scalars
 * To be used with Containermetrics, FlatView and BoxedView
 */

/** \brief Defines `std::string` and `std::wstring` as scalar types.
 * \ingroup custom_scalars
 * \param T (typename, as template parameter)
 */
template<typename T>
struct StringsAsScalars {
    static constexpr bool value =
           std::is_same<typename std::decay<T>::type, std::string>::value
        || std::is_same<typename std::decay<T>::type, std::wstring>::value
    ;
};

/** \brief Defines no scalar types.
 * \ingroup custom_scalars
 * \param T (typename, as template parameter)
 */
template<typename T>
struct NoCustomScalars{static constexpr bool value = false; /*for any T*/};

} // namespace multidim

#endif // MULTIDIM_CONTAINER_METRICS_H

