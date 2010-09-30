
// Sixgill: Static assertion checker for C/C++ programs.
// Copyright (C) 2009-2010  Stanford University
// Author: Brian Hackett
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "mblock.h"
#include "alias.h"
#include "clobber.h"
#include "simplify.h"
#include "serial.h"
#include "mstorage.h"
#include "baked.h"

#include <imlang/storage.h>
#include <util/config.h>
#include <solve/solver.h>

NAMESPACE_XGILL_BEGIN

// enable for expensive checking of constraints on val etc. guards.
// #define CHECK_MEMORY_CONSISTENCY

// maximum number of values that an ExpVal can expand to before it will fail
// to expand using TRK_TryRemoveVal.
#define TRY_REMOVE_CUTOFF 20

/////////////////////////////////////////////////////////////////////
// BlockMemory static
/////////////////////////////////////////////////////////////////////

HashCons<BlockMemory> BlockMemory::g_table;

int BlockMemory::Compare(const BlockMemory *mcfg0, const BlockMemory *mcfg1)
{
  TryCompareObjects(mcfg0->GetCFG(), mcfg1->GetCFG(), BlockCFG);
  TryCompareValues(mcfg0->m_simplify->Kind(),
                   mcfg1->m_simplify->Kind());
  TryCompareValues(mcfg0->m_alias->Kind(),
                   mcfg1->m_alias->Kind());
  TryCompareValues(mcfg0->m_clobber->Kind(),
                   mcfg1->m_clobber->Kind());
  return 0;
}

BlockMemory* BlockMemory::Copy(const BlockMemory *mcfg)
{
  return new BlockMemory(*mcfg);
}

static void WriteGuard(Buffer *buf, PPoint point, Bit *guard)
{
  WriteOpenTag(buf, TAG_MemoryGuardEntry);
  WriteTagUInt32(buf, TAG_Index, point);
  Bit::Write(buf, guard);
  WriteCloseTag(buf, TAG_MemoryGuardEntry);
}

static void WriteAssume(Buffer *buf, PPoint point, const GuardTrueFalse &v)
{
  WriteOpenTag(buf, TAG_MemoryAssumeEntry);
  WriteTagUInt32(buf, TAG_Index, point);

  if (v.true_guard) {
    Bit::Write(buf, v.true_guard);
  }
  else {
    Bit *false_bit = Bit::MakeConstant(false);
    Bit::Write(buf, false_bit);
    false_bit->DecRef();
  }

  if (v.false_guard) {
    Bit::Write(buf, v.false_guard);
  }
  else {
    Bit *false_bit = Bit::MakeConstant(false);
    Bit::Write(buf, false_bit);
    false_bit->DecRef();
  }

  WriteCloseTag(buf, TAG_MemoryAssumeEntry);
}

static void WriteGuardExp(Buffer *buf, tag_t tag, PPoint point,
                          const Vector<GuardExp> &vals)
{
  for (size_t vind = 0; vind < vals.Size(); vind++) {
    const GuardExp &v = vals[vind];

    WriteOpenTag(buf, tag);
    WriteTagUInt32(buf, TAG_Index, point);
    Exp::Write(buf, v.exp);
    Bit::Write(buf, v.guard);
    WriteCloseTag(buf, tag);
  }
}

static void WriteGuardAssign(Buffer *buf, tag_t tag, PPoint point,
                             const Vector<GuardAssign> &vals)
{
  for (size_t vind = 0; vind < vals.Size(); vind++) {
    const GuardAssign &v = vals[vind];

    WriteOpenTag(buf, tag);
    WriteTagUInt32(buf, TAG_Index, point);
    Exp::Write(buf, v.left);
    Exp::Write(buf, v.right);
    if (v.kind)
      Exp::Write(buf, v.kind);
    Bit::Write(buf, v.guard);
    WriteCloseTag(buf, tag);
  }
}

void BlockMemory::Write(Buffer *buf, const BlockMemory *mcfg)
{
  Assert(mcfg->m_cfg && mcfg->m_computed);

  WriteOpenTag(buf, TAG_BlockMemory);

  BlockId::Write(buf, mcfg->GetId());
  WriteTagUInt32(buf, TAG_MemoryKindSimplify, mcfg->m_simplify->Kind());
  WriteTagUInt32(buf, TAG_MemoryKindAlias, mcfg->m_alias->Kind());
  WriteTagUInt32(buf, TAG_MemoryKindClobber, mcfg->m_clobber->Kind());

  HashIteratePtr(mcfg->m_guard_table)
    WriteGuard(buf, mcfg->m_guard_table->ItKey(),
               mcfg->m_guard_table->ItValueSingle());

  HashIteratePtr(mcfg->m_assume_table)
    WriteAssume(buf, mcfg->m_assume_table->ItKey(),
                mcfg->m_assume_table->ItValueSingle());

  HashIteratePtr(mcfg->m_return_table)
    WriteGuardExp(buf, TAG_MemoryReturnEntry,
                  mcfg->m_return_table->ItKey(),
                  mcfg->m_return_table->ItValues());

  HashIteratePtr(mcfg->m_target_table)
    WriteGuardExp(buf, TAG_MemoryTargetEntry,
                  mcfg->m_target_table->ItKey(),
                  mcfg->m_target_table->ItValues());

  HashIteratePtr(mcfg->m_assign_table)
    WriteGuardAssign(buf, TAG_MemoryAssignEntry,
                     mcfg->m_assign_table->ItKey(),
                     mcfg->m_assign_table->ItValues());

  HashIteratePtr(mcfg->m_argument_table)
    WriteGuardAssign(buf, TAG_MemoryArgumentEntry,
                     mcfg->m_argument_table->ItKey(),
                     mcfg->m_argument_table->ItValues());

  HashIteratePtr(mcfg->m_clobber_table)
    WriteGuardAssign(buf, TAG_MemoryClobberEntry,
                     mcfg->m_clobber_table->ItKey(),
                     mcfg->m_clobber_table->ItValues());

  WriteCloseTag(buf, TAG_BlockMemory);
}

// move a reference for bit to nv, or drop the reference if the bit is 'true'.
static inline void TryMoveRef(Bit *bit, ORef nv)
{
  if (bit->IsTrue())
    bit->DecRef();
  else
    bit->MoveRef(NULL, nv);
}

BlockMemory* BlockMemory::Read(Buffer *buf)
{
  BlockMemory *res = NULL;

  PPoint point;
  uint32_t kind;

  Try(ReadOpenTag(buf, TAG_BlockMemory));
  while (!ReadCloseTag(buf, TAG_BlockMemory)) {
    switch (PeekOpenTag(buf)) {
    case TAG_BlockId: {
      Try(!res);
      BlockId *id;
      Try(id = BlockId::Read(buf));
      res = Make(id, MSIMP_Default, MALIAS_Default, MCLB_Default);

      // if the result was already in memory then remove all of its old data.
      // we will overwrite it with whatever we read here.
      res->UnPersist();

      res->m_computed = true;
      res->MakeTables();
      break;
    }
    case TAG_MemoryKindSimplify: {
      Try(res);
      Try(ReadTagUInt32(buf, TAG_MemoryKindSimplify, &kind));
      res->m_simplify = MemorySimplify::Lookup((MemorySimplifyKind)kind);
      break;
    }
    case TAG_MemoryKindAlias: {
      Try(res);
      Try(ReadTagUInt32(buf, TAG_MemoryKindAlias, &kind));
      res->m_alias = MemoryAlias::Lookup((MemoryAliasKind)kind);
      break;
    }
    case TAG_MemoryKindClobber: {
      Try(res);
      Try(ReadTagUInt32(buf, TAG_MemoryKindClobber, &kind));
      res->m_clobber = MemoryClobber::Lookup((MemoryClobberKind)kind);
      break;
    }
    case TAG_MemoryGuardEntry: {
      Try(res);
      Try(ReadOpenTag(buf, TAG_MemoryGuardEntry));
      Try(ReadTagUInt32(buf, TAG_Index, &point));
      Bit *guard = Bit::Read(buf);
      Try(ReadCloseTag(buf, TAG_MemoryGuardEntry));

      Vector<Bit*> *entries = res->m_guard_table->Lookup(point, true);
      entries->PushBack(guard);
      TryMoveRef(guard, entries);
      break;
    }
    case TAG_MemoryAssumeEntry: {
      Try(res);
      Try(ReadOpenTag(buf, TAG_MemoryAssumeEntry));
      Try(ReadTagUInt32(buf, TAG_Index, &point));
      Bit *true_guard = Bit::Read(buf);
      Bit *false_guard = Bit::Read(buf);
      Try(ReadCloseTag(buf, TAG_MemoryAssumeEntry));

      Vector<GuardTrueFalse> *entries =
        res->m_assume_table->Lookup(point, true);
      entries->PushBack(GuardTrueFalse(true_guard, false_guard));
      TryMoveRef(true_guard, entries);
      TryMoveRef(false_guard, entries);
      break;
    }
    case TAG_MemoryReturnEntry:
    case TAG_MemoryTargetEntry: {
      Try(res);
      tag_t tag = PeekOpenTag(buf);

      Try(ReadOpenTag(buf, tag));
      Try(ReadTagUInt32(buf, TAG_Index, &point));
      Exp *exp = Exp::Read(buf);
      Bit *guard = Bit::Read(buf);
      Try(ReadCloseTag(buf, tag));

      GuardExpTable *table;
      if (tag == TAG_MemoryReturnEntry)
        table = res->m_return_table;
      else if (tag == TAG_MemoryTargetEntry)
        table = res->m_target_table;
      else Assert(false);

      Vector<GuardExp> *entries = table->Lookup(point, true);
      entries->PushBack(GuardExp(exp, guard));
      exp->MoveRef(NULL, entries);
      TryMoveRef(guard, entries);
      break;
    }
    case TAG_MemoryAssignEntry:
    case TAG_MemoryArgumentEntry:
    case TAG_MemoryClobberEntry: {
      Try(res);
      tag_t tag = PeekOpenTag(buf);

      Try(ReadOpenTag(buf, tag));
      Try(ReadTagUInt32(buf, TAG_Index, &point));
      Exp *left = Exp::Read(buf);
      Exp *right = Exp::Read(buf);
      Exp *kind = NULL;
      if (PeekOpenTag(buf) == TAG_Exp)
        kind = Exp::Read(buf);
      Bit *guard = Bit::Read(buf);
      Try(ReadCloseTag(buf, tag));

      GuardAssignTable *table;
      if (tag == TAG_MemoryAssignEntry)
        table = res->m_assign_table;
      else if (tag == TAG_MemoryArgumentEntry)
        table = res->m_argument_table;
      else if (tag == TAG_MemoryClobberEntry)
        table = res->m_clobber_table;
      else Assert(false);

      Vector<GuardAssign> *entries = table->Lookup(point, true);
      entries->PushBack(GuardAssign(left, right, guard, kind));
      left->MoveRef(NULL, entries);
      right->MoveRef(NULL, entries);
      TryMoveRef(guard, entries);
      if (kind)
        kind->MoveRef(NULL, entries);
      break;
    }
    default:
      Try(false);
    }
  }

  Try(res);
  return res;
}

void BlockMemory::WriteList(Buffer *buf, const Vector<BlockMemory*> &mcfgs)
{
  Assert(buf->pos == buf->base);
  for (size_t ind = 0; ind < mcfgs.Size(); ind++)
    Write(buf, mcfgs[ind]);
}

void BlockMemory::ReadList(Buffer *buf, Vector<BlockMemory*> *mcfgs)
{
  Assert(buf->pos == buf->base);

  while (buf->pos != buf->base + buf->size) {
    BlockMemory *mcfg;
    Try(mcfg = Read(buf));
    mcfgs->PushBack(mcfg);
  }
}

