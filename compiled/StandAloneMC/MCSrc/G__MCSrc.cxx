// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME G__MCSrc

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Since CINT ignores the std namespace, we need to do so in this file.
namespace std {} using namespace std;

// Header files passed as explicit arguments
#include "/home/ayyadlim/fair_install_ROOT6/ATTPCROOTv2/compiled/StandAloneMC/MCSrc/MCSrc.hh"

// Header files passed via #pragma extra_include

namespace {
  void TriggerDictionaryInitialization_G__MCSrc_Impl() {
    static const char* headers[] = {
"MCSrc.hh",
0
    };
    static const char* includePaths[] = {
"/usr/local/root-6.05.02/include",
"/usr/include",
"/home/ayyadlim/fair_install_ROOT6/ATTPCROOTv2/include",
"/home/ayyadlim/fair_install_ROOT6/FairRoot.v15.07.sp/include",
"/home/ayyadlim/fair_install_ROOT6/ATTPCROOTv2/compiled/StandAloneMC/MCSrc/MCSrc",
"/usr/local/root-6.05.02/include",
"/home/ayyadlim/fair_install_ROOT6/ATTPCROOTv2/compiled/StandAloneMC/MCSrc/",
0
    };
    static const char* fwdDeclCode = 
R"DICTFWDDCLS(
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
#include "MCSrc.hh"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("G__MCSrc",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_G__MCSrc_Impl, {}, classesHeaders);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_G__MCSrc_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_G__MCSrc() {
  TriggerDictionaryInitialization_G__MCSrc_Impl();
}