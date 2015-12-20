#include "ContainerMetrics.h"

namespace multidim {
/** \addtogroup boxed_view BoxedView
 * \brief A class which makes a multilevel range appear as a C array
 * of user-defined bounds, cropping and filling as needed
 */

// **************************************************************************
// Forward declaration of BoxedViewIterator
/** \brief The iterator used in BoxedView.
 *  \ingroup boxed_view
 */
template <
    template<typename> class CustomScalarTrait,
    typename RawIterator,
    bool isConstIterator,
    size_t dimensionality
> class BoxedViewIterator;

// operator+
template <template<typename> class CustomScalarTrait, typename RawIterator, bool isConstIterator>
auto operator+(
        typename BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator>::difference_type n,
        const BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator>& other)
    -> BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator>;

// **************************************************************************

//***************************************************************************
// BoxedView
//***************************************************************************
/** \brief A class which makes a multilevel range appear as a C array
 * of user-defined bounds.
 * It respects many of the General container requirements (§23.2.1),
 * but to carry out its functions, it needs to use proxied iterators
 * (just like vector<bool>).
 * Other discrepancies are due to the fact that the View does not own
 * its elements.
 * In general, we tried to follow the semantics of C++17's `string_view`.
 * \param CustomScalarTrait (trait template)
 * \ingroup boxed_view
 */
template <
    template<typename> class CustomScalarTrait,
    typename RawIterator,
    size_t dimensionality_
>
class BoxedView {
public:
    using iterator = BoxedViewIterator<CustomScalarTrait, RawIterator, dimensionality_>;
    using const_iterator =  BoxedViewIterator<CustomScalarTrait, RawIterator, dimensionality_>;
    using value_type = typename iterator::value_type;
    using reference = typename iterator::reference;
    using const_reference = typename const_iterator::reference;
    using difference_type = typename iterator::difference_type;
    using size_type = size_t;   /**< \todo should actually be generic */

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using ScalarType = typename iterator::ScalarType;

    // **************************************************************************
    // ctors
    // **************************************************************************
    BoxedView() {}
    BoxedView(RawIterator first, RawIterator last, BoundsIterator boundsFirst, BoundsIterator boundsLast) :
        begin_{first},
        end_{last} {
        std::copy(boundsFirst, boundsLast, bounds_);
    }
    BoxedView(const BoxedView& other) = default;  /**< \note It performs a shallow copy! */
    ~BoxedView() = default;
    BoxedView& operator=(const BoxedView& other) = default; /**< \note It performs a shallow copy! */

    // **************************************************************************
    // Standard members
    // **************************************************************************
    bool operator==(const BoxedView& other) const {
        return (begin_ == other.begin_)
            && (end_ == other.end_)
            && std::equal(begin(bounds_), end(bounds_), begin(other.bounds_), end(other.bounds_));
    }
    bool operator!=(const BoxedView& other) const {return !(other == *this);}
    bool operator <(const BoxedView& other) const {return compare(other);}
    bool operator >(const BoxedView& other) const {return other.compare(*this);}
    bool operator<=(const BoxedView& other) const {return !(*this > other);}
    bool operator>=(const BoxedView& other) const {return !(other > *this);}

    iterator begin() {return iterator::makeBegin(begin_, end_, bounds_);}
    const_iterator begin() const {return const_iterator::makeBegin(begin_, end_, bounds_);}
    const_iterator cbegin() const {return const_iterator::makeBegin(begin_, end_, bounds_);}
    iterator end() {return iterator::makeEnd(begin_, end_, bounds_);}
    const_iterator end() const {return const_iterator::makeEnd(begin_, end_, bounds_);}
    const_iterator cend() const {return const_iterator::makeEnd(begin_, end_, bounds_);}
    reverse_iterator rbegin() {return reverse_iterator{iterator::makeEnd(begin_, end_, bounds_)};}
    const_reverse_iterator rbegin() const {return const_reverse_iterator{const_iterator::makeEnd(begin_, end_, bounds_)};}
    const_reverse_iterator crbegin() const {return const_reverse_iterator{const_iterator::makeEnd(begin_, end_, bounds_)};}
    reverse_iterator rend() {return reverse_iterator{iterator::makeBegin(begin_, end_, bounds_)};}
    const_reverse_iterator rend() const {return const_reverse_iterator{const_iterator::makeBegin(begin_, end_, bounds_)};}
    const_reverse_iterator crend() const {return const_reverse_iterator{const_iterator::makeBegin(begin_, end_, bounds_)};}