// mapper to remove invalid portions of a computed annotation bit.
// the resulting bit will hold less often than the original.
// invalid portions of an annotation bit include:
// 1. references to clobbered values or temporaries, i.e. non-functional parts
//    of the annotation.
// 2. initial expressions outside of assertions and postconditions.
class AnnotationBitMapper : public ExpMapper
{
public:
  AnnotationKind kind;
  Exp *exclude;
  AnnotationBitMapper(AnnotationKind _kind)
    : ExpMapper(VISK_All, WIDK_Narrow), kind(_kind), exclude(NULL)
  {}

  Exp* Map(Exp *exp, Exp *old)
  {
    if (!exp)
      return NULL;

    if (exp->IsVar()) {
      Variable *var = exp->AsVar()->GetVariable();
      BlockId *id = var->GetId();

      // the bit should not refer to any variables from the annotation CFG.
      // there is an exception for the 'this' variable, which is used in
      // type invariant annotation CFGs.

      if (var->Kind() == VK_This)
        return exp;

      if (id) {
        switch (id->Kind()) {
        case B_AnnotationFunc:
        case B_AnnotationInit:
        case B_AnnotationComp:
          exclude = exp;
          exp->DecRef();
          return NULL;
        default: break;
        }
      }

      return exp;
    }

    if (exp->IsInitial()) {
      // initial expressions can only appear in assertions and postconditions.
      switch (kind) {
      case AK_Postcondition:
      case AK_PostconditionAssume:
      case AK_Assert:
      case AK_Assume:
      case AK_AssertRuntime:
        break;
      default:
        exclude = exp;
        exp->DecRef();
        return NULL;
      }
      Exp *new_target = exp->GetLvalTarget()->DoMap(this);
      if (new_target) {
        Assert(new_target == exp->GetLvalTarget());
        new_target->DecRef();
        return exp;
      }
      exclude = exp;
      exp->DecRef();
      return NULL;
    }

    if (exp->IsVal() || exp->IsClobber()) {
      exclude = exp;
      exp->DecRef();
      return NULL;
    }
    return exp;
  }
};

Bit* BlockMemory::GetAnnotationBit(BlockCFG *cfg, bool skip_directives,
                                   ostream *msg_out)
{
  if (cfg->IsAnnotationBitComputed() && !msg_out) {
    Bit *bit = cfg->GetAnnotationBit();
    if (skip_directives && bit && BitHasAnyDirective(bit))
        return NULL;
    return bit;
  }

  BlockId *id = cfg->GetId();
  id->IncRef();

  BlockMemory *mcfg = Make(id, MSIMP_Default, MALIAS_Default, MCLB_Modset);
  mcfg->SetCFG(cfg);

  mcfg->ComputeTables();
  PPoint exit_point = cfg->GetExitPoint();

  // check if there was a parse error for the annotation first. these will
  // show up as assignments to a local __error__.
  id->IncRef();
  String *error_name = String::Make("__error__");
  Variable *error_var = Variable::Make(id, VK_Local, error_name, 0, NULL);
  Exp *error_exp = Exp::MakeVar(error_var);

  const Vector<GuardExp> &error_vals =
    mcfg->GetVal(error_exp, NULL, exit_point);
  error_exp->DecRef();

  for (size_t ind = 0; ind < error_vals.Size(); ind++) {
    const GuardExp &gs = error_vals[ind];

    if (ExpString *nexp = gs.exp->IfString()) {
      if (!cfg->IsAnnotationBitComputed())
        cfg->SetAnnotationBit(NULL);
      if (msg_out)
        *msg_out << nexp->GetStringCStr();
      mcfg->DecRef();
      return NULL;
    }
  }

  // get the value of the actual annotation bit. this will show up as an
  // assignment to a local __value__.
  id->IncRef();
  String *assign_name = String::Make("__value__");
  Variable *assign_var = Variable::Make(id, VK_Local, assign_name, 0, NULL);
  Exp *assign_exp = Exp::MakeVar(assign_var);
  Exp *assign_drf = Exp::MakeDrf(assign_exp);

  Bit *assign_bit = Exp::MakeNonZeroBit(assign_drf);
  assign_drf->DecRef();

  Bit *annot_bit = NULL;
  mcfg->TranslateBit(TRK_Point, exit_point, assign_bit, &annot_bit);
  assign_bit->DecRef();

  // scan the bit to make sure it is functional.
  AnnotationBitMapper mapper(cfg->GetAnnotationKind());
  Bit *new_bit = annot_bit->DoMap(&mapper);

  // don't add the bit if it was narrowed to false, i.e. it has no functional
  // component we could determine. allow explicit false annotations, however.
  if (new_bit->IsFalse() && new_bit != annot_bit) {
    Assert(mapper.exclude);
    if (!cfg->IsAnnotationBitComputed())
      cfg->SetAnnotationBit(NULL);
    if (msg_out)
      *msg_out << "Could not get annotation value: unexpected "
               << mapper.exclude;
    new_bit->DecRef();
    new_bit = NULL;
  } else {
    // successfully interpreted the annotation.
    if (!cfg->IsAnnotationBitComputed()) {
      new_bit->IncRef();
      cfg->SetAnnotationBit(new_bit);
    }
    new_bit->DecRef();
  }

  mcfg->DecRef();
  annot_bit->DecRef(&annot_bit);

  if (skip_directives && annot_bit && BitHasAnyDirective(new_bit))
    return NULL;
  return new_bit;
}

/////////////////////////////////////////////////////////////////////
// BlockMemory
/////////////////////////////////////////////////////////////////////

BlockMemory::BlockMemory(BlockId *id,
                         MemorySimplifyKind simplify_kind,
                         MemoryAliasKind alias_kind,
                         MemoryClobberKind clobber_kind)
  : m_id(id), m_cfg(NULL), m_simplify(NULL), m_alias(NULL), m_clobber(NULL),
    m_computed(false), m_true_bit(NULL),
    m_guard_table(NULL), m_assume_table(NULL),
    m_return_table(NULL), m_target_table(NULL),
    m_assign_table(NULL), m_argument_table(NULL),
    m_clobber_table(NULL), m_val_table(NULL), m_translate_table(NULL)
{
  Assert(m_id);
  m_hash = m_id->Hash();
  m_hash = Hash32(m_hash, simplify_kind);
  m_hash = Hash32(m_hash, alias_kind);
  m_hash = Hash32(m_hash, clobber_kind);

  m_simplify = MemorySimplify::Lookup(simplify_kind);
  m_alias = MemoryAlias::Lookup(alias_kind);
  m_clobber = MemoryClobber::Lookup(clobber_kind);
}

void BlockMemory::SetCFG(BlockCFG *cfg)
{
  Assert(cfg);
  Assert(cfg->GetId() == GetId());

  if (m_cfg == NULL) {
    cfg->IncRef(this);
    m_cfg = cfg;
  }
}

void BlockMemory::ComputeTables()
{
  static BaseTimer compute_timer("memory_compute");
  Timer _timer(&compute_timer);

  Assert(!m_computed);
  Assert(m_cfg);

  MakeTables();
  m_computed = true;

  // the points in the CFG have been sorted, so with a simple forward
  // scan we will visit them in topological order.
  for (PPoint point = 1; point <= m_cfg->GetPointCount(); point++) {
    if (TimerAlarm::ActiveExpired()) {
      // ran out of time processing the edges. bail out, our caller will
      // detect that the timer expired.
      break;
    }

    // compute the guard at this point first.
    // all the other info at this point depends on the guard.
    ComputeGuard(point);

    // the other information depends only on val() at the point
    // and does not have interdependencies with each other
    // - outgoing assume edge conds
    // - call retval/target/argument values
    // - assignment lval/rvals

    const Vector<PEdge*> &outgoing = m_cfg->GetOutgoingEdges(point);

    // sanity check the outgoing edges
    CheckOutgoingEdges(outgoing);

    for (size_t oind = 0; oind < outgoing.Size(); oind++) {
      PEdge *edge = outgoing[oind];
      switch (edge->Kind()) {
      case EGK_Assume:
        ComputeEdgeAssume(edge->AsAssume());
        break;
      case EGK_Assign:
        ComputeEdgeAssign(edge->AsAssign());
        break;
      case EGK_Call:
        ComputeEdgeCall(edge->AsCall());
        break;
      case EGK_Loop:
        ComputeEdgeLoop(edge->AsLoop());
        break;
      case EGK_Assembly:
      case EGK_Annotation:
        break;
      default:
        // shouldn't get skips here.
        Assert(false);
      }
    }
  }
}

Bit* BlockMemory::GetGuard(PPoint point) const
{
  Assert(m_computed);
  Assert(point);
  return m_guard_table->LookupSingle(point);
}

Bit* BlockMemory::GetEdgeCond(PEdge *edge) const
{
  Assert(m_computed);

  if (PEdgeAssume *nedge = edge->IfAssume()) {
    bool nonzero = nedge->IsNonZero();

    GuardTrueFalse pair = m_assume_table->LookupSingle(edge->GetSource());
    return nonzero ? pair.true_guard : pair.false_guard;
  }
  else {
    return NULL;
  }
}

Bit* BlockMemory::GetEdgeTransfer(PEdge *edge) const
{
  Assert(m_computed);

  // compute the condition NG = G0 & EG

  PPoint source = edge->GetSource();
  Bit *guard_source = GetGuard(source);
  Bit *edge_cond = GetEdgeCond(edge);

  if (edge_cond) {
    guard_source->IncRef();
    edge_cond->IncRef();
    return Bit::MakeAnd(guard_source, edge_cond);
  }
  else {
    guard_source->IncRef();
    return guard_source;
  }
}

Bit* BlockMemory::GetEdgeGuard(PEdge *edge, Bit *guard) const
{
  Assert(m_computed);

  // compute the condition NG = reduce(G & G0 & EG, G1). reduce is the
  // Bit operator which simplifies its first argument according to anything
  // implied by its second argument.

  PPoint target = edge->GetTarget();

  // if this is the only incoming edge for target then the result will be G.
  // this case should already have been short circuited elsewhere.
  Assert(m_cfg->GetIncomingEdges(target).Size() > 1);

  Bit *guard_target = GetGuard(target);
  Bit *source_transfer = GetEdgeTransfer(edge);  // holds a reference

  // compute the condition G & G0 & EG
  Bit *base_guard =
    Bit::MakeAnd(guard, source_transfer);

  // compute the final condition reduce(G & G0 & EG, G1)
  Bit *new_guard =
    Bit::ReduceBit(base_guard, guard_target);

  return new_guard;
}

const Vector<GuardExp>* BlockMemory::GetReturns(PPoint point) const
{
  Assert(m_computed);
  Assert(point);
  return m_return_table->Lookup(point, false);
}

const Vector<GuardExp>* BlockMemory::GetTargets(PPoint point) const
{
  Assert(m_computed);
  Assert(point);
  return m_target_table->Lookup(point, false);
}

const Vector<GuardAssign>* BlockMemory::GetAssigns(PPoint point) const
{
  Assert(m_computed);
  Assert(point);
  return m_assign_table->Lookup(point, false);
}

const Vector<GuardAssign>* BlockMemory::GetArguments(PPoint point) const
{
  Assert(m_computed);
  Assert(point);
  return m_argument_table->Lookup(point, false);
}

#ifdef CHECK_MEMORY_CONSISTENCY

template <class T>
void CheckMemoryConsistency(Bit *guard, const Vector<T> &values)
{
  Vector<Bit*> new_values;
  for (size_t ind = 0; ind < values.Size(); ind++)
    new_values.PushBack(values[ind].guard);
  Solver::CheckDisjointBits(guard, new_values);
}

#endif

