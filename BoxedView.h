// TODO: begin() and end()

#include "Basics.h"

namespace multidim {
/** \addtogroup boxed_view BoxedView
 * \brief A class which makes a multilevel range appear as a C array
 * of user-defined bounds, cropping and filling as needed.
 */

// **************************************************************************
// Forward declarations
/** \brief The iterator used in BoxedView. Unfortunately, it is not
 * a "proper" iterator, since it is proxied
 *  \ingroup boxed_view
 */
template <
    template<typename> class CustomScalarTrait,
    typename RawIterator,
    bool isConstIterator,
    size_t dimensionality
>
class BoxedViewIterator;

// n + iterator
template <template<typename> class CustomScalarTrait, typename RawIterator, bool isConstIterator, size_t dimensionality>
auto operator+(
        typename BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator, dimensionality>::difference_type n,
        const BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator, dimensionality>& other)
    -> BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator, dimensionality>;

/** \brief Proxy class representing a reference to a (possibly defaulted)
 *  scalar value. Plays the same role as vector<bool>::reference
 *  \ingroup boxed_view
 */
template <typename RawIterator, bool isConstProxy>
class BoxedViewScalarProxy;

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
    using iterator = BoxedViewIterator<CustomScalarTrait, RawIterator, false, dimensionality_>;
    using const_iterator =  BoxedViewIterator<CustomScalarTrait, RawIterator, true, dimensionality_>;
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
    template <typename BoundsIterator>
    BoxedView(RawIterator first, RawIterator last, ScalarType defaultValue, BoundsIterator boundsFirst):
        begin_{first},
        end_{last},
        defaultValue_{std::move(defaultValue)}
    {
        std::copy_n(boundsFirst, dimensionality_, bounds_);
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

    iterator begin() {return iterator::makeBegin(begin_, end_, &defaultValue_, bounds_);}
    const_iterator begin() const {return const_iterator::makeBegin(begin_, end_, &defaultValue_, bounds_);}
    const_iterator cbegin() const {return const_iterator::makeBegin(begin_, end_, &defaultValue_, bounds_);}
    iterator end() {return iterator::makeEnd(begin_, end_, &defaultValue_, bounds_);}
    const_iterator end() const {return const_iterator::makeEnd(begin_, end_, &defaultValue_, bounds_);}
    const_iterator cend() const {return const_iterator::makeEnd(begin_, end_, &defaultValue_, bounds_);}
    reverse_iterator rbegin() {return reverse_iterator{iterator::makeEnd(begin_, end_, &defaultValue_, bounds_)};}
    const_reverse_iterator rbegin() const {return const_reverse_iterator{const_iterator::makeEnd(begin_, end_, &defaultValue_, bounds_)};}
    const_reverse_iterator crbegin() const {return const_reverse_iterator{const_iterator::makeEnd(begin_, end_, &defaultValue_, bounds_)};}
    reverse_iterator rend() {return reverse_iterator{iterator::makeBegin(begin_, end_, &defaultValue_, bounds_)};}
    const_reverse_iterator rend() const {return const_reverse_iterator{const_iterator::makeBegin(begin_, end_, &defaultValue_, bounds_)};}
    const_reverse_iterator crend() const {return const_reverse_iterator{const_iterator::makeBegin(begin_, end_, &defaultValue_, bounds_)};}

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
    bool compare(const BoxedView& other) const {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }

private:
    RawIterator begin_ = nullptr;
    RawIterator end_ = nullptr;
    mutable size_t cachedSize_ = NO_VALUE;
    ScalarType defaultValue_;
    size_t bounds_[dimensionality_] = {0};

};

//***************************************************************************
// makeBoxedView
//***************************************************************************
/** \brief Factory method to build a BoxedView of a range
 * \ingroup flat_view
 * \param CustomScalarTrait : (trait template)
 * \param first, last the range on which the View will be based
 */
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename Iterator = void,
    typename ScalarValue = void,
    typename BoundsContainer = void
>
auto makeBoxedView(Iterator first, Iterator last, ScalarValue&& defaultValue, const BoundsContainer& apparentBounds)
    -> BoxedView<CustomScalarTrait, Iterator, DimensionalityRange<CustomScalarTrait, Iterator>::value>
{
    using std::begin;
    if (begin(apparentBounds) ==  end(apparentBounds)) {
        auto containerBounds = bounds<CustomScalarTrait>(first, last);
        return BoxedView<CustomScalarTrait, Iterator, dimensionality(first, last)>
            (first, last, std::forward<ScalarValue>(defaultValue), begin(containerBounds));
    }

    if (std::distance(begin(apparentBounds), end(apparentBounds)) != dimensionality(first, last))
        throw std::runtime_error("makeBoxedView : limit list has false size");

    return BoxedView<CustomScalarTrait, Iterator, dimensionality(first, last)>
        (first, last, std::forward<ScalarValue>(defaultValue), begin(apparentBounds));
}

// The same, with a initializer list
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename Iterator = void,
    typename ScalarValue = void
>
auto makeBoxedView(Iterator first, Iterator last, ScalarValue&& defaultValue, std::initializer_list<size_t> apparentBounds)
    -> BoxedView<CustomScalarTrait, Iterator, DimensionalityRange<CustomScalarTrait, Iterator>::value>
{
    using std::begin;
    if (begin(apparentBounds) == end(apparentBounds)) {
        auto containerBounds = bounds<CustomScalarTrait>(first, last);
        return BoxedView<CustomScalarTrait, Iterator, dimensionality(first, last)>
            (first, last, std::forward<ScalarValue>(defaultValue), begin(containerBounds));
    }

    if (std::distance(begin(apparentBounds), end(apparentBounds)) != dimensionality(first, last))
        throw std::runtime_error("makeBoxedView : limit list has false size");

    return BoxedView<CustomScalarTrait, Iterator, dimensionality(first, last)>
        (first, last, std::forward<ScalarValue>(defaultValue), begin(apparentBounds));
}

/** \brief Factory method to build a BoxedView of a container
 *  (delegates to the range.based version)
 * \ingroup flat_view
 * \param CustomScalarTrait (trait template)
 * \param container the container on which the View will be based
 */
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename Container = void,
    typename ScalarValue = void,
    typename BoundsContainer = void
>
auto makeBoxedView(Container& container, ScalarValue&& defaultValue, const BoundsContainer& apparentBounds)
    -> BoxedView<CustomScalarTrait, decltype(begin(container)), Dimensionality<CustomScalarTrait, Container>::value>
{
    return makeBoxedView(begin(container), end(container), std::forward<ScalarValue>(defaultValue), apparentBounds);
}

// The same, with a initializer list
template <
    template<typename> class CustomScalarTrait = NoCustomScalars,
    typename Container = void,
    typename ScalarValue = void
>
auto makeBoxedView(Container& container, ScalarValue&& defaultValue, std::initializer_list<size_t> apparentBounds)
    -> BoxedView<CustomScalarTrait, decltype(begin(container)), Dimensionality<CustomScalarTrait, Container>::value>
{
    return makeBoxedView(begin(container), end(container), std::forward<ScalarValue>(defaultValue), apparentBounds);
}




//***************************************************************************
// BoxedViewScalarProxy
//***************************************************************************
/** \brief Proxy class representing a reference to a (possibly defaulted)
 *  scalar value. Plays the same role as vector<bool>::reference
 *  \ingroup boxed_view
 */
template <typename RawIterator, bool isConstProxy>
class BoxedViewScalarProxy {
    using RawScalarType = typename std::iterator_traits<RawIterator>::value_type;
    using ConstScalarType = typename std::add_const<RawScalarType>::type;
    using ScalarType = typename std::conditional<isConstProxy, ConstScalarType, RawScalarType>::type;

public:
    BoxedViewScalarProxy(RawIterator target, ScalarType const* defaultValue, bool valid) :
        target_{target},
        defaultValue_{defaultValue},
        valid_{valid} {}

