set(math_c99
  acosh
  asinh
  atanh
  cbrt
  copysign
  erfc
  erf
  expm1
  fmax
  fmin
  fpclassify
  hypot
  lgamma
  llround
  log1p
  lround
  nextafter
  nexttoward
  round
  tgamma
  trunc
)

set(math_tr1
  assoc_laguerre
  assoc_legendre
  beta
  comp_ellint_1
  comp_ellint_2
  comp_ellint_3
  cyl_bessel_i
  cyl_bessel_j
  cyl_bessel_k
  cyl_neumann
  ellint_1
  ellint_2
  ellint_3
  expint
  hermite
  laguerre
  legendre
  riemann_zeta
  sph_bessel
  sph_legendre
  sph_neumann
)

foreach(c99src ${math_c99})
  list(APPEND math_srcs
    ${BOOST_SOURCE}/libs/math/src/tr1/${c99src}.cpp
    ${BOOST_SOURCE}/libs/math/src/tr1/${c99src}f.cpp
  )
endforeach()

foreach(tr1src ${math_tr1})
  list(APPEND math_srcs
    ${BOOST_SOURCE}/libs/math/src/tr1/${tr1src}.cpp
    ${BOOST_SOURCE}/libs/math/src/tr1/${tr1src}f.cpp
  )
endforeach()

_add_boost_lib(
  NAME math
  SOURCES
    ${math_srcs}
)
target_include_directories(Boost_math PRIVATE ${BOOST_SOURCE}/libs/math/src/tr1)