const Vector<GuardExp>&
BlockMemory::GetVal(Exp *lval, Exp *kind, PPoint point)
{
  Assert(m_cfg);
  Assert(m_computed);
  Assert(point);

  PointValue key(lval, kind, point);
  Vector<GuardExp> *values = m_val_table->Lookup(key, false);
  if (values != NULL)
    return *values;

  values = m_val_table->Lookup(key, true);
  lval->IncRef(values);
  if (kind)
    kind->IncRef(values);

  // just use an empty set of values if there was a timeout.
  // TODO: need to resolve the case when we have a soft timeout and compute
  // incorrect information, then reset that timeout and try to use the data.
  if (TimerAlarm::ActiveExpired())
    return *values;

  // holds the entries we will fill values with.
  GuardExpVector res;

  // if the point is unreachable there are no values.

  Bit *guard = GetGuard(point);
  if (guard->IsFalse())
    return *values;

  // if this is the entry point there is only a single value.

  if (point == m_cfg->GetEntryPoint()) {
    TransferEntry(lval, kind, &res);
    res.FillVector(values, true);
    return *values;
  }

  // otherwise accumulate the values over each incoming edge.

  const Vector<PEdge*> &incoming = m_cfg->GetIncomingEdges(point);
  size_t incoming_count = incoming.Size();
  Assert(incoming_count);

  if (incoming_count == 1) {
    // there is a single incoming edge, just get the values along that edge.
    PEdge *edge = incoming[0];
    TransferEdge(lval, kind, edge, &res);
  }
  else {
    // for each incoming edge, get a list of all the GuardExps
    // along that edge (without applying GetEdgeGuard yet).

    GuardExpVector *incoming_exps = new GuardExpVector[incoming_count];

    for (size_t iind = 0; iind < incoming_count; iind++) {
      PEdge *edge = incoming[iind];
      TransferEdge(lval, kind, edge, &incoming_exps[iind]);
    }

    // determine whether the exps are identical along each incoming edge.
    bool identical = true;
    for (size_t iind = 1; iind < incoming_count; iind++) {
      if (incoming_exps[0].Size() != incoming_exps[iind].Size()) {
        identical = false;
        break;
      }
      for (size_t xind = 0; xind < incoming_exps[0].Size(); xind++) {
        const GuardExp &pgs = incoming_exps[0][xind];
        const GuardExp &ngs = incoming_exps[iind][xind];
        if (pgs.exp != ngs.exp || pgs.guard != ngs.guard) {
          identical = false;
          break;
        }
      }
      if (!identical)
        break;
    }

    if (identical) {
      // the expressions are the same along each incoming edge.
      // just use the exps from one of the edges, this is equivalent
      // to using GetEdgeGuard along each edge and taking the disjunction,
      // but is faster and may construct a simpler guard (if the bit
      // simplifier does not do a maximal simplification).

      res.FillFromVector(incoming_exps[0].m_vector);
    }
    else {
      // apply GetEdgeGuard to each guard and combine each of the lists.

      for (size_t iind = 0; iind < incoming_count; iind++) {
        PEdge *edge = incoming[iind];
        for (size_t xind = 0; xind < incoming_exps[iind].Size(); xind++) {
          const GuardExp &gs = incoming_exps[iind][xind];
          gs.IncRef();

          Bit *edge_guard = GetEdgeGuard(edge, gs.guard);
          res.PushBack(GuardExp(gs.exp, edge_guard));
        }
      }

      // sort and remove duplicates.
      res.SortCombine();
    }

    delete[] incoming_exps;
  }

  res.SimplifyGuards();
  res.FillVector(values, true);

#ifdef CHECK_MEMORY_CONSISTENCY
  CheckMemoryConsistency<GuardExp>(guard, *values);
#endif

  return *values;
}

void BlockMemory::GetValSimplify(Exp *lval, Exp *kind, PPoint point,
                                 GuardExpVector *res)
{
  const Vector<GuardExp> &values = GetVal(lval, kind, point);

  // check to see if we are simplifying the result to a single ExpVal.
  // this simplification is lossy but the precision can be recovered later.
  if (!values.Empty() && m_simplify->SimplifyValues(values)) {
    // make the resulting ExpVal.
    lval->IncRef();
    if (kind)
      kind->IncRef();
    m_true_bit->IncRef();
    Exp *new_exp = Exp::MakeVal(lval, kind, point, false);
    res->PushBack(GuardExp(new_exp, m_true_bit));
  }
  else {
    // use the computed values as-is.
    res->FillFromVector(values);
  }
}

void BlockMemory::GetValComplete(Exp *lval, Exp *kind, PPoint point,
                                 GuardExpVector *res, bool use_try_remove)
{
  // make an ExpVal for the lvalue and then remove it and any transitive
  // ExpVal values it refers to.

  lval->IncRef();
  if (kind)
    kind->IncRef();
  Exp *new_exp = Exp::MakeVal(lval, kind, point, false);

  TranslateKind use_kind = use_try_remove ? TRK_TryRemoveVal : TRK_RemoveVal;
  TranslateExp(use_kind, 0, new_exp, res);
  new_exp->DecRef();

  res->SortCombine();
}

// return whether two argument lvalues should be treated as identical.
// the only way in which they can differ is the name of the underlying
// argument variable. base_arg is from m_argument_table and can only
// be an argument variable or a field of it.
static bool SameArguments(Exp *base_arg, Exp *test_arg)
{
  if (ExpVar *nbase_arg = base_arg->IfVar()) {
    Variable *base_var = nbase_arg->GetVariable();
    Assert(base_var->Kind() == VK_Arg);

    ExpVar *ntest_arg = test_arg->IfVar();
    if (ntest_arg == NULL)
      return false;

    Variable *test_var = ntest_arg->GetVariable();
    if (test_var->Kind() != VK_Arg)
      return false;

    return (base_var->GetIndex() == test_var->GetIndex());
  }

  if (ExpFld *nbase_arg = base_arg->IfFld()) {
    ExpFld *ntest_arg = test_arg->IfFld();
    if (ntest_arg == NULL)
      return false;

    if (nbase_arg->GetField() != ntest_arg->GetField())
      return false;

    return SameArguments(nbase_arg->GetTarget(), ntest_arg->GetTarget());
  }

  logout << "ERROR: Invalid arguments for comparison: "
         << base_arg << " " << test_arg << endl;

  // argument lvalues can only be argument variables or their fields.
  Assert(false);
  return false;
}

void BlockMemory::TranslateExpVal(PPoint point, Exp *value_kind,
                                  const GuardExpVector &target_list,
                                  bool get_value, bool get_edge,
                                  GuardExpVector *res)
{
  Assert(!get_value || !get_edge);

  for (size_t ind = 0; ind < target_list.Size(); ind++) {
    const GuardExp &gt = target_list[ind];

    GuardExpVector base_values;
    if (get_value) {
      GetValSimplify(gt.exp, value_kind, point, &base_values);
    }
    else if (get_edge) {
      PEdge *edge = m_cfg->GetSingleOutgoingEdge(point);
      TransferEdge(gt.exp, value_kind, edge, &base_values);
    }
    else {
      TransferEntry(gt.exp, value_kind, &base_values);
      Assert(base_values.Size() == 1);
    }

    for (size_t vind = 0; vind < base_values.Size(); vind++) {
      const GuardExp &vgt = base_values[vind];
      vgt.IncRef();
      gt.guard->IncRef();

      Bit *guard = Bit::MakeAnd(gt.guard, vgt.guard);
      res->PushBack(GuardExp(vgt.exp, guard));
    }
  }
}

