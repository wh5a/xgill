
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

#include <unistd.h>

#include "filename.h"
#include "interface.h"
#include "loopsplit.h"
#include "storage.h"

#include <backend/backend_block.h>
#include <util/monitor.h>

// we are pulling in escape/callgraph information from memory/.
// this is kind of a forward reference and should be cleaned up.
#include <memory/storage.h>
#include <memory/serial.h>

NAMESPACE_XGILL_USING

#define GET_OBJECT(TYPE, NAME)                  \
  TYPE * new_ ##NAME = (TYPE*) NAME;            \
  if (new_ ##NAME) new_ ##NAME ->IncRef();

/////////////////////////////////////////////////////////////////////
// Utility
/////////////////////////////////////////////////////////////////////

// file which log_stream is wrapping.
static FILE* log_file = NULL;

extern "C" void XIL_SetLogFile(FILE *file)
{
  // the log stream may already have been set if we are using a manager.
  // do the assignment regardless.
  log_file = file;
  log_stream = new PrintOutStream(file);
}

FILE* XIL_GetLogFile()
{
  return log_file ? log_file : stdout;
}

extern "C" void XIL_Print(void *object)
{
  HashObject *new_object = (HashObject*) object;
  logout << flush;
  new_object->Print(logout);
  logout << endl << flush;
}

extern "C" void XIL_SetNormalizeDirectory(const char *path)
{
  const char *cwd = getcwd(NULL, 0);

  SetWorkingDirectory(cwd);
  SetBaseDirectory(path ? path : cwd);
}

// hashtable mapping filenames relative to the cwd to filenames relative
// to the normalize directory.
static HashTable<String*,String*,String> g_filename_map;

extern "C" XIL_Location XIL_MakeLocation(const char *file, int line)
{
  String *base_file = String::Make(file);

  Vector<String*> *normal_files = g_filename_map.Lookup(base_file, true);
  String *normal_file = NULL;

  if (normal_files->Empty()) {
    // first time we saw this file.
    normal_file = String::Make(NormalizeFile(file));
    normal_files->PushBack(normal_file);
  }
  else {
    normal_file = normal_files->At(0);
  }

  normal_file->IncRef();
  return (XIL_Location) Location::Make(normal_file, (uint32_t) line);
}

// CSUs and CFGs that have been constructed through this interface.
static Vector<CompositeCSU*> g_keep_csus;
static Vector<BlockCFG*> g_keep_cfgs;

// CSUs and CFGs that were constructed but will be dropped from the output
// due to errors during generation.
static Vector<CompositeCSU*> g_drop_csus;
static Vector<BlockCFG*> g_drop_cfgs;

// currently active CSU or CFG, if there is one.
static Vector<CompositeCSU*> g_active_csus;
static BlockCFG *g_active_cfg = NULL;

// ID to use for locals in g_active_cfg.
static BlockId *g_active_id = NULL;

// ID to use for locals in the function g_active_cfg is an annotation for.
static BlockId *g_annotation_id = NULL;

// whether we have seen any annotation CFGs.
static bool g_has_annotation = false;

struct AssociateKey {
  String *kind;
  void *value;

  AssociateKey() : kind(NULL), value(NULL) {}
  AssociateKey(String *_kind, void *_value) : kind(_kind), value(_value) {}

  static uint32_t Hash(uint32_t hash, const AssociateKey &key) {
    hash = Hash32(hash, key.kind->Hash());
    return Hash32(hash, (uint32_t) (size_t) key.value);
  }

  bool operator == (const AssociateKey &okey) const {
    return kind == okey.kind && value == okey.value;
  }
};

typedef HashTable<AssociateKey,void*,AssociateKey> AssociateTable;

// association hashtables.
static AssociateTable g_associate_annot;
static AssociateTable g_associate_block;
static AssociateTable g_associate_global;

static AssociateTable& GetAssociateTable(XIL_AssociateKind kind)
{
  switch (kind) {
  case XIL_AscAnnotate: return g_associate_annot;
  case XIL_AscBlock:    return g_associate_block;
  case XIL_AscGlobal:   return g_associate_global;
  default: Assert(false);
  }
}

extern "C" void** XIL_Associate(XIL_AssociateKind table,
                                const char *kind, void *value)
{
  AssociateKey key(String::Make(kind), value);
  Vector<void*> *values = GetAssociateTable(table).Lookup(key, true);

  if (values->Empty())
    values->PushBack(NULL);
  Assert(values->Size() == 1);

  return &values->At(0);
}

extern "C" void XIL_ClearAssociate(XIL_AssociateKind table)
{
  GetAssociateTable(table).Clear();
}

/////////////////////////////////////////////////////////////////////
// Active block
/////////////////////////////////////////////////////////////////////

extern "C" void XIL_SetActiveBlock(XIL_Var var, const char *annot_name,
                                   XIL_AnnotationKind annot_kind,
                                   int annot_type)
{
  Assert(!g_active_cfg);

  GET_OBJECT(Variable, var);
  new_var->IncRef();

  // this is different from the active ID for functions, since we are making
  // the initial whole CFG and not the final loop-free CFG.
  BlockId *cfg_id = NULL;

  if (annot_name) {
    g_has_annotation = true;
    String *new_name = String::Make(annot_name);

    if (annot_type) {
      Assert(new_var->Kind() == VK_Glob);
      cfg_id = BlockId::Make(B_AnnotationComp, new_var, new_name);
    }
    else if (new_var->Kind() == VK_Func) {
      cfg_id = BlockId::Make(B_AnnotationFunc, new_var, new_name);
      g_annotation_id = BlockId::Make(B_Function, new_var);
    }
    else if (new_var->Kind() == VK_Glob) {
      cfg_id = BlockId::Make(B_AnnotationInit, new_var, new_name);
    }

    cfg_id->IncRef();
    g_active_id = cfg_id;
  }
  else if (new_var->Kind() == VK_Func) {
    cfg_id = BlockId::Make(B_FunctionWhole, new_var);
    g_active_id = BlockId::Make(B_Function, new_var);
  }
  else if (new_var->Kind() == VK_Glob) {
    cfg_id = BlockId::Make(B_Initializer, new_var);
    g_active_id = BlockId::Make(B_Initializer, new_var);
  }
  else {
    Assert(false);
  }

  g_active_cfg = BlockCFG::Make(cfg_id);

  if (annot_kind)
    g_active_cfg->SetAnnotationKind((AnnotationKind) annot_kind);

  if (!g_keep_cfgs.Contains(g_active_cfg))
    g_keep_cfgs.PushBack(g_active_cfg);
}

extern "C" void XIL_ClearActiveBlock(int drop)
{
  Assert(g_active_cfg);

  if (drop) {
    if (!g_drop_cfgs.Contains(g_active_cfg))
      g_drop_cfgs.PushBack(g_active_cfg);
  }

  g_active_cfg = NULL;
  g_active_id = NULL;
  g_annotation_id = NULL;
}

/////////////////////////////////////////////////////////////////////
// Types
/////////////////////////////////////////////////////////////////////

extern "C" XIL_Type XIL_TypeError()
{
  return (XIL_Type) Type::MakeError();
}

extern "C" XIL_Type XIL_TypeVoid()
{
  return (XIL_Type) Type::MakeVoid();
}

extern "C" XIL_Type XIL_TypeInt(int width, int sign)
{
  return (XIL_Type) Type::MakeInt((size_t) width, (size_t) sign);
}

extern "C" XIL_Type XIL_TypeFloat(int width)
{
  return (XIL_Type) Type::MakeFloat((size_t) width);
}

extern "C" XIL_Type XIL_TypePointer(XIL_Type target_type, int width)
{
  GET_OBJECT(Type, target_type);
  return (XIL_Type) Type::MakePointer(new_target_type, (size_t) width);
}

extern "C" XIL_Type XIL_TypeArray(XIL_Type element_type, int count)
{
  GET_OBJECT(Type, element_type);
  return (XIL_Type) Type::MakeArray(new_element_type, (size_t) count);
}

// set of CSU names we have asked to generate.
static HashSet<String*,HashObject> g_generated_csus;

extern "C" XIL_Type XIL_TypeCSU(const char *csu_name, int *generate)
{
  String *new_csu_name = String::Make(csu_name);

  if (generate) {
    bool exists = g_generated_csus.Insert(new_csu_name);
    *generate = exists ? 0 : 1;
  }

  return (XIL_Type) Type::MakeCSU(new_csu_name);
}

extern "C"
XIL_Type XIL_TypeFunction(XIL_Type return_type, const char *this_csu, int varargs,
                          XIL_Type *arg_types, int arg_count)
{
  GET_OBJECT(Type, return_type);

  String *new_this_csu = this_csu ?  String::Make(this_csu) : NULL;
  TypeCSU *csu_type = new_this_csu ? Type::MakeCSU(new_this_csu) : NULL;

  Vector<Type*> new_arg_types;
  for (int ind = 0; ind < arg_count; ind++) {
    Type *arg_type = (Type*) arg_types[ind];
    arg_type->IncRef();
    new_arg_types.PushBack(arg_type);
  }

  return (XIL_Type)
    Type::MakeFunction(new_return_type, csu_type,
                       (bool) varargs, new_arg_types);
}

const char* XIL_GetTypeCSUName(XIL_Type csu_type)
{
  GET_OBJECT(Type, csu_type);

  if (new_csu_type->IsCSU())
    return new_csu_type->AsCSU()->GetCSUName()->Value();
  return NULL;
}

extern "C"
XIL_Field XIL_MakeField(const char *name, const char *source_name,
                        const char *csu_name, XIL_Type type, int is_func)
{
  String *new_name = String::Make(name);
  String *new_source_name = source_name ? String::Make(source_name) : NULL;
  String *new_csu_name = String::Make(csu_name);
  TypeCSU *csu_type = Type::MakeCSU(new_csu_name);

  GET_OBJECT(Type, type);

  return (XIL_Field)
    Field::Make(new_name, new_source_name, csu_type,
                new_type, (bool) is_func);
}

extern "C" void XIL_PushActiveCSU(const char *name)
{
  String *new_name = String::Make(name);
  CompositeCSU *csu = CompositeCSU::Make(new_name);

  g_active_csus.PushBack(csu);

  if (!g_keep_csus.Contains(csu))
    g_keep_csus.PushBack(csu);
}

extern "C" void XIL_PopActiveCSU(int drop)
{
  CompositeCSU *csu = g_active_csus.Back();
  g_active_csus.PopBack();

  if (drop) {
    if (!g_drop_csus.Contains(csu))
      g_drop_csus.PushBack(csu);
  }
}

extern "C" void XIL_CSUSetKind(int kind)
{
  CompositeCSU *csu = g_active_csus.Back();
  csu->SetKind((CSUKind)kind);
}

extern "C" void XIL_CSUSetWidth(int width)
{
  CompositeCSU *csu = g_active_csus.Back();
  csu->SetWidth((size_t)width);
}

extern "C" void XIL_CSUSetCommand(const char *command)
{
  CompositeCSU *csu = g_active_csus.Back();

  String *new_command = String::Make(command);
  csu->SetCommand(new_command);
}

extern "C" void XIL_CSUSetBeginLocation(XIL_Location begin_loc)
{
  CompositeCSU *csu = g_active_csus.Back();

  GET_OBJECT(Location, begin_loc);
  csu->SetBeginLocation(new_begin_loc);
}

extern "C" void XIL_CSUSetEndLocation(XIL_Location end_loc)
{
  CompositeCSU *csu = g_active_csus.Back();

  GET_OBJECT(Location, end_loc);
  csu->SetEndLocation(new_end_loc);
}

extern "C" void XIL_CSUAddBaseClass(const char *base_class)
{
  CompositeCSU *csu = g_active_csus.Back();

  String *new_base_class = String::Make(base_class);
  csu->AddBaseClass(new_base_class);
}

extern "C" void XIL_CSUAddDataField(XIL_Field field, int offset)
{
  CompositeCSU *csu = g_active_csus.Back();

  GET_OBJECT(Field, field);
  csu->AddField(new_field, (size_t) offset);
}

extern "C" void XIL_CSUAddFunctionField(XIL_Field field, XIL_Var function)
{
  CompositeCSU *csu = g_active_csus.Back();

  GET_OBJECT(Field, field);
  GET_OBJECT(Variable, function);
  csu->AddFunctionField(new_field, new_function);
}

/////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////

extern "C" XIL_Var XIL_VarGlob(const char *name, const char *source_name)
{
  String *new_name = String::Make(name);
  String *new_source_name = source_name ? String::Make(source_name) : NULL;
  return (XIL_Var)
    Variable::Make(NULL, VK_Glob, new_name, 0, new_source_name);
}

extern "C" XIL_Var XIL_VarFunc(const char *name, const char *source_name)
{
  String *new_name = String::Make(name);
  String *new_source_name = source_name ? String::Make(source_name) : NULL;
  return (XIL_Var)
    Variable::Make(NULL, VK_Func, new_name, 0, new_source_name);
}

extern "C" XIL_Var XIL_VarArg(int index, const char *name, int annot)
{
  BlockId *id = annot ? g_annotation_id : g_active_id;
  Assert(id);
  id->IncRef();

  String *new_name = name ? String::Make(name) : NULL;
  if (new_name) new_name->IncRef();

  return (XIL_Var)
    Variable::Make(id, VK_Arg, new_name, (size_t) index, new_name);
}

extern "C" XIL_Var XIL_VarLocal(const char *name,
                                const char *source_name, int annot)
{
  BlockId *id = annot ? g_annotation_id : g_active_id;
  Assert(id);
  id->IncRef();

  String *new_name = String::Make(name);
  String *new_source_name = source_name ? String::Make(source_name) : NULL;

  return (XIL_Var)
    Variable::Make(id, VK_Local, new_name, 0, new_source_name);
}

extern "C" XIL_Var XIL_VarReturn(int annot)
{
  BlockId *id = annot ? g_annotation_id : g_active_id;
  Assert(id);
  id->IncRef();

  return (XIL_Var) Variable::Make(id, VK_Return, NULL, 0, NULL);
}

extern "C" XIL_Var XIL_VarThis(int annot)
{
  BlockId *id = annot ? g_annotation_id : g_active_id;
  Assert(id);
  id->IncRef();

  return (XIL_Var) Variable::Make(id, VK_This, NULL, 0, NULL);
}

extern "C" XIL_Var XIL_VarTemp(const char *name)
{
  Assert(g_active_id);
  g_active_id->IncRef();

  String *new_name = String::Make(name);
  if (new_name) new_name->IncRef();

  return (XIL_Var)
    Variable::Make(g_active_id, VK_Temp, new_name, 0, new_name);
}

extern "C" const char* XIL_GetVarName(XIL_Var var)
{
  GET_OBJECT(Variable, var);
  if (new_var->GetName())
    return new_var->GetName()->Value();
  return NULL;
}

/////////////////////////////////////////////////////////////////////
// Expressions
/////////////////////////////////////////////////////////////////////

extern "C" XIL_Exp XIL_ExpVar(XIL_Var var)
{
  GET_OBJECT(Variable, var);
  return (XIL_Exp) Exp::MakeVar(new_var);
}

extern "C" XIL_Exp XIL_ExpDrf(XIL_Exp target)
{
  GET_OBJECT(Exp, target);
  return (XIL_Exp) Exp::MakeDrf(new_target);
}

extern "C" XIL_Exp XIL_ExpFld(XIL_Exp target, XIL_Field field)
{
  GET_OBJECT(Exp, target);
  GET_OBJECT(Field, field);
  return (XIL_Exp) Exp::MakeFld(new_target, new_field);
}


extern "C" XIL_Exp XIL_ExpRfld(XIL_Exp target, XIL_Field field)
{
  GET_OBJECT(Exp, target);
  GET_OBJECT(Field, field);
  return (XIL_Exp) Exp::MakeRfld(new_target, new_field);
}

extern "C"
XIL_Exp XIL_ExpIndex(XIL_Exp target, XIL_Type element_type, XIL_Exp index)
{
  GET_OBJECT(Exp, target);
  GET_OBJECT(Type, element_type);
  GET_OBJECT(Exp, index);
  return (XIL_Exp) Exp::MakeIndex(new_target, new_element_type, new_index);
}

extern "C" XIL_Exp XIL_ExpString(XIL_Type type, void *data, int data_length)
{
  GET_OBJECT(Type, type);
  DataString *new_data = DataString::Make((uint8_t*) data, data_length);
  return (XIL_Exp) Exp::MakeString(new_type->AsArray(), new_data);
}

extern "C" XIL_Exp XIL_ExpVPtr(XIL_Exp target, int vtable_index)
{
  GET_OBJECT(Exp, target);
  return (XIL_Exp) Exp::MakeVPtr(new_target, (uint32_t) vtable_index);
}

extern "C" XIL_Exp XIL_ExpInt(long value)
{
  return (XIL_Exp) Exp::MakeInt(value);
}

extern "C" XIL_Exp XIL_ExpIntStr(const char *value)
{
  return (XIL_Exp) Exp::MakeIntStr(value);
}

extern "C" XIL_Exp XIL_ExpFloat(const char *value)
{
  return (XIL_Exp) Exp::MakeFloat(value);
}

extern "C"
XIL_Exp XIL_ExpUnop(XIL_UnopKind unop, XIL_Exp op, int bits, int sign)
{
  GET_OBJECT(Exp, op);
  return (XIL_Exp) Exp::MakeUnop((UnopKind) unop, new_op,
                                 (size_t) bits, (bool) sign);
}

extern "C"
XIL_Exp XIL_ExpBinop(XIL_BinopKind binop, XIL_Exp left_op, XIL_Exp right_op,
                     XIL_Type stride_type, int bits, int sign)
{
  GET_OBJECT(Exp, left_op);
  GET_OBJECT(Exp, right_op);
  GET_OBJECT(Type, stride_type);
  return (XIL_Exp)
    Exp::MakeBinop((BinopKind) binop, new_left_op, new_right_op,
                   new_stride_type, (size_t) bits, (bool) sign);
}

extern "C" XIL_Exp XIL_ExpInitial(XIL_Exp target)
{
  GET_OBJECT(Exp, target);
  return (XIL_Exp) Exp::MakeInitial(new_target, NULL);
}

extern "C" XIL_Exp XIL_ExpLBound(XIL_Exp target, XIL_Type stride_type)
{
  GET_OBJECT(Exp, target);
  GET_OBJECT(Type, stride_type);
  return (XIL_Exp) Exp::MakeBound(BND_Lower, new_target, new_stride_type);
}

extern "C" XIL_Exp XIL_ExpUBound(XIL_Exp target, XIL_Type stride_type)
{
  GET_OBJECT(Exp, target);
  GET_OBJECT(Type, stride_type);
  return (XIL_Exp) Exp::MakeBound(BND_Upper, new_target, new_stride_type);
}

extern "C" XIL_Exp XIL_ExpZTerm(XIL_Exp target, XIL_Type stride_type)
{
  GET_OBJECT(Exp, target);
  GET_OBJECT(Type, stride_type);

  Exp *empty_exp = Exp::MakeEmpty();
  ExpInt *zero_exp = Exp::MakeInt(0);

  return (XIL_Exp) Exp::MakeTerminate(new_target, new_stride_type,
                                      empty_exp, zero_exp);
}

extern "C" int XIL_GetExpInt(XIL_Exp exp, long *value)
{
  GET_OBJECT(Exp, exp);

  if (ExpInt *nexp = new_exp->IfInt()) {
    if (nexp->GetInt(value))
      return 1;
  }

  return 0;
}

extern "C" XIL_Exp XIL_ExpSign(XIL_Exp exp, int bits, bool sign)
{
  GET_OBJECT(Exp, exp);

  ExpInt *nexp = new_exp->IfInt();
  if (!nexp) return exp;

  mpz_t value;
  mpz_init(value);

  nexp->GetMpzValue(value);

  Exp *res = Exp::MakeIntMpz(value, (size_t) bits, sign);
  mpz_clear(value);

  return (XIL_Exp) res;
}

extern "C" XIL_Exp XIL_ExpAddress(XIL_Exp target)
{
  GET_OBJECT(Exp, target);

  if (ExpDrf *nnew_target = new_target->IfDrf()) {
    Exp *res = nnew_target->GetTarget();
    res->IncRef();
    return (XIL_Exp) res;
  }

  return NULL;
}

/////////////////////////////////////////////////////////////////////
// Blocks
/////////////////////////////////////////////////////////////////////

extern "C" void XIL_CFGSetCommand(const char *command)
{
  Assert(g_active_cfg);
  String *new_command = String::Make(command);
  g_active_cfg->SetCommand(new_command);
}

extern "C" void XIL_CFGSetBeginLocation(XIL_Location begin_loc)
{
  Assert(g_active_cfg);
  GET_OBJECT(Location, begin_loc);
  g_active_cfg->SetBeginLocation(new_begin_loc);
}

extern "C" void XIL_CFGSetEndLocation(XIL_Location end_loc)
{
  Assert(g_active_cfg);
  GET_OBJECT(Location, end_loc);
  g_active_cfg->SetEndLocation(new_end_loc);
}

extern "C" void XIL_CFGAddVar(XIL_Var var, XIL_Type type, int override)
{
  Assert(g_active_cfg);
  GET_OBJECT(Variable, var);
  GET_OBJECT(Type, type);

  if (override)
    new_var->SetType(new_type, true);

  g_active_cfg->AddVariable(new_var, new_type);
}

extern "C" XIL_PPoint XIL_CFGAddPoint(XIL_Location loc)
{
  Assert(g_active_cfg);
  GET_OBJECT(Location, loc);

  return (XIL_PPoint) g_active_cfg->AddPoint(new_loc);
}

extern "C" XIL_Location XIL_CFGGetPointLocation(XIL_PPoint point)
{
  Assert(g_active_cfg);

  Location *loc = g_active_cfg->GetPointLocation((PPoint) point);
  loc->IncRef();

  return (XIL_Location) loc;
}

extern "C" void XIL_CFGSetPointLocation(XIL_PPoint point, XIL_Location loc)
{
  Assert(g_active_cfg);
  GET_OBJECT(Location, loc);

  g_active_cfg->SetPointLocation((PPoint) point, new_loc);
}

extern "C"
void XIL_CFGSetEntryPoint(XIL_PPoint point)
{
  Assert(g_active_cfg);
  g_active_cfg->SetEntryPoint((PPoint) point);
}

extern "C"
void XIL_CFGSetExitPoint(XIL_PPoint point)
{
  Assert(g_active_cfg);
  g_active_cfg->SetExitPoint((PPoint) point);
}

extern "C"
void XIL_CFGAddLoopHead(XIL_PPoint point, XIL_Location end_loc)
{
  Assert(g_active_cfg);
  if (!point) return;

  Location *new_end_loc = end_loc ? (Location*) end_loc : NULL;
  g_active_cfg->AddLoopHead((PPoint) point, new_end_loc);
}

extern "C"
void XIL_CFGEdgeSkip(XIL_PPoint source, XIL_PPoint target)
{
  Assert(g_active_cfg);
  if (!source) return;

  PEdge *edge = PEdge::MakeSkip((PPoint) source, (PPoint) target);
  g_active_cfg->AddEdge(edge);
}

extern "C"
void XIL_CFGEdgeAssume(XIL_PPoint source, XIL_PPoint target,
                       XIL_Exp condition, int nonzero)
{
  Assert(g_active_cfg);
  if (!source) return;

  GET_OBJECT(Exp, condition);

  PEdge *edge = PEdge::MakeAssume((PPoint) source, (PPoint) target,
                                  new_condition, (bool) nonzero);
  g_active_cfg->AddEdge(edge);
}

extern "C"
void XIL_CFGEdgeAssign(XIL_PPoint source, XIL_PPoint target,
                       XIL_Type type, XIL_Exp left_side, XIL_Exp right_side)
{
  Assert(g_active_cfg);
  if (!source) return;

  GET_OBJECT(Type, type);
  GET_OBJECT(Exp, left_side);
  GET_OBJECT(Exp, right_side);

  PEdge *edge = PEdge::MakeAssign((PPoint) source, (PPoint) target,
                                  new_type, new_left_side, new_right_side);
  g_active_cfg->AddEdge(edge);
}

extern "C" XIL_Exp XIL_CFGInstanceFunction(XIL_Exp instance, XIL_Exp func)
{
  GET_OBJECT(Exp, instance);
  GET_OBJECT(Exp, func);

  // if there is an instance object the function should be relative to it.
  // this isn't the case for the function we are passed though, fix it up.
  if (new_instance)
    new_func = ExpReplaceExp(new_func, new_instance, Exp::MakeEmpty());

  if (new_func->IsVar() || new_func->IsRelative())
    return (XIL_Exp) new_func;
  return NULL;
}

extern "C"
void XIL_CFGEdgeCall(XIL_PPoint source, XIL_PPoint target, XIL_Type func_type,
                     XIL_Exp return_assign, XIL_Exp instance, XIL_Exp func,
                     XIL_Exp *args, int arg_count)
{
  Assert(g_active_cfg);
  if (!source) return;

  GET_OBJECT(Type, func_type);
  GET_OBJECT(Exp, return_assign);
  GET_OBJECT(Exp, instance);
  GET_OBJECT(Exp, func);

  Vector<Exp*> new_args;
  for (int ind = 0; ind < arg_count; ind++) {
    Exp *arg = (Exp*) args[ind];
    arg->IncRef();
    new_args.PushBack(arg);
  }

  PEdge *edge = PEdge::MakeCall((PPoint) source, (PPoint) target,
                                new_func_type->AsFunction(),
                                new_return_assign, new_instance, new_func, new_args);
  g_active_cfg->AddEdge(edge);
}

extern "C"
void XIL_CFGEdgeAssembly(XIL_PPoint source, XIL_PPoint target)
{
  Assert(g_active_cfg);
  if (!source) return;

  PEdge *edge = PEdge::MakeAssembly((PPoint) source, (PPoint) target);
  g_active_cfg->AddEdge(edge);
}

extern "C"
void XIL_CFGEdgeAnnotation(XIL_PPoint source, XIL_PPoint target,
                           const char *annot_name)
{
  Assert(g_active_cfg);
  if (!source) return;

  Variable *func_var = g_active_id->BaseVar();
  Assert(func_var->Kind() == VK_Func);

  func_var->IncRef();
  String *new_name = String::Make(annot_name);
  BlockId *annot = BlockId::Make(B_AnnotationFunc, func_var, new_name);

  PEdge *edge = PEdge::MakeAnnotation((PPoint) source, (PPoint) target, annot);
  g_active_cfg->AddEdge(edge);
}

/////////////////////////////////////////////////////////////////////
// Databases
/////////////////////////////////////////////////////////////////////

extern "C" void XIL_SetupGenerate(const char *remote_address)
{
  // just setup the remote address. we don't submit initial or final
  // transactions during generation, as we only process one translation unit
  // at a time and don't know when we're actually done. xsource can be used
  // to signal the manager to finish by the script controlling the build.
  AnalysisPrepare(remote_address);
  SkipHashConsCounts();
}

extern "C" void XIL_PrintGenerated()
{
  logout << "Generated Objects:" << endl << endl;

  for (size_t ind = 0; ind < g_keep_csus.Size(); ind++) {
    CompositeCSU *csu = g_keep_csus[ind];

    if (g_drop_csus.Contains(csu))
      continue;

    csu->Print(logout);
    logout << endl;
  }

  for (size_t ind = 0; ind < g_keep_cfgs.Size(); ind++) {
    BlockCFG *cfg = g_keep_cfgs[ind];

    if (g_drop_cfgs.Contains(cfg))
      continue;

    cfg->Print(logout);
    logout << endl;
  }
}

// buffer for writing out query or processed data.
static Buffer g_data_buf;

// whether the amount of data in g_data_buf exceeds the soft data limit.
// this limit is checked before compressing, so the amount of transmitted
// data will be considerably less.
#define TRANSACTION_DATA_EXCEEDED                               \
  (g_data_buf.pos - g_data_buf.base > TRANSACTION_DATA_LIMIT)

// lists for constructing batch queries on which CSUs/blocks to process.
static Vector<CompositeCSU*> g_query_csus;
static Vector<BlockCFG*> g_query_blocks;

// list of the CSUs and blocks we will actually be processing.
static Vector<CompositeCSU*> g_write_csus;
static Vector<BlockCFG*> g_write_blocks;

// data_buf contains the identifiers for all current query CSUs/blocks.
// submit a transaction to see which of these should be processed,
// adding entries to the write lists as necessary.
static void ProcessQueryList(Transaction *t)
{
  size_t result_var = t->MakeVariable(true);

  TOperand *list_op = TOperandString::Compress(t, &g_data_buf);
  t->PushAction(Backend::BlockQueryList(t, list_op, result_var));

  SubmitTransaction(t);
  g_data_buf.Reset();

  TOperandString *result_op = t->LookupString(result_var);

  if (result_op->GetDataLength() == 0) {
    // none of the queried CSUs/blocks needs to be processed.
    t->Clear();
    return;
  }

  TOperandString::Uncompress(result_op, &g_data_buf);

  Vector<String*> found_csus;
  Vector<BlockId*> found_blocks;

  Buffer read_buf(g_data_buf.base, g_data_buf.pos - g_data_buf.base);
  while (read_buf.pos != read_buf.base + read_buf.size) {
    switch (PeekOpenTag(&read_buf)) {
    case TAG_Name:
      found_csus.PushBack(String::ReadWithTag(&read_buf, TAG_Name));
      break;
    case TAG_BlockId:
      found_blocks.PushBack(BlockId::Read(&read_buf));
      break;
    default:
      Assert(false);
    }
  }

  t->Clear();

  for (size_t ind = 0; ind < g_query_csus.Size(); ind++) {
    CompositeCSU *csu = g_query_csus[ind];
    if (found_csus.Contains(csu->GetName()))
      g_write_csus.PushBack(csu);
  }

  for (size_t ind = 0; ind < g_query_blocks.Size(); ind++) {
    BlockCFG *cfg = g_query_blocks[ind];
    if (found_blocks.Contains(cfg->GetId()))
      g_write_blocks.PushBack(cfg);
  }

  g_query_csus.Clear();
  g_query_blocks.Clear();
  g_data_buf.Reset();
}

// write out the contents of data_buf to t as the results of processing
// some number of CSUs and/or blocks.
static void ProcessWriteList(Transaction *t)
{
  TOperand *list_op = TOperandString::Compress(t, &g_data_buf);
  t->PushAction(Backend::BlockWriteList(t, list_op));

  SubmitTransaction(t);
  t->Clear();

  g_data_buf.Reset();
}

extern "C" void XIL_WriteGenerated()
{
  Transaction *t = new Transaction();

  if (g_has_annotation) {
    // if we are making an annotation CFG there should be only one CFG
    // and no CSUs.
    Assert(g_keep_csus.Empty());
    Assert(g_keep_cfgs.Size() <= 1);

    if (!g_keep_cfgs.Empty() && g_drop_cfgs.Empty()) {
      BlockCFG *cfg = g_keep_cfgs[0];

      // do loop splitting (well, check for loops and eliminate skips).
      Vector<BlockCFG*> split_cfgs;
      SplitLoops(cfg, &split_cfgs);

      // if there were actually loops, only look at the non-loop CFG.
      // this will be the last entry in the split list.
      BlockCFG *base_cfg = split_cfgs.Back();

      String *name = base_cfg->GetId()->Loop();
      Assert(name);

      // list of CFGs to write out. this is usually just the one we just got,
      // but in the case of global invariants will include CFGs with identical
      // bodies for every global mentioned in the annotation.
      Vector<BlockCFG*> write_list;
      write_list.PushBack(base_cfg);

      if (base_cfg->GetId()->Kind() == B_AnnotationInit) {
        Vector<Exp*> lval_list;
        LvalListVisitor visitor(&lval_list);
        for (size_t eind = 0; eind < base_cfg->GetEdgeCount(); eind++)
          base_cfg->GetEdge(eind)->DoVisit(&visitor);

        // clone the CFG for all global variables we find in the annotation.
        // TODO: this doesn't account for cases where globals are mentioned
        // in functions called by the annotation.
        for (size_t lind = 0; lind < lval_list.Size(); lind++) {
          Variable *root = lval_list[lind]->Root();
          if (root && root->Kind() == VK_Glob) {
            // check if there is already an annotation CFG for this variable.
            bool have_cfg = false;
            for (size_t ind = 0; ind < write_list.Size(); ind++) {
              if (root == write_list[ind]->GetId()->BaseVar())
                have_cfg = true;
            }

            if (!have_cfg) {
              // clone the base CFG for the new variable.
              root->IncRef();
              name->IncRef();
              BlockId *new_id = BlockId::Make(B_AnnotationInit, root, name);
              BlockCFG *new_cfg = BlockCFG::Make(new_id);
              new_cfg->SetAnnotationKind(base_cfg->GetAnnotationKind());
              CopyCFGLocationsVariables(base_cfg, new_cfg);
              CopyCFGPointsEdges(base_cfg, new_cfg);
              write_list.PushBack(new_cfg);
            }
          }
        }

        DecRefVector<Exp>(lval_list, &lval_list);
      }

      // write out all the CFGs we generated.
      for (size_t ind = 0; ind < write_list.Size(); ind++) {
        BlockCFG::Write(&g_data_buf, write_list[ind]);

        TOperand *data = TOperandString::Compress(t, &g_data_buf);
        g_data_buf.Reset();

        t->PushAction(Backend::BlockWriteAnnot(t, data));
      }

      SubmitTransaction(t);
    }

    delete t;
    AnalysisCleanup();
    return;
  }

  for (size_t ind = 0; ind < g_keep_csus.Size(); ind++) {
    CompositeCSU *csu = g_keep_csus[ind];

    if (g_drop_csus.Contains(csu))
      continue;

    g_query_csus.PushBack(csu);
    String::WriteWithTag(&g_data_buf, csu->GetName(), TAG_Name);

    if (TRANSACTION_DATA_EXCEEDED)
      ProcessQueryList(t);
  }

  for (size_t ind = 0; ind < g_keep_cfgs.Size(); ind++) {
    BlockCFG *cfg = g_keep_cfgs[ind];

    if (g_drop_cfgs.Contains(cfg))
      continue;

    g_query_blocks.PushBack(cfg);
    BlockId::Write(&g_data_buf, cfg->GetId());

    if (TRANSACTION_DATA_EXCEEDED)
      ProcessQueryList(t);
  }

  if (g_data_buf.pos != g_data_buf.base)
    ProcessQueryList(t);
  Assert(g_data_buf.pos == g_data_buf.base);

  for (size_t ind = 0; ind < g_write_csus.Size(); ind++) {
    CompositeCSU::Write(&g_data_buf, g_write_csus[ind]);

    if (TRANSACTION_DATA_EXCEEDED)
      ProcessWriteList(t);
  }

  // the CSU database is empty but all the CSUs the escape analysis will
  // need are still in memory.
  EscapeUseLocalCSUs();

  for (size_t ind = 0; ind < g_write_blocks.Size(); ind++) {
    BlockCFG *cfg = g_write_blocks[ind];

    // do loop splitting.
    Vector<BlockCFG*> split_cfgs;
    SplitLoops(cfg, &split_cfgs);

    // remember the direct callees of this function.
    Vector<Variable*> callees;
    bool indirect_callee = false;

    // do callgraph and escape processing.
    for (size_t cind = 0; cind < split_cfgs.Size(); cind++) {
      EscapeProcessCFG(split_cfgs[cind]);
      CallgraphProcessCFG(split_cfgs[cind], &callees, &indirect_callee);
    }

    // add the direct call edges to the callgraph hash. only do this for
    // functions; ignore calls from global variable initializers.
    if (cfg->GetId()->Kind() == B_FunctionWhole) {
      TOperand *key_arg =
        new TOperandString(t, cfg->GetId()->Function()->Value());
      for (size_t cind = 0; cind < callees.Size(); cind++) {
        const char *name = callees[cind]->GetName()->Value();
        TOperand *callee_arg = new TOperandString(t, name);

        t->PushAction(
          Backend::HashInsertValue(t, CALLGRAPH_EDGES, key_arg, callee_arg));
      }

      if (indirect_callee) {
        t->PushAction(
          Backend::HashInsertKey(t, CALLGRAPH_INDIRECT, key_arg));
      }
    }

    Assert(split_cfgs.Size());
    WriteUInt32(&g_data_buf, split_cfgs.Size());

    for (size_t cind = 0; cind < split_cfgs.Size(); cind++)
      BlockCFG::Write(&g_data_buf, split_cfgs[cind]);

    if (TRANSACTION_DATA_EXCEEDED)
      ProcessWriteList(t);
  }

  if (g_data_buf.pos != g_data_buf.base)
    ProcessWriteList(t);
  Assert(g_data_buf.pos == g_data_buf.base);

  delete t;

  WritePendingEscape();

  AnalysisCleanup();
}

extern "C"
int XIL_HasAnnotation(XIL_Var var, const char *annot_name, int annot_type)
{
  GET_OBJECT(Variable, var);
  const char *var_name = new_var->GetName()->Value();

  const char *db_name = NULL;
  if (annot_type)
    db_name = COMP_ANNOT_DATABASE;
  else if (new_var->Kind() == VK_Func)
    db_name = BODY_ANNOT_DATABASE;
  else if (new_var->Kind() == VK_Glob)
    db_name = INIT_ANNOT_DATABASE;
  else
    Assert(false);

  Transaction *t = new Transaction();
  size_t result = t->MakeVariable(true);

  t->PushAction(
    Backend::BlockQueryAnnot(t, db_name, var_name, annot_name, result));
  SubmitTransaction(t);

  TOperandBoolean *data = t->LookupBoolean(result);
  int has_data = data->IsTrue();

  delete t;
  return has_data;
}

extern "C"
void XIL_AddAnnotationMsg(XIL_Var var, const char *annot_name,
                          XIL_AnnotationKind annot_kind, int annot_type,
                          XIL_Location loc, const char *annot_message)
{
  GET_OBJECT(Variable, var);
  GET_OBJECT(Location, loc);
  Assert(!g_has_annotation);

  BlockId *cfg_id = NULL;
  String *new_name = String::Make(annot_name);

  if (annot_type) {
    Assert(new_var->Kind() == VK_Glob);
    cfg_id = BlockId::Make(B_AnnotationComp, new_var, new_name);
  }
  else if (new_var->Kind() == VK_Func) {
    cfg_id = BlockId::Make(B_AnnotationFunc, new_var, new_name);
  }
  else if (new_var->Kind() == VK_Glob) {
    cfg_id = BlockId::Make(B_AnnotationInit, new_var, new_name);
  }

  BlockCFG *cfg = BlockCFG::Make(cfg_id);
  cfg->SetAnnotationKind((AnnotationKind) annot_kind);

 // make a single local variable '__error__'.
  cfg_id->IncRef();
  String *error_name = String::Make("__error__");
  Variable *error_var = Variable::Make(cfg_id, VK_Local, error_name, 0, NULL);

  error_var->IncRef();
  cfg->AddVariable(error_var, Type::MakeError());

  new_loc->IncRef();
  new_loc->IncRef();
  cfg->SetBeginLocation(new_loc);
  cfg->SetEndLocation(new_loc);

  new_loc->IncRef();
  new_loc->IncRef();
  PPoint entry_point = cfg->AddPoint(new_loc);
  PPoint exit_point = cfg->AddPoint(new_loc);

  cfg->SetEntryPoint(entry_point);
  cfg->SetExitPoint(exit_point);

  Exp *error_exp = Exp::MakeVar(error_var);

  String *new_message = String::Make(annot_message);
  Exp *message_exp = Exp::MakeString(new_message);

  PEdge *edge = PEdge::MakeAssign(entry_point, exit_point, Type::MakeError(),
                                  error_exp, message_exp);
  cfg->AddEdge(edge);

  Transaction *t = new Transaction();

  Assert(g_data_buf.pos == g_data_buf.base);
  BlockCFG::Write(&g_data_buf, cfg);

  TOperand *data = TOperandString::Compress(t, &g_data_buf);
  g_data_buf.Reset();

  t->PushAction(Backend::BlockWriteAnnot(t, data));

  SubmitTransaction(t);
  delete t;
}
