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

/** \brief Trivial tag class to indicate the seek direction of an iterator */
struct Forward {};
/** \brief Trivial tag class to indicate the seek direction of an iterator */
struct Backward {};


//***************************************************************************
// FlatIterator
//***************************************************************************
/** \brief An iterator which makes the pointed container appear as a linear array
 * \param Container (typename, as template parameter)
 */
template <
    template<typename> class IsCustomScalar,
    typename TopIterator,
    bool hasSubIterator = ContainerMetrics<IsCustomScalar>::template isContainer<typename PointedType<TopIterator>::type>()
> class FlatIterator;

// Version pointing to a subcontainer
// (and hence having a subIterator)
template <template<typename> class IsCustomScalar, typename TopIterator>
class FlatIterator<IsCustomScalar, TopIterator, true>
:   public boost::iterator_facade<
        FlatIterator<IsCustomScalar, TopIterator, true>, /*Derived=*/
        typename ContainerMetrics<IsCustomScalar>::template ScalarType<typename PointedType<TopIterator>::type>::type, /*Value=*/
        boost::bidirectional_traversal_tag /*CategoryOrTraversal=*/
        /*Reference = Value&, Difference = ptrdiff_t*/
    >
{
public:
    FlatIterator() : valid_{false}, begin_{nullptr}, curr_{nullptr}, end_{nullptr} {}
    /**< \brief Default constructor. Useless but needed by the requirements on iterators */

    FlatIterator(const FlatIterator&) = default;
    /**< \brief Copy constructor. Note that FlatIterator is TriviallyCopyable */

    static FlatIterator makeBegin(TopIterator first, TopIterator last) {
        return FlatIterator<IsCustomScalar, TopIterator>{first, last, Forward{}};
    }
    static FlatIterator makeEnd(TopIterator first, TopIterator last) {
        return FlatIterator<IsCustomScalar, TopIterator>{first, last, Backward{}};
    }

    bool valid() const {return valid_;}

private: // funcs
    friend class boost::iterator_core_access;

    using ChildIterator = typename IteratorType<typename PointedType<TopIterator>::type>::type;

    FlatIterator(TopIterator first, TopIterator last, Forward)
        : valid_{false}
        , begin_{first}
        , curr_{first}
        , end_{last}
    {
        increment();
    }
    FlatIterator(TopIterator first, TopIterator last, Backward)
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
            subIterator_ = FlatIterator<IsCustomScalar, ChildIterator>::makeBegin(begin(*curr_), end(*curr_));
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
            subIterator_ = FlatIterator<IsCustomScalar, ChildIterator>::makeEnd(begin(*curr_), end(*curr_));
            --subIterator_;
            if (subIterator_.valid() == true) {valid_ = true; return;}
        }
    }

    auto dereference() const -> typename ContainerMetrics<IsCustomScalar>::template ScalarType<typename PointedType<TopIterator>::type>::type& {
        if (!valid_) throw std::runtime_error("FlatIterator: access out of bounds");
        return *subIterator_;
    }

    bool equal(const FlatIterator& other) const {
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
    FlatIterator<IsCustomScalar, ChildIterator> subIterator_;
};

// **************************************************************************