    reference front() {return *begin();}
    const_reference front() const {return *cbegin();}
    reference back() {return *--end();}
    const_reference back() const {return *--cend();}

    reference operator[](size_type n) {return *(begin() + n);}
    const_reference operator[](size_type n) const {return *(begin() + n);}

    void swap(const BoxedView& other) {std::swap(*this, other);}
    size_type size() const {
        // size should be constant, I guess that amortized constant will do
        if (cachedSize_ == NO_VALUE) {
            cachedSize_ = std::distance(begin_, end_);
        }
        return cachedSize_;
    }
    size_type max_size() const {return std::numeric_limits<size_type>::max();};
     /**<  \note Since C++17 the result of max_size should be divided by `sizeof(value_type)`,
           see http://en.cppreference.com/w/cpp/memory/allocator_traits/max_size */

    bool empty() const {return (size() == 0);}

private:
    bool compare(const FlatView& other) const {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }

private:
    RawIterator begin_ = nullptr;
    RawIterator end_ = nullptr;
    mutable size_t cachedSize_ = NO_VALUE;
    ScalarType defaultValue;

};

//***************************************************************************
// BoxedViewIterator
//***************************************************************************
// Specialization for RawIterator pointing to a subcontainer

template <template<typename> class CustomScalarTrait, typename RawIterator, bool isConstIterator, size_t dimensionality_ /*!= 1*/>
class BoxedViewIterator
{
public:
    using RawScalarType = typename IteratorScalarType<CustomScalarTrait, RawIterator>::type;
    using ConstScalarType = typename std::add_const<RawScalarType>::type;
    using ScalarType = typename std::conditional<isConstIterator, ConstScalarType, RawScalarType>::type;

    using ChildRawIterator = typename IteratorType<typename std::iterator_traits<RawIterator>::reference>::type;
    using ChildIterator = BoxedViewIterator<CustomScalarTrait, ChildRawIterator, isConstIterator, dimensionality_-1>

//    using RawValueType = typename std::iterator_traits<RawIterator>::value_type;
//    using ConstValueType = typename std::add_const<RawValueType>::type;

    // [iterator.traits]
    using iterator_category = std::random_access_iterator_tag;
    using value_type = ChildIterator;
    using difference_type = ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    // **************************************************************************
    // ctors
    // **************************************************************************
    BoxedViewIterator() :
        current_{nullptr},
        physicalBound_{0}, apparentBound_{0}, currentIndex_{0},
        defaultValue_{nullptr}
        {}
    /* \brief Default constructor. */

    BoxedViewIterator(const BoxedViewIterator&) = default;
    /* \brief Copy constructor. Note that BoxedViewIterator is TriviallyCopyable */

    // Precondition: apparentBound_ must refer to an array of at least `dimensionality_` elements
    static BoxedViewIterator makeBegin(
        RawIterator first, RawIterator last,
        size_t const& apparentBound, ScalarType* defaultValue = nullptr)
    {
        return BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator, dimensionality_>
            {first, last, apparentBound, defaultValue, Forward{}};
    }
    static BoxedViewIterator makeEnd(
        RawIterator first, RawIterator last,
        size_t const& apparentBound, ScalarType* defaultValue = nullptr)
    {
        return BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator, dimensionality_>
            {first, last, apparentBound, defaultValue, Backward{}};
    }

    // conversion to const_iterator
    template<bool _isConstIterator = isConstIterator>
    BoxedViewIterator(BoxedViewIterator<CustomScalarTrait, RawIterator, false, dimensionality_> other,
                     typename std::enable_if<_isConstIterator,int>::type* = nullptr) :
        current_{other.current_},
        physicalBound_{other.physicalBound_}, apparentBound_{other.apparentBound_},
        currentIndex_{other.currentIndex_},
        defaultValue_{other.defaultValue_}
    {}

    // **************************************************************************
    // Standard members
    // **************************************************************************
    // [iterator.iterators]
    reference operator*() const {return dereference();}
    BoxedViewIterator& operator++() {increment(); return *this;}

    // [input.iterators] and [output.iterators]
    bool operator==(const BoxedViewIterator& other) const {return equal(other);}
    bool operator!=(const BoxedViewIterator& other) const {return !((*this) == other);}
    pointer operator->() const {return &(**this);}
    BoxedViewIterator operator++(int) {auto tmp = *this; ++(*this); return tmp;}

    // [bidirectional.iterators]
    BoxedViewIterator& operator--() {decrement(); return *this;}
    BoxedViewIterator operator--(int) {auto other = *this; --(*this); return other;}