void BlockMemory::TranslateExp(TranslateKind kind, PPoint point, Exp *exp,
                               GuardExpVector *res)
{
  Assert(m_cfg);
  Assert(m_computed);

  PointTranslate key(kind, point, exp);
  Vector<GuardExp> *translated = m_translate_table->Lookup(key, false);
  if (translated) {
    res->FillFromVector(*translated);
    return;
  }

  translated = m_translate_table->Lookup(key, true);
  exp->IncRef(translated);

  PEdge *call_edge = NULL;
  bool translating_call = false;

  if (kind == TRK_Callee || kind == TRK_CalleeExit) {
    call_edge = m_cfg->GetSingleOutgoingEdge(point);
    if (call_edge->IsCall())
      translating_call = true;
    else
      Assert(call_edge->IsLoop());
  }

  switch (exp->Kind()) {

  case EK_String:
  case EK_Int:
  case EK_Float:
  case EK_Directive: {
    exp->IncRef();
    Bit *guard = Bit::MakeConstant(true);
    res->PushBack(GuardExp(exp, guard));
    break;
  }

  case EK_Var: {
    Variable *var = exp->AsVar()->GetVariable();

    if (translating_call && var->Kind() == VK_Return) {
      // return value is translated to the caller's return value.

      const Vector<GuardExp> *returns =
        m_return_table->Lookup(point, false);

      if (returns) {
        for (size_t rind = 0; rind < returns->Size(); rind++) {
          const GuardExp &gt = returns->At(rind);
          gt.IncRef();
          res->PushBack(gt);
        }
      }
    }
    else {
      // check to see if the variable should just be preserved.

      bool preserve_var = false;
      if (var->IsGlobal())
        preserve_var = true;
      else if (!translating_call)
        preserve_var = true;

      if (preserve_var) {
        Exp *new_exp;

        Variable *new_var = m_cfg->FindMatchingVariable(var);
        if (new_var && new_var != var) {
          new_var->IncRef();
          new_exp = Exp::MakeVar(new_var);
        }
        else {
          new_exp = exp;
          new_exp->IncRef();
        }

        Bit *guard = Bit::MakeConstant(true);
        res->PushBack(GuardExp(new_exp, guard));
      }
      else {
        // error to try to translate another kind of variable here.
        logout << "ERROR: TranslateExp: Unexpected var: " << var << endl;
        Assert(false);
      }
    }

    break;
  }

  case EK_Drf: {
    Exp *target = exp->AsDrf()->GetTarget();

    // the target is an argument expression if it is rooted at an argument and
    // contains no dereferences.
    bool call_argument = false;
    size_t argument_index = 0;

    // the target is the 'this' expression.
    bool call_this = false;

    if (translating_call && target->DrfCount() == 0) {
      Variable *root = target->Root();
      if (root) {
        if (root->Kind() == VK_Arg) {
          call_argument = true;
          argument_index = root->GetIndex();
        }
        if (root->Kind() == VK_This) {
          Assert(target->IsVar());
          call_this = true;
        }
      }
    }

    if (call_argument) {
      // check to see if there is even an argument on the edge at this point.
      if (argument_index >= call_edge->AsCall()->GetArgumentCount()) {
        logout << "WARNING: TranslateExp missing argument " << argument_index
               << " at " << call_edge
               << ": " << BlockPPoint(m_id, point) << endl;
      }

      // scan the arguments to look for a matching expression.
      const Vector<GuardAssign> *arguments
        = m_argument_table->Lookup(point, false);

      if (arguments) {
        for (size_t aind = 0; aind < arguments->Size(); aind++) {
          const GuardAssign &gasn = arguments->At(aind);
          if (SameArguments(gasn.left, target)) {
            gasn.right->IncRef();
            gasn.guard->IncRef();
            res->PushBack(GuardExp(gasn.right, gasn.guard));
          }
        }
      }
    }
    else if (call_this) {
      // use the value of the instance object at the call site.
      Vector<GuardExp> *targets = m_target_table->Lookup(point, false);

      if (targets) {
        for (size_t tind = 0; tind < targets->Size(); tind++) {
          const GuardExp &gt = targets->At(tind);
          gt.IncRef();
          res->PushBack(gt);
        }
      }
    }
    else {
      GuardExpVector target_res;
      TranslateExp(kind, point, target, &target_res);

      bool get_value = (kind == TRK_Point || kind == TRK_Callee);
      bool get_edge = (kind == TRK_CalleeExit);

      // identify the possible values of the target lvalues.
      TranslateExpVal(point, NULL, target_res, get_value, get_edge, res);
    }

    // check if we should reduce to a single ExpVal expression.
    // only do this during point translation.

    if (kind == TRK_Point &&
        !res->Empty() && m_simplify->SimplifyValues(res->m_vector)) {
      res->Clear();

      // make the resulting ExpVal.
      target->IncRef();
      m_true_bit->IncRef();
      Exp *new_exp = Exp::MakeVal(target, NULL, point, true);
      res->PushBack(GuardExp(new_exp, m_true_bit));
    }

    break;
  }

  case EK_Fld: {
    ExpFld *nexp = exp->AsFld();
    Exp *target = nexp->GetTarget();
    Field *field = nexp->GetField();

    GuardExpVector target_res;
    TranslateExp(kind, point, target, &target_res);

    for (size_t pind = 0; pind < target_res.Size(); pind++) {
      const GuardExp &pgt = target_res[pind];
      pgt.IncRef();
      field->IncRef();

      Exp *new_exp = Exp::MakeFld(pgt.exp, field);
      res->PushBack(GuardExp(new_exp, pgt.guard));
    }

    break;
  }

  case EK_Rfld: {
    ExpRfld *nexp = exp->AsRfld();
    Exp *target = nexp->GetTarget();
    Field *field = nexp->GetField();

    GuardExpVector target_res;
    TranslateExp(kind, point, target, &target_res);

    for (size_t pind = 0; pind < target_res.Size(); pind++) {
      const GuardExp &pgt = target_res[pind];
      pgt.IncRef();
      field->IncRef();

      Exp *new_exp = Exp::MakeRfld(pgt.exp, field);
      res->PushBack(GuardExp(new_exp, pgt.guard));
    }

    break;
  }

  case EK_Index: {
    ExpIndex *nexp = exp->AsIndex();
    Exp *target = nexp->GetTarget();
    Exp *index = nexp->GetIndex();
    Type *elem_type = nexp->GetElementType();

    GuardExpVector target_res;
    TranslateExp(kind, point, target, &target_res);

    GuardExpVector index_res;
    TranslateExp(kind, point, index, &index_res);

    for (size_t pind = 0; pind < target_res.Size(); pind++) {
      for (size_t sind = 0; sind < index_res.Size(); sind++) {
        const GuardExp &pgt = target_res[pind];
        const GuardExp &igs = index_res[sind];

        pgt.IncRef();
        igs.IncRef();
        elem_type->IncRef();

        Exp *new_exp = Exp::MakeIndex(pgt.exp, elem_type, igs.exp);
        Bit *guard = Bit::MakeAnd(pgt.guard, igs.guard);
        res->PushBack(GuardExp(new_exp, guard));
      }
    }

    break;
  }

  case EK_Unop: {
    ExpUnop *nexp = exp->AsUnop();
    UnopKind unop = nexp->GetUnopKind();
    Exp *op = nexp->GetOperand();

    GuardExpVector op_res;
    TranslateExp(kind, point, op, &op_res);

    for (size_t oind = 0; oind < op_res.Size(); oind++) {
      const GuardExp &gs = op_res[oind];
      gs.IncRef();

      Exp *new_exp = Exp::MakeUnop(unop, gs.exp, exp->Bits(), exp->Sign());
      res->PushBack(GuardExp(new_exp, gs.guard));
    }

    break;
  }

  case EK_Binop: {
    ExpBinop *nexp = exp->AsBinop();
    BinopKind binop = nexp->GetBinopKind();
    Exp *left_op = nexp->GetLeftOperand();
    Exp *right_op = nexp->GetRightOperand();
    Type *stride_type = nexp->GetStrideType();

    GuardExpVector left_res;
    TranslateExp(kind, point, left_op, &left_res);

    GuardExpVector right_res;
    TranslateExp(kind, point, right_op, &right_res);

    for (size_t lind = 0; lind < left_res.Size(); lind++) {
      for (size_t rind = 0; rind < right_res.Size(); rind++) {
        const GuardExp &lgs = left_res[lind];
        const GuardExp &rgs = right_res[rind];

        lgs.IncRef();
        rgs.IncRef();
        if (stride_type != NULL)
          stride_type->IncRef();

        Exp *new_exp =
          Exp::MakeBinop(binop, lgs.exp, rgs.exp,
                         stride_type, exp->Bits(), exp->Sign());
        Bit *new_guard = Bit::MakeAnd(lgs.guard, rgs.guard);

        res->PushBack(GuardExp(new_exp, new_guard));
      }
    }

    break;
  }

  case EK_Clobber: {
    ExpClobber *nexp = exp->AsClobber();
    Exp *callee = nexp->GetCallee();
    Exp *value_kind = nexp->GetValueKind();
    PPoint clobber_point = nexp->GetPoint();

    if (kind == TRK_SkipClobber && point == clobber_point) {
      // this had better be a loop invocation.
      Assert(!m_cfg->PointEdgeIsCall(point));

      GuardExpVector callee_res;
      TranslateExp(kind, point, callee, &callee_res);

      // perform a point translation to remove the Clobber.
      TranslateExpVal(point, value_kind, callee_res, true, false, res);
    }
    else {
      // preserved as is.
      exp->IncRef();
      Bit *guard = Bit::MakeConstant(true);
      res->PushBack(GuardExp(exp, guard));
    }

    break;
  }

  case EK_Exit: {
    ExpExit *nexp = exp->AsExit();
    Exp *target = nexp->GetTarget();
    Exp *value_kind = nexp->GetValueKind();

    if (kind == TRK_SkipClobber) {
      // the callee target used in SkipClobber can contain Exit expressions,
      // which we need to remove here.
      Assert(!m_cfg->PointEdgeIsCall(point));

      GuardExpVector target_res;
      TranslateExp(kind, point, target, &target_res);

      // perform a point translation to remove the exit.
      TranslateExpVal(point, value_kind, target_res, true, false, res);
    }
    else {
      GuardExpVector target_res;
      TranslateExp(kind, point, target, &target_res);

      bool get_value = (kind == TRK_Exit);
      bool get_edge = (kind == TRK_Callee || kind == TRK_CalleeExit);
      Assert(get_value || get_edge);

      TranslateExpVal(point, value_kind, target_res, get_value, get_edge, res);
    }

    break;
  }

  case EK_Initial: {
    ExpInitial *nexp = exp->AsInitial();
    Exp *target = nexp->GetTarget();
    Exp *value_kind = nexp->GetValueKind();

    GuardExpVector target_res;
    TranslateExp(kind, point, target, &target_res);

    if (kind == TRK_Callee || kind == TRK_CalleeExit) {
      // get the values at the call/loop point, remove the initial.
      TranslateExpVal(point, value_kind, target_res, true, false, res);
    }
    else if (kind == TRK_Point) {
      if (m_cfg->IsLoopIsomorphic(point)) {
        // the initial is relative to when the closest loop started.
        // walk back and find that closest loop head. this is the innermost
        // loop surrounding the point. the loop head should dominate this
        // point, thus we only need to follow a single path backwards.
        PPoint loop_point = point;
        while (true) {
          const Vector<PEdge*> &incoming = m_cfg->GetIncomingEdges(loop_point);
          Assert(!incoming.Empty());
          loop_point = incoming[0]->GetSource();
          if (incoming[0]->IsLoop()) break;
        }

        // get the value of the expression when the loop was invoked.
        TranslateExpVal(loop_point, value_kind, target_res, true, false, res);
      }
      else if (m_id->Kind() == B_Function) {
        // the initial is relative to function entry.
        TranslateExpVal(0, value_kind, target_res, false, false, res);
      }
      else {
        // leave the initial alone. either we are in a loop and the initial
        // is relative to the entry state of the loop, or we are generating
        // the annotation bit itself.
        Assert(m_id->Kind() == B_Loop || m_id->Kind() == B_AnnotationFunc);

        for (size_t ind = 0; ind < target_res.Size(); ind++) {
          const GuardExp &gt = target_res[ind];

          gt.IncRef();
          if (value_kind)
            value_kind->IncRef();
          Exp *new_exp = Exp::MakeInitial(gt.exp, value_kind);
          res->PushBack(GuardExp(new_exp, gt.guard));
        }
      }
    }
    else {
      for (size_t ind = 0; ind < target_res.Size(); ind++) {
        const GuardExp &gt = target_res[ind];

        gt.IncRef();
        if (value_kind)
          value_kind->IncRef();
        Exp *new_exp = Exp::MakeInitial(gt.exp, value_kind);
        res->PushBack(GuardExp(new_exp, gt.guard));
      }
    }

    break;
  }

  case EK_Val: {
    ExpVal *nexp = exp->AsVal();
    Exp *lval = nexp->GetLvalue();
    Exp *value_kind = nexp->GetValueKind();
    PPoint value_point = nexp->GetPoint();

    if (kind == TRK_Exit) {
      // preserve the value unchanged. for TRK_Exit we are only operating
      // on the exit expressions.
      exp->IncRef();
      m_true_bit->IncRef();
      res->PushBack(GuardExp(exp, m_true_bit));
      break;
    }

    Assert(kind == TRK_RemoveVal || kind == TRK_TryRemoveVal);
    Assert(point == 0);

    // get the actual lvalues being referred to.
    GuardExpVector lval_res;
    if (nexp->IsRelative()) {
      TranslateExp(TRK_Point, value_point, lval, &lval_res);
    }
    else {
      lval->IncRef();
      m_true_bit->IncRef();
      lval_res.PushBack(GuardExp(lval, m_true_bit));
    }

    // whether we hit the expansion cutoff and are going to revert to
    // the original ExpVal expression.
    bool remove_cutoff = false;

    for (size_t lind = 0; lind < lval_res.Size(); lind++) {
      const GuardExp &lv = lval_res[lind];

      // get the possible values of the lvalue at the point.
      const Vector<GuardExp> &values =
        GetVal(lv.exp, value_kind, value_point);

      for (size_t bind = 0; bind < values.Size(); bind++) {
        const GuardExp &bv = values[bind];

        if (TimerAlarm::ActiveExpired())
          break;

        // transitively remove Val expressions from this expression.
        GuardExpVector remove_res;
        TranslateExp(kind, 0, bv.exp, &remove_res);

        for (size_t rind = 0; rind < remove_res.Size(); rind++) {
          const GuardExp &rv = remove_res[rind];
          rv.IncRef();
          lv.guard->IncRef();
          bv.guard->IncRef();

          Bit *new_guard = Bit::MakeAnd(bv.guard, rv.guard);
          new_guard = Bit::MakeAnd(new_guard, lv.guard);

          res->PushBack(GuardExp(rv.exp, new_guard));
        }

        if (kind == TRK_TryRemoveVal && res->Size() > TRY_REMOVE_CUTOFF) {
          remove_cutoff = true;
          break;
        }
      }

      if (remove_cutoff)
        break;
    }

    if (remove_cutoff) {
      // hit the cutoff for expanding this value, revert to the original
      // ExpVal expression.
      res->Clear();

      exp->IncRef();
      m_true_bit->IncRef();
      res->PushBack(GuardExp(exp, m_true_bit));
    }

    break;
  }

  case EK_Frame: {
    exp->IncRef();
    Bit *guard = Bit::MakeConstant(true);
    res->PushBack(GuardExp(exp, guard));
    break;
  }

  case EK_NullTest:
  case EK_Bound: {
    Exp *target = exp->GetLvalTarget();
    Assert(target);

    GuardExpVector target_res;
    TranslateExp(kind, point, target, &target_res);

    for (size_t tind = 0; tind < target_res.Size(); tind++) {
      const GuardExp &gt = target_res[tind];
      gt.IncRef();

      Exp *new_exp = exp->ReplaceLvalTarget(gt.exp);
      res->PushBack(GuardExp(new_exp, gt.guard));
    }

    break;
  }

  case EK_Terminate: {
    ExpTerminate *nexp = exp->AsTerminate();
    Exp *target = nexp->GetTarget();
    Type *stride_type = nexp->GetStrideType();
    Exp *terminate_test = nexp->GetTerminateTest();
    ExpInt *terminate_int = nexp->GetTerminateInt();

    GuardExpVector target_res;
    TranslateExp(kind, point, target, &target_res);

    stride_type->IncRef();
    terminate_test->IncRef();
    terminate_int->IncRef();

    Exp *value_kind = Exp::MakeTerminate(NULL, stride_type,
                                         terminate_test, terminate_int);

    bool get_value = (kind == TRK_Point || kind == TRK_Callee);
    bool get_edge = (kind == TRK_CalleeExit);

    TranslateExpVal(point, value_kind, target_res, get_value, get_edge, res);
    value_kind->DecRef();

    break;
  }

  default:
    Assert(false);
  }

  res->SimplifyGuards();

#ifdef CHECK_MEMORY_CONSISTENCY
  Bit *guard = (point ? GetGuard(point) : NULL);
  CheckMemoryConsistency<GuardExp>(guard, res->m_vector);
#endif

  res->FillVector(translated, true);
}

