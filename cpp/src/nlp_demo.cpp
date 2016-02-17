#include <algorithm>
#include <sstream>

#include <json/json.h>

#include "meta/logging/logger.h"
#include "meta/util/shim.h"
#include "nlp_demo.h"

using namespace meta;

nlp_demo::nlp_demo(const cpptoml::table& config)
{
    auto seq_group = config.get_table("sequence");
    if (!seq_group)
        throw std::runtime_error{"[sequence] group needed in config file"};

    auto tagger_prefix = seq_group->get_as<std::string>("prefix");
    if (!tagger_prefix)
        throw std::runtime_error{"[sequence] group needs a prefix key"};

    tagger_ = make_unique<meta::sequence::perceptron>(*tagger_prefix);

    auto parser_group = config.get_table("parser");
    if (!parser_group)
        throw std::runtime_error{"[parser] group needed in config file"};

    auto parser_prefix = parser_group->get_as<std::string>("prefix");
    if (!parser_prefix)
        throw std::runtime_error{"[parser] group needs a prefix key"};

    parser_ = make_unique<meta::parser::sr_parser>(*parser_prefix);
}

std::string nlp_demo::analyze(std::string text)
{
    using namespace meta;

    LOG(info) << "Analyzing text" << ENDLG;
    auto start = std::chrono::high_resolution_clock::now();

    std::unique_ptr<analyzers::token_stream> stream
        = make_unique<analyzers::tokenizers::icu_tokenizer>();
    stream = make_unique<analyzers::filters::ptb_normalizer>(std::move(stream));
    stream->set_content(std::move(text));

    Json::Value result{Json::objectValue};
    result["sentences"] = Json::arrayValue;
    meta::sequence::sequence seq;

    while (*stream)
    {
        auto token = stream->next();
        if (token == "<s>")
        {
            seq = {};
        }
        else if (token == "</s>")
        {
            tagger_->tag(seq);
            auto tree = parser_->parse(seq);
            result["sentences"].append(json_sentence(seq, tree));
        }
        else
        {
            seq.add_symbol(sequence::symbol_t{token});
        }
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

Json::Value nlp_demo::json_sentence(const meta::sequence::sequence& seq,
                                    const meta::parser::parse_tree& tree) const
{
    Json::Value obj{Json::objectValue};
    std::string tokenized{""};
    std::string tagged{""};
    for (const auto& obs : seq)
    {
        std::string word{obs.symbol()};
        std::string tag{obs.tag()};
        tokenized += word + " ";
        tagged += word + "_" + tag + " ";
    }
    obj["tokenized"] = tokenized;
    obj["tagged"] = tagged;
    std::stringstream ss;
    ss << tree;
    obj["tree"] = ss.str();
    return obj;
}
