/**
 * @file nlp_demo.h
 * @author Sean Massung
 */

#ifndef META_NLP_DEMO_H_
#define META_NLP_DEMO_H_

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "cpptoml.h"
#include "meta/analyzers/analyzer.h"
#include "meta/analyzers/tokenizers/icu_tokenizer.h"
#include "meta/analyzers/filters/all.h"
#include "meta/parser/sr_parser.h"
#include "meta/sequence/io/ptb_parser.h"
#include "meta/sequence/perceptron.h"
#include "meta/sequence/sequence.h"
#include "meta/sequence/sequence_analyzer.h"

namespace Json
{
class Value;
}

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
    std::string analyze(std::string text);

  private:
    Json::Value json_sentence(const meta::sequence::sequence& seq,
                              const meta::parser::parse_tree& tree) const;

    std::unique_ptr<meta::sequence::perceptron> tagger_;
    std::unique_ptr<meta::parser::sr_parser> parser_;
};

#endif
