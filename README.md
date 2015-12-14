# The `multidim` library

The `multidim` library is a small C++11 library, helping to deal with nested containers of arbitrary dimensionality. Among them:

  - Function  `dimensionality`  returns at compile-time the number of dimensions of the container.
  - Class `FlatView` makes a multidimensional container appear as a linear range.
  - Class `BoxedView` (**not yet available**) makes a multidimensional container appear as a C array of user-defined bounds, allowing square-bracket access. The behaviour for an access out of the bounds can be customized (throw exception or return default element) 


### Version
0.0.1


### Installation

`multidim` is a header-only library. It requires a C++11-compliant compiler and the Boost libraries (a version not requiring Boost will be hopefully available in a near future)


### Development

`multidim` is still in pre-alpha stage


### License

MIT
