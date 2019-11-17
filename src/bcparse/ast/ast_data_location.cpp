#include <bcparse/ast/ast_data_location.hpp>

#include <bcparse/ast_visitor.hpp>
#include <bcparse/compilation_unit.hpp>

#include <bcparse/emit/emit.hpp>

#include <common/my_assert.hpp>

namespace bcparse {
  AstDataLocation::AstDataLocation(const std::string &ident,
    const Pointer<AstIntegerLiteral> &offset,
    const SourceLocation &location)
    : AstExpression(location),
      m_ident(ident),
      m_offset(offset),
      m_storagePath(-1) {
  }

  void AstDataLocation::visit(AstVisitor *visitor, Module *mod) {
    int specialDataValue = -1;

    std::transform(m_ident.begin(), m_ident.end(), m_ident.begin(),
      [](unsigned char c){ return std::tolower(c); });

    if (m_ident == "l") {
      m_storagePath = (int)ObjLoc::DataStoreLocation::LocalDataStore;
    } else if (m_ident == "r") {
      m_storagePath = (int)ObjLoc::DataStoreLocation::RegisterDataStore;
    } else if (m_ident == "s") {
      m_storagePath = (int)ObjLoc::DataStoreLocation::StaticDataStore;
    } else if (m_ident == "pc") {
      m_storagePath = (int)ObjLoc::DataStoreLocation::VMDataStore;
      specialDataValue = 0;
    } else if (m_ident == "sp") {
      m_storagePath = (int)ObjLoc::DataStoreLocation::VMDataStore;
      specialDataValue = 3;
    }

    if (specialDataValue != -1) {
      if (m_offset != nullptr) {
        visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
          LEVEL_ERROR,
          Msg_custom_error,
          m_location,
          "Data storage type '%' does not accept arguments",
          m_ident
        ));
      } else {
        m_offset.reset(new AstIntegerLiteral(
          specialDataValue,
          m_location
        ));
      }
    }

    if (m_storagePath == -1) {
      // error
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "Unknown data storage path: %",
        m_ident
      ));
    }

    if (m_offset == nullptr) {
      visitor->getCompilationUnit()->getErrorList().addError(CompilerError(
        LEVEL_ERROR,
        Msg_custom_error,
        m_location,
        "No value provided for data location"
      ));
    }
  }

  void AstDataLocation::build(AstVisitor *visitor, Module *mod, BytecodeChunk *out) {
    ASSERT(m_offset != nullptr);
    ASSERT(m_storagePath != -1);

    m_objLoc = ObjLoc(
      m_offset->getValue(),
      (ObjLoc::DataStoreLocation)m_storagePath
    );
  }

  void AstDataLocation::optimize(AstVisitor *visitor, Module *mod) {
  }

  Pointer<AstStatement> AstDataLocation::clone() const {
    return CloneImpl();
  }
}
