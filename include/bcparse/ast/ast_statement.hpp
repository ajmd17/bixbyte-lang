#pragma once

#include <memory>
#include <vector>

#include <bcparse/source_location.hpp>

template <typename T>
using Pointer = std::shared_ptr<T>;

using std::enable_if;
using std::is_base_of;
using std::shared_ptr;
using std::vector;
using std::static_pointer_cast;

namespace bcparse {
  class AstVisitor;
  class Module;
  class BytecodeChunk;

  class AstStatement {
  public:
    AstStatement(const SourceLocation &location);
    virtual ~AstStatement() = default;

    inline SourceLocation &getLocation() { return m_location; }
    inline const SourceLocation &getLocation() const { return m_location; }

    virtual void visit(AstVisitor *visitor, Module *mod) = 0;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) = 0;
    virtual void optimize(AstVisitor *visitor, Module *mod) = 0;

    virtual Pointer<AstStatement> clone() const = 0;

  protected:
    SourceLocation m_location;
  };

  template <typename T>
  typename enable_if<is_base_of<AstStatement, T>::value, Pointer<T>>::type
  CloneAstNode(const Pointer<T> &stmt) { 
    if (stmt == nullptr) {
      return nullptr;
    }

    return static_pointer_cast<T>(stmt->Clone());
  }

  template <typename T>
  typename enable_if<is_base_of<AstStatement, T>::value, vector<Pointer<T>>>::type
  CloneAllAstNodes(const vector<Pointer<T>> &stmts) {
    vector<Pointer<T>> res;
    res.reserve(stmts.size());

    for (auto &stmt : stmts) {
      res.push_back(CloneAstNode(stmt));
    }

    return res;
  }
}
