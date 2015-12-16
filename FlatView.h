#include "ContainerMetrics.h"

namespace multidim {
/** \addtogroup flat_view FlatView
* \brief Helper templates extending the `<type_traits> library
*/

// **************************************************************************
// Forward declaration of FlatViewIterator
/** \brief The iterator used in FlatView.
 *  \ingroup flat_view
 */
template <
    template<typename> class IsCustomScalar,
    typename TopIterator,
    bool isConstIterator,
    bool hasSubIterator = ContainerMetrics<IsCustomScalar>::template isContainer<typename PointedType<TopIterator>::type>()
> class FlatViewIterator;
template <template<typename> class IsCustomScalar, typename TopIterator, bool isConstIterator>
auto operator+(
        typename FlatViewIterator<IsCustomScalar, TopIterator, isConstIterator>::difference_type n,
        const FlatViewIterator<IsCustomScalar, TopIterator, isConstIterator>& other)
    -> FlatViewIterator<IsCustomScalar, TopIterator, isConstIterator>;

// **************************************************************************

//***************************************************************************
// FlatView
//***************************************************************************
/** \brief An class which makes the pointed range appear as a linear array
 * \param IsCustomScalar
 * \param TopIterator
 * \ingroup flat_view
 * \note This **almost** respects the General container requirements (§23.2.1),
 * but there are some differences due to the fact that the View does not own
 * its elements.
 * In general, we tried to follow the semantics of C++17's `string_view`.
 */
template <template<typename> class IsCustomScalar, typename TopIterator>
class FlatView {
public:
    using iterator = FlatViewIterator<IsCustomScalar, TopIterator, false>;
    using const_iterator =  FlatViewIterator<IsCustomScalar, TopIterator, true>;
    using value_type = typename iterator::value_type;
    using reference = typename iterator::reference;
    using const_reference = typename const_iterator::reference;
    using difference_type = typename iterator::difference_type;
    using size_type = size_t;   /**< \todo should actually be generic */

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    FlatView() {}
    FlatView(TopIterator first, TopIterator last) : begin_{first}, end_{last} {}
    FlatView(const FlatView& other) = default;  /**< \note It performs a shallow copy! */
    ~FlatView() = default;
    FlatView& operator=(const FlatView& other) = default; /**< \note It performs a shallow copy! */

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
    TopIterator begin_ = nullptr;
    TopIterator end_ = nullptr;
    mutable size_t cachedSize_ = 0;
    mutable bool cachedSizeIsValid_ = false;

    bool compare(const FlatView& other) const {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }
};

//***************************************************************************
// FlatViewIterator
//***************************************************************************
// Specialization for TopIterator pointing to a subcontainer
// Invariant: the iterator can only be in one of these states:
// (1) (valid_ == false && curr_ == begin_) :
//     represents the element "one before the first".
// (2) (valid_ == true && curr_ >= begin_ && curr_ < end_ && subIterator_.valid_ == true) :
//     represents a valid scalar element
// (3) (valid_ == false && curr_ == end_) :
//     represents the element "one past the end"
// If (begin_ == end_) then states (1) and (3) collapse, but this does not pose a problem
template <template<typename> class IsCustomScalar, typename TopIterator, bool isConstIterator>
class FlatViewIterator<IsCustomScalar, TopIterator, isConstIterator, true>
{
public:
    FlatViewIterator() : begin_{nullptr}, curr_{nullptr}, end_{nullptr} {}
    /* \brief Default constructor. Useless but needed by the requirements on iterators */
    FlatViewIterator(const FlatViewIterator&) = default;
    /* \brief Copy constructor. Note that FlatViewIterator is TriviallyCopyable */

    static FlatViewIterator makeBegin(TopIterator first, TopIterator last) {
        return FlatViewIterator<IsCustomScalar, TopIterator, isConstIterator>{first, last, Forward{}};
    }
    static FlatViewIterator makeEnd(TopIterator first, TopIterator last) {
        return FlatViewIterator<IsCustomScalar, TopIterator, isConstIterator>{first, last, Backward{}};
    }

