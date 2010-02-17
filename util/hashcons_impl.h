
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

// register a new global HashCons
void RegisterHashCons(HashCons<HashObject> *hash);

/////////////////////////////////////////////////////////////////////
// HashCons
/////////////////////////////////////////////////////////////////////

template <class T>
HashCons<T>::HashBucket::HashBucket()
{
  LinkedListInit<HashObject>(&e_begin, &e_pend);
}

template <class T>
HashCons<T>::HashCons(size_t min_bucket_count)
  : m_buckets(NULL), m_bucket_count(0), m_object_count(0),
    m_min_bucket_count(min_bucket_count), m_hash_next(NULL)
{
  Assert(m_min_bucket_count != 0);
  Resize(m_min_bucket_count);

  RegisterHashCons((HashCons<HashObject>*) this);
}

template <class T>
T* HashCons<T>::Lookup(T &o)
{
  // do this on all lookups as objects in this table will
  // remove themselves from this table without notifying this table.
  CheckBucketCount();

  size_t ind = o.Hash() % m_bucket_count;
  HashBucket *bucket = &m_buckets[ind];

  HashObject *xo = bucket->e_begin;
  while (xo != NULL) {
    if (o.Hash() == xo->Hash() && T::Compare(&o, (T*) xo) == 0) {
      o.DecMoveChildRefs(NULL, NULL);
      xo->IncRef();
      return (T*) xo;
    }
    xo = xo->m_next;
  }

  T *no = T::Copy(&o);

#ifdef DEBUG
  no->DecMoveChildRefs(NULL, no);
#endif

  no->HashInsert((HashObject***) &bucket->e_pend, &m_object_count);
  no->Persist();

  no->IncRef();
  return no;
}

template <class T>
bool HashCons<T>::IsMember(const T *o)
{
  Assert(o);

  size_t ind = o->Hash() % m_bucket_count;
  HashBucket *bucket = &m_buckets[ind];

  HashObject *xo = bucket->e_begin;
  while (xo != NULL) {
    if (o == xo)
      return true;
    xo = xo->m_next;
  }

  return false;
}

template <class T>
void HashCons<T>::DropAllChildRefs()
{
  // can only drop child references if it will not end up deleting objects,
  // i.e. we are finding reference leaks at program teardown.
  Assert(!HashObject::g_delete_unused);

  for (size_t ind = 0; ind < m_bucket_count; ind++) {
    HashBucket *bucket = &m_buckets[ind];

    HashObject *o = bucket->e_begin;
    while (o != NULL) {
      o->DecMoveChildRefs(o, NULL);
      o = o->m_next;
    }
  }
}

template <class T>
void HashCons<T>::PrintLiveObjects(uint64_t &min_stamp)
{
  for (size_t ind = 0; ind < m_bucket_count; ind++) {
    HashBucket *bucket = &m_buckets[ind];

    HashObject *o = bucket->e_begin;
    while (o != NULL) {
      if (o->Refs() != 0) {
        logout << "  " << o->Refs() << ": [" << o->Hash() << "]" << endl;

#ifdef DEBUG
        o->PrintRefStamps();
        logout << endl;

        // remember the earliest leaked reference.
        uint64_t o_min_stamp = o->MinRefStamp();
        if (o_min_stamp < min_stamp)
          min_stamp = o_min_stamp;
#else
        logout << "  " << o << endl << endl;
#endif
      }

      o = o->m_next;
    }
  }
}

template <class T>
void HashCons<T>::Resize(size_t bucket_count)
{
  Assert(bucket_count >= m_min_bucket_count);
  HashBucket *buckets = new HashBucket[bucket_count];

  for (size_t ind = 0; ind < m_bucket_count; ind++) {
    HashBucket *bucket = &m_buckets[ind];

    while (bucket->e_begin != NULL) {
      HashObject *o = bucket->e_begin;
      o->HashRemove();

      size_t nind = o->Hash() % bucket_count;
      HashBucket *nbucket = &buckets[nind];
      o->HashInsert((HashObject***) &nbucket->e_pend, &m_object_count);
    }
  }

  if (m_buckets != NULL) {
    delete[] m_buckets;
    m_buckets = NULL;
  }

  m_buckets = buckets;
  m_bucket_count = bucket_count;
}

template <class T>
void HashCons<T>::PrintObjects()
{
  logout << "HashCons contents:" << endl;
  for (size_t ind = 0; ind < m_bucket_count; ind++) {
    HashBucket *bucket = &m_buckets[ind];

    HashObject *o = bucket->e_begin;
    while (o != NULL) {
      logout << o->Refs() << ": " << o << endl;
      o = o->m_next;
    }
  }
}
