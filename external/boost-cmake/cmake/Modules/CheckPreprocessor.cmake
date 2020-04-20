include(CheckCXXSymbolExists)

function(check_preprocessor output_variable symbol)
  set(CMAKE_REQUIRED_QUIET OFF)
  check_cxx_symbol_exists(${symbol} "" ${output_variable})
  if(NOT ${output_variable})
    set(${output_variable} 0 CACHE INTERNAL "Have symbol ${symbol}" FORCE)
  endif()
endfunction()

