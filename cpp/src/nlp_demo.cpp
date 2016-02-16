#include <algorithm>
#include <sstream>

#include <json/json.h>

#include "meta/logging/logger.h"
#include "meta/util/shim.h"
#include "nlp_demo.h"

using namespace meta;

nlp_demo::nlp_demo(const cpptoml::table& config)
    : seq_analyzer_{meta::sequence::default_pos_analyzer()}
{
    auto crf_group = config.get_table("crf");

    if (!crf_group)
        throw std::runtime_error{"[crf] group needed in config file"};

    auto prefix = crf_group->get_as<std::string>("prefix");
    if (!prefix)
        throw std::runtime_error{
            "prefix to learned model needed in [crf] group"};

    crf_ = make_unique<meta::sequence::crf>(*prefix);
    seq_analyzer_.load(*prefix);
}

std::string nlp_demo::analyze(const std::string& text)
{
    using namespace meta;

    LOG(info) << "Analyzing text:" << ENDLG;
    std::cout << text << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    Json::Value result{Json::objectValue};
    result["sentences"] = Json::arrayValue;

    const auto& analyzer = seq_analyzer_; // readonly
    auto tagger = crf_->make_tagger();

    auto sequences = extract_sequences(text);
    for (auto& seq : sequences)
    {
        Json::Value obj{Json::objectValue};
        std::string output{""};
        analyzer.analyze(seq);
        tagger.tag(seq);
        for (const auto& obs : seq)
        {
            output += std::string{obs.symbol()} + "_"
                      + std::string{analyzer.tag(obs.label())} + " ";
        }
        obj["sentence"] = output;
        result["sentences"].append(obj);
    }

    Json::StyledWriter styled_writer;
    auto json_str = styled_writer.write(result);
    std::cout << json_str;

    LOG(info) << "Done. ("
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::high_resolution_clock::now() - start)
                     .count()
              << "ms)" << ENDLG;
    return json_str;
}

std::vector<meta::sequence::sequence>
nlp_demo::extract_sequences(std::string text) const
{
    using namespace meta;

    std::unique_ptr<analyzers::token_stream> stream
        = make_unique<analyzers::tokenizers::icu_tokenizer>();
    stream->set_content(std::move(text));

    std::vector<sequence::sequence> sequences;
    sequence::sequence seq;
    while (*stream)
    {
        auto token = stream->next();
        if (token == " " || token == "<s>")
            continue;
        else if (token == "</s>")
            sequences.emplace_back(std::move(seq));
        else
            seq.add_observation(
                {sequence::symbol_t{token}, sequence::tag_t{"[UNK]"}});
    }

    return sequences;
}