    bool valid() const {return (curr_ < end_) && (subIterator_.valid());}

    // **************************************************************************
    // Standard members
    // **************************************************************************
    // iterator.traits
    using iterator_category = std::random_access_iterator_tag;
    using value_type =
        typename std::conditional<isConstIterator,
            typename std::add_const<typename ContainerMetrics<IsCustomScalar>::template ScalarValueType<typename PointedType<TopIterator>::type>::type>::type,
            typename ContainerMetrics<IsCustomScalar>::template ScalarValueType<typename PointedType<TopIterator>::type>::type
        >::type;
    using difference_type = ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    // iterator.iterators
    reference operator*() const {return dereference();}
    FlatViewIterator& operator++() {increment(); return *this;}

    // input.iterators and output.iterators
    bool operator==(const FlatViewIterator& other) const {return equal(other);}
    bool operator!=(const FlatViewIterator& other) const {return !((*this) == other);}
    pointer operator->() const {return &(**this);}
    FlatViewIterator operator++(int) {auto tmp = *this; ++(*this); return tmp;}

    // bidirectional.iterators
    FlatViewIterator& operator--() {decrement(); return *this;}
    FlatViewIterator operator--(int) {auto other = *this; --(*this); return other;}

    // random.access.iterators
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
    using ChildIterator = typename IteratorType<typename PointedType<TopIterator>::type>::type;

    FlatViewIterator(TopIterator first, TopIterator last, Forward)
        : begin_{first}
        , curr_{first}
        , end_{last}
    {
        increment();
    }
    FlatViewIterator(TopIterator first, TopIterator last, Backward)
        : begin_{first}
        , curr_{last}
        , end_{last}
    {
    }

    // Note the asimmetry between increment and decrement:
    // it is provoked by the asimmetry between begin(),
    // which points to a potentially valid element,
    // and end(), which does not
    void increment() {
        if (valid()) {
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
            if (curr_ == end_) return;
            subIterator_ = FlatViewIterator<IsCustomScalar, ChildIterator, isConstIterator>::makeBegin(begin(*curr_), end(*curr_));
            if (subIterator_.valid()) return;
            ++curr_;
        }
    }

    void decrement() {
        if (valid()) {
            // subiterator_ is already in a valid state,
            // move it backward
            --subIterator_;
            if (subIterator_.valid()) return;
        }

        // no more scalar elements at *curr,
        // move ourselves backward
        while(1) {
            if (curr_ == begin_) return;
            --curr_;
            subIterator_ = FlatViewIterator<IsCustomScalar, ChildIterator, isConstIterator>::makeEnd(begin(*curr_), end(*curr_));
            --subIterator_;
            if (subIterator_.valid()) return;
        }
    }

    void advance(difference_type n) {
        if (n > 0) for (difference_type i = 0; i < n; ++i) ++(*this);
        if (n < 0) for (difference_type i = 0; i > n; --i) --(*this);
    }

    reference dereference() const {
        if (!valid()) throw std::runtime_error("FlatViewIterator: access out of bounds");
        return *subIterator_;
    }

    bool equal(const FlatViewIterator& other) const {
        return (begin_ == other.begin_)
            && (curr_ == other.curr_)
            && (end_ == other.end_)
            && (!valid() || (subIterator_ == other.subIterator_));
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
    FlatViewIterator<IsCustomScalar, ChildIterator, isConstIterator> subIterator_;
    TopIterator begin_;
    TopIterator curr_;
    TopIterator end_;
};

// **************************************************************************
// Specialization for TopIterator pointing to a scalar
template <template<typename> class IsCustomScalar, typename TopIterator, bool isConstIterator>
class FlatViewIterator<IsCustomScalar, TopIterator, isConstIterator, false>
{
public:
    FlatViewIterator() : valid_{false}, begin_{nullptr}, curr_{nullptr}, end_{nullptr} {}
    /* \brief Default constructor. Useless but needed by the requirements on iterators */

