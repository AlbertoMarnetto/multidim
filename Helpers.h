#ifndef MULTIDIM_HELPERS_H
#define MULTIDIM_HELPERS_H

#include <iterator> // begin, end
#include <type_traits> // is_lvalue_reference, enable_if etc.

namespace multidim {

using std::begin;
using std::end;

/** \addtogroup multidim Multidim library
 * \brief Functions and classes to work with nested containers of arbitrary dimensionality
 * @{
 */

/** \addtogroup basic_templates Basic templates
 * \brief Helper templates extending the `<type_traits> library
 * @{
 */

//***************************************************************************
// hasContainedType
//***************************************************************************
/** \fn hasContainedType<T>
* \param T (typename, as template parameter)
* \return
* - \c true if `*begin(t)` is a valid expression of reference type for an object `t` of type `T`,
* - \c false otherwise
* \note  For uniformity it would have been better to make this a struct with `::value`,
* but SFINAE works better with a function
*/
template <typename T>
constexpr auto _hasContainedType(int) -> decltype(*begin(std::declval<typename std::add_lvalue_reference<T>::type>()), bool()) {
    return std::is_lvalue_reference<decltype(*begin(std::declval<typename std::add_lvalue_reference<T>::type>()))>::value;
    //return true;
}
template <typename T>
constexpr auto _hasContainedType(long) -> decltype(bool()) {
    return false;
}
template <typename T>
constexpr bool hasContainedType() {return _hasContainedType<T>(0);}

//***************************************************************************
// hasPointedType
//***************************************************************************
/** \fn hasPointedType<T>
* \param T (typename, as template parameter)
* \return
* - \c true if `*t` is a valid expression of reference type for an object `t` of type `T`,
* - \c false otherwise
* \note  For uniformity it would have been better to make this a struct with `::value`,
* but SFINAE works better with a function
*/
template <typename T>
constexpr auto _hasPointedType(int) -> decltype(*std::declval<T>(), bool()) {
    return std::is_lvalue_reference<decltype(*std::declval<T>())>::value;
    //return true;
}
template <typename T>
constexpr auto _hasPointedType(long) -> decltype(bool()) {
    return false;
}
template <typename T>
constexpr bool hasPointedType() {return _hasPointedType<T>(0);}

//***************************************************************************
// IteratorType
//***************************************************************************
/** \brief Provides the member typedef `type` as the type of `begin(T)`, as long as `*begin(t)` exists and is lvalue
 * \param T (typename, as template parameter)
 */
template <typename T, typename = typename std::enable_if<hasContainedType<T>()>::type>
struct IteratorType {
    // To correctly manage arrays we must transform into reference type
    using type = decltype(begin(std::declval<typename std::add_lvalue_reference<T>::type>()));
};

//***************************************************************************
// ContainedType
//***************************************************************************
/** \brief Provides the member typedef `type` as the type of `*begin(T)`, stripped of the reference
 * \param T (typename, as template parameter)
 */
template <typename T, typename = typename std::enable_if<hasContainedType<T>()>::type>
struct ContainedType {
    // Get the iterator type and dereference it
    using type = typename std::remove_reference<decltype(*(std::declval<typename IteratorType<T>::type>()))>::type;
};

//***************************************************************************
// PointedType
//***************************************************************************
/** \brief Provides the member typedef `type` as the type of `*T`, stripped of the reference
 * \param T (typename, as template parameter)
 */
template <typename T, typename = typename std::enable_if<hasPointedType<T>()>::type>
struct PointedType {
    // Get the iterator type and dereference it
    using type = typename std::remove_reference<decltype(*(std::declval<T>()))>::type;
};

//***************************************************************************
// isFirstElementOf
//***************************************************************************
/** \brief Returns `true` is `element` is the first element of `container`
 * \param element
 * \param container
 */
template <typename T, typename Container>
inline bool isFirstElementOf(const T& element, const Container& container) {
    using std::begin;
    return &element == &*begin(container);
}

/** @} basic_templates */

} // namespace multidim

#endif // MULTIDIM_HELPERS_H

