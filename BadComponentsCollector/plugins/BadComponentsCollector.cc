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

#include "CondCore/SiPixelPlugins/interface/Phase1PixelMaps.h"
#include "CondCore/SiStripPlugins/interface/SiStripTkMaps.h"

#include <iostream>
#include <cstdio>
#include <sys/time.h>

//
// class declaration
//

class BadComponentsCollector : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit BadComponentsCollector(const edm::ParameterSet&);
  ~BadComponentsCollector() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // ----------member data ---------------------------
  std::string formatedOutput_;
  edm::ESGetToken<SiPixelQuality, SiPixelQualityFromDbRcd> pixQualityToken_;
  edm::ESGetToken<SiStripBadStrip, SiStripBadChannelRcd> badStripToken_;

  std::unique_ptr<Phase1PixelMaps> pixelmap;
  std::unique_ptr<SiStripTkMaps> stripmap;
  std::unique_ptr<SiStripTkMaps> s_ghost;
};

//
// constructors and destructor
//
BadComponentsCollector::BadComponentsCollector(const edm::ParameterSet& iConfig)
    : formatedOutput_(iConfig.getUntrackedParameter<std::string>("outputFile", "")) {
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
    int subid = DetId(mod.DetID).subdetId();
    if (pixelQuality->IsModuleBad(mod.DetID)) {
      if (subid == PixelSubdetector::PixelBarrel) {
        pixelmap->fillBarrelBin("bcBarrel", mod.DetID, 1);
      } else {
        pixelmap->fillForwardBin("bcForward", mod.DetID, 1);
      }
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
      stripmap->fill(detid, 1);
      edm::LogPrint("BadComponentsCollector") << detid << std::endl;
      if (pFile) {
        fprintf(pFile, "%i ", detid);
        fprintf(pFile, "\n");
      }
    }
  }
  if (pFile) {
    fclose(pFile);
  }
}

// ------------ method called once each job just before starting event loop  ------------
void BadComponentsCollector::beginJob() {
  pixelmap = std::make_unique<Phase1PixelMaps>("COLZ0 L");
  pixelmap->bookBarrelHistograms("bcBarrel", "inactive modules", "inactive modules");
  pixelmap->bookBarrelBins("bcBarrel");
  pixelmap->bookForwardHistograms("bcForward", "inactive modules", "inactive modules");
  pixelmap->bookForwardBins("bcForward");

  std::string titleMap = "Bad components map";

  stripmap = std::make_unique<SiStripTkMaps>("COLZA0 L");
  stripmap->bookMap(titleMap, "Disabled Strip modules");

  s_ghost = std::make_unique<SiStripTkMaps>("AL");
  s_ghost->bookMap(titleMap, "");
}

// ------------ method called once each job just after ending the event loop  ------------
void BadComponentsCollector::endJob() {
  gStyle->SetPalette(kRainBow);
  pixelmap->beautifyAllHistograms();

  TCanvas cB("CanvBarrel", "CanvBarrel", 1200, 1000);
  pixelmap->DrawBarrelMaps("bcBarrel", cB);
  cB.SaveAs("pixelBarrelBadComponents.png");

  TCanvas cF("CanvForward", "CanvForward", 1600, 1000);
  pixelmap->DrawForwardMaps("bcForward", cF);
  cF.SaveAs("pixelForwardBadComponents.png");

  TCanvas cS("CanvStrips", "Strip disabled modules");
  stripmap->drawMap(cS, "");
  s_ghost->drawMap(cS, "same");
  cS.SaveAs("stripBadComponents.png");
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void BadComponentsCollector::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.addUntracked<std::string>("outputFile", "badComponents.txt");
  descriptions.addWithDefaultLabel(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(BadComponentsCollector);