    BoxedViewScalarProxy& operator=(const ScalarType& rhs) {
        if (valid_) *target_ = rhs;
        return *this;
    }

    operator ScalarType() const {
        if (valid_)
            return *target_;
        else
            return *defaultValue_;
    }

private: // members
    RawIterator target_;
    ScalarType const* defaultValue_;
    bool valid_;
};

//***************************************************************************
// BoxedViewIteratorTraits
//***************************************************************************

// typedefs for dimensionality_ > 1
template <template<typename> class CustomScalarTrait, typename RawIterator, bool isConstIterator, size_t dimensionality_>
struct BoxedViewIteratorTraits {
    using RawScalarType = typename IteratorScalarType<CustomScalarTrait, RawIterator>::type;
    using ConstScalarType = typename std::add_const<RawScalarType>::type;
    using ScalarType = typename std::conditional<isConstIterator, ConstScalarType, RawScalarType>::type;

    using ChildRawIterator = typename IteratorType<typename std::iterator_traits<RawIterator>::reference>::type;
    using ChildIterator = BoxedViewIterator<CustomScalarTrait, ChildRawIterator, isConstIterator, dimensionality_ - 1>;

    // [iterator.traits]
    using value_type = ChildIterator;
    using pointer = value_type*;
    using reference = value_type;
};

// typedefs for dimensionality_ == 1
template <template<typename> class CustomScalarTrait, typename RawIterator, bool isConstIterator>
struct BoxedViewIteratorTraits<CustomScalarTrait, RawIterator, isConstIterator, /*dimensionality_*/ 1> {
    using RawScalarType = typename IteratorScalarType<CustomScalarTrait, RawIterator>::type;
    using ConstScalarType = typename std::add_const<RawScalarType>::type;
    using ScalarType = typename std::conditional<isConstIterator, ConstScalarType, RawScalarType>::type;

