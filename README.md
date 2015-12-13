# The `multidim` library

The `multidim` library is a small C++11 library, helping to deal with nested containers of arbitrary dimensionality. Among them:

  - Function  `dimensionality`  returns at compile-time the number of dimensions of the container.
  - Class `FlatView` makes a multidimensional container appear as a linear array.
  - Class `BoxedView` (**not yet available**) makes a multidimensional container appear as a [Hyperrectangle](https://en.wikipedia.org/wiki/Hyperrectangle), i.e. eliminating "jaggedness" (returning a default result for access outside the bounds).


### Version
0.0.1


### Installation

`multidim` is a header-only library. It requires a C++11-compliant compiler and the Boost libraries (a version not requiring Boost will be hopefully available in a near future)


### Development

`multidim` is still in pre-alpha stage


### License

MIT