    // [random.access.iterators]
    BoxedViewIterator& operator+=(difference_type n) {advance(n); return *this;}
    BoxedViewIterator operator+(difference_type n) const {auto result = (*this); result += n; return result;}
    template <template<typename> class, typename, bool>
    friend BoxedViewIterator operator+(difference_type n, const BoxedViewIterator& other);
    BoxedViewIterator& operator-=(difference_type n) {return (*this += (-n));}
    BoxedViewIterator operator-(difference_type n) const {return (*this + (-n));}

    difference_type operator-(const BoxedViewIterator& other) const {return other.distance_to(*this);}
    bool operator<(const BoxedViewIterator& other) const {return (other - *this) > 0;}
    bool operator>(const BoxedViewIterator& other) const {return (other - *this) < 0;}
    bool operator>=(const BoxedViewIterator& other) const {return !((*this) < other);}
    bool operator<=(const BoxedViewIterator& other) const {return !((*this) > other);}

    reference operator[](difference_type n) const {return *(*this + n);}

    // **************************************************************************

private:
    // needed for conversion to const_iterator
    friend class BoxedViewIterator<CustomScalarTrait, RawIterator, true, true>;

    // **************************************************************************
    // private ctors
    // **************************************************************************
    BoxedViewIterator(
        RawIterator first, RawIterator last,
        size_t const& apparentBound, ScalarType* defaultValue = nullptr,
        Forward
    ) :
        current_{first),
        physicalBound_{static_cast<size_t>(std::distance(last-first))},
        apparentBound_{apparentBound},
        currentIndex_{0},
        defaultValue_{defaultValue}
    {}

    BoxedViewIterator(
        RawIterator first, RawIterator last,
        size_t const& apparentBound, ScalarType* defaultValue = nullptr,
        Backward
    ) :
        current_{last),
        physicalBound_{static_cast<size_t>(std::distance(last-first))},
        apparentBound_{apparentBound},
        currentIndex_{apparentBound},
        defaultValue_{defaultValue}
    {}


    // **************************************************************************
    // Basic operations, on the model of Boost's iterator_facade
    // **************************************************************************
    constexpr size_t ONE_BEFORE_THE_FIRST = static_cast<size_t>(-1);

    void increment() {
        // We would are not requested to perform this check, but given the nature
        // of this iterator, seems appropriate
        if (currentIndex_ == apparentBound_)
            throw std::runtime_error("FlatViewIterator: access out of bounds");

        if (currentIndex_ < physicalBound_) ++current_;
        ++currentIndex_;
    }

    void decrement() {
        // We would are not requested to perform this check, but given the nature
        // of this iterator, seems appropriate
        if (currentIndex_ == ONE_BEFORE_THE_FIRST)
            throw std::runtime_error("FlatViewIterator: access out of bounds");

        if (currentIndex_ > 0 && currentIndex_ <= physicalBound_) ++current_;
        --currentIndex_;
        // Note: if the old currentIndex_ was == 0,
        // it will assume value ONE_BEFORE_THE_FIRST
    }

    // Version if the underlying raw iterator
    // supports random access
    template<bool rawIteratorIsRandomAccess =
        std::is_same<
            typename std::iterator_traits<RawIterator>::iterator_category,
            std::random_access_iterator_tag
        >::value_type
    >
    auto advance(difference_type n) -> typename std::enable_if<rawIteratorIsRandomAccess, void>::type {
        // We would are not requested to perform this check, but given the nature
        // of this iterator, seems appropriate
        // Note the (-1) and not ONE_BEFORE_THE_FIRST in the second condition,
        // since the operands are of signed type
        if (
                (currentIndex_ + n > apparentBound_)
             || (currentIndex_ + n < -1)
        ) {
            throw std::runtime_error("FlatViewIterator: access out of bounds");
        }

        if (currentIndex_ + n > physicalBound_) {
            current_ += (physicalBound_ - currentIndex_);
        } else if (currentIndex_ + n < 0) {
            current_ -= currentIndex_;
        } else {
            current_ += n;
        }

        currentIndex += n;
    }

    // Version if the underlying raw iterator
    // does not support random access
    template<bool rawIteratorIsRandomAccess =
        std::is_same<
            typename std::iterator_traits<RawIterator>::iterator_category,
            std::random_access_iterator_tag
        >::value_type
    >
    auto advance(difference_type n) -> typename std::enable_if<false == rawIteratorIsRandomAccess, void>::type {
        if (n > 0) for (difference_type i = 0; i < n; ++i) ++(*this);
        if (n < 0) for (difference_type i = 0; i > n; --i) --(*this);
    }

