#include <bcparse/ast/ast_binop_statement.hpp>
#include <bcparse/ast/ast_variable.hpp>
#include <bcparse/ast/ast_label.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>
#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  const std::vector<std::string> AstBinOpStatement::binaryOperations = {
    "add",
    "addf",
    "addfl",
    "addfr",
    "sub",
    "subf",
    "subfl",
    "subfr",
    "mul",
    "mulf",
    "mulfl",
    "mulfr",
    "div",
    "divf",
    "divfl",
    "divfr",
    "mod",
    "xor",
    "and",
    "or",
    "shl",
    "shr"
  };

  AstBinOpStatement::AstBinOpStatement(const std::string &opName,
    Pointer<AstExpression> left,
    Pointer<AstExpression> right,
    const SourceLocation &location)
    : AstStatement(location),
      m_opName(opName),
      m_left(left),
      m_right(right) {
  }

  void AstBinOpStatement::visit(AstVisitor *visitor, Module *mod) {
    ASSERT(m_left != nullptr);
    ASSERT(m_right != nullptr);

    auto it = std::find(binaryOperations.begin(), binaryOperations.end(), m_opName);

    if (it == binaryOperations.end()) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "Unknown binary operation: '%'",
        m_opName
      ));

      return;
    }

    m_left->visit(visitor, mod);
    m_right->visit(visitor, mod);
  }

  void AstBinOpStatement::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_left != nullptr);
    ASSERT(m_right != nullptr);

    m_left->build(visitor, mod, out);
    visitor->getCompilationUnit()->getRegisterUsage().inc();

    m_right->build(visitor, mod, out);
    visitor->getCompilationUnit()->getRegisterUsage().dec();

    const std::string substr = m_opName.substr(0, 3);
    const std::string flagsStr = m_opName.substr(3);

    Op_Cmp::Flags flags = Op_Cmp::Flags::None;

    if (flagsStr == "f") {
      flags = Op_Cmp::Flags::Float;
    } else if (flagsStr == "fl") {
      flags = Op_Cmp::Flags::FloatLeft;
    } else if (flagsStr == "fr") {
      flags = Op_Cmp::Flags::FloatRight;
    }

    if (substr == "add") {
      out->append(std::unique_ptr<Op_Add>(new Op_Add(
        m_left->getObjLoc(),
        m_right->getObjLoc(),
        flags
      )));
    } else if (substr == "sub") {
      out->append(std::unique_ptr<Op_Sub>(new Op_Sub(
        m_left->getObjLoc(),
        m_right->getObjLoc(),
        flags
      )));
    } else if (substr == "mul") {
      out->append(std::unique_ptr<Op_Mul>(new Op_Mul(
        m_left->getObjLoc(),
        m_right->getObjLoc(),
        flags
      )));
    } else if (substr == "div") {
      out->append(std::unique_ptr<Op_Div>(new Op_Div(
        m_left->getObjLoc(),
        m_right->getObjLoc(),
        flags
      )));
    } else if (substr == "mod") {
      out->append(std::unique_ptr<Op_Mod>(new Op_Mod(
        m_left->getObjLoc(),
        m_right->getObjLoc()
      )));
    } else if (substr == "xor") {
      out->append(std::unique_ptr<Op_Xor>(new Op_Xor(
        m_left->getObjLoc(),
        m_right->getObjLoc()
      )));
    } else if (substr == "and") {
      out->append(std::unique_ptr<Op_And>(new Op_And(
        m_left->getObjLoc(),
        m_right->getObjLoc()
      )));
    } else if (substr == "or") {
      out->append(std::unique_ptr<Op_Or>(new Op_Or(
        m_left->getObjLoc(),
        m_right->getObjLoc()
      )));
    } else if (substr == "shl") {
      out->append(std::unique_ptr<Op_Shl>(new Op_Shl(
        m_left->getObjLoc(),
        m_right->getObjLoc()
      )));
    } else if (substr == "shr") {
      out->append(std::unique_ptr<Op_Shr>(new Op_Shr(
        m_left->getObjLoc(),
        m_right->getObjLoc()
      )));
    }
  }

  void AstBinOpStatement::optimize(AstVisitor *visitor, Module *mod) {
    ASSERT(m_left != nullptr);
    ASSERT(m_right != nullptr);

    m_left->optimize(visitor, mod);
    m_right->optimize(visitor, mod);
  }

  Pointer<AstStatement> AstBinOpStatement::clone() const {
    return CloneImpl();
  }
}