    using ScalarProxy = BoxedViewScalarProxy<RawIterator, isConstIterator>;

    // [iterator.traits]
    using value_type = ScalarType;
    using pointer = ScalarProxy;
    using reference = ScalarProxy;
};

//***************************************************************************
// BoxedViewIterator
//***************************************************************************
// Specialization for RawIterator pointing to a subcontainer
// Precondition: apparentBounds_ must be an array of at least `dimensionality_` elements

template <template<typename> class CustomScalarTrait, typename RawIterator, bool isConstIterator, size_t dimensionality_>
class BoxedViewIterator {
    constexpr static const size_t nullBounds_[dimensionality_] = {0};
public:
    using Traits = BoxedViewIteratorTraits<CustomScalarTrait, RawIterator, isConstIterator, dimensionality_>;
    using ScalarType = typename Traits::ScalarType;

    // [iterator.traits]
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename Traits::value_type;
    using difference_type = ptrdiff_t;
    using pointer = typename Traits::pointer;     // Child iterator is dimensionality_ > 1, proxy class if == 1
    using reference = typename Traits::reference; // Child iterator is dimensionality_ > 1, proxy class if == 1

    // **************************************************************************
    // ctors
    // **************************************************************************
    BoxedViewIterator() :
        current_{nullptr},
        physicalBound_{0}, apparentBounds_{nullBounds_}, index_{0},
        defaultValue_{nullptr}
        {}
    /* \brief Default constructor. */

    BoxedViewIterator(const BoxedViewIterator&) = default;
    /* \brief Copy constructor. Note that BoxedViewIterator is TriviallyCopyable */

    static BoxedViewIterator makeBegin(
        RawIterator first, RawIterator last,
        ScalarType const* defaultValue, size_t const* apparentBounds)
    {
        return BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator, dimensionality_>
            {first, last, defaultValue, apparentBounds, Forward{}};
    }
    static BoxedViewIterator makeEnd(
        RawIterator first, RawIterator last,
        ScalarType const* defaultValue, size_t const* apparentBounds)
    {
        return BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator, dimensionality_>
            {first, last, defaultValue, apparentBounds, Backward{}};
    }

