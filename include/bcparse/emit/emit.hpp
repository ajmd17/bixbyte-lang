#pragma once

#include <bcparse/emit/buildable.hpp>
#include <bcparse/emit/bytecode_stream.hpp>
#include <bcparse/emit/bytecode_chunk.hpp>
#include <bcparse/emit/value.hpp>

namespace bcparse {
  class Op_NoOp : public Buildable {
  public:
    Op_NoOp();
    Op_NoOp(const Op_NoOp &other) = delete;
    virtual ~Op_NoOp() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(Formatter *f) override;
  };

  class Op_Load : public Buildable {
  public:
    Op_Load(const ObjLoc &objLoc, const Value &value);
    Op_Load(const Op_Load &other) = delete;
    virtual ~Op_Load() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(Formatter *f) override;

  private:
    ObjLoc m_objLoc;
    Value m_value;
  };

  class Op_Mov : public Buildable {
  public:
    Op_Mov(const ObjLoc &left, const ObjLoc &right);
    Op_Mov(const Op_Load &other) = delete;
    virtual ~Op_Mov() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(Formatter *f) override;

  private:
    ObjLoc m_left;
    ObjLoc m_right;
  };

  class Op_Halt : public Buildable {
  public:
    Op_Halt();
    Op_Halt(const Op_Halt &other) = delete;
    virtual ~Op_Halt() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(Formatter *f) override;
  };
}
