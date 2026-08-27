###########################################
#
#  ConfigureChecks.cmake for Q4WIN10
#
###########################################

# Required TQt and TDE packages
find_package( TQt REQUIRED )
find_package( TDE REQUIRED )

tde_setup_architecture_flags()

include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)

tde_setup_largefiles()

if( WITH_GCC_VISIBILITY )
  tde_setup_gcc_visibility()
endif( WITH_GCC_VISIBILITY )
