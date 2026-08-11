#
set (subpkg "print")
set (dict   "Stntuple_${subpkg}_dict")
set (lib    "Stntuple_${subpkg}")

include_directories(
  $ENV{ROOTSYS}/include/root
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/${subpkg}
)

# link_directories($ENV{ROOTSYS}/lib)

set (include_files
  Stntuple_print_functions.hh TAnaDump.hh
)

ROOT_GENERATE_DICTIONARY(${dict} ${include_files} LINKDEF ${subpkg}/Linkdef.h)

set_property(SOURCE ${dict}.cxx PROPERTY generated)

set (source_files
  print_calo_cluster_colls.cc
  print_ch_colls.cc
  print_combo_hit_coll.cc
  print_cts_colls.cc
  print_genp_colls.cc
  print_helix_seed_colls.cc print_kalrep_colls.cc print_kalseed_colls.cc print_sd_colls.cc
  print_sdmc_colls.cc print_sh_colls.cc print_shf_colls.cc print_simp_colls.cc print_spmc_colls.cc
  print_tc_colls.cc
  TAnaDump.cc TAnaDump_comboHit.cc TAnaDump_helixSeed.cc TAnaDump_kalSeed.cc TAnaDump_SimParticle.cc
  TAnaDump_timeCluster.cc
  TAnaDump_CosmicTrackSeed.cc
)

add_library(${lib} SHARED ${source_files} ${dict}.cxx)

target_link_libraries(${lib}
  Stntuple_geom
  Stntuple_obj
  Stntuple_base
  Offline::CalPatRec
  Offline::CaloCluster
  #  Offline::TrkDiag
  Offline::TrkReco
  
  Offline::CalorimeterGeom
  Offline::GeometryService
  Offline::SeedService_SeedService_service
  Offline::GlobalConstantsService
  Offline::GlobalConstantsService_GlobalConstantsService_service
  Offline::Mu2eUtilities
  Offline::GeneralUtilities
  Offline::MCDataProducts
  Offline::RecoDataProducts
  Offline::DataProducts
  Offline::CaloMC

#  BTrk_BbrGeom
#  BTrk_TrkBase
#  BTrk_difAlgebra
#  BTrk_BField
#  BTrk_DetectorModel
#  BTrkLegacy
#  BTrk_MatEnv
#  BTrk_BaBar

  art::Persistency_Provenance
  
  ROOT::Physics ROOT::GenVector ROOT::Graf ROOT::Graf3d ROOT::Geom ROOT::Matrix ROOT::Core
)

cet_add_compiler_flags(-g -O0)

install(TARGETS ${lib})
# install_headers()
# install_source()
install(DIRECTORY ${subpkg} DESTINATION include/Stntuple)
