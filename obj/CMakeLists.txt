#
set (subpkg "obj"                    )
set (dict   "Stntuple_${subpkg}_dict")
set (lib    "Stntuple_${subpkg}"     )

include_directories(
  $ENV{ROOTSYS}/include/root
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/${subpkg}
)

# link_directories($ENV{ROOTSYS}/lib)

set (include_files
  TCalDigiBlock.hh TCalDigi.hh
  TCalDataBlock.hh TCalHitData.hh TCrvClusterBlock.hh TCrvCoincidenceCluster.hh TCrvCoincidence.hh 
  TCrvPulseBlock.hh TCrvRecoPulse.hh TGenParticle.hh TGenpBlock.hh TSimParticle.hh TSimpBlock.hh 
  TStepPointMCBlock.hh TStepPointMC.hh TStnClusterBlock.hh TStnCluster.hh TStnDataBlock.hh
  TStnDBManager.hh TStnElectron.hh TStnErrorLogger.hh TStnEvent.hh TStnGoodRunList.hh 
  TStnHeaderBlock.hh TStnHelixBlock.hh TStnHelix.hh TStnLinkBlock.hh TStnNode.hh TStnPidBlock.hh 
  TStnPid.hh TStnRunSummary.hh TStnTimeClusterBlock.hh TStnTimeCluster.hh TStnTrackBlock.hh 
  TStnTrack.hh TStnTrackSeedBlock.hh TStnTrackSeed.hh TStnTriggerBlock.hh TStnTrigger.hh 
  TStnTriggerTable.hh
  TStrawDigiBlock.hh TStrawDigi.hh
  TStrawHitBlock.hh TStrawHit.hh TStrWaveform.hh TTrackStrawHitBlock.hh 
  TTrackStrawHit.hh TVDetDataBlock.hh TVDetHitData.hh 
)

ROOT_GENERATE_DICTIONARY(${dict} ${include_files} LINKDEF ${subpkg}/Linkdef.h)

set_property(SOURCE ${dict}.cxx PROPERTY generated)

set (source_files
  TCalDigiBlock.cc TCalDigi.cc
  TCalDataBlock.cc TCalHitData.cc TCrvClusterBlock.cc TCrvCoincidence.cc TCrvCoincidenceCluster.cc
  TCrvPulseBlock.cc TCrvRecoPulse.cc TGenParticle.cc TGenpBlock.cc TSimParticle.cc TSimpBlock.cc
  TStepPointMCBlock.cc TStepPointMC.cc TStnClusterBlock.cc TStnCluster.cc TStnDataBlock.cc
  TStnDBManager.cc TStnElectron.cc TStnErrorLogger.cc TStnEvent.cc TStnGoodRunList.cc TStnHeaderBlock.cc
  TStnHelixBlock.cc TStnHelix.cc TStnInitDataBlock.cc TStnLinkBlock.cc TStnNode.cc TStnPidBlock.cc
  TStnPid.cc TStnRunSummary.cc TStnTimeClusterBlock.cc TStnTimeCluster.cc TStnTrackBlock.cc TStnTrack.cc
  TStnTrackSeedBlock.cc TStnTrackSeed.cc TStnTriggerBlock.cc TStnTrigger.cc TStnTriggerTable.cc
  TStrawDigiBlock.cc TStrawDigi.cc
  TStrawHitBlock.cc TStrawHit.cc TStrWaveform.cc 
  TTrackStrawHit.cc TTrackStrawHitBlock.cc
  TVDetDataBlock.cc TVDetHitData.cc
)

add_library(${lib} SHARED ${source_files} ${dict}.cxx)

target_link_libraries(${lib}
  Stntuple_base
  ROOT::Physics ROOT::EG ROOT::Graf3d ROOT::Graf ROOT::Geom ROOT::Matrix ROOT::GenVector ROOT::Core)

cet_add_compiler_flags(-g -O0)

install(TARGETS ${lib})
# install_headers()
# install_source()
install(DIRECTORY ${subpkg} DESTINATION include/Stntuple)
