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
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;
  };

  class Op_Load : public Buildable {
  public:
    Op_Load(const ObjLoc &objLoc, const Value &value);
    Op_Load(const Op_Load &other) = delete;
    virtual ~Op_Load() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_objLoc;
    Value m_value;
  };

  class Op_Mov : public Buildable {
  public:
    Op_Mov(const ObjLoc &left, const ObjLoc &right);
    Op_Mov(const Op_Mov &other) = delete;
    virtual ~Op_Mov() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_left;
    ObjLoc m_right;
  };

  class Op_Push : public Buildable {
  public:
    Op_Push(const ObjLoc &arg);
    Op_Push(const Op_Push &other) = delete;
    virtual ~Op_Push() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_arg;
  };

  class Op_PushConst : public Buildable {
  public:
    Op_PushConst(const Value &arg);
    Op_PushConst(const Op_PushConst &other) = delete;
    virtual ~Op_PushConst() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    Value m_arg;
  };

  class Op_Pop : public Buildable {
  public:
    Op_Pop(size_t amt);
    Op_Pop(const Op_Pop &other) = delete;
    virtual ~Op_Pop() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    size_t m_amt;
  };

  class Op_Jmp : public Buildable {
  public:
    enum class Flags {
      None = 0,
      JumpIfEqual = 1,
      JumpIfNotEqual = 2,
      JumpIfGreater = 3,
      JumpIfGreaterOrEqual = 4
    };

    Op_Jmp(const ObjLoc &objLoc, Flags flags = Flags::None);
    Op_Jmp(const Op_Jmp &other) = delete;
    virtual ~Op_Jmp() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_objLoc;
    Flags m_flags;
  };

  // TODO: cmp flags
  class Op_Cmp : public Buildable {
  public:
    Op_Cmp(const ObjLoc &left, const ObjLoc &right);
    Op_Cmp(const Op_Jmp &other) = delete;
    virtual ~Op_Cmp() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

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
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;
  };

  class LabelMarker : public Buildable {
  public:
    LabelMarker(size_t labelId);
    LabelMarker(const LabelMarker &other) = delete;
    virtual ~LabelMarker() override;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    size_t m_labelId;

    Op_Load *m_opLoad;
  };
}
