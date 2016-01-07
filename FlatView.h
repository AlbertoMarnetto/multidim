#include "Basics.h"

namespace multidim {
/** \addtogroup flat_view FlatView
* \brief A class which makes a multilevel range appear as a linear range
*/

// **************************************************************************
// Forward declaration of FlatViewIterator
/** \brief The iterator used in FlatView.
 *  \ingroup flat_view
 */
template <
    template<typename> class CustomScalarTrait,
    typename RawIterator,
    bool isConstIterator,
    bool hasSubIterator = !IsScalar<CustomScalarTrait, typename std::iterator_traits<RawIterator>::value_type>::value
> class FlatViewIterator;

// operator+
template <template<typename> class CustomScalarTrait, typename RawIterator, bool isConstIterator>
auto operator+(
        typename FlatViewIterator<CustomScalarTrait, RawIterator, isConstIterator>::difference_type n,
        const FlatViewIterator<CustomScalarTrait, RawIterator, isConstIterator>& other)
    -> FlatViewIterator<CustomScalarTrait, RawIterator, isConstIterator>;

// **************************************************************************

//***************************************************************************
// FlatView
//***************************************************************************
/** \brief A class which makes the pointed range appear as a linear array.
 * It respects many of the General container requirements (§23.2.1),
 * the discrepancies being due to the fact that the View does not own
 * its elements.
 * In general, we tried to follow the semantics of C++17's `string_view`.
 * \param CustomScalarTrait (trait template)
 * \ingroup flat_view
 */
template <template<typename> class CustomScalarTrait, typename RawIterator>
class FlatView {
public:
    using iterator = FlatViewIterator<CustomScalarTrait, RawIterator, false>;
    using const_iterator =  FlatViewIterator<CustomScalarTrait, RawIterator, true>;
    using value_type = typename iterator::value_type;
    using reference = typename iterator::reference;
    using const_reference = typename const_iterator::reference;
    using difference_type = typename iterator::difference_type;
    using size_type = size_t;   /**< \todo should actually be generic */

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    FlatView() {}
    FlatView(RawIterator first, RawIterator last) : begin_{first}, end_{last} {}
    FlatView(const FlatView& other) = default;  /**< \note It performs a shallow copy! */
    ~FlatView() = default;
    FlatView& operator=(const FlatView& other) = default; /**< \note It performs a shallow copy! */

    // **************************************************************************
    // Standard members
    // **************************************************************************
    bool operator==(const FlatView& other) const {
        return (size() == other.size()) && std::equal(begin(), end(), other.begin());
    }
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

    reference operator[](size_type n) {return *(begin()+ n);}
    const_reference operator[](size_type n) const {return *(begin()+ n);}