    // conversion to const_iterator
    template<bool _isConstIterator = isConstIterator>
    BoxedViewIterator(BoxedViewIterator<CustomScalarTrait, RawIterator, false, dimensionality_> other,
                     typename std::enable_if<_isConstIterator,int>::type* = nullptr) :
        current_{other.current_},
        physicalBound_{other.physicalBound_},
        apparentBounds_{other.apparentBounds_},
        index_{other.index_},
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
    // Not part of the "iterator interface", but since we want to emulate a C array
    // they must be provided
    friend BoxedViewIterator begin(const BoxedViewIterator& it) {
        return it - it.index_;
    }
    friend BoxedViewIterator end(const BoxedViewIterator& it) {
        return it + (it.apparentBounds_[0] - it.index_);
    }
private:
    // needed for conversion to const_iterator
    friend class BoxedViewIterator<CustomScalarTrait, RawIterator, true, true>;

    // **************************************************************************
    // private ctors
    // **************************************************************************
    BoxedViewIterator(
        RawIterator first, RawIterator last,
        ScalarType const* defaultValue, size_t const* apparentBounds,
        Forward
    ) :
        current_{first},
        physicalBound_{static_cast<size_t>(std::distance(first, last))},
        apparentBounds_{apparentBounds},
        index_{0},
        defaultValue_{defaultValue}
    {}

    BoxedViewIterator(
        RawIterator first, RawIterator last,
        ScalarType const* defaultValue, size_t const* apparentBounds,
        Backward
    ) :
        current_{last},
        physicalBound_{static_cast<size_t>(std::distance(first, last))},
        apparentBounds_{apparentBounds},
        index_{apparentBounds[0]},
        defaultValue_{defaultValue}
    {
        if (apparentBounds[0] < physicalBound_) {
            // in this case, current_ points to the wrong place
            // (i.e. to the physical end of the container
            // insted of the end of the view)
            // and must be corrected
            size_t currentIndex_ = physicalBound_;
            while (currentIndex_ > apparentBounds[0]) {
                --current_;
                --currentIndex_;
            }
            // TODO: overload for random access oterators
        }
    }


    // **************************************************************************
    // Basic operations, on the model of Boost's iterator_facade
    // **************************************************************************
    static constexpr size_t ONE_BEFORE_THE_FIRST = static_cast<size_t>(-1);

    void increment() {
        // We were not requested to perform this check, but given the nature
        // of this iterator, seems appropriate
        if (index_ == apparentBounds_[0])
            throw std::runtime_error("BoxedViewIterator: access out of bounds");

        if (index_ < physicalBound_) ++current_;
        ++index_;
    }

    void decrement() {
        // We would are not requested to perform this check, but given the nature
        // of this iterator, seems appropriate
        if (index_ == ONE_BEFORE_THE_FIRST)
            throw std::runtime_error("BoxedViewIterator: access out of bounds");

        if (index_ > 0 && index_ <= physicalBound_) --current_;
        --index_;
        // Note: if the old index_ was == 0,
        // it will assume value ONE_BEFORE_THE_FIRST
    }

    // **************************************************************************
    // advance - Version if the underlying raw iterator
    // supports random access
    template<bool rawIteratorIsRandomAccess =
        std::is_same<
            typename std::iterator_traits<RawIterator>::iterator_category,
            std::random_access_iterator_tag
        >::value
    >
    auto advance(difference_type n) -> typename std::enable_if<rawIteratorIsRandomAccess, void>::type {
        // We would not be requested to perform this check, but given the nature
        // of this iterator, seems appropriate
        if (
                (static_cast<difference_type>(index_) + n > static_cast<difference_type>(apparentBounds_[0]))
             || (static_cast<difference_type>(index_) + n < -1                )
        ) {
            throw std::runtime_error("BoxedViewIterator: access out of bounds");
        }

        if (index_ + n > physicalBound_) { // over the upper bound
            current_ += (physicalBound_ - index_);
        } else if (index_ + n < 0) {  // under to lower bound (==0)
            current_ -= index_;
        } else {
            current_ += n;
        }

        index_ += n;
    }

