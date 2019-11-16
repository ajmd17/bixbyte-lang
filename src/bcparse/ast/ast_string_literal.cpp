#include <bcparse/ast/ast_string_literal.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

#include <common/str_util.hpp>

namespace bcparse {
  AstStringLiteral::AstStringLiteral(const std::string &value,
    const SourceLocation &location)
    : AstExpression(location),
      m_value(value) {
  }

  void AstStringLiteral::visit(AstVisitor *visitor, Module *mod) {
  }

  void AstStringLiteral::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    size_t id = visitor->getCompilationUnit()->getDataStorage()->addStaticData(
      getRuntimeValue()
    );

    m_objLoc = ObjLoc(id, ObjLoc::DataStoreLocation::StaticDataStore);
  }

  void AstStringLiteral::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstStringLiteral::clone() const {
    return CloneImpl();
  }

  std::string AstStringLiteral::toString() const {
    return std::string("\"") + str_util::escape_string(m_value) + "\"";
  }

  Value AstStringLiteral::getRuntimeValue() const {
    return Value(std::vector<uint8_t>(m_value.begin(), m_value.end()));
  }
}