void BlockMemory::TranslateBit(TranslateKind kind, PPoint point,
                               Bit *bit, GuardBitVector *res)
{
  Assert(m_cfg);
  Assert(m_computed);

  switch (bit->Kind()) {
  case BIT_True:
  case BIT_False: {
    // preserve the bit as is.
    bit->IncRef();
    Bit *guard = Bit::MakeConstant(true);
    res->PushBack(GuardBit(bit, guard));
    break;
  }

  case BIT_Var: {
    // translate the underlying expression.
    GuardExpVector var_res;
    TranslateExp(kind, point, bit->GetVar(), &var_res);

    for (size_t sind = 0; sind < var_res.Size(); sind++) {
      const GuardExp &gs = var_res[sind];

      Bit *new_bit = Exp::MakeNonZeroBit(gs.exp);
      gs.guard->IncRef();
      res->PushBack(GuardBit(new_bit, gs.guard));
    }

    break;
  }

  // handling negation can be tricky as there are two ways of doing this which
  // end up being equivalent. if there are multiple possible values for the
  // negated bit, say a and b with distinguishing guards !g and g, then the
  // final non-negated condition is as follows:
  //
  // C1: (a & !g) | (b & g)
  //
  // we can directly negate this to get the negated final condition:
  //
  //     !((a & !g) | (b & g))
  //     !(a & !g) & !(b & g)
  // C2: (!a | g) & (!b | !g)
  //
  // alternatively, we can negate each component of the bit individually.
  //
  // C3: (!a & !g) | (!b & g)
  //
  // as the following truth table shows, C2 and C3 are equivalent and are both
  // the negation of C1.
  //
  // a b g | C1 | C2 | C3
  //       |    |    |
  // 0 0 0 | 0  | 1  | 1
  // 0 0 1 | 0  | 1  | 1
  // 0 1 0 | 0  | 1  | 1
  // 0 1 1 | 1  | 0  | 0
  // 1 0 0 | 1  | 0  | 0
  // 1 0 1 | 0  | 1  | 1
  // 1 1 0 | 1  | 0  | 0
  // 1 1 1 | 1  | 0  | 0
  //
  // we will be computing condition C3 instead of C2. note that these formulas
  // are only equivalent if the distinguishing guards are pairwise disjoint
  // and have 'true' as their collective disjunction.

  case BIT_Not: {
    Bit *neg_bit = bit->GetOperand(0);

    GuardBitVector neg_res;
    TranslateBit(kind, point, neg_bit, &neg_res);

    for (size_t ind = 0; ind < neg_res.Size(); ind++) {
      const GuardBit &gb = neg_res[ind];
      gb.IncRef();

      Bit *neg_bit = Bit::MakeNot(gb.bit);
      res->PushBack(GuardBit(neg_bit, gb.guard));
    }

    break;
  }

  case BIT_And:
  case BIT_Or: {
    Assert(bit->GetOperandCount() > 1);

    // fill res with the result of translating the first bit.
    Bit *first_bit = bit->GetOperand(0);
    TranslateBit(kind, point, first_bit, res);

    // before/after each iteration of this loop, res holds the result
    // of translating and combining the operand range [0,oind-1]
    for (size_t oind = 1; oind < bit->GetOperandCount(); oind++) {
      Bit *op_bit = bit->GetOperand(oind);

      // clear out and remember all the previous bit results.
      GuardBitVector prev_res;
      for (size_t ind = 0; ind < res->Size(); ind++) {
        const GuardBit &gb = res->At(ind);
        gb.IncRef();
        prev_res.PushBack(gb);
      }
      res->Clear();

      GuardBitVector next_res;
      TranslateBit(kind, point, op_bit, &next_res);

      // combine the bits in prev_res and op_res.
      for (size_t pind = 0; pind < prev_res.Size(); pind++) {
        const GuardBit &pgs = prev_res[pind];

        for (size_t nind = 0; nind < next_res.Size(); nind++) {
          const GuardBit &ngs = next_res[nind];

          if (TimerAlarm::ActiveExpired())
            break;

          pgs.IncRef();
          ngs.IncRef();

          Bit *new_bit = NULL;
          if (bit->Kind() == BIT_And)
            new_bit = Bit::MakeAnd(pgs.bit, ngs.bit);
          else
            new_bit = Bit::MakeOr(pgs.bit, ngs.bit);

          Bit *new_guard = Bit::MakeAnd(pgs.guard, ngs.guard);
          res->PushBack(GuardBit(new_bit, new_guard));
        }
      }
    }

    break;
  }

  default:
    Assert(false);
  }

  res->SimplifyGuards();

#ifdef CHECK_MEMORY_CONSISTENCY
  Bit *guard = (point ? GetGuard(point) : NULL);
  CheckMemoryConsistency<GuardBit>(guard, res->m_vector);
#endif
}

void BlockMemory::TranslateBit(TranslateKind kind, PPoint point,
                               Bit *bit, Bit **res)
{
  GuardBitVector base_res;
  TranslateBit(kind, point, bit, &base_res);

  // accumulate the disjunction over each of the possibilities.
  // holds a reference if not NULL.
  Bit *disjunct = NULL;

  for (size_t ind = 0; ind < base_res.Size(); ind++) {
    const GuardBit &gb = base_res[ind];

    if (TimerAlarm::ActiveExpired())
      break;

    gb.IncRef();

    Bit *conjunct = Bit::MakeAnd(gb.bit, gb.guard);
    if (disjunct) {
      disjunct = Bit::MakeOr(disjunct, conjunct);
    }
    else {
      disjunct = conjunct;
    }
  }

  if (disjunct == NULL) {
    // point should be unreachable. TODO: test for this.
    disjunct = Bit::MakeConstant(false);
  }

  *res = disjunct;
  disjunct->MoveRef(NULL, res);
}

void BlockMemory::TranslateAssign(TranslateKind kind, PPoint point,
                             Type *type, Exp *left, Exp *right, Bit *when,
                             Vector<GuardAssign> *res)
{
  Assert(m_cfg);
  Assert(m_computed);
  Assert(point);

  GuardExpVector left_res;
  TranslateExp(kind, point, left, &left_res);

  GuardExpVector right_res;
  TranslateExp(kind, point, right, &right_res);

  Bit *when_res = NULL;
  if (when && !when->IsTrue())
    TranslateBit(kind, point, when, &when_res);

  // check each combination of lval and rval
  for (size_t tind = 0; tind < left_res.Size(); tind++) {
    for (size_t sind = 0; sind < right_res.Size(); sind++) {
      const GuardExp &lgt = left_res[tind];
      const GuardExp &rgs = right_res[sind];

      lgt.IncRef();
      rgs.IncRef();

      // get the conjunction of the two conditions.
      Bit *guard = Bit::MakeAnd(lgt.guard, rgs.guard);

      // combine with the condition where the assignment occurs.
      if (when_res) {
        when_res->IncRef();
        guard = Bit::MakeAnd(guard, when_res);
      }

      // if the conjunction might be satisfiable then add the assign.
      if (guard->IsFalse()) {
        // trivially unsat
        guard->DecRef();
        lgt.exp->DecRef();
        rgs.exp->DecRef();
      }
      else {
        ComputeSingleAssign(type, lgt.exp, rgs.exp, guard, res);
      }
    }
  }

  if (when_res)
    when_res->DecRef(&when_res);
}

void BlockMemory::TranslateReceiver(PPoint point, GuardExpVector *res)
{
  Assert(m_cfg);
  PEdgeCall *edge = m_cfg->GetSingleOutgoingEdge(point)->AsCall();
  Assert(edge->GetDirectCallee() == NULL);

  Exp *function = edge->GetFunction();

  if (Exp *call_instance = edge->GetInstanceObject()) {
    // the function is relative to this object so compose the two.
    Exp *combine = Exp::Compose(call_instance, function);
    TranslateExp(TRK_Point, point, combine, res);
    combine->DecRef();
  }
  else {
    // standard indirect call through a function pointer.
    TranslateExp(TRK_Point, point, function, res);
    return;
  }
}

class TranslateCalleeVisitor : public ExpVisitor
{
 public:
  PEdgeCall *edge;
  Exp *exclude;

  TranslateCalleeVisitor(PEdgeCall *_edge)
    : ExpVisitor(VISK_All), edge(_edge), exclude(NULL)
  {}

  void Visit(Exp *exp)
  {
    if (ExpExit *nexp = exp->IfExit())
      nexp->GetTarget()->DoVisit(this);

    if (ExpVar *nexp = exp->IfVar()) {
      Variable *var = nexp->GetVariable();
      if (var->Kind() == VK_Return) {
        if (edge->GetReturnValue() == NULL)
          exclude = exp;
      }
      else if (var->Kind() == VK_Arg) {
        if (var->GetIndex() >= edge->GetArgumentCount())
          exclude = exp;
      }
    }
  }
};

bool BlockMemory::CanTranslateCalleeExp(PPoint point, Exp *exp)
{
  Assert(m_cfg);
  PEdge *edge = m_cfg->GetSingleOutgoingEdge(point);

  if (PEdgeCall *nedge = edge->IfCall()) {
    TranslateCalleeVisitor visitor(nedge);
    exp->DoVisit(&visitor);
    return (visitor.exclude == NULL);
  }

  return true;
}

bool BlockMemory::CanTranslateCalleeBit(PPoint point, Bit *bit)
{
  Assert(m_cfg);
  PEdge *edge = m_cfg->GetSingleOutgoingEdge(point);

  if (PEdgeCall *nedge = edge->IfCall()) {
    TranslateCalleeVisitor visitor(nedge);
    bit->DoVisit(&visitor);
    return (visitor.exclude == NULL);
  }

  return true;
}

bool BlockMemory::IsLvalClobbered(Exp *lval, Exp *kind, PEdge *edge,
                                  Exp **inner, Bit **guard)
{
  if (!edge->IsCall() && !edge->IsLoop())
    return false;

  Vector<GuardAssign> *clobbered =
    m_clobber_table->Lookup(edge->GetSource(), false);

  if (!clobbered)
    return false;

  for (size_t ind = 0; ind < clobbered->Size(); ind++) {
    const GuardAssign &gasn = clobbered->At(ind);

    if (gasn.kind != kind)
      continue;

    Bit *alias = IsLvalAliased(gasn.left, lval, kind);
    if (!alias)
      continue;

    Bit *when;
    if (gasn.guard) {
      gasn.guard->IncRef();
      when = Bit::MakeAnd(gasn.guard, alias);
    }
    else {
      when = alias;
    }      

    *inner = gasn.right;
    *guard = when;

    // short circuit the common case where the clobbering is unconditional.
    if (when->IsTrue())
      return true;

    // accumulate the guard to use. account for any other clobbered
    // inner lvals which might map to the same outer lval due to
    // aliasing at entry to the callee.

    for (size_t pind = ind + 1; pind < clobbered->Size(); pind++) {
      const GuardAssign &pgasn = clobbered->At(pind);

      if (pgasn.kind != kind)
        continue;

      alias = IsLvalAliased(pgasn.left, lval, kind);
      if (!alias)
        continue;

      if (pgasn.guard) {
        pgasn.guard->IncRef();
        when = Bit::MakeAnd(pgasn.guard, alias);
      }
      else {
        when = alias;
      }

      *guard = Bit::MakeOr(*guard, when);
    }

    return true;
  }

  return false;
}