    // advance - Version if the underlying raw iterator
    // does not support random access
    template<bool rawIteratorIsRandomAccess =
        std::is_same<
            typename std::iterator_traits<RawIterator>::iterator_category,
            std::random_access_iterator_tag
        >::value
    >
    auto advance(difference_type n) -> typename std::enable_if<false == rawIteratorIsRandomAccess, void>::type {
        // Error checking is delegated to increment() and decrement()
        if (n > 0) for (difference_type i = 0; i < n; ++i) ++(*this);
        if (n < 0) for (difference_type i = 0; i > n; --i) --(*this);
    }

    // **************************************************************************
    difference_type distance_to(const BoxedViewIterator& other) const {
        return
            std::distance(this->current_, other.current_) -
            (
                (index_ > physicalBound_) ?
                    (index_ - physicalBound_) : 0
            ) +
            (
                (other.index_ > other.physicalBound_) ?
                    (other.index_ - other.physicalBound_) : 0
            );
    }

    // **************************************************************************
    // dereference - Version for dimensionality_ > 1
    // returns a BoxedViewIterator to the subcontainer
    template<bool hasSubRange = (dimensionality_ > 1)>
    auto dereference() const -> typename std::enable_if<true == hasSubRange, reference>::type
    {
        if (
                (index_ == ONE_BEFORE_THE_FIRST)
             || (index_ == apparentBounds_[0])
        ) {
            throw std::runtime_error("BoxedViewIterator: access out of bounds");
        }
        if (index_ < physicalBound_) {
            return reference::makeBegin(
                begin(*current_), end(*current_),
                defaultValue_, apparentBounds_ + 1  // will shift the bound list forward
            );
        } else {
            using ChildRawIterator = typename Traits::ChildRawIterator;
            return reference::makeBegin(
                ChildRawIterator{}, ChildRawIterator{},
                defaultValue_, apparentBounds_ + 1  // will shift the bound list forward
            );
        }
    }

    // dereference - Version for dimensionality_ == 1
    // returns a proxy to the value
    template<bool hasSubRange = (dimensionality_ > 1)>
    auto dereference() const -> typename std::enable_if<false == hasSubRange, reference>::type
    {
        if (
                (index_ == ONE_BEFORE_THE_FIRST)
             || (index_ == apparentBounds_[0])
        ) {
            throw std::runtime_error("BoxedViewIterator: access out of bounds");
        }
        if (index_ < physicalBound_) {
            return reference{current_, defaultValue_, true};
        } else {
            return reference{current_, defaultValue_, false};
        }
    }

    bool equal(const BoxedViewIterator& other) const {
        return (current_ == other.current_)
            && (physicalBound_ == other.physicalBound_)
            && (std::equal(apparentBounds_, apparentBounds_+dimensionality_,
                    other.apparentBounds_)
                )
            && (index_ == other.index_)
            && (*defaultValue_ == *other.defaultValue_);
    }


private: // members
    RawIterator   current_;
    size_t        physicalBound_; // size of the underlying container
    size_t const* apparentBounds_; // size the user of the class will see (throug filling or cropping)
    size_t        index_;
    ScalarType const* defaultValue_; // observer_ptr
};


// **************************************************************************

template <template<typename> class CustomScalarTrait, typename RawIterator, bool isConstIterator, size_t dimensionality>
auto operator+(
        typename BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator, dimensionality>::difference_type n,
        const BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator, dimensionality>& other)
    -> BoxedViewIterator<CustomScalarTrait, RawIterator, isConstIterator, dimensionality>
{
    return other + n;
}

} // namespace
