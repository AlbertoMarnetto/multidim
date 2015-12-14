#ifndef MULTIDIM_HELPERS_H
#define MULTIDIM_HELPERS_H

#include <iterator> // begin, end
#include <type_traits> // is_lvalue_reference, enable_if etc.

namespace multidim {

using std::begin;
using std::end;

/** \addtogroup basic_templates Basic templates
 * \brief Helper templates extending the `<type_traits> library
 */

//***************************************************************************
// hasContainedType
//***************************************************************************
/** \fn hasContainedType<T>
 * \ingroup basic_templates
 * \param T (typename, as template parameter)
 * \return
 * - \c true if `*begin(t)` is a valid expression of reference type for an object `t` of type `T`,
 * - \c false otherwise
 * \note For uniformity it would have been better to make this a struct with `::value`,
 * but SFINAE is simpler with a function
 * \note Will not work with `std::vector<bool>`, as it is not a container.
 * See http://www.gotw.ca/publications/mill09.htm, in particular the quote
 “a container<T>::reference must be a true reference (T&), never a proxy”
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
// IteratorType
//***************************************************************************
/** \brief Provides the member typedef `type` as the type of `begin(T)`, as long as `*begin(t)` exists and is lvalue
 * \ingroup basic_templates
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
 * \ingroup basic_templates
 * \param T (typename, as template parameter)
 */
template <typename T, typename = typename std::enable_if<hasContainedType<T>()>::type>
struct ContainedType {
    // Get the iterator type and dereference it
    using type = decltype(*(std::declval<typename IteratorType<T>::type>()));
};

//***************************************************************************
// hasPointedType
//***************************************************************************
/** \fn hasPointedType<T>
 * \ingroup basic_templates
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
// PointedType
//***************************************************************************
/** \brief Provides the member typedef `type` as the type of `*T`, stripped of the reference
 * \ingroup basic_templates
 * \param T (typename, as template parameter)
 */
template <typename T, typename = typename std::enable_if<hasPointedType<T>()>::type>
struct PointedType {
    // Get the iterator type and dereference it
    using type = decltype(*(std::declval<T>()));
};

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
/** \fn auto size(const Container& c) -> size_t
 * \ingroup basic_templates
 * \brief Returns the size of the container `c`
 * \param c A container
 */
template<typename Container>
static auto size(const Container& c, long) -> decltype(begin(c), end(c), size_t()) {
    return std::distance(begin(c), end(c));
}
template<typename Container>
static auto size(const Container& c, int) -> decltype(c.size(), size_t()) {
    return c.size();
}

// Entry point
template<typename Container>
static auto size(const Container& c) -> size_t {
    return size(c, 0);
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

} // namespace multidim

#endif // MULTIDIM_HELPERS_H

