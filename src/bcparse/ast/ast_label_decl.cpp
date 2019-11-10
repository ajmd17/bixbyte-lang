#include <bcparse/ast/ast_label_decl.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  AstLabelDecl::AstLabelDecl(const std::string &name,
    Pointer<AstLabel> astLabel,
    const SourceLocation &location)
    : AstStatement(location),
      m_name(name),
      m_astLabel(astLabel) {
  }

  void AstLabelDecl::visit(AstVisitor *visitor, Module *mod) {
    if (!visitor->getCompilationUnit()->getBoundGlobals().get(m_name)) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        std::string("The label '") + m_name + "' should have been declared during the parsing stage, but it was not found."
      ));

      return;
    }
  }

  void AstLabelDecl::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_astLabel != nullptr);

    m_astLabel->build(visitor, mod, out);

    size_t id = m_astLabel->getObjLoc().getLocation();

    out->append(std::unique_ptr<LabelMarker>(new LabelMarker(
      m_astLabel->getObjLoc().getLocation()
    )));
  }

  void AstLabelDecl::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstLabelDecl::clone() const {
    return CloneImpl();
  }
}