class PreservedBlockVisitor : public ExpVisitor
{
public:
  BlockMemory *mcfg;
  bool preserved;

  PreservedBlockVisitor(BlockMemory *_mcfg)
    : ExpVisitor(VISK_All), mcfg(_mcfg), preserved(true)
  {}

  void Visit(Exp *exp)
  {
    if (exp->IsClobber() || exp->IsVal())
      preserved = false;

    if (ExpDrf *nexp = exp->IfDrf()) {
      Exp *target = nexp->GetTarget();

      PPoint exit_point = mcfg->GetCFG()->GetExitPoint();
      Assert(exit_point);

      const Vector<GuardExp> &values =
        mcfg->GetVal(target, NULL, exit_point);

      if (values.Size() == 1) {
        if (ExpDereference(values[0].exp) != target)
          preserved = false;
      }
      else {
        preserved = false;
      }
    }
  }
};

bool BlockMemory::IsExpPreserved(Exp *exp)
{
  Assert(m_cfg);
  Assert(m_computed);

  PreservedBlockVisitor visitor(this);
  exp->DoVisit(&visitor);

  return visitor.preserved;
}

bool BlockMemory::IsBitPreserved(Bit *bit)
{
  Assert(m_cfg);
  Assert(m_computed);

  PreservedBlockVisitor visitor(this);
  bit->DoVisit(&visitor);

  return visitor.preserved;
}

Exp* BlockMemory::GetBaseBuffer(Exp *lval, Type *stride_type)
{
  Assert(m_cfg);
  Assert(m_computed);

  // strip off any index from lval.
  if (ExpIndex *nlval = lval->IfIndex()) {
    if (nlval->GetElementType() == stride_type)
      lval = nlval->GetTarget();
  }

  lval->IncRef();
  return lval;
}

ExpTerminate* BlockMemory::GetTerminateAssign(PPoint point,
                                              Exp *left, Exp *right,
                                              Exp **lval)
{
  // only handling direct assignments of a terminator constant for now.

  Type *stride_type = left->GetType();
  if (!stride_type)
    return NULL;

  ExpInt *terminate_int = right->IfInt();
  if (!terminate_int)
    return NULL;

  Exp *terminate_test = Exp::MakeEmpty();
  Exp *use_lval = left;

  if (ExpFld *nleft = left->IfFld()) {
    Field *field = nleft->GetField();

    field->IncRef();
    terminate_test = Exp::MakeFld(terminate_test, field);

    use_lval = nleft->GetTarget();
    stride_type = field->GetCSUType();
  }

  if (stride_type->Width() == 0) {
    terminate_test->DecRef();
    return NULL;
  }

  // do some filtering on the possible assignments we will consider
  // as establishing a terminator. TODO: should make IsLvalAliased
  // less dumb and get rid of this filter.
  if (use_lval->IsIndex() || use_lval->IsDrf() || use_lval->IsClobber() ||
      (use_lval->IsVar() && use_lval->AsVar()->GetVariable()->IsGlobal())) {

    stride_type->IncRef();
    terminate_int->IncRef();
    Exp *res = Exp::MakeTerminate(NULL, stride_type,
                                  terminate_test, terminate_int);

    *lval = use_lval;
    return res->AsTerminate();
  }
  else {
    terminate_test->DecRef();
    return NULL;
  }
}

void BlockMemory::Print(OutStream &out) const
{
  out << "memory: " << m_id << endl;

  if (!m_computed) {
    out << "  [unknown]" << endl;
    return;
  }

  if (!m_cfg) {
    out << "  [missing CFG]" << endl;
    return;
  }

  for (PPoint point = 1; point <= m_cfg->GetPointCount(); point++) {
    Bit *guard = GetGuard(point);

    out << "point " << point << ": guard " << guard << endl;

    Vector<GuardAssign> *assigns = m_assign_table->Lookup(point, false);
    if (assigns != NULL) {
      for (size_t aind = 0; aind < assigns->Size(); aind++) {
        const GuardAssign &gasn = assigns->At(aind);
        out << "assign " << gasn.left << " := " << gasn.right
            << " " << gasn.guard << endl;
      }
    }

    Vector<GuardAssign> *arguments = m_argument_table->Lookup(point, false);
    if (arguments != NULL) {
      for (size_t aind = 0; aind < arguments->Size(); aind++) {
        const GuardAssign &gasn = arguments->At(aind);
        out << "argument " << gasn.left << " := " << gasn.right
            << " " << gasn.guard << endl;
      }
    }

    Vector<GuardAssign> *clobbers = m_clobber_table->Lookup(point, false);
    if (clobbers != NULL) {
      for (size_t aind = 0; aind < clobbers->Size(); aind++) {
        const GuardAssign &gasn = clobbers->At(aind);
        out << "clobber " << gasn.left << " := " << gasn.right
            << " " << gasn.guard;
        if (gasn.kind)
          out << " [" << gasn.kind << "]";
        out << endl;
      }
    }
  }
}

// as with TryMoveRef, drop a reference on bit unless it is true.
static inline void TryDecRef(Bit *bit, ORef ov)
{
  if (!bit->IsTrue())
    bit->DecRef(ov);
}

static void DecRefGuard(const Vector<Bit*> &bits)
{
  Assert(bits.Size() == 1);
  TryDecRef(bits[0], &bits);
}

static void DecRefAssume(const Vector<GuardTrueFalse> &vals)
{
  Assert(vals.Size() == 1);
  if (vals[0].true_guard)
    TryDecRef(vals[0].true_guard, &vals);
  if (vals[0].false_guard)
    TryDecRef(vals[0].false_guard, &vals);
}

static void DecRefGuardExp(const Vector<GuardExp> &vals)
{
  for (size_t ind = 0; ind < vals.Size(); ind++) {
    const GuardExp &v = vals[ind];
    v.exp->DecRef(&vals);
    TryDecRef(v.guard, &vals);
  }
}

static void DecRefGuardAssign(const Vector<GuardAssign> &vals)
{
  for (size_t ind = 0; ind < vals.Size(); ind++) {
    const GuardAssign &v = vals[ind];
    v.left->DecRef(&vals);
    v.right->DecRef(&vals);
    if (v.kind)
      v.kind->DecRef(&vals);
    TryDecRef(v.guard, &vals);
  }
}

void BlockMemory::DecMoveChildRefs(ORef ov, ORef nv)
{
  m_id->DecMoveRef(ov, nv);

  if (m_cfg)
    m_cfg->DecMoveRef(ov, nv);

  if (!m_computed)
    return;

  Assert(nv == NULL);

  // remaining table references are removed during UnPersist. we don't
  // really need to call this since it will get called during normal
  // teardown, but if we leak references then this will be called without
  // unpersist so we want to make sure the extra references are cleaned up.
  UnPersist();
}

void BlockMemory::Persist()
{
  Assert(!m_computed);
}

void BlockMemory::UnPersist()
{
  if (!m_computed)
    return;

  HashIteratePtr(m_guard_table)
    DecRefGuard(m_guard_table->ItValues());

  HashIteratePtr(m_assume_table)
    DecRefAssume(m_assume_table->ItValues());

  HashIteratePtr(m_return_table)
    DecRefGuardExp(m_return_table->ItValues());

  HashIteratePtr(m_target_table)
    DecRefGuardExp(m_target_table->ItValues());

  HashIteratePtr(m_assign_table)
    DecRefGuardAssign(m_assign_table->ItValues());

  HashIteratePtr(m_argument_table)
    DecRefGuardAssign(m_argument_table->ItValues());

  HashIteratePtr(m_clobber_table)
    DecRefGuardAssign(m_clobber_table->ItValues());

  HashIteratePtr(m_val_table) {
    const Vector<GuardExp> &vals = m_val_table->ItValues();
    DecRefGuardExp(vals);

    const PointValue &o = m_val_table->ItKey();
    o.lval->DecRef(&vals);
    if (o.kind)
      o.kind->DecRef(&vals);
  }

  HashIteratePtr(m_translate_table) {
    const Vector<GuardExp> &vals = m_translate_table->ItValues();
    DecRefGuardExp(vals);

    const PointTranslate &o = m_translate_table->ItKey();
    o.exp->DecRef(&vals);
  }

  delete m_guard_table;
  delete m_assume_table;
  delete m_return_table;
  delete m_target_table;
  delete m_assign_table;
  delete m_argument_table;
  delete m_clobber_table;
  delete m_val_table;
  delete m_translate_table;

  m_guard_table = NULL;
  m_assume_table = NULL;
  m_return_table = NULL;
  m_target_table = NULL;
  m_assign_table = NULL;
  m_argument_table = NULL;
  m_clobber_table = NULL;
  m_val_table = NULL;
  m_translate_table = NULL;

  m_computed = false;

  // have to remove this reference last as the tables depend on it.
  m_true_bit->DecRef(this);
}

void BlockMemory::MakeTables()
{
  m_guard_table = new GuardTable();
  m_assume_table = new AssumeTable();
  m_return_table = new GuardExpTable();
  m_target_table = new GuardExpTable();
  m_assign_table = new GuardAssignTable();
  m_argument_table = new GuardAssignTable();
  m_clobber_table = new GuardAssignTable();
  m_val_table = new ValueTable();
  m_translate_table = new TranslateTable();

  m_true_bit = Bit::MakeConstant(true);
  m_true_bit->MoveRef(NULL, this);
}

void BlockMemory::CheckOutgoingEdges(const Vector<PEdge*> &outgoing)
{
  // can't have more than two outgoing edges.
  Assert(outgoing.Size() <= 2);

  // except for skips, it's always fine to have one outgoing edge.
  if (outgoing.Size() == 1) {
    PEdge *edge = outgoing[0];
    Assert(!edge->IsSkip());
  }

  // can only have two outgoing edges if they are negated assumes.
  if (outgoing.Size() == 2) {
    PEdge *edge0 = outgoing[0];
    PEdge *edge1 = outgoing[1];

    PEdgeAssume *ne0 = edge0->AsAssume();
    PEdgeAssume *ne1 = edge1->AsAssume();

    Assert(ne0->GetCondition() == ne1->GetCondition());
    Assert(ne0->IsNonZero() != ne1->IsNonZero());
  }
}

void BlockMemory::ComputeGuard(PPoint point)
{
  Vector<Bit*> *entries = m_guard_table->Lookup(point, true);
  Assert(entries->Empty());

  // accumulate the guard. we have a reference on this if it isn't NULL.
  Bit *guard_bit = NULL;

  if (point == m_cfg->GetEntryPoint()) {
    // the guard at block entry is always true.
    guard_bit = Bit::MakeConstant(true);
  }
  else {
    // the guard is the disjunction of the transfer condition over each
    // incoming edge.

    const Vector<PEdge*> &incoming = m_cfg->GetIncomingEdges(point);
    Assert(!incoming.Empty());

    for (size_t iind = 0; iind < incoming.Size(); iind++) {
      PEdge *edge = incoming[iind];
      Bit *source_transfer = GetEdgeTransfer(edge);

      if (guard_bit != NULL) {
        guard_bit = Bit::MakeOr(guard_bit, source_transfer);
      }
      else {
        guard_bit = source_transfer;
      }
    }
  }

  Assert(guard_bit != NULL);

  entries->PushBack(guard_bit);
  TryMoveRef(guard_bit, entries);
}

