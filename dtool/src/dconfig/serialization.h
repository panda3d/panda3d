// Filename: serialization.h
// Created by:  cary (26Aug98)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef __SERIALIZATION_H__
#define __SERIALIZATION_H__

#include "dtoolbase.h"

#include "config_setup.h"

namespace Serialize {

template <class X>
class StdIns {
   public:
      INLINE ConfigString operator()(const X& val) {
         ostringstream oss;
         oss << val;
         return oss.str();
      }
};

template <class X>
class StdExt {
   public:
      INLINE X operator()(ConfigString S) {
         istringstream iss(S);
         X ret;
         iss >> ret;
         return ret;
      }
};

template <class X>
INLINE int Length(X c) {
   return c.length();
}

template <class Collection, class Inserter = StdIns<TYPENAME Collection::value_type> >
class Serializer {
   private:
      ConfigString _result;

      ConfigString SerializeToString(const Collection&, const ConfigString&);
      Serializer() {}
   public:
      Serializer(const Collection& C, ConfigString Delim = ":") :
        _result(Serializer::SerializeToString(C, Delim)) {}
      Serializer(const Serializer<Collection, Inserter>& c) :
        _result(c._result) {}
      ~Serializer() {}
      INLINE ConfigString operator()() { return _result; }
      INLINE ConfigString operator()(const Collection& C,
                                     const ConfigString& Delim = ":") {
         _result = SerializeToString(C, Delim);
         return _result;
      }
      INLINE operator ConfigString() { return _result; }
};

template <class Collection, class Inserter>
ConfigString
Serializer<Collection, Inserter>::SerializeToString(const Collection& C,
                                                    const ConfigString& Delim)
{
   ConfigString ret;
   Inserter in;

   for (TYPENAME Collection::const_iterator i=C.begin(); i!=C.end(); ++i) {
      if (i != C.begin())
         ret += Delim;
      ret += in(*i);
   }
   return ret;
}

template <class Collection, class Extractor = StdExt<TYPENAME Collection::value_type> >
class Deserializer {
   private:
      Collection _result;

      INLINE void Clear() { _result.erase(_result.begin(), _result.end()); }
      template <class ForwardIterator>
      int FindFirstOfInString(ConfigString S, ForwardIterator DelimBegin,
                              ForwardIterator DelimEnd) {
         int i = ConfigString::npos;
         ForwardIterator j = DelimBegin;

         while (j != DelimEnd) {
            int k = S.find(*j);
            if (k != ConfigString::npos)
               if ((i == ConfigString::npos) || (i > k))
                  i = k;
            ++j;
         }
         return i;
      }
      template <class ForwardIterator>
      int FindFirstNotOfInString(ConfigString S, ForwardIterator DelimBegin,
                                 ForwardIterator DelimEnd) {
         int i = ConfigString::npos;
         ForwardIterator j = DelimBegin;
         ForwardIterator k = DelimBegin;

         while (j != DelimEnd) {
            int l = S.find(*j);
            if (l != ConfigString::npos)
               if ((i == ConfigString::npos) || (i > l)) {
                  i = l;
                  k = j;
               }
            ++j;
         }
         if (i != ConfigString::npos) {
            i += Serialize::Length(*k);
            if (i >= S.length())
               i = ConfigString::npos;
         }
         return i;
      }
      template <class ForwardIterator>
      void DeserializeFromString(ConfigString S, ForwardIterator DelimBegin,
                                 ForwardIterator DelimEnd) {
         Clear();
         Extractor ex;

         while (!S.empty()) {
            int i = FindFirstOfInString(S, DelimBegin, DelimEnd);
            _result.push_back(ex(S.substr(0, i)));
            S.erase(0, i);
            i = FindFirstNotOfInString(S, DelimBegin, DelimEnd);
            S.erase(0, i);
         }
      }
      void DeserializeFromString(ConfigString S, ConfigString Delim) {
         Clear();
         Extractor ex;

         while (!S.empty()) {
            size_t i = S.find_first_of(Delim);
            _result.push_back(ex(S.substr(0, i)));
            if (i == ConfigString::npos)
               S.erase(0, i);
            else
               S.erase(0, i+1);
         }
      }
      Deserializer() {}
   public:
      Deserializer(ConfigString S, ConfigString Delim = ":") {
         Deserializer::DeserializeFromString(S, Delim);
      }
      template <class ForwardIterator>
      Deserializer(ConfigString S, ForwardIterator DelimBegin,
                   ForwardIterator DelimEnd) {
         Deserializer::DeserializeFromString(S, DelimBegin, DelimEnd);
      }
      ~Deserializer() {}
      INLINE const Collection& operator()() { return _result; }
      template <class ForwardIterator>
      INLINE const Collection& operator()(ConfigString S,
                                          ForwardIterator DelimBegin,
                                          ForwardIterator DelimEnd) {
         Deserializer::DeserializeFromString(S, DelimBegin, DelimEnd);
      }
      INLINE operator const Collection&() { return _result; }
};

#include "serialization.I"

} // close Serialize namespace

#endif /* __SERIALIZATION_H__ */
