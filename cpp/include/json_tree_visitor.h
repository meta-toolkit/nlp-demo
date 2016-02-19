/**
 * @file json_tree_visitor.h
 * @author Chase Geigle
 */

#ifndef META_NLP_DEMO_JSON_TREE_VISITOR_H_
#define META_NLP_DEMO_JSON_TREE_VISITOR_H_

#include <json/json.h>

#include "meta/parser/trees/visitors/visitor.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace parser
{
class json_tree_visitor : public const_visitor<Json::Value>
{
  public:
    Json::Value operator()(const leaf_node& leaf) override
    {
        Json::Value result{Json::objectValue};
        result["tag"] = std::string{leaf.category()};
        result["word"] = *leaf.word();
        return result;
    }

    Json::Value operator()(const internal_node& in) override
    {
        Json::Value result{Json::objectValue};
        result["tag"] = std::string{in.category()};
        result["children"] = Json::arrayValue;

        in.each_child([&](const node* n)
                      {
                          result["children"].append(n->accept(*this));
                      });
        return result;
    }
};
}
}

#endif