void BlockMemory::ComputeEdgeAssume(PEdgeAssume *edge)
{
  Vector<GuardTrueFalse> *entries =
    m_assume_table->Lookup(edge->GetSource(), true);
  if (entries->Empty())
    entries->PushBack(GuardTrueFalse());
  Assert(entries->Size() == 1);

  Exp *condition = edge->GetCondition();
  bool nonzero = edge->IsNonZero();

  Bit *cond_holds = Exp::MakeNonZeroBit(condition);
  if (!nonzero)
    cond_holds = Bit::MakeNot(cond_holds);

  Bit *cond_res;
  TranslateBit(TRK_Point, edge->GetSource(), cond_holds, &cond_res);

  cond_holds->DecRef();

  if (nonzero) {
    Assert(entries->At(0).true_guard == NULL);
    entries->At(0).true_guard = cond_res;
  }
  else {
    Assert(entries->At(0).false_guard == NULL);
    entries->At(0).false_guard = cond_res;
  }

  cond_res->MoveRef(&cond_res, NULL);
  TryMoveRef(cond_res, entries);
}

void BlockMemory::ComputeEdgeAssign(PEdgeAssign *edge)
{
  PPoint point = edge->GetSource();

  Vector<GuardAssign> *assigns = m_assign_table->Lookup(point, true);
  Assert(assigns->Empty());

  Type *type = edge->GetType();
  Exp *lval = edge->GetLeftSide();
  Exp *rval = edge->GetRightSide();

  TranslateAssign(TRK_Point, point, type, lval, rval, NULL, assigns);
}

void BlockMemory::ComputeEdgeCall(PEdgeCall *edge)
{
  PPoint point = edge->GetSource();
  TypeFunction *type = edge->GetType();

  // add the function and instance object information.

  Exp *call_target = edge->GetInstanceObject();
  if (call_target == NULL)
    call_target = edge->GetFunction();
  Assert(call_target);

  Vector<GuardExp> *target_values = m_target_table->Lookup(point, true);
  Assert(target_values->Empty());

  GuardExpVector target_res;
  TranslateExp(TRK_Point, point, call_target, &target_res);
  target_res.FillVector(target_values, true);

  // add the call argument assignments.

  if (edge->GetArgumentCount() > 0) {

    Vector<GuardAssign> *arguments =
      m_argument_table->Lookup(point, true);
    Assert(arguments->Empty());

    for (size_t aind = 0; aind < edge->GetArgumentCount(); aind++) {
      Exp *argval = edge->GetArgument(aind);

      Type *arg_type = NULL;
      if (aind < type->GetArgumentCount()) {
        arg_type = type->GetArgumentType(aind);
        arg_type->IncRef();
      }
      else {
        // just use a void type. this should only show up for varargs
        // calls or where there is no prototype for the callee. in these
        // cases the compiler will use 'int', but for analysis the type
        // does not matter.
        arg_type = Type::MakeVoid();
      }

      Exp *arg_exp = Exp::MakeVar(NULL, VK_Arg, NULL, aind, NULL);

      GuardExpVector argval_res;
      TranslateExp(TRK_Point, point, argval, &argval_res);

      for (size_t sind = 0; sind < argval_res.Size(); sind++) {
        const GuardExp &ags = argval_res[sind];
        ags.IncRef();
        arg_exp->IncRef();
        ComputeSingleAssign(arg_type, arg_exp, ags.exp, ags.guard, arguments);
      }

      arg_type->DecRef();
      arg_exp->DecRef();
    }
  }

  // add the call return value.

  Vector<GuardExp> *returns = NULL;

  Type *return_type = type->GetReturnType();
  Exp *retval = edge->GetReturnValue();

  if (retval != NULL) {
    returns = m_return_table->Lookup(point, true);
    Assert(returns->Empty());

    if (return_type->Kind() == YK_Void)
      logout << "ERROR: Assigning call result of type void: " << edge << endl;

    GuardExpVector retval_res;
    TranslateExp(TRK_Point, point, retval, &retval_res);
    retval_res.FillVector(returns, true);
  }

  // generate clobbering info and any additional assignments.

  Vector<GuardAssign> *assigns = m_assign_table->Lookup(point, true);
  Assert(assigns->Empty());

  Vector<GuardAssign> *clobbered = m_clobber_table->Lookup(point, true);
  Assert(clobbered->Empty());

  m_clobber->ComputeClobber(this, edge, assigns, clobbered);

  // add a return value assignment if there isn't already an explicit one.

  if (returns != NULL) {
    for (size_t rind = 0; rind < returns->Size(); rind++) {
      const GuardExp &rgt = returns->At(rind);

      // check to see if there is already an assignment to the return lvalue.
      // TODO: this is pretty cheesy.
      bool has_assign = false;

      for (size_t aind = 0; aind < assigns->Size(); aind++) {
        if (rgt.exp == assigns->At(aind).left) {
          has_assign = true;
          break;
        }
      }

      if (has_assign)
        continue;

      // generate assignments for special __ubound function.

      Variable *function = edge->GetDirectFunction();

      if (function && TextNameMatch(function, UBOUND_FUNCTION) &&
          edge->GetArgumentCount() >= 1) {
        Exp *arg = edge->GetArgument(0);
        if (Type *type = arg->GetType()) {
          GuardExpVector arg_res;
          TranslateExp(TRK_Point, point, arg, &arg_res);

          for (size_t ind = 0; ind < arg_res.Size(); ind++) {
            const GuardExp &ag = arg_res[ind];
            ag.IncRef();
            rgt.IncRef();

            type->IncRef();
            Exp *rval = Exp::MakeBound(BND_Upper, ag.exp, type);
            Bit *guard = Bit::MakeAnd(rgt.guard, ag.guard);

            ComputeSingleAssign(NULL, rgt.exp, rval, guard, assigns);
          }

          continue;
        }
      }

      // otherwise assign the default clobbered value.
      Exp *ret_exp = Exp::MakeVar(NULL, VK_Return, NULL, 0, NULL);

      Location *location = m_cfg->GetPointLocation(point);
      location->IncRef();

      rgt.exp->IncRef();
      Exp *rval = Exp::MakeClobber(ret_exp, NULL, rgt.exp, point, location);

      rgt.IncRef();
      ComputeSingleAssign(return_type, rgt.exp, rval, rgt.guard, assigns);
    }
  }
}

void BlockMemory::ComputeEdgeLoop(PEdgeLoop *edge)
{
  PPoint point = edge->GetSource();

  // generate clobbering info. we can't assign across a loop since it may
  // run for only zero iterations.

  Vector<GuardAssign> *clobbered = m_clobber_table->Lookup(point, true);
  Assert(clobbered->Empty());

  m_clobber->ComputeClobber(this, edge, NULL, clobbered);
}

void BlockMemory::ComputeSingleAssign(Type *type,
                                      Exp *left, Exp *right, Bit *guard,
                                      Vector<GuardAssign> *assigns)
{
  if (type && type->IsCSU()) {
    if (Exp *target = ExpDereference(right)) {
      String *csu_name = type->AsCSU()->GetCSUName();
      CompositeCSU *csu = CompositeCSUCache.Lookup(csu_name);

      if (csu != NULL) {
        for (size_t find = 0; find < csu->GetFieldCount(); find++) {
          const DataField &df = csu->GetField(find);

          left->IncRef();
          df.field->IncRef();
          Exp *left_fld = Exp::MakeFld(left, df.field);

          target->IncRef();
          df.field->IncRef();
          Exp *target_fld = Exp::MakeFld(target, df.field);
          Exp *right_fld = Exp::MakeDrf(target_fld);

          guard->IncRef();
          ComputeSingleAssign(df.field->GetType(),
                              left_fld, right_fld, guard, assigns);
        }
      }

      CompositeCSUCache.Release(csu_name);
    }

    left->DecRef();
    right->DecRef();
    guard->DecRef();
  }
  else {
    assigns->PushBack(GuardAssign(left, right, guard));
    left->MoveRef(NULL, assigns);
    right->MoveRef(NULL, assigns);
    TryMoveRef(guard, assigns);
  }
}

void BlockMemory::TransferEntryDrf(Exp *lval, GuardExpVector *res)
{
  lval->IncRef();
  Exp *value = Exp::MakeDrf(lval);
  Bit *guard = Bit::MakeConstant(true);
  res->PushBack(GuardExp(value, guard));
}

void BlockMemory::TransferEdgeDrf(Exp *lval, PEdge *edge, GuardExpVector *res)
{
  PPoint source = edge->GetSource();
  Vector<GuardAssign> *assigns = m_assign_table->Lookup(source, false);

  if (!assigns)
    return;

  for (size_t aind = 0; aind < assigns->Size(); aind++) {
    const GuardAssign &gasn = assigns->At(aind);
    Assert(gasn.kind == NULL);

    Bit *alias = IsLvalAliased(gasn.left, lval, NULL);
    if (!alias)
      continue;

    // found a direct or indirect aliased assignment to the lvalue.

    gasn.guard->IncRef();
    Bit *when = Bit::MakeAnd(gasn.guard, alias);

    gasn.right->IncRef();
    res->PushBack(GuardExp(gasn.right, when));
  }
}

void BlockMemory::TransferEntryTerminate(Exp *lval, ExpTerminate *kind,
                                         GuardExpVector *res)
{
  lval->IncRef();
  Exp *value = kind->ReplaceLvalTarget(lval);
  Bit *guard = Bit::MakeConstant(true);
  res->PushBack(GuardExp(value, guard));
}