    reference dereference() const {
        if (
                (currentIndex_ == ONE_BEFORE_THE_FIRST)
             || (currentIndex_ == apparentBound_)
        ) {
            throw std::runtime_error("BoxedViewIterator: access out of bounds");
        }
        return ;
    }

    bool equal(const BoxedViewIterator& other) const {
        return (begin_ == other.begin_)
            && (current_ == other.current_)
            && (end_ == other.end_)
            && (
                     (!valid() && !other.valid())
                  || (valid()  &&  other.valid() && child_ == other.child_)
                );
    }

    difference_type distance_to(const BoxedViewIterator& other) const {
        // Since we have no guarantee about
        // uniform allocation, we have to explicitly search forwards and backwards
        if ((*this) == other) return 0;

        // search forward
        difference_type result = 0;
        auto tmp = (*this);
        while (tmp.valid()) {
            ++tmp;
            ++result;
            if (tmp == other) return result;
        }

        // if not found: search backward
        result = 0;
        tmp = (*this);
        do {
            --tmp;
            --result;
            if (tmp == other) return result;
        } while (tmp.valid());

        throw std::runtime_error("Attempted to compare iterators not belonging to same container");
    }

private: // members
    RawIterator   current_;
    size_t        physicalBound_;  // size of the undelying container
    size_t const& apparentBound_; // size the user of the class will see (throug filling or cropping)
    size_t        currentIndex_;
    ScalarType*   defaultValue_; // observer_ptr
};

#if 0
template <typename Container, typename ContainerMetrics_>
class BoxedView {
public:
    class Iterator;

    BoxedView (Container& container, const typename ContainerMetrics_::ContainerGeometry& containerGeometry)
        : container_{container}
        , bounds_{containerGeometry.bounds}
        , numberOfScalarElements_{containerGeometry.numberOfScalarElements}
    {
        MY_ASSERT(bounds_.size() == dimensionality_);
    }

    explicit BoxedView(Container& container)
        : BoxedView(container, ContainerMetrics_::computeContainerGeometry(container) {
    }

    auto operator[](size_t index) const -> typename ScalarReferenceType<Container>::type& {
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
        static auto delve (Subcontainer& subcontainer, const std::vector<size_t>& coordinates) -> typename ScalarReferenceType<Container>::type& {
            using std::begin; // ADL
            auto iter = begin(subcontainer);
            for (size_t i = 0; i < coordinates[dimensionality_ - remainingLevels]; ++i) ++iter;
            return Delve<decltype(*iter), remainingLevels-1>::delve(*iter, coordinates);
        }
        // TODO: specialize if iter is random access
    };
    template <typename Subcontainer>
    struct Delve<Subcontainer, 1> {
        static auto delve (Subcontainer& subcontainer, const std::vector<size_t>& coordinates) -> typename ScalarReferenceType<Container>::type& {
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
        if (index > 0) throw std::runtime_error("BoxedView : access out of bounds");
        return result;
    }
};

// **************************************************************************
// And finally, our BoxedViewIterator
// http://www.boost.org/doc/libs/1_47_0/libs/iterator/doc/iterator_facade.html#tutorial-example
template <typename Container>
class BoxedView<Container>::Iterator
:   public boost::iterator_facade<
        /*Derived=*/ Iterator,
        /*Value=*/ typename ScalarReferenceType<Container>::type,
        /*CategoryOrTraversal=*/ boost::random_access_traversal_tag
        /*Reference = Value&, Difference = ptrdiff_t*/
    >
{
public:
    Iterator(const BoxedView& parent)
        : currentIndex_{0}
        , parent_(parent)
    {}

private: // funcs
    friend class boost::iterator_core_access;

    // since BoxedView does all the heavy lifting, writing the reqs is strightforward
    void increment() { ++currentIndex_;}
    void decrement() { --currentIndex_;}
    void advance(ptrdiff_t n) {currentIndex_ += n;}
    auto distance_to(const BoxedView<Container>::ForwardIterator other) const -> ptrdiff_t {return other.currentIndex_ - currentIndex_;}
    auto dereference() const -> typename ScalarReferenceType<Container>::type& {return parent_[currentIndex_];}

    bool equal(BoxedView const& other) const {
        return (parent_ == other.parent_) && (currentIndex_ == other.currentIndex_);
    }

private: // members
    size_t currentIndex_;  // contains the index of the pointee, in a linearized view
    BoxedView<Container> parent_;
};

#endif // 0

