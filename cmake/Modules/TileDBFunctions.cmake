function(tdb_status message)
  message(STATUS "${message}")
endfunction()

function(tdb_warn message)
  message(WARNING "${message}")
endfunction()

function(tdb_fatal message)
  message(FATAL_ERROR "${message}")
endfunction()
