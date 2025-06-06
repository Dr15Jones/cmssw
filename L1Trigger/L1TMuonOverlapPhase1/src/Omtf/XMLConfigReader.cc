#include "L1Trigger/L1TMuonOverlapPhase1/interface/Omtf/XMLConfigReader.h"
#include "L1Trigger/L1TMuonOverlapPhase1/interface/Omtf/GoldenPattern.h"
#include "L1Trigger/L1TMuonOverlapPhase1/interface/Omtf/GoldenPatternWithStat.h"
#include "L1Trigger/L1TMuonOverlapPhase1/interface/Omtf/OMTFinput.h"
#include "L1Trigger/RPCTrigger/interface/RPCConst.h"

#include "CondFormats/L1TObjects/interface/L1TMuonOverlapParams.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "Utilities/Xerces/interface/Xerces.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <utility>
#include <array>

#include "xercesc/framework/StdOutFormatTarget.hpp"
#include "xercesc/framework/LocalFileFormatTarget.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/dom/DOM.hpp"
#include "xercesc/dom/DOMException.hpp"
#include "xercesc/dom/DOMImplementation.hpp"
#include "xercesc/sax/HandlerBase.hpp"
#include "xercesc/util/XMLString.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/XercesDefs.hpp"

#include <boost/multiprecision/integer.hpp>

XERCES_CPP_NAMESPACE_USE

//////////////////////////////////
// XMLConfigReader
//////////////////////////////////
inline std::string _toString(XMLCh const *toTranscode) {
  std::string tmp(xercesc::XMLString::transcode(toTranscode));
  return tmp;
}

inline XMLCh *_toDOMS(std::string temp) {
  XMLCh *buff = XMLString::transcode(temp.c_str());
  return buff;
}
////////////////////////////////////
////////////////////////////////////
XMLConfigReader::XMLConfigReader() { cms::concurrency::xercesInitialize(); }

