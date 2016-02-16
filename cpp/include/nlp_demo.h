/**
 * @file nlp_demo.h
 * @author Sean Massung
 */

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "cpptoml.h"
#include "meta/analyzers/analyzer.h"
#include "meta/analyzers/tokenizers/icu_tokenizer.h"
#include "meta/sequence/crf/crf.h"
#include "meta/sequence/crf/tagger.h"
#include "meta/sequence/sequence.h"
#include "meta/sequence/sequence_analyzer.h"

class nlp_demo
{
  public:
    /**
     * @param config A config file for running the demo; this will essentially
     * just be the standard MeTA config file
     */
    nlp_demo(const cpptoml::table& config);

    /**
     * @param text The text to run MeTA's NLP tools on
     */
    std::string analyze(const std::string& text);

  private:
    std::vector<meta::sequence::sequence>
    extract_sequences(std::string text) const;

    meta::sequence::sequence_analyzer seq_analyzer_;
    std::unique_ptr<meta::sequence::crf> crf_;
};