// Version for FlatIterator pointing to a scalar type
template <template<typename> class IsCustomScalar, typename TopIterator>
class FlatIterator<IsCustomScalar, TopIterator, false>
:   public boost::iterator_facade<
        FlatIterator<IsCustomScalar, TopIterator, false>, /*Derived=*/
        typename ContainerMetrics<IsCustomScalar>::template ScalarType<typename PointedType<TopIterator>::type>::type, /*Value=*/
        boost::bidirectional_traversal_tag /*CategoryOrTraversal=*/
        /*Reference = Value&, Difference = ptrdiff_t*/
    >
{
public:
    FlatIterator() : valid_{false}, begin_{nullptr}, curr_{nullptr}, end_{nullptr} {}
    /**< \brief Default constructor. Useless but needed by the requirements on iterators */

    FlatIterator(const FlatIterator&) = default;
    /**< \brief Copy constructor. Note that FlatIterator is TriviallyCopyable */

    static FlatIterator makeBegin(TopIterator first, TopIterator last) {
        return FlatIterator<IsCustomScalar, TopIterator>{first, last, Forward{}};
    }
    static FlatIterator makeEnd(TopIterator first, TopIterator last) {
        return FlatIterator<IsCustomScalar, TopIterator>{first, last, Backward{}};
    }

    bool valid() const {return valid_;}

private: // funcs
    friend class boost::iterator_core_access;

    FlatIterator(TopIterator first, TopIterator last, Forward)
        : valid_{false}
        , begin_{first}
        , curr_{first}
        , end_{last}
    {
        increment();
    }
    FlatIterator(TopIterator first, TopIterator last, Backward)
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

    auto dereference() const -> typename ContainerMetrics<IsCustomScalar>::template ScalarType<typename PointedType<TopIterator>::type>::type& {
        if (!valid_) throw std::runtime_error("FlatIterator: access out of bounds");
        return *curr_;
        // This is the key difference: being the iterator at the bottom
        // it will return a value, instead of delegating to the subordinate
    }

    bool equal(const FlatIterator& other) const {
        return (begin_ == other.begin_) && (curr_ == other.curr_) && (end_ == other.end_);
    }

private: // members
    bool valid_;
    TopIterator begin_;
    TopIterator curr_;
    TopIterator end_;
};

//***************************************************************************
// makeFlatIteratorBegin
//***************************************************************************
/** \brief Factory method to build a FlatIterator pointing at the first scalar element of a container.
 * \param ContainerMetrics (typename, as template parameter) : an instantiation of ContainerMetrics template,
 *        used to determine what the scalar type of the container is
 * \param container : the container on which the iterator will run
 */
template <template<typename> class IsCustomScalar, typename Container>
auto makeFlatIteratorBegin(Container& container) -> FlatIterator<IsCustomScalar, decltype(begin(container))>  {
    return FlatIterator<IsCustomScalar, decltype(begin(container))>::makeBegin(begin(container), end(container));
}
/** \brief Factory method to build a FlatIterator pointing at the first scalar element of a range.
 * \param ContainerMetrics (typename, as template parameter) : an instantiation of ContainerMetrics template,
 *        used to determine what the scalar type of the container is
 * \param first, last : iterators delimiting the range
 */
template <template<typename> class IsCustomScalar, typename T>
auto makeFlatIteratorBegin(T first, T last) -> FlatIterator<IsCustomScalar, T>  {
    return FlatIterator<IsCustomScalar, T>::makeBegin(first, last);
}

//***************************************************************************
// makeFlatIteratorEnd
//***************************************************************************
/** \brief Factory method to build a FlatIterator pointing at the one-past-the-last element of a container.
 * \param ContainerMetrics (typename, as template parameter) : an instantiation of ContainerMetrics template,
 *        used to determine what the scalar type of the container is
 * \param container : the container on which the iterator will run
 */
template <template<typename> class IsCustomScalar, typename Container>
auto makeFlatIteratorEnd(Container& container) -> FlatIterator<IsCustomScalar, decltype(begin(container))> {
    return FlatIterator<IsCustomScalar, decltype(begin(container))>::makeEnd(begin(container), end(container));
}
/** \brief Factory method to build a FlatIterator pointing at the one-past-the-last element of a container.
 * \param ContainerMetrics (typename, as template parameter) : an instantiation of ContainerMetrics template,
 *        used to determine what the scalar type of the container is
 * \param first, last : iterators delimiting the range
 */
template <template<typename> class IsCustomScalar, typename T>
auto makeFlatIteratorEnd(T first, T last) -> FlatIterator<IsCustomScalar, T>  {
    return FlatIterator<IsCustomScalar, T>::makeEnd(first, last);
}

#if 0
template <typename Container, typename ContainerMetrics_>
class FlatView {
public:
    class Iterator;

