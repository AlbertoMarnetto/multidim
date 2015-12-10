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

// **************************************************************************
// Forward declarations (to make the friend declaration work
// **************************************************************************
template <
    typename Container,
    typename ContainerMetrics_,
    bool hasSubIterator = ContainerMetrics_::template isContainer<typename ContainedType<Container>::type>()
> class FlatIterator;
template <typename ContainerMetrics_, typename Container>
auto makeFlatIteratorBegin(Container& container) -> FlatIterator<Container, ContainerMetrics_>;
template <typename ContainerMetrics_, typename Container>
auto makeFlatIteratorEnd(Container& container) -> FlatIterator<Container, ContainerMetrics_>;

//***************************************************************************
// FlatIterator
//***************************************************************************
/** \brief An iterator which makes the pointed container appear as a linear array
 * \param Container (typename, as template parameter)
 */

// The unspecialized version, for FlatIterator pointing to a Container of dimensionality >= 2
// (and hence having a subIterator)
template <typename Container, typename ContainerMetrics_, bool hasSubIterator>
class FlatIterator
:   public boost::iterator_facade<
        FlatIterator<Container, ContainerMetrics_, hasSubIterator>, /*Derived=*/
        typename ContainerMetrics_::template ScalarType<Container>::type, /*Value=*/
        boost::bidirectional_traversal_tag /*CategoryOrTraversal=*/
        /*Reference = Value&, Difference = ptrdiff_t*/
    >
{
public:
    FlatIterator() : valid_{false}, begin_{nullptr}, curr_{nullptr}, end_{nullptr} {}
    /**< \brief Default constructor. Useless but needed by the requirements on iterators */

    FlatIterator(const FlatIterator&) = default;
    /**< \brief Copy constructor. Note that FlatIterator is TriviallyCopyable */

private: // funcs
    friend class boost::iterator_core_access;
    friend auto makeFlatIteratorBegin<ContainerMetrics_, Container>(Container& container) -> FlatIterator<Container, ContainerMetrics_>;
    friend auto makeFlatIteratorEnd<ContainerMetrics_, Container>(Container& container) -> FlatIterator<Container, ContainerMetrics_>;
    template <typename Dummy1, typename Dummy2, bool dummy3> friend class FlatIterator;

    using RawIteratorType = typename IteratorType<Container>::type;

    FlatIterator(RawIteratorType myBegin, RawIteratorType myEnd, Forward)
        : valid_{false}
        , begin_{myBegin}
        , curr_{myBegin}
        , end_{myEnd}
    {
        seekForward();
    }
    FlatIterator(RawIteratorType myBegin, RawIteratorType myEnd, Backward)
        : valid_{false}
        , begin_{myBegin}
        , curr_{myEnd}
        , end_{myEnd}
    {
    }

    // Note the asimmetry between increment and decrement:
    // it is provoked by the asimmetry between begin(),
    // which points to a potentially valid element,
    // and end(), which does not
    void increment() {
        if (curr_ == end_) {valid_ = false; return;}
        ++subIterator_;
        if (subIterator_.valid_) return;
        ++curr_;
        seekForward();
    }
    void decrement() {
        --subIterator_;
        if (subIterator_.valid_) return;
        seekBackward();
    }
    auto dereference() const -> typename ContainerMetrics_::template ScalarType<Container>::type& {
        if (!valid_) throw std::runtime_error("FlatIterator: access out of bounds");
        return *subIterator_;
    }

    bool equal(FlatIterator<Container, ContainerMetrics_, hasSubIterator> const& other) const {
        return (begin_ == other.begin_)
            && (curr_ == other.curr_)
            && (end_ == other.end_)
            && (!valid_ || (subIterator_ == other.subIterator_));
    }

    void seekForward() {
        while(1) {
            if (curr_ == end_) {valid_ = false; return;}
            subIterator_ = FlatIterator<typename ContainedType<Container>::type, ContainerMetrics_>(begin(*curr_), end(*curr_), Forward{});
            if (subIterator_.valid_ == true) {valid_ = true; return;}
            ++curr_;
        }
    }
    void seekBackward() {
        while(1) {
            if (curr_ == begin_) {valid_ = false; return;}
            --curr_;
            subIterator_ = FlatIterator<typename ContainedType<Container>::type, ContainerMetrics_>(begin(*curr_), end(*curr_), Backward{});
            if (subIterator_.valid_ == true) {valid_ = true; return;}
        }
    }

private: // members
    bool valid_;
    typename IteratorType<Container>::type begin_;
    typename IteratorType<Container>::type curr_;
    typename IteratorType<Container>::type end_;
    FlatIterator<typename ContainedType<Container>::type, ContainerMetrics_> subIterator_;
};

