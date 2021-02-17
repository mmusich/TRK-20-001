// -*- C++ -*-
//
// Package:    PaperWork/BadComponentsCollector
// Class:      BadComponentsCollector
//
/**\class BadComponentsCollector BadComponentsCollector.cc PaperWork/BadComponentsCollector/plugins/BadComponentsCollector.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Marco Musich
//         Created:  Wed, 17 Feb 2021 09:27:52 GMT
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "CondFormats/DataRecord/interface/SiPixelQualityFromDbRcd.h"
#include "CondFormats/DataRecord/interface/SiPixelQualityRcd.h"
#include "CondFormats/DataRecord/interface/SiStripBadChannelRcd.h"
#include "CondFormats/SiPixelObjects/interface/SiPixelQuality.h"
#include "CondFormats/SiStripObjects/interface/SiStripBadStrip.h"
#include "CalibFormats/SiStripObjects/interface/SiStripQuality.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include <iostream>
#include <cstdio>
#include <sys/time.h>

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

class BadComponentsCollector : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit BadComponentsCollector(const edm::ParameterSet&);
  ~BadComponentsCollector();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // ----------member data ---------------------------
  std::string formatedOutput_;
  edm::ESGetToken<SiPixelQuality, SiPixelQualityFromDbRcd> pixQualityToken_;
  edm::ESGetToken<SiStripBadStrip, SiStripBadChannelRcd> badStripToken_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
BadComponentsCollector::BadComponentsCollector(const edm::ParameterSet& iConfig):
  formatedOutput_(iConfig.getUntrackedParameter<std::string>("outputFile", ""))
{
  pixQualityToken_ = esConsumes<SiPixelQuality, SiPixelQualityFromDbRcd>();
  badStripToken_ = esConsumes<SiStripBadStrip, SiStripBadChannelRcd>();
}

BadComponentsCollector::~BadComponentsCollector() {}

//
// member functions
//

// ------------ method called for each event  ------------
void BadComponentsCollector::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  FILE* pFile = nullptr;
  if (!formatedOutput_.empty())
    pFile = fopen(formatedOutput_.c_str(), "w");

  // pixel bad components
  const auto& pixelQuality = &iSetup.getData(pixQualityToken_);
  auto theDisabledModules = pixelQuality->getBadComponentList();
  for (const auto& mod : theDisabledModules) {
    if (pixelQuality->IsModuleBad(mod.DetID)) { 
      edm::LogPrint("BadComponentsCollector") << mod.DetID << std::endl;
      if (pFile) {
	fprintf(pFile, "%i ", mod.DetID);
	fprintf(pFile, "\n");
      }
    }
  }

  // strip bad components
  const auto& payload = &iSetup.getData(badStripToken_);
  SiStripQuality* siStripQuality_ = new SiStripQuality();
  siStripQuality_->add(payload);
  siStripQuality_->cleanUp();
  siStripQuality_->fillBadComponents();

  std::vector<uint32_t> detids;
  payload->getDetIds(detids);
  for (const auto& detid : detids) {
    if (siStripQuality_->IsModuleBad(detid)) { 
      edm::LogPrint("BadComponentsCollector") << detid << std::endl;
      if (pFile) {
	fprintf(pFile, "%i ", detid);
	fprintf(pFile, "\n");
      }
    }
  }
 if (pFile){
   fclose(pFile);
 }
}


// ------------ method called once each job just before starting event loop  ------------
void BadComponentsCollector::beginJob() {
  // please remove this method if not needed
}

// ------------ method called once each job just after ending the event loop  ------------
void BadComponentsCollector::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void BadComponentsCollector::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.addUntracked<std::string>("outputFile","badComponents.txt");
  descriptions.addWithDefaultLabel(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(BadComponentsCollector);
