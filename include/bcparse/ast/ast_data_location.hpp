#pragma once

#include <bcparse/ast/ast_expression.hpp>

#include <bcparse/emit/obj_loc.hpp>

namespace bcparse {
  class AstDataLocation : public AstExpression {
  public:
    AstDataLocation(int value,
      ObjLoc::DataStoreLocation storagePath,
      const SourceLocation &location);
    virtual ~AstDataLocation() = default;

    const int getValue() const { return m_value; }
    const ObjLoc::DataStoreLocation getStoragePath() const { return m_storagePath; }

    virtual void visit(AstVisitor *visitor, Module *mod) override;
    virtual void build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) override;
    virtual void optimize(AstVisitor *visitor, Module *mod) override;

    virtual Pointer<AstStatement> clone() const override;

  private:
    int m_value;
    ObjLoc::DataStoreLocation m_storagePath;

    inline Pointer<AstDataLocation> CloneImpl() const {
      return Pointer<AstDataLocation>(new AstDataLocation(
        m_value,
        m_storagePath,
        m_location
      ));
    }
  };
}
