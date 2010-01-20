
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

#pragma once

// data structures for storing the various callers and callees of a function.

#include <imlang/block.h>

NAMESPACE_XGILL_BEGIN

extern ConfigOption print_indirect_calls;

// a particular call edge. specifies the point of the call site
// and a callee of that site (for indirect calls, the same call site
// can have many callees).
struct CallEdge
{
  // call site of this edge.
  BlockPPoint where;

  // function being called.
  Variable *callee;

  CallEdge() : callee(NULL) {}
  CallEdge(BlockPPoint _where, Variable *_callee)
    : where(_where), callee(_callee) {}
};

// set of callers or callees of a function and its loops.
class CallEdgeSet : public HashObject
{
 public:
  static int Compare(const CallEdgeSet *cset0, const CallEdgeSet *cset1);
  static CallEdgeSet* Copy(const CallEdgeSet *cset);
  static void Write(Buffer *buf, const CallEdgeSet *cset);
  static CallEdgeSet* Read(Buffer *buf);

  static void WriteMerge(Buffer *buf, Variable *function, bool callers,
                         const Vector<CallEdge> &edges);
  static void ReadMerge(Buffer *buf, Variable **pfunction, bool *pcallers,
                        Vector<CallEdge> *pedges);

  static CallEdgeSet* Make(Variable *function, bool callers, bool merge) {
    CallEdgeSet xcset(function, callers, merge);
    return g_table.Lookup(xcset);
  }

 public:
  // get the function for which this is storing the callers or callees.
  Variable* GetFunction() const { return m_function; }

  // whether this set represents the callers or the callees of the function.
  bool IsCallers() const { return m_callers; }

  // get the call edges in this set.
  size_t GetEdgeCount() const;
  const CallEdge& GetEdge(size_t ind) const;

  // add a call edge to this set. if this stores the callers of the function
  // then callee is equal to the function, and if this stores the callees
  // then where is within the outer body or an inner loop of the function.
  // consumes references on edge.where.id and edge.callee.
  void AddEdge(const CallEdge &edge);

  // inherited methods
  void Print(OutStream &out) const;
  void DecMoveChildRefs(ORef ov, ORef nv);
  void Persist();
  void UnPersist();

 private:
  Variable *m_function;
  bool m_callers;

  // all known call edges for this function and direction.
  Vector<CallEdge> *m_edges;

  // whether this set is being used to merge data into.
  bool m_merge;

  CallEdgeSet(Variable *function, bool callers, bool merge);
  static HashCons<CallEdgeSet> g_table;
};

// callgraph edge generation

// these functions modify the edges stored in the append caller/callee caches
// from storage.h; flushing these caches will write out the modified data.

// add to the append callgraph caches any direct call edges in cfg.
// also adds those direct call edges to the callees list.
void CallgraphProcessCFG(BlockCFG *cfg, Vector<Variable*> *callees);

// add to the append callgraph caches the call edges for any indirect
// calls in CFG. uses the escape analysis to find the possible targets.
void CallgraphProcessCFGIndirect(BlockCFG *cfg);

NAMESPACE_XGILL_END
