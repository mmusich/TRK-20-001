import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

process = cms.Process("BadComponentsCollector")

###################################################
# Prepare options
###################################################
options = VarParsing.VarParsing("analysis")

# options.register ('globalTag',
#                   "DONOTEXIST",
#                   VarParsing.VarParsing.multiplicity.singleton, # singleton or list
#                   VarParsing.VarParsing.varType.string,         # string, int, or float
#                   "GlobalTag")

options.register ('runNumber',
                  1,
                  VarParsing.VarParsing.multiplicity.singleton, # singleton or list
                  VarParsing.VarParsing.varType.int,            # string, int, or float
                  "run number")

options.parseArguments()

###################################################
# Global Tag
###################################################
#process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
#from Configuration.AlCa.GlobalTag import GlobalTag
#process.GlobalTag = GlobalTag(process.GlobalTag, options.globalTag, '')

###################################################
# Database output service
###################################################
process.load("CondCore.CondDB.CondDB_cfi")
process.CondDB.connect = "frontier://FrontierProd/CMS_CONDITIONS"
process.dbInput = cms.ESSource("PoolDBESSource",
                               process.CondDB,
                               toGet = cms.VPSet(cms.PSet(record = cms.string("SiPixelQualityFromDbRcd"),
                                                          #tag = cms.string("SiPixelQuality_phase1_2017_v10")  # 2017
                                                          tag = cms.string("SiPixelQuality_phase1_2018_v8_mc")  # 2018
                                                          ),
                                                 cms.PSet(record = cms.string("SiStripBadChannelRcd"),
                                                          #tag = cms.string("SiStripBadComponents_realisticMC_for2017_v1_mc")  # 2017
                                                          tag = cms.string("SiStripBadComponents_realisticMC_for2018_v0_mc")  # 2018
                                                          )
                                                 )
                               )

###################################################
# Empty Source
###################################################
process.source = cms.Source("EmptyIOVSource",
                            firstValue = cms.uint64(options.runNumber),
                            lastValue  = cms.uint64(options.runNumber),
                            timetype  = cms.string('runnumber'),
                            interval = cms.uint64(1))

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1))

###################################################################
# Analyzer
###################################################################
process.myanalysis = cms.EDAnalyzer("BadComponentsCollector",
                                    #outputFile = cms.untracked.string("2017_badComponentsList.txt")
                                    outputFile = cms.untracked.string("2018_badComponentsList.txt")
                                    )

###################################################################
# Path
###################################################################
process.p1 = cms.Path(process.myanalysis)