    FlatViewIterator(const FlatViewIterator&) = default;
    /* \brief Copy constructor. Note that FlatViewIterator is TriviallyCopyable */

    static FlatViewIterator makeBegin(TopIterator first, TopIterator last) {
        return FlatViewIterator<IsCustomScalar, TopIterator, isConstIterator>{first, last, Forward{}};
    }
    static FlatViewIterator makeEnd(TopIterator first, TopIterator last) {
        return FlatViewIterator<IsCustomScalar, TopIterator, isConstIterator>{first, last, Backward{}};
    }

    bool valid() const {return valid_;}

    // **************************************************************************
    // Standard members
    // **************************************************************************
    // iterator.traits
    using iterator_category = std::random_access_iterator_tag;
    using value_type =
        typename std::conditional<isConstIterator,
            typename std::add_const<typename ContainerMetrics<IsCustomScalar>::template ScalarValueType<typename PointedType<TopIterator>::type>::type>::type,
            typename ContainerMetrics<IsCustomScalar>::template ScalarValueType<typename PointedType<TopIterator>::type>::type
        >::type;
    using difference_type = ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    // iterator.iterators
    reference operator*() const {return dereference();}
    FlatViewIterator& operator++() {increment(); return *this;}

    // input.iterators and output.iterators
    bool operator==(const FlatViewIterator& other) const {return equal(other);}
    bool operator!=(const FlatViewIterator& other) const {return !((*this) == other);}
    pointer operator->() const {return &(**this);}
    FlatViewIterator operator++(int) {auto tmp = *this; ++(*this); return tmp;}

    // bidirectional.iterators
    FlatViewIterator& operator--() {decrement(); return *this;}
    FlatViewIterator operator--(int) {auto other = *this; --(*this); return other;}

    // random.access.iterators
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

    void advance(difference_type n) {
        if (n > 0) for (difference_type i = 0; i < n; ++i) ++(*this);
        if (n < 0) for (difference_type i = 0; i > n; --i) --(*this);
    }

    reference& dereference() const {
        if (!valid_) throw std::runtime_error("FlatViewIterator: access out of bounds");
        return *curr_;
        // This is the key difference: being the iterator at the bottom
        // it will return a value, instead of delegating to the subordinate
    }

    bool equal(const FlatViewIterator& other) const {
        return (begin_ == other.begin_) && (curr_ == other.curr_) && (end_ == other.end_);
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
    TopIterator begin_;
    TopIterator curr_;
    TopIterator end_;
};

// **************************************************************************
template <template<typename> class IsCustomScalar, typename TopIterator, bool isConstIterator>
auto operator+(
        typename FlatViewIterator<IsCustomScalar, TopIterator, isConstIterator>::difference_type n,
        const FlatViewIterator<IsCustomScalar, TopIterator, isConstIterator>& other)
    -> FlatViewIterator<IsCustomScalar, TopIterator, isConstIterator>
{
    return other + n;
}


//***************************************************************************
// makeFlatView
//***************************************************************************
/** \brief Factory method to build a FlatView of a container.
 * \param IsCustomScalar : trait template for custom scalars. If omitted, `NoCustomScalars` will be used
 * \param container : the container on which the View will be based
 */
template <template<typename> class IsCustomScalar = NoCustomScalars, typename Container = void>
auto makeFlatView(Container& container) -> FlatView<IsCustomScalar, decltype(begin(container))>  {
    return FlatView<IsCustomScalar, decltype(begin(container))>{begin(container), end(container)};
}
/** \brief Factory method to build a FlatView of a range
 * \param IsCustomScalar : trait template for custom scalars. If omitted, `NoCustomScalars` will be used
 * \param first, last : iterators delimiting the range
 */
template <template<typename> class IsCustomScalar = NoCustomScalars, typename T = void>
auto makeFlatView(T first, T last) -> FlatView<IsCustomScalar, T>  {
    return FlatView<IsCustomScalar, T>{first, last};
}

} // namespace multidim