void BlockMemory::TransferEdgeTerminate(Exp *lval, ExpTerminate *kind,
                                        PEdge *edge, GuardExpVector *res)
{
  // TODO: for now we are only looking for points where the terminator is
  // established, not points where it might be overwritten and invalidated.

  // we are assuming constant strings are immutable so don't have to
  // compute their intermediate values. just get the value at entry,
  // which will be a constant.
  if (lval->IsString()) {
    TransferEntryTerminate(lval, kind, res);
    return;
  }

  Type *stride_type = kind->GetStrideType();

  // check for direct writes of the terminator to the buffer. if there is a
  // terminating write 'buf[i] = 0', the new position of the terminator will
  // be max(old_position, i).

  PPoint point = edge->GetSource();
  Vector<GuardAssign> *assigns = m_assign_table->Lookup(point, false);

  size_t assign_count = assigns ? assigns->Size() : 0;

  for (size_t aind = 0; aind < assign_count; aind++) {
    const GuardAssign &gasn = assigns->At(aind);
    Assert(gasn.kind == NULL);

    Exp *use_left = NULL;
    ExpTerminate *new_kind =
      GetTerminateAssign(point, gasn.left, gasn.right, &use_left);

    if (!new_kind)
      continue;

    bool equal_kind = (new_kind == kind);

    if (!equal_kind) {
      // don't need exact equality for NULL terminators. just check the
      // written stride type is >= the expected stride type.
      if (new_kind->IsNullTerminate() && kind->IsNullTerminate()) {
        if (new_kind->GetStrideType()->Width() >=
            kind->GetStrideType()->Width())
          equal_kind = true;
      }
    }

    if (!equal_kind) {
      new_kind->DecRef();
      continue;
    }

    new_kind->DecRef();

    // check for aliasing between the updated lvalue and the interested buffer.
    // this aliasing will test whether the two locations are within the
    // same buffer.

    Bit *alias = IsLvalAliased(use_left, lval, kind);
    if (!alias)
      continue;

    // found an update which could modify the buffer's terminator. we will
    // treat the terminator as unchanged if it is greater than the offset
    // of this updated position, so use a Max binop.

    gasn.guard->IncRef();
    Bit *when = Bit::MakeAnd(gasn.guard, alias);
    when->MoveRef(NULL, &when);

    use_left->IncRef();
    lval->IncRef();
    stride_type->IncRef();
    Exp *value = Exp::MakeBinop(B_MinusPP, use_left, lval, stride_type);
    value->MoveRef(NULL, &value);

    GuardExpVector terminators;
    GetValSimplify(lval, kind, point, &terminators);

    for (size_t ind = 0; ind < terminators.Size(); ind++) {
      const GuardExp &term = terminators[ind];
      term.IncRef();

      value->IncRef();
      when->IncRef();
      Exp *maximum = Exp::MakeBinop(B_Max, value, term.exp);
      Bit *combine = Bit::MakeAnd(when, term.guard);
      res->PushBack(GuardExp(maximum, combine));
    }

    when->DecRef(&when);
    value->DecRef(&value);
  }

  Exp *target;
  Exp *source;
  Exp *length;

  PEdgeCall *nedge = edge->IfCall();
  if (!nedge)
    return;

  // check for memory copies which can introduce a terminator.

  if (GetCallMemoryCopy(nedge, &target, &source, &length)) {

    GuardExpVector caller_target;
    TranslateExp(TRK_Callee, point, target, &caller_target);

    GuardExpVector caller_source;
    TranslateExp(TRK_Callee, point, source, &caller_source);

    GuardExpVector caller_length;
    TranslateExp(TRK_Callee, point, length, &caller_length);

    GuardExpVector terminators;
    GetValSimplify(lval, kind, point, &terminators);

    for (size_t tind = 0; tind < caller_target.Size(); tind++) {
      const GuardExp &ctarg = caller_target[tind];

      Bit *alias = IsLvalAliased(ctarg.exp, lval, kind);
      if (!alias)
        continue;

      for (size_t sind = 0; sind < caller_source.Size(); sind++) {
        const GuardExp &csrc = caller_source[sind];

        const Vector<GuardExp>& source_term = GetVal(csrc.exp, kind, point);

        for (size_t lind = 0; lind < caller_length.Size(); lind++) {
          const GuardExp &clen = caller_length[lind];

          for (size_t ltind = 0; ltind < terminators.Size(); ltind++) {
            const GuardExp &lvterm = terminators[ltind];

            for (size_t stind = 0; stind < source_term.Size(); stind++) {
              const GuardExp &srcterm = source_term[stind];

              // term(lval) equals term(target) + (target -p lval)
              // term(target) is assigned term(source) if:
              // term(target) < (length / sizeof(stride_type))

              // thus:

              // term(lval) is assigned term(source) + (target -p lval) if:
              // term(target) < (length / sizeof(stride_type))

              ctarg.exp->IncRef();
              lval->IncRef();
              stride_type->IncRef();
              Exp *diff = Exp::MakeBinop(B_MinusPP, ctarg.exp, lval,
                                         stride_type);

              srcterm.exp->IncRef();
              lvterm.exp->IncRef();
              Exp *value = Exp::MakeBinop(B_Plus, srcterm.exp, diff);
              Exp *maximum = Exp::MakeBinop(B_Max, value, lvterm.exp);

              ctarg.exp->IncRef();
              Exp *target_term = kind->ReplaceLvalTarget(ctarg.exp);

              clen.exp->IncRef();
              Exp *size_exp = Exp::MakeInt(stride_type->Width());
              Exp *len_div = Exp::MakeBinop(B_Div, clen.exp, size_exp);

              Bit *less = Exp::MakeCompareBit(B_LessThan,
                                              target_term, len_div);

              alias->IncRef();
              ctarg.guard->IncRef();
              clen.guard->IncRef();
              lvterm.guard->IncRef();
              srcterm.guard->IncRef();
              Bit *combine = Bit::MakeAnd(alias, less);
              combine = Bit::MakeAnd(combine, ctarg.guard);
              combine = Bit::MakeAnd(combine, clen.guard);
              combine = Bit::MakeAnd(combine, lvterm.guard);
              combine = Bit::MakeAnd(combine, srcterm.guard);

              res->PushBack(GuardExp(maximum, combine));
            }
          }
        }
      }

      alias->DecRef();
    }

    target->DecRef();
    source->DecRef();
    length->DecRef();
  }

  // check for memory initialization which can introduce a terminator.

  long term_int_value;
  if (kind->GetTerminateInt()->GetInt(&term_int_value) &&
      term_int_value == 0 &&
      GetCallMemoryZero(nedge, &target, &length)) {

    GuardExpVector caller_target;
    TranslateExp(TRK_Callee, point, target, &caller_target);

    GuardExpVector caller_length;
    TranslateExp(TRK_Callee, point, length, &caller_length);

    GuardExpVector terminators;
    GetValSimplify(lval, kind, point, &terminators);

    for (size_t tind = 0; tind < caller_target.Size(); tind++) {
      const GuardExp &ctarg = caller_target[tind];

      Bit *alias = IsLvalAliased(ctarg.exp, lval, kind);
      if (!alias)
        continue;

      for (size_t lind = 0; lind < caller_length.Size(); lind++) {
        const GuardExp &clen = caller_length[lind];

        for (size_t ind = 0; ind < terminators.Size(); ind++) {
          const GuardExp &term = terminators[ind];

          // term(lval) equals term(target) + (target -p lval)
          // term(target) is assigned (length / sizeof(stride_type)) - 1

          // thus:

          // term(lval) is assigned:
          // (length / sizeof(stride_type)) - 1 + (target -p lval)

          ctarg.exp->IncRef();
          lval->IncRef();
          stride_type->IncRef();
          Exp *diff = Exp::MakeBinop(B_MinusPP, ctarg.exp, lval, stride_type);

          clen.exp->IncRef();
          Exp *size_exp = Exp::MakeInt(stride_type->Width());
          Exp *len_div = Exp::MakeBinop(B_Div, clen.exp, size_exp);

          Exp *one_exp = Exp::MakeInt(1);
          Exp *len_sub = Exp::MakeBinop(B_Minus, len_div, one_exp);

          term.exp->IncRef();
          Exp *value = Exp::MakeBinop(B_Plus, len_sub, diff);
          Exp *maximum = Exp::MakeBinop(B_Max, value, term.exp);

          alias->IncRef();
          ctarg.guard->IncRef();
          clen.guard->IncRef();
          term.guard->IncRef();
          Bit *combine = Bit::MakeAnd(alias, ctarg.guard);
          combine = Bit::MakeAnd(combine, clen.guard);
          combine = Bit::MakeAnd(combine, term.guard);

          res->PushBack(GuardExp(maximum, combine));
        }
      }

      alias->DecRef();
    }

    target->DecRef();
    length->DecRef();
  }
}

void BlockMemory::TransferClobberTerminate(Exp *lval, ExpTerminate *kind,
                                           ExpClobber *clobber,
                                           GuardExpVector *res)
{
  // when the terminator of the buffer at lval is overwritten via clobber,
  // the actual result of the terminator depends on the difference between
  // the relative buffer offsets of lval and the base buffer within the call.

  Exp *callee = clobber->GetCallee();
  PPoint point = clobber->GetPoint();
  Type *stride_type = kind->GetStrideType();

  GuardExpVector entry_res;
  TranslateExp(TRK_Callee, point, callee, &entry_res);

  for (size_t ind = 0; ind < entry_res.Size(); ind++) {
    const GuardExp &gs = entry_res[ind];
    gs.IncRef();

    lval->IncRef();
    clobber->IncRef();
    stride_type->IncRef();
    Exp *diff = Exp::MakeBinop(B_MinusPP, gs.exp, lval, stride_type);
    Exp *term = Exp::MakeBinop(B_Plus, diff, clobber);

    res->PushBack(GuardExp(term, gs.guard));
  }
}

void BlockMemory::TransferEntry(Exp *lval, Exp *kind, GuardExpVector *res)
{
  if (!kind)
    TransferEntryDrf(lval, res);
  else if (ExpTerminate *nkind = kind->IfTerminate())
    TransferEntryTerminate(lval, nkind, res);
  else
    Assert(false);
}

void BlockMemory::TransferEdge(Exp *lval, Exp *kind, PEdge *edge,
                               GuardExpVector *res)
{
  PPoint point = edge->GetSource();

  if (!kind)
    TransferEdgeDrf(lval, edge, res);
  else if (ExpTerminate *nkind = kind->IfTerminate())
    TransferEdgeTerminate(lval, nkind, edge, res);
  else
    Assert(false);

  // compute the guard under which the lvalue is not directly updated.
  Bit *preserve = Bit::MakeConstant(true);

  for (size_t ind = 0; ind < res->Size(); ind++) {
    Bit *bit = res->At(ind).guard;
    bit->IncRef();

    Bit *not_bit = Bit::MakeNot(bit);
    preserve = Bit::MakeAnd(preserve, not_bit);
  }

  if (preserve->IsFalse()) {
    preserve->DecRef();
    res->SortCombine();
    return;
  }

  // compute the value after the edge if the lvalue is clobbered.
  Exp *clobber_inner = NULL;
  Bit *clobber_guard = NULL;
  if (IsLvalClobbered(lval, kind, edge, &clobber_inner, &clobber_guard)) {
    Assert(clobber_inner);
    Assert(clobber_guard);

    // get the condition where the clobber occurs. and restrict
    // the preserved guard to cover only the non-clobber cases.
    clobber_guard->IncRef();
    Bit *not_clobber = Bit::MakeNot(clobber_guard);

    preserve->IncRef();
    clobber_guard = Bit::MakeAnd(preserve, clobber_guard);
    preserve = Bit::MakeAnd(preserve, not_clobber);

    Location *location = m_cfg->GetPointLocation(point);
    location->IncRef();

    lval->IncRef();
    clobber_inner->IncRef();
    if (kind)
      kind->IncRef();
    ExpClobber *clobber = Exp::MakeClobber(clobber_inner, kind, lval,
                                           point, location);

    if (kind && kind->IsTerminate()) {
      GuardExpVector clobber_res;
      TransferClobberTerminate(lval, kind->AsTerminate(),
                               clobber, &clobber_res);

      for (size_t ind = 0; ind < clobber_res.Size(); ind++) {
        const GuardExp &gs = clobber_res[ind];
        gs.IncRef();
        clobber_guard->IncRef();
        Bit *combine_guard = Bit::MakeAnd(clobber_guard, gs.guard);
        res->PushBack(GuardExp(gs.exp, combine_guard));
      }

      clobber->DecRef();
      clobber_guard->DecRef();
    }
    else {
      res->PushBack(GuardExp(clobber, clobber_guard));
    }
  }

  if (preserve->IsFalse()) {
    preserve->DecRef();
    res->SortCombine();
    return;
  }

  // check if the lvalue is derived from a value clobbered at this point.
  // modsets are not computed for values themselves computed by a call/loop,
  // so there is an implicit modset for everything derived from a clobbered
  // value.

  if (ExpClobber *clobber = lval->ClobberRoot()) {
    Assert(clobber->GetValueKind() == NULL);

    if (clobber->GetPoint() == point) {
      // need to compute the callee representation at exit. compose the
      // callee's representation in the clobber with the offset of this lvalue
      // from the clobber.
      Exp *callee = clobber->GetCallee();
      Exp *remainder = Exp::GetSubExprRemainder(lval, clobber);

      callee->IncRef();
      Exp *callee_exit = Exp::MakeExit(callee, NULL);
      Exp *callee_lval = Exp::Compose(callee_exit, remainder);
      callee_exit->DecRef();
      remainder->DecRef();

      Location *location = m_cfg->GetPointLocation(point);
      location->IncRef();

      if (kind)
        kind->IncRef();
      lval->IncRef();
      Exp *exit_value = Exp::MakeClobber(callee_lval, kind, lval,
                                         point, location);

      res->PushBack(GuardExp(exit_value, preserve));
      res->SortCombine();
      return;
    }
  }

  // combine the preserve guard with the old values of the lvalue.
  GuardExpVector old_values;
  GetValSimplify(lval, kind, point, &old_values);

  for (size_t vind = 0; vind < old_values.Size(); vind++) {
    const GuardExp &ogs = old_values[vind];

    preserve->IncRef();
    ogs.guard->IncRef();
    Bit *guard = Bit::MakeAnd(preserve, ogs.guard);

    ogs.exp->IncRef();
    res->PushBack(GuardExp(ogs.exp, guard));
  }

  preserve->DecRef();
  res->SortCombine();
}

NAMESPACE_XGILL_END