// **************************************************************************

// The specialized version, for FlatIterator pointing to a Container of dimensionality = 1
template <typename Container, typename ContainerMetrics_>
class FlatIterator<Container, ContainerMetrics_, false>
:   public boost::iterator_facade<
        FlatIterator<Container, ContainerMetrics_, false>, /*Derived=*/
        typename ContainerMetrics_::template ScalarType<Container>::type, /*Value=*/
        boost::bidirectional_traversal_tag /*CategoryOrTraversal=*/
        /*Reference = Value&, Difference = ptrdiff_t*/
    >
{
public:
    FlatIterator() : valid_{false}, begin_{nullptr}, curr_{nullptr}, end_{nullptr} {}
    /**< \brief Default constructor. Useless but needed by the requirements on iterators */

    FlatIterator(const FlatIterator&) = default;
    /**< \brief Copy constructor. Note that FlatIterator is TriviallyCopyable */

private: // funcs
    friend class boost::iterator_core_access;
    friend auto makeFlatIteratorBegin<ContainerMetrics_, Container>(Container& container) -> FlatIterator<Container, ContainerMetrics_>;
    friend auto makeFlatIteratorEnd<ContainerMetrics_, Container>(Container& container) -> FlatIterator<Container, ContainerMetrics_>;
    template <typename Dummy1, typename Dummy2, bool dummy3> friend class FlatIterator;

    using RawIteratorType = typename IteratorType<Container>::type;

    FlatIterator(RawIteratorType myBegin, RawIteratorType myEnd, Forward)
        : valid_{false}
        , begin_{myBegin}
        , curr_{myBegin}
        , end_{myEnd}
    {
        seekForward();
    }
    FlatIterator(RawIteratorType myBegin, RawIteratorType myEnd, Backward)
        : valid_{false}
        , begin_{myBegin}
        , curr_{myEnd}
        , end_{myEnd}
    {
        seekBackward();
    }

    void increment() {
        if (curr_ == end_) {valid_ = false; return;}
        ++curr_;
        seekForward();
    }
    void decrement() {
        seekBackward();
    }

    auto dereference() const -> typename ContainerMetrics_::template ScalarType<Container>::type& {
        if (!valid_) throw std::runtime_error("FlatIterator: access out of bounds");
        return *curr_;
        // This is the key difference: being the iterator at the bottom
        // it will return a value, instead of delegating to the subordinate
    }

    bool equal(const FlatIterator<Container, ContainerMetrics_, false>& other) const {
        return (begin_ == other.begin_) && (curr_ == other.curr_) && (end_ == other.end_);
    }

    void seekForward() {
        valid_ = !(curr_ == end_);
    }

    void seekBackward() {
        if (curr_ == begin_) {valid_ = false; return;}
        --curr_;
        valid_ = true;
    }

private: // members
    bool valid_;
    typename IteratorType<Container>::type begin_;
    typename IteratorType<Container>::type curr_;
    typename IteratorType<Container>::type end_;
};

//***************************************************************************
// makeFlatIteratorBegin
//***************************************************************************
/** \brief Factory method to build a FlatIterator pointing at the first scalar element of a container.
 * \param ContainerMetrics (typename, as template parameter) : an instantiation of ContainerMetrics template,
 *        used to detect what the scalar type of the container is
 * \param container : the container on which the iterator will run
 */

// Note that here the order of the template arguments is inverted.
// This is because typename Container will be deduced, while ContainerMetrics
// must be specified by the caller
template <typename ContainerMetrics_, typename Container>
auto makeFlatIteratorBegin(Container& container) -> FlatIterator<Container, ContainerMetrics_>  {
    return FlatIterator<Container, ContainerMetrics_>{begin(container), end(container), Forward{}};
}
//***************************************************************************
// makeFlatIteratorEnd
//***************************************************************************
/** \brief Factory method to build a FlatIterator pointing at the one-past-the-last element of a container.
 * \param ContainerMetrics (typename, as template parameter) : an instantiation of ContainerMetrics template,
 *        used to detect what the scalar type of the container is
 * \param container : the container on which the iterator will run
 */
template <typename ContainerMetrics_, typename Container>
auto makeFlatIteratorEnd(Container& container) -> FlatIterator<Container, ContainerMetrics_> {
    return FlatIterator<Container, ContainerMetrics_>{begin(container), end(container), Backward{}};
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