    void swap(const FlatView& other) {std::swap(*this, other);}
    size_type size() const {
        // size should be constant, I guess that amortized constant will do
        if (cachedSize_ == NO_VALUE) {
            cachedSize_ = std::distance(begin(), end());
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
};


//***************************************************************************
// makeFlatView
//***************************************************************************
/** \brief Factory method to build a FlatView of a container.
 * \ingroup flat_view
 * \param CustomScalarTrait (trait template)
 * \param container the container on which the View will be based
 */
template <template<typename> class CustomScalarTrait = NoCustomScalars, typename Container = void>
auto makeFlatView(Container& container) -> FlatView<CustomScalarTrait, decltype(begin(container))>  {
    return FlatView<CustomScalarTrait, decltype(begin(container))>{begin(container), end(container)};
}
/** \brief Factory method to build a FlatView of a range
 * \ingroup flat_view
 * \param CustomScalarTrait : (trait template)
 * \param first, last the range on which the View will be based
 */
template <template<typename> class CustomScalarTrait = NoCustomScalars, typename Iterator = void>
auto makeFlatView(Iterator first, Iterator last) -> FlatView<CustomScalarTrait, Iterator>  {
    return FlatView<CustomScalarTrait, Iterator>{first, last};
}

//***************************************************************************
// FlatViewIterator
//***************************************************************************
// Specialization for RawIterator pointing to a subcontainer
// Invariant: the iterator can only be in one of these states:
// (1) (valid() == false && current_ == begin_) :
//     represents the element "one before the first".
// (2) (valid() == true && current_ >= begin_ && current_ < end_ && child_.valid() == true) :
//     represents a valid scalar element
// (3) (valid() == false && current_ == end_) :
//     represents the element "one past the end"
// If (begin_ == end_) then states (1) and (3) collapse, but this does not pose a problem

template <template<typename> class CustomScalarTrait, typename RawIterator, bool isConstIterator>
class FlatViewIterator<CustomScalarTrait, RawIterator, isConstIterator, true>
{
public:
    using RawValueType = typename IteratorScalarType<CustomScalarTrait, RawIterator>::type;
    using ConstValueType = typename std::add_const<RawValueType>::type;
    using RawReferenceType = typename IteratorScalarType<CustomScalarTrait, RawIterator>::reference;
    using ConstReferenceType = typename std::add_const<RawValueType>::type;
    // [iterator.traits]
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename std::conditional<isConstIterator, ConstValueType, RawValueType>::type;
    using difference_type = ptrdiff_t;
    using pointer = value_type*;
    using reference = typename std::conditional<isConstIterator, ConstReferenceType, RawReferenceType>::type;

    // **************************************************************************
    // ctors
    // **************************************************************************
    FlatViewIterator() : child_{}, begin_{nullptr}, current_{nullptr}, end_{nullptr} {}
    /* \brief Default constructor. */

    FlatViewIterator(const FlatViewIterator&) = default;
    /* \brief Copy constructor. Note that FlatViewIterator is TriviallyCopyable */

    static FlatViewIterator makeBegin(RawIterator first, RawIterator last) {
        return FlatViewIterator<CustomScalarTrait, RawIterator, isConstIterator>{first, last, Forward{}};
    }
    static FlatViewIterator makeEnd(RawIterator first, RawIterator last) {
        return FlatViewIterator<CustomScalarTrait, RawIterator, isConstIterator>{first, last, Backward{}};
    }

    // conversion to const_iterator
    template<bool _isConstIterator = isConstIterator>
    FlatViewIterator(FlatViewIterator<CustomScalarTrait, RawIterator, false, true> other,
                     typename std::enable_if<_isConstIterator,int>::type* = nullptr) :
        child_{other.child_},
        begin_{other.begin_},
        current_{other.current_},
        end_{other.end_}
    {}

    // **************************************************************************
    // Standard members
    // **************************************************************************
    // [iterator.iterators]
    reference operator*() const {return dereference();}
    FlatViewIterator& operator++() {increment(); return *this;}

    // [input.iterators] and [output.iterators]
    bool operator==(const FlatViewIterator& other) const {return equal(other);}
    bool operator!=(const FlatViewIterator& other) const {return !((*this) == other);}
    pointer operator->() const {return &(**this);}
    FlatViewIterator operator++(int) {auto tmp = *this; ++(*this); return tmp;}

    // [bidirectional.iterators]
    FlatViewIterator& operator--() {decrement(); return *this;}
    FlatViewIterator operator--(int) {auto other = *this; --(*this); return other;}

    // [random.access.iterators]
    FlatViewIterator& operator+=(difference_type n) {advance(n); return *this;}
    FlatViewIterator operator+(difference_type n) const {auto result = (*this); result += n; return result;}
    template <template<typename> class, typename, bool>
    friend FlatViewIterator operator+(difference_type n, const FlatViewIterator& other);
    FlatViewIterator& operator-=(difference_type n) {return (*this += (-n));}
    FlatViewIterator operator-(difference_type n) const {return (*this + (-n));}

    difference_type operator-(const FlatViewIterator& other) const {return other.distance_to(*this);}
    bool operator<(const FlatViewIterator& other) const {return (other - *this) > 0;}
    bool operator>(const FlatViewIterator& other) const {return (other - *this) < 0;}
    bool operator>=(const FlatViewIterator& other) const {return !((*this) < other);}
    bool operator<=(const FlatViewIterator& other) const {return !((*this) > other);}

    reference operator[](difference_type n) const {return *(*this + n);}


    // **************************************************************************

    bool valid() const {return (current_ < end_) && (child_.valid());}

private: // funcs
    using ChildRawIterator = typename IteratorType<typename std::iterator_traits<RawIterator>::reference>::type;
    using ChildIterator = FlatViewIterator<CustomScalarTrait, ChildRawIterator, isConstIterator>;

    // needed for conversion to const_iterator
    friend class FlatViewIterator<CustomScalarTrait, RawIterator, true, true>;

    FlatViewIterator(RawIterator first, RawIterator last, Forward)
        : begin_{first}
        , current_{first}
        , end_{last}
    {
        increment();
    }
    FlatViewIterator(RawIterator first, RawIterator last, Backward)
        : begin_{first}
        , current_{last}
        , end_{last}
    {
    }

    // Note the asimmetry between increment and decrement:
    // it is provoked by the asimmetry between begin(),
    // which points to a potentially valid element,
    // and end(), which does not
    void increment() {
        if (valid()) {
            // child_ is already in a valid state,
            // move it forward
            ++child_;
            if (child_.valid()) return;

            // no more scalar elements at *current_
            // move ourselves forward
            ++current_;
        }

        while(1) {
            // just updated current_, try to build
            // a valid subiterator at this location
            if (current_ == end_) return;
            child_ = FlatViewIterator<CustomScalarTrait, ChildRawIterator, isConstIterator>::makeBegin(begin(*current_), end(*current_));
            if (child_.valid()) return;
            ++current_;
        }
    }

    void decrement() {
        if (valid()) {
            // child_ is already in a valid state,
            // move it backward
            --child_;
            if (child_.valid()) return;
        }

        // no more scalar elements at *curr,
        // move ourselves backward
        while(1) {
            if (current_ == begin_) return;
            --current_;
            child_ = FlatViewIterator<CustomScalarTrait, ChildRawIterator, isConstIterator>::makeEnd(begin(*current_), end(*current_));
            --child_;
            if (child_.valid()) return;
        }
    }

    void advance(difference_type n) {
        if (n > 0) for (difference_type i = 0; i < n; ++i) ++(*this);
        if (n < 0) for (difference_type i = 0; i > n; --i) --(*this);
    }

    reference dereference() const {
        if (!valid()) throw std::runtime_error("FlatViewIterator: access out of bounds");
        return *child_;
    }

    bool equal(const FlatViewIterator& other) const {
        return (begin_ == other.begin_)
            && (current_ == other.current_)
            && (end_ == other.end_)
            && (
                     (!valid() && !other.valid())
                  || (valid()  &&  other.valid() && child_ == other.child_)
                );
    }

    difference_type distance_to(const FlatViewIterator& other) const {
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
    ChildIterator child_;
    RawIterator begin_;
    RawIterator current_;
    RawIterator end_;
};

// **************************************************************************
// Specialization for RawIterator pointing to a scalar
template <template<typename> class CustomScalarTrait, typename RawIterator, bool isConstIterator>
class FlatViewIterator<CustomScalarTrait, RawIterator, isConstIterator, false>
{
public:
    using RawValueType = typename IteratorScalarType<CustomScalarTrait, RawIterator>::type;
    using ConstValueType = typename std::add_const<RawValueType>::type;
    using RawReferenceType = typename IteratorScalarType<CustomScalarTrait, RawIterator>::reference;
    using ConstReferenceType = typename std::add_const<RawValueType>::type;

    // [iterator.traits]
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename std::conditional<isConstIterator, ConstValueType, RawValueType>::type;
    using difference_type = ptrdiff_t;
    using pointer = value_type*;
    using reference = typename std::conditional<isConstIterator, ConstReferenceType, RawReferenceType>::type;

    // **************************************************************************
    // ctors
    // **************************************************************************
    FlatViewIterator() : valid_{false}, begin_{}, current_{}, end_{} {}
    /* \brief Default constructor. */

    FlatViewIterator(const FlatViewIterator&) = default;
    /* \brief Copy constructor. Note that FlatViewIterator is TriviallyCopyable */

    static FlatViewIterator makeBegin(RawIterator first, RawIterator last) {
        return FlatViewIterator<CustomScalarTrait, RawIterator, isConstIterator>{first, last, Forward{}};
    }
    static FlatViewIterator makeEnd(RawIterator first, RawIterator last) {
        return FlatViewIterator<CustomScalarTrait, RawIterator, isConstIterator>{first, last, Backward{}};
    }

    // conversion to const_iterator
    template<bool _isConstIterator = isConstIterator>
    FlatViewIterator(FlatViewIterator<CustomScalarTrait, RawIterator, false, false> other,
                     typename std::enable_if<_isConstIterator,int>::type* = nullptr) :
        valid_{other.valid_},
        begin_{other.begin_},
        current_{other.current_},
        end_{other.end_}
    {}

    // **************************************************************************

    bool valid() const {return valid_;}

    // **************************************************************************
    // Standard members
    // **************************************************************************

    // [iterator.iterators]
    reference operator*() const {return dereference();}
    FlatViewIterator& operator++() {increment(); return *this;}

    // [input.iterators] and [output.iterators]
    bool operator==(const FlatViewIterator& other) const {return equal(other);}
    bool operator!=(const FlatViewIterator& other) const {return !((*this) == other);}
    pointer operator->() const {return &(**this);}
    FlatViewIterator operator++(int) {auto tmp = *this; ++(*this); return tmp;}

    // [bidirectional.iterators]
    FlatViewIterator& operator--() {decrement(); return *this;}
    FlatViewIterator operator--(int) {auto other = *this; --(*this); return other;}

    // [random.access.iterators]
    FlatViewIterator& operator+=(difference_type n) {advance(n); return *this;}
    FlatViewIterator operator+(difference_type n) const {auto result = (*this); result += n; return result;}
    template <template<typename> class, typename, bool>
    friend FlatViewIterator operator+(difference_type n, const FlatViewIterator& other);
    FlatViewIterator& operator-=(difference_type n) {return (*this += (-n));}
    FlatViewIterator operator-(difference_type n) const {return (*this + (-n));}

    difference_type operator-(const FlatViewIterator& other) const {return other.distance_to(*this);}
    bool operator<(const FlatViewIterator& other) const {return (other - *this) > 0;}
    bool operator>(const FlatViewIterator& other) const {return (other - *this) < 0;}
    bool operator>=(const FlatViewIterator& other) const {return !((*this) < other);}
    bool operator<=(const FlatViewIterator& other) const {return !((*this) > other);}

    reference operator[](difference_type n) const {return *(*this + n);}

private: // funcs
    // needed for conversion to const_iterator
    friend class FlatViewIterator<CustomScalarTrait, RawIterator, true, false>;

    // **************************************************************************
    // private ctors
    // **************************************************************************
    FlatViewIterator(RawIterator first, RawIterator last, Forward)
        : valid_{false}
        , begin_{first}
        , current_{first}
        , end_{last}
    {
        increment();
    }
    FlatViewIterator(RawIterator first, RawIterator last, Backward)
        : valid_{false}
        , begin_{first}
        , current_{last}
        , end_{last}
    {
    }

    // **************************************************************************
    // Basic operations, on the model of Boost's iterator_facade
    // **************************************************************************
    void increment() {
        if (valid_) ++current_;
        valid_ = !(current_ == end_);
    }

    void decrement() {
        if (current_ == begin_) {valid_ = false; return;}
        --current_;
        valid_ = true;
    }

    void advance(difference_type n) {
        if (n > 0) for (difference_type i = 0; i < n; ++i) ++(*this);
        if (n < 0) for (difference_type i = 0; i > n; --i) --(*this);
    }

    reference dereference() const {
        if (!valid_) throw std::runtime_error("FlatViewIterator: access out of bounds");
        return *current_;
        // This is the key difference: being the iterator at the bottom
        // it will return a value, instead of delegating to the subordinate
    }

    bool equal(const FlatViewIterator& other) const {
        return (begin_ == other.begin_) && (current_ == other.current_) && (end_ == other.end_) && (valid_ == other.valid_);
    }

    difference_type distance_to(const FlatViewIterator& other) const {
        // Since this is the bottom level iterator, it could be possible
        // check if the underlying iterator supports random access.
        // But the added complication would buy us very few

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
    bool valid_;
    RawIterator begin_;
    RawIterator current_;
    RawIterator end_;
};

// **************************************************************************

template <template<typename> class CustomScalarTrait, typename RawIterator, bool isConstIterator>
auto operator+(
        typename FlatViewIterator<CustomScalarTrait, RawIterator, isConstIterator>::difference_type n,
        const FlatViewIterator<CustomScalarTrait, RawIterator, isConstIterator>& other)
    -> FlatViewIterator<CustomScalarTrait, RawIterator, isConstIterator>
{
    return other + n;
}

} // namespace multidim
