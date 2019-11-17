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

  class Op_Add : public Buildable {
  public:
    Op_Add(const ObjLoc &left, const ObjLoc &right);
    Op_Add(const Op_Add &other) = delete;
    virtual ~Op_Add() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_left;
    ObjLoc m_right;
  };

  class Op_Sub : public Buildable {
  public:
    Op_Sub(const ObjLoc &left, const ObjLoc &right);
    Op_Sub(const Op_Sub &other) = delete;
    virtual ~Op_Sub() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_left;
    ObjLoc m_right;
  };

  class Op_Mul : public Buildable {
  public:
    Op_Mul(const ObjLoc &left, const ObjLoc &right);
    Op_Mul(const Op_Mul &other) = delete;
    virtual ~Op_Mul() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_left;
    ObjLoc m_right;
  };

  class Op_Div : public Buildable {
  public:
    Op_Div(const ObjLoc &left, const ObjLoc &right);
    Op_Div(const Op_Div &other) = delete;
    virtual ~Op_Div() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_left;
    ObjLoc m_right;
  };

  class Op_Mod : public Buildable {
  public:
    Op_Mod(const ObjLoc &left, const ObjLoc &right);
    Op_Mod(const Op_Mod &other) = delete;
    virtual ~Op_Mod() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_left;
    ObjLoc m_right;
  };

  class Op_Xor : public Buildable {
  public:
    Op_Xor(const ObjLoc &left, const ObjLoc &right);
    Op_Xor(const Op_Xor &other) = delete;
    virtual ~Op_Xor() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_left;
    ObjLoc m_right;
  };

  class Op_And : public Buildable {
  public:
    Op_And(const ObjLoc &left, const ObjLoc &right);
    Op_And(const Op_And &other) = delete;
    virtual ~Op_And() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_left;
    ObjLoc m_right;
  };

  class Op_Or : public Buildable {
  public:
    Op_Or(const ObjLoc &left, const ObjLoc &right);
    Op_Or(const Op_Or &other) = delete;
    virtual ~Op_Or() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_left;
    ObjLoc m_right;
  };

  class Op_Shl : public Buildable {
  public:
    Op_Shl(const ObjLoc &left, const ObjLoc &right);
    Op_Shl(const Op_Shl &other) = delete;
    virtual ~Op_Shl() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_left;
    ObjLoc m_right;
  };

  class Op_Shr : public Buildable {
  public:
    Op_Shr(const ObjLoc &left, const ObjLoc &right);
    Op_Shr(const Op_Shr &other) = delete;
    virtual ~Op_Shr() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_left;
    ObjLoc m_right;
  };

  class Op_Print : public Buildable {
  public:
    Op_Print(const ObjLoc &objLoc);
    Op_Print(const Op_Print &other) = delete;
    virtual ~Op_Print() = default;

    virtual void accept(BytecodeStream *bs) override;
    virtual void debugPrint(BytecodeStream *bs, Formatter *f) override;

  private:
    ObjLoc m_objLoc;
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
