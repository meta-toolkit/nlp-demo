/**
 * @file html_tree_visitor.h
 * @author Sean Massung
 */

#ifndef META_NLP_DEMO_HTML_TREE_VISITOR_H_
#define META_NLP_DEMO_HTML_TREE_VISITOR_H_

#include "meta/parser/trees/visitors/visitor.h"
#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"

namespace meta
{
namespace parser
{
class html_tree_visitor : public const_visitor<void>
{
  public:
    void operator()(const leaf_node& leaf) override
    {
        ss_ << "<li><a href=\"#\">" << std::string{leaf.category()} << " "
            << *leaf.word() << "</a></li>";
    }

    void operator()(const internal_node& in) override
    {
        ss_ << "<li><a href=\"#\">" << std::string{in.category()} << "</a><ul>";
        in.each_child([&](const node* n)
                      {
                          n->accept(*this);
                      });
        ss_ << std::string{"</ul></li>"};
    }

    std::string html() const
    {
        return "<div class=\"tree\"><ul>" + ss_.str() + "</ul></div>";
    }

  private:
    std::stringstream ss_;
};
}
}

#endif
