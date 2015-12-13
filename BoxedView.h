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
        /*Value=*/ typename ScalarReferenceType<Container>::type,
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
    auto dereference() const -> typename ScalarReferenceType<Container>::type& {return parent_[currentIndex_];}

    bool equal(FlatView const& other) const {
        return (parent_ == other.parent_) && (currentIndex_ == other.currentIndex_);
    }

private: // members
    size_t currentIndex_;  // contains the index of the pointee, in a linearized view
    FlatView<Container> parent_;
};

#endif // 0