    FlatView (Container& container, const typename ContainerMetrics_::ContainerGeometry& containerGeometry)
        : container_{container}
        , bounds_{containerGeometry.bounds}
        , numberOfScalarElements_{containerGeometry.numberOfScalarElements}
    {
        MY_ASSERT(bounds_.size() == dimensionality_);
    }

    explicit FlatView(Container& container)
        : FlatView(container, ContainerMetrics_::computeContainerGeometry(container) {
    }

    auto operator[](size_t index) const -> typename ScalarType<Container>::type& {
        auto coordinates = getCoordinates(index);
        //std::copy(begin(coordinates), end(coordinates), std::ostream_iterator<size_t>(debugLog, ", "));
        return Delve<Container, dimensionality_>::delve(container_, coordinates);
    }

    static constexpr size_t dimensionality_ = Dimensionality<Container>::value;

    ForwardIterator begin() {
        return ForwardIterator(*this);
    }
    ForwardIterator end() {
        return ForwardIterator(*this)+numberOfScalarElements_;
    }

private:
    Container& container_;
    const std::vector<size_t> bounds_;
    const size_t numberOfScalarElements_;

    // As the type of container changes with every level, we must again resort to templates
    // And since we need partial specializations, we have to define a class
    template <typename Subcontainer, size_t remainingLevels>
    struct Delve {
        static auto delve (Subcontainer& subcontainer, const std::vector<size_t>& coordinates) -> typename ScalarType<Container>::type& {
            using std::begin; // ADL
            auto iter = begin(subcontainer);
            for (size_t i = 0; i < coordinates[dimensionality_ - remainingLevels]; ++i) ++iter;
            return Delve<decltype(*iter), remainingLevels-1>::delve(*iter, coordinates);
        }
        // TODO: specialize if iter is random access
    };
    template <typename Subcontainer>
    struct Delve<Subcontainer, 1> {
        static auto delve (Subcontainer& subcontainer, const std::vector<size_t>& coordinates) -> typename ScalarType<Container>::type& {
            using std::begin; // ADL
            auto iter = begin(subcontainer);
            for (size_t i = 0; i < coordinates[dimensionality_ - 1]; ++i) ++iter;
            return *iter;
        }
    };

    std::vector<size_t> getCoordinates(size_t index) const {
        std::vector<size_t> result(bounds_.size());
        size_t dimIdx = bounds_.size();
        do {
            --dimIdx;
            result[dimIdx] = index % bounds_[dimIdx];
            index /= bounds_[dimIdx];
        } while (dimIdx > 0);
        if (index > 0) throw std::runtime_error("FlatView : access out of bounds");
        return result;
    }
};

// **************************************************************************
// And finally, our FlatViewIterator
// http://www.boost.org/doc/libs/1_47_0/libs/iterator/doc/iterator_facade.html#tutorial-example
template <typename Container>
class FlatView<Container>::Iterator
:   public boost::iterator_facade<
        /*Derived=*/ Iterator,
        /*Value=*/ typename ScalarType<Container>::type,
        /*CategoryOrTraversal=*/ boost::random_access_traversal_tag
        /*Reference = Value&, Difference = ptrdiff_t*/
    >
{
public:
    Iterator(const FlatView& parent)
        : currentIndex_{0}
        , parent_(parent)
    {}

private: // funcs
    friend class boost::iterator_core_access;

    // since FlatView does all the heavy lifting, writing the reqs is strightforward
    void increment() { ++currentIndex_;}
    void decrement() { --currentIndex_;}
    void advance(ptrdiff_t n) {currentIndex_ += n;}
    auto distance_to(const FlatView<Container>::ForwardIterator other) const -> ptrdiff_t {return other.currentIndex_ - currentIndex_;}
    auto dereference() const -> typename ScalarType<Container>::type& {return parent_[currentIndex_];}

    bool equal(FlatView const& other) const {
        return (parent_ == other.parent_) && (currentIndex_ == other.currentIndex_);
    }

private: // members
    size_t currentIndex_;  // contains the index of the pointee, in a linearized view
    FlatView<Container> parent_;
};

#endif // 0


} // namespace multidim
