#include "ContainerMetrics.h"
#include <boost/iterator/iterator_facade.hpp>

namespace multidim {
/** \addtogroup multidim Multidim library
 * @{
 */

/** \addtogroup flat_view Flat views
* \brief Helper templates extending the `<type_traits> library
* @{
*/

// **************************************************************************
// Forward declaration of FlatViewIterator
/** \brief The iterator used in FlatView.
 */
template <
    template<typename> class IsCustomScalar,
    typename TopIterator,
    bool hasSubIterator = ContainerMetrics<IsCustomScalar>::template isContainer<typename PointedType<TopIterator>::type>()
> class FlatViewIterator;
// **************************************************************************

//***************************************************************************
// FlatView
//***************************************************************************
/** \brief An class which makes the pointed range appear as a linear array
 * \param IsCustomScalar
 * \param TopIterator
 * \note This **almost** respect the General container requirements (§23.2.1),
 * but there are some differences due to the fact that the View does not own
 * its elements. Additionally, `size()` is `O(N)`.
 * In general, we tried to follow the semantics of C++17's `string_view`.
 */

template <template<typename> class IsCustomScalar, typename TopIterator>
class FlatView {
public:
    using iterator = FlatViewIterator<IsCustomScalar, TopIterator>;
    using const_iterator = FlatViewIterator<IsCustomScalar, typename std::add_const<TopIterator>::type>;
    using value_type = typename iterator::value_type;
    using reference = typename iterator::reference;
    using const_reference = typename const_iterator::reference;
    using difference_type = typename iterator::difference_type;
    using size_type = size_t;   /**< \todo should actually be generic */

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    FlatView() : begin_{nullptr}, end_{nullptr} {}
    FlatView(TopIterator first, TopIterator last) : begin_{first}, end_{last} {}
    FlatView(const FlatView& other) = default;  /**< \note It performs a shallow copy! */
    ~FlatView() = default;

    FlatView& operator=(const FlatView& other) = default; /**< \note It performs a shallow copy! */
    bool operator==(const FlatView& other) const {return std::equal(begin(), end(), other.begin());}
    bool operator!=(const FlatView& other) const {return !(other == *this);}
    bool operator <(const FlatView& other) const {return compare(other);}
    bool operator >(const FlatView& other) const {return other.compare(*this);}
    bool operator<=(const FlatView& other) const {return !(*this > other);}
    bool operator>=(const FlatView& other) const {return !(other > *this);}

    iterator begin() {return iterator::makeBegin(begin_, end_);}
    const_iterator begin() const {return const_iterator::makeBegin(begin_, end_);}
    const_iterator cbegin() const {return const_iterator::makeBegin(begin_, end_);}
    iterator end() {return iterator::makeEnd(begin_, end_);}
    const_iterator end() const {return const_iterator::makeEnd(begin_, end_);}
    const_iterator cend() const {return const_iterator::makeEnd(begin_, end_);}
    reverse_iterator rbegin() {return reverse_iterator{iterator::makeEnd(begin_, end_)};}
    const_reverse_iterator rbegin() const {return const_reverse_iterator{const_iterator::makeEnd(begin_, end_)};}
    const_reverse_iterator crbegin() const {return const_reverse_iterator{const_iterator::makeEnd(begin_, end_)};}
    reverse_iterator rend() {return reverse_iterator{iterator::makeBegin(begin_, end_)};}
    const_reverse_iterator rend() const {return const_reverse_iterator{const_iterator::makeBegin(begin_, end_)};}
    const_reverse_iterator crend() const {return const_reverse_iterator{const_iterator::makeBegin(begin_, end_)};}

    reference front() {return *begin();}
    const_reference front() const {return *cbegin();}
    reference back() {return *--end();}
    const_reference back() const {return *--cend();}

    void swap(const FlatView& other) {std::swap(*this, other);}
    size_type size() const {
        // size should be constant, I guess that amortized constant will do
        if (!cachedSizeIsValid_) {
            cachedSize_ = std::distance(begin(), end());
            cachedSizeIsValid_ = true;
        }
        return cachedSize_;
    }
    size_type max_size() const {return std::numeric_limits<size_type>::max();};
     /**<  \note Since C++17 the result of max_size should be divided by `sizeof(value_type)`,
           see http://en.cppreference.com/w/cpp/memory/allocator_traits/max_size */