XMLConfigReader::~XMLConfigReader() { cms::concurrency::xercesTerminate(); }
//////////////////////////////////////////////////
//////////////////////////////////////////////////
void XMLConfigReader::readLUTs(std::vector<l1t::LUT *> luts,
                               const L1TMuonOverlapParams &aConfig,
                               const std::vector<std::string> &types) {
  ///Fill payload string
  auto aGPs = readPatterns<GoldenPattern>(aConfig, patternsFiles, true);

  edm::LogVerbatim("OMTFReconstruction") << "XMLConfigReader::readLUTs: aGPs.size() " << aGPs.size()
                                         << " L1TMuonOverlapParams::nGoldenPatterns() " << aConfig.nGoldenPatterns()
                                         << std::endl;

  if ((int)aGPs.size() != aConfig.nGoldenPatterns()) {
    throw cms::Exception(
        "XMLConfigReader::readLUTs: aGPs.size() != aConfig.nGoldenPatterns(). Fix nGoldenPatterns in the "
        "hwToLogicLayer_0x000x.xml");
  }

  bool useMeanDistPhi1 = false;
  //if false, only getMeanDistPhi()[iLayer][iRefLayer][0]
  //if true,  also getMeanDistPhi()[iLayer][iRefLayer][1]

  for (unsigned int i = 0; i < luts.size(); i++) {
    l1t::LUT *lut = luts[i];
    const std::string &type = types[i];

    std::stringstream strStream;

    //totalInWidth and outWidth are initialized here for the type == "iCharge", "iEta", "iPt"
    //i.e. the global pattern parameters
    //Number of bits used to address LUT,
    int totalInWidth = boost::multiprecision::msb(aGPs.size()) + 1;  //=7 in run2 and run3 patterns
    //Number of bits used to store LUT value
    int outWidth = 6;

    if (type == "iCharge")
      outWidth = 1;
    if (type == "iEta")
      outWidth = 2;
    if (type == "iPt")
      outWidth = 9;
    if (type == "meanDistPhi") {
      outWidth = aConfig.nPhiBits();

      int meanDistPhiValCnt =
          aGPs.size() * aGPs.at(0)->getMeanDistPhi().size() * aGPs.at(0)->getMeanDistPhi()[0].size();

      totalInWidth = boost::multiprecision::msb(meanDistPhiValCnt) + 1;  //totalInWidth = 14;
      //the index of msb is zero-based, so +1 is needed to have the number of bits

      if (useMeanDistPhi1)
        totalInWidth = totalInWidth + 1;
      //if two meanDistPhi values for each gp, iLayer,iRefLayer, are used - we need one bit more for the address

      edm::LogVerbatim("OMTFReconstruction")
          << "XMLConfigReader::readLUTs: meanDistPhi LUT address width: " << totalInWidth
          << " meanDistPhiValCnt: " << meanDistPhiValCnt << " useMeanDistPhi1 " << useMeanDistPhi1 << std::endl;
    }
    if (type == "pdf") {
      outWidth = aConfig.nPdfValBits();
      int pdfValCnt = aGPs.size() * aGPs.at(0)->getPdf().num_elements();
      totalInWidth = boost::multiprecision::msb(pdfValCnt) + 1;  //totalInWidth = 21;
      //the index of msb is zero-based, so +1 is needed to have the number of bits

      edm::LogVerbatim("OMTFReconstruction") << "XMLConfigReader::readLUTs: pdf LUT address width: " << totalInWidth
                                             << " pdfValCnt: " << pdfValCnt << std::endl;
    }
    if (type == "selDistPhiShift") {
      outWidth = 2;

      int distPhiShiftValCnt =
          aGPs.size() * aGPs.at(0)->getMeanDistPhi().size() * aGPs.at(0)->getMeanDistPhi()[0].size();
      //distPhiShiftValCnt = aGPs.size() * omtfConfig->nLayers() * omtfConfig->nRefLayers() - should give the same as above

      totalInWidth = boost::multiprecision::msb(distPhiShiftValCnt) + 1;  //totalInWidth = 14;
      //the index of msb is zero-based, so +1 is needed to have the number of bits

      edm::LogVerbatim("OMTFReconstruction")
          << "XMLConfigReader::readLUTs: distPhiShift LUT address width: " << totalInWidth
          << " distPhiShiftValCnt: " << distPhiShiftValCnt << std::endl;
    }

    ///Prepare the header
    strStream << "#<header> V1 " << totalInWidth << " " << outWidth << " </header> " << std::endl;

    unsigned int in = 0;
    int out = 0;
    for (auto &it : aGPs) {
      if (type == "iCharge")
        out = it->key().theCharge == -1 ? 0 : 1;
      //changing only -1 (negative charge) to 0 (to avoid negative numbers in LUT?) -N.B. that this is not the uGMT charge convention!!!!
      if (type == "iEta")
        out = it->key().theEtaCode;
      if (type == "iPt")
        out = it->key().thePt;
      if (type == "meanDistPhi") {
        int meanDistPhiSize = aConfig.nGoldenPatterns() * aConfig.nLayers() * aConfig.nRefLayers();
        for (unsigned int iLayer = 0; iLayer < (unsigned)aConfig.nLayers(); ++iLayer) {
          for (unsigned int iRefLayer = 0; iRefLayer < (unsigned)aConfig.nRefLayers(); ++iRefLayer) {
            out = (1 << (outWidth - 1)) + it->getMeanDistPhi()[iLayer][iRefLayer][0];
            //making the LUT values positive - it is needed because the outWidth is not 32 and the dataMask_ in LUT affects the negative values. Would be better to just use outWidth=32
            strStream << in << " " << out << std::endl;

            if (useMeanDistPhi1) {
              out = (1 << (outWidth - 1)) + it->getMeanDistPhi()[iLayer][iRefLayer][1];
              //making the LUT values positive - it is needed because the outWidth is not 32 and the dataMask_ in LUT affects the negative values. Would be better to just use outWidth=32
              strStream << (in + meanDistPhiSize) << " " << out << std::endl;
              //writing the second value of the getMeanDistPhi at the position (in+meanDistPhiSize)
            }
            ++in;
          }
        }
      }
      if (type == "selDistPhiShift") {
        for (unsigned int iLayer = 0; iLayer < (unsigned)aConfig.nLayers(); ++iLayer) {
          for (unsigned int iRefLayer = 0; iRefLayer < (unsigned)aConfig.nRefLayers(); ++iRefLayer) {
            out = it->getDistPhiBitShift(iLayer, iRefLayer);
            strStream << in << " " << out << std::endl;
            ++in;
          }
        }
      }

      //edm::LogVerbatim("OMTFReconstruction")<<"serializing pattern "<<it->key()<<std::endl;
      if (type == "pdf") {
        for (unsigned int iLayer = 0; iLayer < (unsigned)aConfig.nLayers(); ++iLayer) {
          for (unsigned int iRefLayer = 0; iRefLayer < (unsigned)aConfig.nRefLayers(); ++iRefLayer) {
            for (unsigned int iPdf = 0; iPdf < exp2(aConfig.nPdfAddrBits()); ++iPdf) {
              out = it->pdfValue(iLayer, iRefLayer, iPdf);
              strStream << in << " " << out << std::endl;
              //edm::LogVerbatim("OMTFReconstruction")<<" iLayer "<<iLayer<<" iRefLayer "<<iRefLayer<<" iPdf "<<iPdf << " address "<<in<<" value "<<out<<std::endl;
              ++in;
            }
          }
        }
      }
      if (type != "meanDistPhi" && type != "pdf" && type != "selDistPhiShift") {
        strStream << in << " " << out << std::endl;
        ++in;
      }
    }

    ///Read the data into LUT
    int result = lut->read(strStream);

    if (result != l1t::LUT::SUCCESS) {
      throw cms::Exception(
          "OMTF::XMLConfigReader::readLUTs: lut->read(strStream) did not returned l1t::LUT::SUCCESS but " +
          std::to_string(result));
    }
  }
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
unsigned int XMLConfigReader::getPatternsVersion() const {
  if (patternsFiles.empty())
    return 0;
  std::string patternsFile = patternsFiles[0];
  if (patternsFile.empty())
    return 0;

  unsigned int version = 0;
  {
    XercesDOMParser parser;
    parser.setValidationScheme(XercesDOMParser::Val_Auto);
    parser.setDoNamespaces(false);

    parser.parse(patternsFile.c_str());
    xercesc::DOMDocument *doc = parser.getDocument();
    assert(doc);

    XMLCh *xmlOmtf = _toDOMS("OMTF");
    XMLCh *xmlVersion = _toDOMS("version");
    DOMNode *aNode = doc->getElementsByTagName(xmlOmtf)->item(0);
    DOMElement *aOMTFElement = static_cast<DOMElement *>(aNode);

    version = std::stoul(_toString(aOMTFElement->getAttribute(xmlVersion)), nullptr, 16);
    XMLString::release(&xmlOmtf);
    XMLString::release(&xmlVersion);
    parser.resetDocumentPool();
  }

  return version;
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
template <class GoldenPatternType>
GoldenPatternVec<GoldenPatternType> XMLConfigReader::readPatterns(const L1TMuonOverlapParams &aConfig,
                                                                  const std::string &patternsFile,
                                                                  bool buildEmptyPatterns,
                                                                  bool resetNumbering) {
  GoldenPatternVec<GoldenPatternType> aGPs;
  aGPs.clear();

  if (resetNumbering) {
    iGPNumber = 0;
    iPatternGroup = 0;
  }

  XMLCh *xmlGP = _toDOMS("GP");
  std::array<XMLCh *, 4> xmliPt = {{_toDOMS("iPt1"), _toDOMS("iPt2"), _toDOMS("iPt3"), _toDOMS("iPt4")}};

  {
    XercesDOMParser parser;
    parser.setValidationScheme(XercesDOMParser::Val_Auto);
    parser.setDoNamespaces(false);

    parser.parse(patternsFile.c_str());
    xercesc::DOMDocument *doc = parser.getDocument();
    assert(doc);

    unsigned int nElem = doc->getElementsByTagName(xmlGP)->getLength();
    if (nElem < 1) {
      edm::LogError("critical") << "Problem parsing XML file " << patternsFile << std::endl;
      edm::LogError("critical") << "No GoldenPattern items: GP found" << std::endl;
      return aGPs;
    }

    DOMNode *aNode = nullptr;
    DOMElement *aGPElement = nullptr;
    //unsigned int iGPNumber=0;

    for (unsigned int iItem = 0; iItem < nElem; ++iItem, ++iPatternGroup) {
      aNode = doc->getElementsByTagName(xmlGP)->item(iItem);
      aGPElement = static_cast<DOMElement *>(aNode);

      for (unsigned int index = 1; index < 5; ++index) {
        ///Patterns XML format backward compatibility. Can use both packed by 4, or by 1 XML files.
        if (aGPElement->getAttributeNode(xmliPt[index - 1])) {
          std::unique_ptr<GoldenPatternType> aGP =
              buildGP<GoldenPatternType>(aGPElement, aConfig, iPatternGroup, index, iGPNumber);
          if (aGP && (aGP->key().thePt || buildEmptyPatterns)) {
            aGPs.emplace_back(std::move(aGP));
            iGPNumber++;
          }
        } else {
          std::unique_ptr<GoldenPatternType> aGP = buildGP<GoldenPatternType>(aGPElement, aConfig, iPatternGroup);
          if (aGP && (aGP->key().thePt || buildEmptyPatterns)) {
            aGPs.emplace_back(std::move(aGP));
            iGPNumber++;
          }
          break;
        }
      }
    }

    // Reset the documents vector pool and release all the associated memory back to the system.
    //parser->resetDocumentPool();
    parser.resetDocumentPool();
  }
  XMLString::release(&xmlGP);
  XMLString::release(&xmliPt[0]);
  XMLString::release(&xmliPt[1]);
  XMLString::release(&xmliPt[2]);
  XMLString::release(&xmliPt[3]);

  return aGPs;
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
template <class GoldenPatternType>
GoldenPatternVec<GoldenPatternType> XMLConfigReader::readPatterns(const L1TMuonOverlapParams &aConfig,
                                                                  const std::vector<std::string> &patternsFiles,
                                                                  bool buildEmptyPatterns) {
  iGPNumber = 0;
  iPatternGroup = 0;
  GoldenPatternVec<GoldenPatternType> aGPs;
  for (const auto &aPatternsFile : patternsFiles) {
    auto tmpGPs = readPatterns<GoldenPatternType>(aConfig, aPatternsFile, buildEmptyPatterns, false);
    for (auto &gp : tmpGPs)
      aGPs.push_back(std::move(gp));
  }
  return aGPs;
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
template <class GoldenPatternType>
std::unique_ptr<GoldenPatternType> XMLConfigReader::buildGP(DOMElement *aGPElement,
                                                            const L1TMuonOverlapParams &aConfig,
                                                            unsigned int patternGroup,
                                                            unsigned int index,
                                                            unsigned int aGPNumber) {
  XMLCh *xmliEta = _toDOMS("iEta");
  //index 0 means no number at the end
  std::ostringstream stringStr;
  if (index > 0)
    stringStr << "iPt" << index;
  else
    stringStr.str("iPt");
  XMLCh *xmliPt = _toDOMS(stringStr.str());
  stringStr.str("");

  if (index > 0)
    stringStr << "value" << index;
  else
    stringStr.str("value");
  XMLCh *xmlValue = _toDOMS(stringStr.str());

  XMLCh *xmliCharge = _toDOMS("iCharge");
  XMLCh *xmlLayer = _toDOMS("Layer");
  XMLCh *xmlRefLayer = _toDOMS("RefLayer");
  XMLCh *xmlmeanDistPhi = _toDOMS("meanDistPhi");  //for old version

  XMLCh *xmlmeanDistPhi0 = _toDOMS("meanDistPhi0");  //for new version
  XMLCh *xmlmeanDistPhi1 = _toDOMS("meanDistPhi1");  //for new version

  XMLCh *xmlSelDistPhiShift = _toDOMS("selDistPhiShift");

  XMLCh *xmlPDF = _toDOMS("PDF");

  unsigned int iPt = std::atoi(_toString(aGPElement->getAttribute(xmliPt)).c_str());
  int iEta = std::atoi(_toString(aGPElement->getAttribute(xmliEta)).c_str());
  int iCharge = std::atoi(_toString(aGPElement->getAttribute(xmliCharge)).c_str());
  unsigned int nLayers = aGPElement->getElementsByTagName(xmlLayer)->getLength();

  if (nLayers)
    assert(nLayers == (unsigned)aConfig.nLayers());

  DOMNode *aNode = nullptr;
  DOMElement *aLayerElement = nullptr;
  DOMElement *aItemElement = nullptr;

  if (iPt == 0) {  ///Build empty GP
    Key aKey(iEta, iPt, iCharge, aGPNumber);
    auto aGP =
        std::make_unique<GoldenPatternType>(aKey, aConfig.nLayers(), aConfig.nRefLayers(), aConfig.nPdfAddrBits());
    return aGP;
  }

  stringStr.str("");
  XMLCh *xmlRefLayerThresh = _toDOMS("RefLayerThresh");
  if (index > 0)
    stringStr << "tresh" << index;
  else
    stringStr.str("tresh");
  XMLCh *xmlTresh = _toDOMS(stringStr.str());
  stringStr.str("");

  std::vector<PdfValueType> thresholds(aConfig.nRefLayers(), 0);
  unsigned int nItems = aGPElement->getElementsByTagName(xmlRefLayerThresh)->getLength();
  if (nItems > 0 && nItems != thresholds.size()) {
    throw cms::Exception("OMTF::XMLConfigReader: nItems != thresholds.size()");
  }
  for (unsigned int iItem = 0; iItem < nItems; ++iItem) {
    aNode = aGPElement->getElementsByTagName(xmlRefLayerThresh)->item(iItem);
    aItemElement = dynamic_cast<DOMElement *>(aNode);
    if (aItemElement == nullptr)
      throw cms::Exception("OMTF::XMLConfigReader: aItemElement is 0");
    std::string strVal = _toString(aItemElement->getAttribute(xmlTresh));
    thresholds[iItem] = std::stof(strVal);
  }

  ///Loop over layers
  Key aKey(iEta, iPt, iCharge, aGPNumber);
  aKey.theGroup = patternGroup;
  aKey.theIndexInGroup = index;
  auto aGP = std::make_unique<GoldenPatternType>(aKey, aConfig.nLayers(), aConfig.nRefLayers(), aConfig.nPdfAddrBits());
  if (dynamic_cast<GoldenPatternWithThresh *>(aGP.get())) {
    dynamic_cast<GoldenPatternWithThresh *>(aGP.get())->setThresholds(thresholds);
  }
  for (unsigned int iLayer = 0; iLayer < nLayers; ++iLayer) {
    aNode = aGPElement->getElementsByTagName(xmlLayer)->item(iLayer);
    aLayerElement = static_cast<DOMElement *>(aNode);
    ///MeanDistPhi vector
    unsigned int nItems = aLayerElement->getElementsByTagName(xmlRefLayer)->getLength();
    assert(nItems == (unsigned)aConfig.nRefLayers());
    for (unsigned int iItem = 0; iItem < nItems; ++iItem) {
      aNode = aLayerElement->getElementsByTagName(xmlRefLayer)->item(iItem);
      aItemElement = static_cast<DOMElement *>(aNode);

      std::string strVal = _toString(aItemElement->getAttribute(xmlmeanDistPhi));
      if (!strVal.empty()) {
        aGP->setMeanDistPhiValue(std::stoi(strVal), iLayer, iItem, 0);
      } else {
        strVal = _toString(aItemElement->getAttribute(xmlmeanDistPhi0));
        aGP->setMeanDistPhiValue(std::stoi(strVal), iLayer, iItem, 0);
        strVal = _toString(aItemElement->getAttribute(xmlmeanDistPhi1));
        aGP->setMeanDistPhiValue(std::stoi(strVal), iLayer, iItem, 1);
      }

      strVal = _toString(aItemElement->getAttribute(xmlSelDistPhiShift));
      if (!strVal.empty()) {
        aGP->setDistPhiBitShift(std::stoi(strVal), iLayer, iItem);
      }
    }

    ///PDF vector
    nItems = aLayerElement->getElementsByTagName(xmlPDF)->getLength();

    assert(nItems == aConfig.nRefLayers() * exp2(aConfig.nPdfAddrBits()));
    for (unsigned int iRefLayer = 0; iRefLayer < (unsigned)aConfig.nRefLayers(); ++iRefLayer) {
      for (unsigned int iPdf = 0; iPdf < exp2(aConfig.nPdfAddrBits()); ++iPdf) {
        aNode = aLayerElement->getElementsByTagName(xmlPDF)->item(iRefLayer * exp2(aConfig.nPdfAddrBits()) + iPdf);
        aItemElement = static_cast<DOMElement *>(aNode);
        PdfValueType val = std::atof(_toString(aItemElement->getAttribute(xmlValue)).c_str());
        aGP->setPdfValue(val, iLayer, iRefLayer, iPdf);
      }
    }
  }
  XMLString::release(&xmliEta);
  XMLString::release(&xmliPt);
  XMLString::release(&xmliCharge);
  XMLString::release(&xmlLayer);
  XMLString::release(&xmlRefLayer);
  XMLString::release(&xmlmeanDistPhi);
  XMLString::release(&xmlPDF);
  XMLString::release(&xmlValue);

  return aGP;
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
std::vector<std::vector<int> > XMLConfigReader::readEvent(unsigned int iEvent, unsigned int iProcessor, bool readEta) {
  return std::vector<std::vector<int> >();
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
void XMLConfigReader::readConfig(L1TMuonOverlapParams *aConfig) const {
  {
    XercesDOMParser parser;
    parser.setValidationScheme(XercesDOMParser::Val_Auto);
    parser.setDoNamespaces(false);

    XMLCh *xmlOMTF = _toDOMS("OMTF");
    XMLCh *xmlversion = _toDOMS("version");
    XMLCh *xmlGlobalData = _toDOMS("GlobalData");
    XMLCh *xmlnPdfAddrBits = _toDOMS("nPdfAddrBits");
    XMLCh *xmlnPdfValBits = _toDOMS("nPdfValBits");
    XMLCh *xmlnPhiBits = _toDOMS("nPhiBits");
    XMLCh *xmlnPhiBins = _toDOMS("nPhiBins");
    XMLCh *xmlnProcessors = _toDOMS("nProcessors");
    XMLCh *xmlnLogicRegions = _toDOMS("nLogicRegions");
    XMLCh *xmlnInputs = _toDOMS("nInputs");
    XMLCh *xmlnLayers = _toDOMS("nLayers");
    XMLCh *xmlnRefLayers = _toDOMS("nRefLayers");
    XMLCh *xmliProcessor = _toDOMS("iProcessor");
    XMLCh *xmlbarrelMin = _toDOMS("barrelMin");
    XMLCh *xmlbarrelMax = _toDOMS("barrelMax");
    XMLCh *xmlendcap10DegMin = _toDOMS("endcap10DegMin");
    XMLCh *xmlendcap10DegMax = _toDOMS("endcap10DegMax");
    XMLCh *xmlendcap20DegMin = _toDOMS("endcap20DegMin");
    XMLCh *xmlendcap20DegMax = _toDOMS("endcap20DegMax");
    XMLCh *xmlLayerMap = _toDOMS("LayerMap");
    XMLCh *xmlhwNumber = _toDOMS("hwNumber");
    XMLCh *xmllogicNumber = _toDOMS("logicNumber");
    XMLCh *xmlbendingLayer = _toDOMS("bendingLayer");
    XMLCh *xmlconnectedToLayer = _toDOMS("connectedToLayer");
    XMLCh *xmlRefLayerMap = _toDOMS("RefLayerMap");
    XMLCh *xmlrefLayer = _toDOMS("refLayer");
    XMLCh *xmlProcessor = _toDOMS("Processor");
    XMLCh *xmlRefLayer = _toDOMS("RefLayer");
    XMLCh *xmliRefLayer = _toDOMS("iRefLayer");
    XMLCh *xmliGlobalPhiStart = _toDOMS("iGlobalPhiStart");
    XMLCh *xmlRefHit = _toDOMS("RefHit");
    XMLCh *xmliRefHit = _toDOMS("iRefHit");
    XMLCh *xmliPhiMin = _toDOMS("iPhiMin");
    XMLCh *xmliPhiMax = _toDOMS("iPhiMax");
    XMLCh *xmliInput = _toDOMS("iInput");
    XMLCh *xmliRegion = _toDOMS("iRegion");
    XMLCh *xmlLogicRegion = _toDOMS("LogicRegion");
    XMLCh *xmlLayer = _toDOMS("Layer");
    XMLCh *xmliLayer = _toDOMS("iLayer");
    XMLCh *xmliFirstInput = _toDOMS("iFirstInput");
    XMLCh *xmlnHitsPerLayer = _toDOMS("nHitsPerLayer");
    XMLCh *xmlnRefHits = _toDOMS("nRefHits");
    XMLCh *xmlnTestRefHits = _toDOMS("nTestRefHits");
    XMLCh *xmlnGoldenPatterns = _toDOMS("nGoldenPatterns");
    XMLCh *xmlConnectionMap = _toDOMS("ConnectionMap");
    parser.parse(configFile.c_str());
    xercesc::DOMDocument *doc = parser.getDocument();
    assert(doc);
    unsigned int nElem = doc->getElementsByTagName(xmlOMTF)->getLength();
    if (nElem != 1) {
      edm::LogError("critical") << "Problem parsing XML file " << configFile << std::endl;
      assert(nElem == 1);
    }
    DOMNode *aNode = doc->getElementsByTagName(xmlOMTF)->item(0);
    DOMElement *aOMTFElement = static_cast<DOMElement *>(aNode);

    unsigned int version = std::stoul(_toString(aOMTFElement->getAttribute(xmlversion)), nullptr, 16);
    aConfig->setFwVersion(version);

    ///Addresing bits numbers
    nElem = aOMTFElement->getElementsByTagName(xmlGlobalData)->getLength();
    assert(nElem == 1);
    aNode = aOMTFElement->getElementsByTagName(xmlGlobalData)->item(0);
    DOMElement *aElement = static_cast<DOMElement *>(aNode);

    unsigned int nPdfAddrBits = std::atoi(_toString(aElement->getAttribute(xmlnPdfAddrBits)).c_str());
    unsigned int nPdfValBits = std::atoi(_toString(aElement->getAttribute(xmlnPdfValBits)).c_str());
    unsigned int nHitsPerLayer = std::atoi(_toString(aElement->getAttribute(xmlnHitsPerLayer)).c_str());
    unsigned int nPhiBits = std::atoi(_toString(aElement->getAttribute(xmlnPhiBits)).c_str());
    unsigned int nPhiBins = std::atoi(_toString(aElement->getAttribute(xmlnPhiBins)).c_str());

    unsigned int nRefHits = std::atoi(_toString(aElement->getAttribute(xmlnRefHits)).c_str());
    unsigned int nTestRefHits = std::atoi(_toString(aElement->getAttribute(xmlnTestRefHits)).c_str());
    unsigned int nProcessors = std::atoi(_toString(aElement->getAttribute(xmlnProcessors)).c_str());
    unsigned int nLogicRegions = std::atoi(_toString(aElement->getAttribute(xmlnLogicRegions)).c_str());
    unsigned int nInputs = std::atoi(_toString(aElement->getAttribute(xmlnInputs)).c_str());
    unsigned int nLayers = std::atoi(_toString(aElement->getAttribute(xmlnLayers)).c_str());
    unsigned int nRefLayers = std::atoi(_toString(aElement->getAttribute(xmlnRefLayers)).c_str());
    unsigned int nGoldenPatterns = std::atoi(_toString(aElement->getAttribute(xmlnGoldenPatterns)).c_str());

    std::vector<int> paramsVec(L1TMuonOverlapParams::GENERAL_NCONFIG);
    paramsVec[L1TMuonOverlapParams::GENERAL_ADDRBITS] = nPdfAddrBits;
    paramsVec[L1TMuonOverlapParams::GENERAL_VALBITS] = nPdfValBits;
    paramsVec[L1TMuonOverlapParams::GENERAL_HITSPERLAYER] = nHitsPerLayer;
    paramsVec[L1TMuonOverlapParams::GENERAL_PHIBITS] = nPhiBits;
    paramsVec[L1TMuonOverlapParams::GENERAL_PHIBINS] = nPhiBins;
    paramsVec[L1TMuonOverlapParams::GENERAL_NREFHITS] = nRefHits;
    paramsVec[L1TMuonOverlapParams::GENERAL_NTESTREFHITS] = nTestRefHits;
    paramsVec[L1TMuonOverlapParams::GENERAL_NPROCESSORS] = nProcessors;
    paramsVec[L1TMuonOverlapParams::GENERAL_NLOGIC_REGIONS] = nLogicRegions;
    paramsVec[L1TMuonOverlapParams::GENERAL_NINPUTS] = nInputs;
    paramsVec[L1TMuonOverlapParams::GENERAL_NLAYERS] = nLayers;
    paramsVec[L1TMuonOverlapParams::GENERAL_NREFLAYERS] = nRefLayers;
    paramsVec[L1TMuonOverlapParams::GENERAL_NGOLDENPATTERNS] = nGoldenPatterns;
    aConfig->setGeneralParams(paramsVec);

    ///Chamber sectors connections to logic processors.
    ///Start/End values for all processors, and chamber types are put into a single vector
    std::vector<int> sectorsStart(3 * nProcessors), sectorsEnd(3 * nProcessors);
    nElem = aOMTFElement->getElementsByTagName(xmlConnectionMap)->getLength();
    DOMElement *aConnectionElement = nullptr;
    for (unsigned int i = 0; i < nElem; ++i) {
      aNode = aOMTFElement->getElementsByTagName(xmlConnectionMap)->item(i);
      aConnectionElement = static_cast<DOMElement *>(aNode);
      unsigned int iProcessor = std::atoi(_toString(aConnectionElement->getAttribute(xmliProcessor)).c_str());
      unsigned int barrelMin = std::atoi(_toString(aConnectionElement->getAttribute(xmlbarrelMin)).c_str());
      unsigned int barrelMax = std::atoi(_toString(aConnectionElement->getAttribute(xmlbarrelMax)).c_str());
      unsigned int endcap10DegMin = std::atoi(_toString(aConnectionElement->getAttribute(xmlendcap10DegMin)).c_str());
      unsigned int endcap10DegMax = std::atoi(_toString(aConnectionElement->getAttribute(xmlendcap10DegMax)).c_str());
      unsigned int endcap20DegMin = std::atoi(_toString(aConnectionElement->getAttribute(xmlendcap20DegMin)).c_str());
      unsigned int endcap20DegMax = std::atoi(_toString(aConnectionElement->getAttribute(xmlendcap20DegMax)).c_str());

      sectorsStart[iProcessor] = barrelMin;
      sectorsStart[iProcessor + nProcessors] = endcap10DegMin;
      sectorsStart[iProcessor + 2 * nProcessors] = endcap20DegMin;

      sectorsEnd[iProcessor] = barrelMax;
      sectorsEnd[iProcessor + nProcessors] = endcap10DegMax;
      sectorsEnd[iProcessor + 2 * nProcessors] = endcap20DegMax;
    }
    aConfig->setConnectedSectorsStart(sectorsStart);
    aConfig->setConnectedSectorsEnd(sectorsEnd);

    ///hw <-> logic numbering map
    std::vector<L1TMuonOverlapParams::LayerMapNode> aLayerMapVec;
    L1TMuonOverlapParams::LayerMapNode aLayerMapNode;

    nElem = aOMTFElement->getElementsByTagName(xmlLayerMap)->getLength();
    DOMElement *aLayerElement = nullptr;
    for (unsigned int i = 0; i < nElem; ++i) {
      aNode = aOMTFElement->getElementsByTagName(xmlLayerMap)->item(i);
      aLayerElement = static_cast<DOMElement *>(aNode);
      unsigned int hwNumber = std::atoi(_toString(aLayerElement->getAttribute(xmlhwNumber)).c_str());
      unsigned int logicNumber = std::atoi(_toString(aLayerElement->getAttribute(xmllogicNumber)).c_str());
      unsigned int isBendingLayer = std::atoi(_toString(aLayerElement->getAttribute(xmlbendingLayer)).c_str());
      unsigned int iConnectedLayer = std::atoi(_toString(aLayerElement->getAttribute(xmlconnectedToLayer)).c_str());
      aLayerMapNode.logicNumber = logicNumber;
      aLayerMapNode.hwNumber = hwNumber;
      aLayerMapNode.connectedToLayer = iConnectedLayer;
      aLayerMapNode.bendingLayer = isBendingLayer;
      aLayerMapVec.push_back(aLayerMapNode);
    }
    aConfig->setLayerMap(aLayerMapVec);

    ///ref<->logic numberig map
    std::vector<L1TMuonOverlapParams::RefLayerMapNode> aRefLayerMapVec;
    L1TMuonOverlapParams::RefLayerMapNode aRefLayerNode;

    nElem = aOMTFElement->getElementsByTagName(xmlRefLayerMap)->getLength();
    DOMElement *aRefLayerElement = nullptr;
    for (unsigned int i = 0; i < nElem; ++i) {
      aNode = aOMTFElement->getElementsByTagName(xmlRefLayerMap)->item(i);
      aRefLayerElement = static_cast<DOMElement *>(aNode);
      unsigned int refLayer = std::atoi(_toString(aRefLayerElement->getAttribute(xmlrefLayer)).c_str());
      unsigned int logicNumber = std::atoi(_toString(aRefLayerElement->getAttribute(xmllogicNumber)).c_str());
      aRefLayerNode.refLayer = refLayer;
      aRefLayerNode.logicNumber = logicNumber;
      aRefLayerMapVec.push_back(aRefLayerNode);
    }
    aConfig->setRefLayerMap(aRefLayerMapVec);

    std::vector<int> aGlobalPhiStartVec(nProcessors * nRefLayers);

    std::vector<L1TMuonOverlapParams::RefHitNode> aRefHitMapVec(nProcessors * nRefHits);
    L1TMuonOverlapParams::RefHitNode aRefHitNode;

    std::vector<L1TMuonOverlapParams::LayerInputNode> aLayerInputMapVec(nProcessors * nLogicRegions * nLayers);
    L1TMuonOverlapParams::LayerInputNode aLayerInputNode;

    nElem = aOMTFElement->getElementsByTagName(xmlProcessor)->getLength();
    assert(nElem == nProcessors);
    DOMElement *aProcessorElement = nullptr;
    for (unsigned int i = 0; i < nElem; ++i) {
      aNode = aOMTFElement->getElementsByTagName(xmlProcessor)->item(i);
      aProcessorElement = static_cast<DOMElement *>(aNode);
      unsigned int iProcessor = std::atoi(_toString(aProcessorElement->getAttribute(xmliProcessor)).c_str());
      unsigned int nElem1 = aProcessorElement->getElementsByTagName(xmlRefLayer)->getLength();
      assert(nElem1 == nRefLayers);
      DOMElement *aRefLayerElement = nullptr;
      for (unsigned int ii = 0; ii < nElem1; ++ii) {
        aNode = aProcessorElement->getElementsByTagName(xmlRefLayer)->item(ii);
        aRefLayerElement = static_cast<DOMElement *>(aNode);
        unsigned int iRefLayer = std::atoi(_toString(aRefLayerElement->getAttribute(xmliRefLayer)).c_str());
        int iPhi = std::atoi(_toString(aRefLayerElement->getAttribute(xmliGlobalPhiStart)).c_str());
        aGlobalPhiStartVec[iRefLayer + iProcessor * nRefLayers] = iPhi;
      }
      ///////////
      nElem1 = aProcessorElement->getElementsByTagName(xmlRefHit)->getLength();
      assert((iProcessor == 0 && nElem1 == nRefHits) || (iProcessor != 0 && nElem1 == 0));
      DOMElement *aRefHitElement = nullptr;
      for (unsigned int ii = 0; ii < nElem1; ++ii) {
        aNode = aProcessorElement->getElementsByTagName(xmlRefHit)->item(ii);
        aRefHitElement = static_cast<DOMElement *>(aNode);
        unsigned int iRefHit = std::atoi(_toString(aRefHitElement->getAttribute(xmliRefHit)).c_str());
        int iPhiMin = std::atoi(_toString(aRefHitElement->getAttribute(xmliPhiMin)).c_str());
        int iPhiMax = std::atoi(_toString(aRefHitElement->getAttribute(xmliPhiMax)).c_str());
        unsigned int iInput = std::atoi(_toString(aRefHitElement->getAttribute(xmliInput)).c_str());
        unsigned int iRegion = std::atoi(_toString(aRefHitElement->getAttribute(xmliRegion)).c_str());
        unsigned int iRefLayer = std::atoi(_toString(aRefHitElement->getAttribute(xmliRefLayer)).c_str());

        aRefHitNode.iRefHit = iRefHit;
        aRefHitNode.iPhiMin = iPhiMin;
        aRefHitNode.iPhiMax = iPhiMax;
        aRefHitNode.iInput = iInput;
        aRefHitNode.iRegion = iRegion;
        aRefHitNode.iRefLayer = iRefLayer;
        for (unsigned int iProcessor = 0; iProcessor < nProcessors; iProcessor++)
          aRefHitMapVec[iRefHit + iProcessor * nRefHits] = aRefHitNode;
      }
      ///////////
      unsigned int nElem2 = aProcessorElement->getElementsByTagName(xmlLogicRegion)->getLength();
      assert((iProcessor == 0 && nElem2 == nLogicRegions) || (iProcessor != 0 && nElem2 == 0));
      DOMElement *aRegionElement = nullptr;
      for (unsigned int ii = 0; ii < nElem2; ++ii) {
        aNode = aProcessorElement->getElementsByTagName(xmlLogicRegion)->item(ii);
        aRegionElement = static_cast<DOMElement *>(aNode);
        unsigned int iRegion = std::atoi(_toString(aRegionElement->getAttribute(xmliRegion)).c_str());
        unsigned int nElem3 = aRegionElement->getElementsByTagName(xmlLayer)->getLength();
        assert(nElem3 == nLayers);
        DOMElement *aLayerElement = nullptr;
        for (unsigned int iii = 0; iii < nElem3; ++iii) {
          aNode = aRegionElement->getElementsByTagName(xmlLayer)->item(iii);
          aLayerElement = static_cast<DOMElement *>(aNode);
          unsigned int iLayer = std::atoi(_toString(aLayerElement->getAttribute(xmliLayer)).c_str());
          unsigned int iFirstInput = std::atoi(_toString(aLayerElement->getAttribute(xmliFirstInput)).c_str());
          unsigned int nInputs = std::atoi(_toString(aLayerElement->getAttribute(xmlnInputs)).c_str());
          aLayerInputNode.iLayer = iLayer;
          aLayerInputNode.iFirstInput = iFirstInput;
          aLayerInputNode.nInputs = nInputs;
          for (unsigned int iProcessor = 0; iProcessor < nProcessors; ++iProcessor)
            aLayerInputMapVec[iLayer + iRegion * nLayers + iProcessor * nLayers * nLogicRegions] = aLayerInputNode;
        }
      }
    }

    aConfig->setGlobalPhiStartMap(aGlobalPhiStartVec);
    aConfig->setLayerInputMap(aLayerInputMapVec);
    aConfig->setRefHitMap(aRefHitMapVec);

    // Reset the documents vector pool and release all the associated memory back to the system.
    parser.resetDocumentPool();

    XMLString::release(&xmlOMTF);
    XMLString::release(&xmlversion);
    XMLString::release(&xmlGlobalData);
    XMLString::release(&xmlnPdfAddrBits);
    XMLString::release(&xmlnPdfValBits);
    XMLString::release(&xmlnPhiBits);
    XMLString::release(&xmlnPhiBins);
    XMLString::release(&xmlnProcessors);
    XMLString::release(&xmlnLogicRegions);
    XMLString::release(&xmlnInputs);
    XMLString::release(&xmlnLayers);
    XMLString::release(&xmlnRefLayers);
    XMLString::release(&xmliProcessor);
    XMLString::release(&xmlbarrelMin);
    XMLString::release(&xmlbarrelMax);
    XMLString::release(&xmlendcap10DegMin);
    XMLString::release(&xmlendcap10DegMax);
    XMLString::release(&xmlendcap20DegMin);
    XMLString::release(&xmlendcap20DegMax);
    XMLString::release(&xmlLayerMap);
    XMLString::release(&xmlhwNumber);
    XMLString::release(&xmllogicNumber);
    XMLString::release(&xmlbendingLayer);
    XMLString::release(&xmlconnectedToLayer);
    XMLString::release(&xmlRefLayerMap);
    XMLString::release(&xmlrefLayer);
    XMLString::release(&xmlProcessor);
    XMLString::release(&xmlRefLayer);
    XMLString::release(&xmliRefLayer);
    XMLString::release(&xmliGlobalPhiStart);
    XMLString::release(&xmlRefHit);
    XMLString::release(&xmliRefHit);
    XMLString::release(&xmliPhiMin);
    XMLString::release(&xmliPhiMax);
    XMLString::release(&xmliInput);
    XMLString::release(&xmliRegion);
    XMLString::release(&xmlLogicRegion);
    XMLString::release(&xmlLayer);
    XMLString::release(&xmliLayer);
    XMLString::release(&xmliFirstInput);
    XMLString::release(&xmlnHitsPerLayer);
    XMLString::release(&xmlnRefHits);
    XMLString::release(&xmlnTestRefHits);
    XMLString::release(&xmlnGoldenPatterns);
    XMLString::release(&xmlConnectionMap);
  }
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////

template GoldenPatternVec<GoldenPattern> XMLConfigReader::readPatterns<GoldenPattern>(
    const L1TMuonOverlapParams &aConfig,
    const std::string &patternsFile,
    bool buildEmptyPatterns,
    bool resetNumbering = true);

template GoldenPatternVec<GoldenPattern> XMLConfigReader::readPatterns<GoldenPattern>(
    const L1TMuonOverlapParams &aConfig, const std::vector<std::string> &patternsFiles, bool buildEmptyPatterns);

template GoldenPatternVec<GoldenPatternWithStat> XMLConfigReader::readPatterns<GoldenPatternWithStat>(
    const L1TMuonOverlapParams &aConfig,
    const std::string &patternsFile,
    bool buildEmptyPatterns,
    bool resetNumbering = true);

template GoldenPatternVec<GoldenPatternWithStat> XMLConfigReader::readPatterns<GoldenPatternWithStat>(
    const L1TMuonOverlapParams &aConfig, const std::vector<std::string> &patternsFiles, bool buildEmptyPatterns);