    bool empty() const {return (size() == 0);}

private:
    TopIterator begin_;
    TopIterator end_;
    mutable size_t cachedSize_;
    mutable bool cachedSizeIsValid_;

    bool compare(const FlatView& other) {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }
};


//***************************************************************************
// FlatViewIterator
//***************************************************************************
// Specialization for TopIterator pointing to a subcontainer
template <template<typename> class IsCustomScalar, typename TopIterator>
class FlatViewIterator<IsCustomScalar, TopIterator, true>
:   public boost::iterator_facade<
        FlatViewIterator<IsCustomScalar, TopIterator, true>, /*Derived=*/
        typename ContainerMetrics<IsCustomScalar>::template ScalarValueType<typename PointedType<TopIterator>::type>::type, /*Value=*/
        boost::bidirectional_traversal_tag /*CategoryOrTraversal=*/
        /*Reference = Value&, Difference = ptrdiff_t*/
    >
{
public:
    FlatViewIterator() : valid_{false}, begin_{nullptr}, curr_{nullptr}, end_{nullptr} {}
    /* \brief Default constructor. Useless but needed by the requirements on iterators */

    FlatViewIterator(const FlatViewIterator&) = default;
    /* \brief Copy constructor. Note that FlatViewIterator is TriviallyCopyable */

    static FlatViewIterator makeBegin(TopIterator first, TopIterator last) {
        return FlatViewIterator<IsCustomScalar, TopIterator>{first, last, Forward{}};
    }
    static FlatViewIterator makeEnd(TopIterator first, TopIterator last) {
        return FlatViewIterator<IsCustomScalar, TopIterator>{first, last, Backward{}};
    }

    bool valid() const {return valid_;}

private: // funcs
    friend class boost::iterator_core_access;

    using ChildIterator = typename IteratorType<typename PointedType<TopIterator>::type>::type;

    FlatViewIterator(TopIterator first, TopIterator last, Forward)
        : valid_{false}
        , begin_{first}
        , curr_{first}
        , end_{last}
    {
        increment();
    }
    FlatViewIterator(TopIterator first, TopIterator last, Backward)
        : valid_{false}
        , begin_{first}
        , curr_{last}
        , end_{last}
    {
    }

    // Invariant: the iterator can only be in one of these states:
    // (1) (valid_ == false && curr_ == begin_) :
    //     represents the element "one before the first".
    // (2) (valid_ == true && curr_ >= begin_ && curr_ < end_ && subIterator_.valid_ == true) :
    //     represents a valid scalar element
    // (3) (valid_ == false && curr_ == end_) :
    //     represents the element "one past the end"
    // If (begin_ == end_) then states (1) and (3) collapse, but this does not pose a problem

    // Note the asimmetry between increment and decrement:
    // it is provoked by the asimmetry between begin(),
    // which points to a potentially valid element,
    // and end(), which does not
    void increment() {
        if (valid_) {
            // subiterator_ is already in a valid state,
            // move it forward
            ++subIterator_;
            if (subIterator_.valid()) return;

            // no more scalar elements at *curr_
            // move ourselves forward
            ++curr_;
        }

        while(1) {
            // just updated curr_, try to build
            // a valid subiterator at this location
            if (curr_ == end_) {valid_ = false; return;}
            subIterator_ = FlatViewIterator<IsCustomScalar, ChildIterator>::makeBegin(begin(*curr_), end(*curr_));
            if (subIterator_.valid() == true) {valid_ = true; return;}
            ++curr_;
        }
    }

    void decrement() {
        if (valid_) {
            // subiterator_ is already in a valid state,
            // move it backward
            --subIterator_;
            if (subIterator_.valid()) return;
        }

        // no more scalar elements at *curr,
        // move ourselves backward
        while(1) {
            if (curr_ == begin_) {valid_ = false; return;}
            --curr_;
            subIterator_ = FlatViewIterator<IsCustomScalar, ChildIterator>::makeEnd(begin(*curr_), end(*curr_));
            --subIterator_;
            if (subIterator_.valid() == true) {valid_ = true; return;}
        }
    }

    auto dereference() const -> typename ContainerMetrics<IsCustomScalar>::template ScalarReferenceType<typename PointedType<TopIterator>::type>::type& {
        if (!valid_) throw std::runtime_error("FlatViewIterator: access out of bounds");
        return *subIterator_;
    }

    bool equal(const FlatViewIterator& other) const {
        return (begin_ == other.begin_)
            && (curr_ == other.curr_)
            && (end_ == other.end_)
            && (!valid_ || (subIterator_ == other.subIterator_));
    }

    void seekBackward() {
    }

private: // members
    bool valid_;
    TopIterator begin_;
    TopIterator curr_;
    TopIterator end_;
    FlatViewIterator<IsCustomScalar, ChildIterator> subIterator_;
};

// **************************************************************************
// Specialization for TopIterator pointing to a scalar
template <template<typename> class IsCustomScalar, typename TopIterator>
class FlatViewIterator<IsCustomScalar, TopIterator, false>
:   public boost::iterator_facade<
        FlatViewIterator<IsCustomScalar, TopIterator, false>, /*Derived=*/
        typename ContainerMetrics<IsCustomScalar>::template ScalarValueType<typename PointedType<TopIterator>::type>::type, /*Value=*/
        boost::bidirectional_traversal_tag /*CategoryOrTraversal=*/
        /*Reference = Value&, Difference = ptrdiff_t*/
    >
{
public:
    FlatViewIterator() : valid_{false}, begin_{nullptr}, curr_{nullptr}, end_{nullptr} {}
    /* \brief Default constructor. Useless but needed by the requirements on iterators */

    FlatViewIterator(const FlatViewIterator&) = default;
    /* \brief Copy constructor. Note that FlatViewIterator is TriviallyCopyable */

    static FlatViewIterator makeBegin(TopIterator first, TopIterator last) {
        return FlatViewIterator<IsCustomScalar, TopIterator>{first, last, Forward{}};
    }
    static FlatViewIterator makeEnd(TopIterator first, TopIterator last) {
        return FlatViewIterator<IsCustomScalar, TopIterator>{first, last, Backward{}};
    }

    bool valid() const {return valid_;}

private: // funcs
    friend class boost::iterator_core_access;

    FlatViewIterator(TopIterator first, TopIterator last, Forward)
        : valid_{false}
        , begin_{first}
        , curr_{first}
        , end_{last}
    {
        increment();
    }
    FlatViewIterator(TopIterator first, TopIterator last, Backward)
        : valid_{false}
        , begin_{first}
        , curr_{last}
        , end_{last}
    {
    }

    void increment() {
        if (valid_) ++curr_;
        valid_ = !(curr_ == end_);
    }

    void decrement() {
        if (curr_ == begin_) {valid_ = false; return;}
        --curr_;
        valid_ = true;
    }

    auto dereference() const -> typename ContainerMetrics<IsCustomScalar>::template ScalarReferenceType<typename PointedType<TopIterator>::type>::type& {
        if (!valid_) throw std::runtime_error("FlatViewIterator: access out of bounds");
        return *curr_;
        // This is the key difference: being the iterator at the bottom
        // it will return a value, instead of delegating to the subordinate
    }

    bool equal(const FlatViewIterator& other) const {
        return (begin_ == other.begin_) && (curr_ == other.curr_) && (end_ == other.end_);
    }

private: // members
    bool valid_;
    TopIterator begin_;
    TopIterator curr_;
    TopIterator end_;
};

//***************************************************************************
// makeFlatView
//***************************************************************************
/** \brief Factory method to build a FlatView of a container.
 * \param ContainerMetrics (typename, as template parameter) :
 *        an instantiation of ContainerMetrics template,
 *        used to determine what the scalar type of the container is
 * \param container : the container on which the View will be based
 */
template <template<typename> class IsCustomScalar, typename Container>
auto makeFlatView(Container& container) -> FlatView<IsCustomScalar, decltype(begin(container))>  {
    return FlatView<IsCustomScalar, decltype(begin(container))>{begin(container), end(container)};
}
/** \brief Factory method to build a FlatView of a range
 * \param ContainerMetrics (typename, as template parameter) :
 *        an instantiation of ContainerMetrics template,
 *        used to determine what the scalar type of the container is
 * \param first, last : iterators delimiting the range
 */
template <template<typename> class IsCustomScalar, typename T>
auto makeFlatView(T first, T last) -> FlatView<IsCustomScalar, T>  {
    return FlatView<IsCustomScalar, T>{first, last};
}

} // namespace multidim
