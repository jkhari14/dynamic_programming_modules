////////////////////////////////////////////////////////////////////////////
// DO NOT EDIT THIS FILE
//
// Written by Konstantin Makarychev & Yury Makarychev
//

#ifndef _test_framework_h_
#define _test_framework_h_

#include <algorithm>
#include <cassert>
#include <cctype>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TestFramework
{
   constexpr int GetTestFrameworkVersion ()
   {
      return 7;
   }

   void ExitIfConditionFails(bool bCondition, const char* error)
   {
      if (!bCondition)
      {
         std::cout << std::endl << std::endl
                   << "Error: " << error
                   << std::endl << std::endl;
         exit(1);
      }
   }

   void ExitIfConditionFails(bool bCondition, const char* error, const char* msg)
   {
      if (!bCondition)
      {
         std::cout << std::endl << std::endl
                   << "Error: " << error
                   << " Debug message: <" << msg << ">"
                   << std::endl << std::endl;
         exit(1);
      }
   }

   class StringSegment
   {
   public:
      StringSegment(const std::string& s);

      void CopyTo(std::string& dest) const;
      bool CopyTo(char* buffer, size_t buffer_size) const;
      std::string ToString() const;

      size_t CountChars(char c) const;

      bool IsEmpty() const;
      size_t Length() const;

      char ReadLeft();
      char ReadRight();

      char FirstChar() const;
      char LastChar() const;

      void RemovePrefix(size_t count);
      void RemoveSuffix(size_t count);

      void Trim();
      void TrimLeft();
      void TrimRight();

      bool Match(const char* s, bool bCaseSensitive = false);
      bool Match(StringSegment pattern, bool bCaseSensitive = false);

      bool Split(char delimiter, StringSegment& prefix);

      char operator [](size_t index) const;

   private:
      size_t iBegin;
      size_t iEnd;
      const std::string& str;
   };

   class AbstractLineParser
   {
   public:
      AbstractLineParser();
      void ParseFile(const char* filename, bool shouldExitOnError = false);
      bool IsOK() const;

   protected:
      void CheckCondition(bool bCondition, const char* error);

   private:
      virtual void PreParse() {};
      virtual void PostParse() {};
      virtual void ParseLine(StringSegment s) = 0;

   private:
      void ClearErrors();

   private:
      size_t lineNumber;
      bool bExitOnError;
      bool isOK;
   };

   template<class T>
   class BaseFieldAdapter
   {
   public:
      virtual bool FromString(T& var, StringSegment s) const = 0;
      virtual bool FromString(T& var, const std::string& str) const;

      virtual void ToString(const T& var, std::string& s /* out */) const = 0;

      virtual bool EqualsDefaultValue(const T& var) const = 0;
      virtual void SetDefaultValue(T& var) const = 0;

      virtual ~BaseFieldAdapter() {}
   };

   template<class T, class C>
   class FieldAdapter : public BaseFieldAdapter<T>
   {
   public:
      FieldAdapter(C T::*field_pointer) : field_pointer(field_pointer) {};
      FieldAdapter(C T::*field_pointer, C defaultValue) :
         field_pointer(field_pointer), defaultValue(defaultValue) {};

      bool FromString(T& var, StringSegment s) const override;
      void ToString(const T& var, std::string& s /* out */) const override;

      bool EqualsDefaultValue(const T& var) const override;
      void SetDefaultValue(T& var) const override;

   private:
      C T::*field_pointer;
      C defaultValue;
   };

   class ITable
   {
   public:
      virtual bool NewRow(size_t& row) = 0;
      virtual void SetDefaultValues(size_t row) = 0;
      virtual bool IsFixedSize() const = 0;

      virtual const std::string& GetColumnName(size_t col) const = 0;

      virtual bool GetValue(size_t row, size_t col, std::string& value) const = 0;
      virtual bool SetValue(size_t row, size_t col, StringSegment value) = 0;

      virtual bool GetValue(size_t row, StringSegment key, std::string& value) const;
      virtual bool SetValue(size_t row, StringSegment key, StringSegment value);

      virtual bool GetColumnByName(StringSegment key, size_t& col) const = 0;

      virtual bool EqualsDefaultValue(size_t row, size_t col) const = 0;

      virtual size_t ColumnCount() const = 0;
      virtual size_t RowCount() const = 0;

      virtual ~ITable() {}
   };

   template<class T>
   class AbstractTableAdapter : public ITable
   {
   public:
      typedef T DataType;

   public:
      bool AddNamedColumn(const char* name, std::shared_ptr<BaseFieldAdapter<T>> pField);
      const std::string& GetColumnName(size_t col) const override;

      bool EqualsDefaultValue(size_t row, size_t col) const override;
      void SetDefaultValues(size_t row) override;

      bool GetValue(size_t row, size_t col, std::string& value) const override;
      bool SetValue(size_t row, size_t col, StringSegment value) override;

      bool GetColumnByName(StringSegment key, size_t& col) const override;

      size_t ColumnCount() const override;

      virtual ~AbstractTableAdapter(){};
   private:
      virtual T& GetRecord(size_t i) = 0;
      virtual const T& GetRecord(size_t i) const = 0;

   private:
      struct ColumnSpec
      {
         std::string name;
         std::shared_ptr< BaseFieldAdapter<T> > fieldAdapter;
      };

   private:
      std::map<std::string, size_t> name2id;
      std::vector<ColumnSpec> columnSpecs;
   };

   template<class T>
   class RecordAdapter : public AbstractTableAdapter<T>
   {
   public:
      typedef T DataType;

   public:
      RecordAdapter(T& data) : data(data) {};

      bool NewRow(size_t& row) override;
      bool IsFixedSize() const override;
      size_t RowCount() const override;

   private:
      T & GetRecord(size_t i) override;
      const T& GetRecord(size_t i) const override;

   private:
      T & data;
   };

   template<class T>
   class TableAdapter : public AbstractTableAdapter<T>
   {
   public:
      typedef T DataType;

   public:
      TableAdapter(std::vector<T>& data) : data(data) {};

      bool NewRow(size_t& row) override;
      bool IsFixedSize() const override;
      size_t RowCount() const override;

   private:
      T & GetRecord(size_t i) override;
      const T& GetRecord(size_t i) const override;

   private:
      std::vector<T>& data;
   };

   class BasicYamlParser : public AbstractLineParser
   {
   public:
      BasicYamlParser() :
         AbstractLineParser(), pHeader(nullptr),
         pTable(nullptr), isHeaderSection(true) {};

      BasicYamlParser(ITable* pHeader, ITable* pTable);

      void SetHeaderAdapter(ITable* newAdapter);
      void SetTableAdapter(ITable* newAdapter);

      void PreParse() override;
      void ParseLine(StringSegment s) override;

   private:
      ITable* pHeader;
      ITable* pTable;
      bool isHeaderSection;
   };

   struct ProblemSetHeader
   {
      int id = -1;
      int problem_count = 0;
      int test_mistakes = -1;
      int time = -1;
      std::string student_name;
      std::chrono::high_resolution_clock::time_point tStart;
   };

   struct BasicProblem
   {
      int id;
      int correct_answer;
      int student_answer;
   };

   ///////////////////////////////////////////////////////////////////////////////

   size_t IntLen(int value)
   {
      if (value == 0) return 1;

      size_t len = 0;

      if (value < 0)
      {
         len = 1;
      }

      while (value != 0)
      {
         value = value / 10;
         len++;
      }

      return len;
   }

   size_t IntToStrHelper(int value, std::string& result, size_t pos = 0)
   {
      size_t len = IntLen(value);

      if (result.length() < pos + len)
      {
         result.resize(pos + len);
      }

      if (value == 0)
      {
         result[pos] = '0';
         return pos + 1;
      }

      int sign = 1; // '+'

      if (value < 0)
      {
         result[pos] = '-';
         sign = -1; // '-'
      }

      //position of the least significant digit in the string
      size_t lsPos = pos + len - 1;

      while (value != 0)
      {
         int nextValue = value / 10;
         int digit = sign * (value % 10);
         result[lsPos] = digit + '0';
         value = nextValue;
         lsPos--;
      }

      return (pos + len);
   }

   void Encode(int value, std::string& result)
   {
      IntToStrHelper(value, result);
   }

   void Encode(bool value, std::string& result)
   {
      result.assign(value ? "yes" : "no");
   }

   void Encode(const std::string& value, std::string& result)
   {
      result.clear();
      result.reserve(value.length() + 2);
      result.append("\"").append(value).append("\"");
   }

   void Encode(const std::vector<int>& value, std::string& result)
   {
      if (value.empty())
      {
         result.assign("[]");
         return;
      }

      size_t totalLen = 2;
      for (std::vector<int>::const_iterator it = value.begin();
           it != value.end();
            ++it)
      {
         totalLen += IntLen(*it);
         totalLen++;
      }

      totalLen--;
      result.resize(totalLen);

      result[0] = '[';
      size_t pos = 1;
      for (std::vector<int>::const_iterator it = value.begin();
           it != value.end();
            ++it)
      {
         pos = IntToStrHelper(*it, result, pos);
         result[pos] = ',';
         pos++;
      }

      result[pos - 1] = ']';
   }

   // Parse integer values
   bool Parse(StringSegment segment, int& result)
   {
      // if x < posOverflowGuard,  then we can append any digit to x without
      // integer overflow.
      // if x == posOverflowGuard, then we can append only some digits
      // if x > posOverflowGuard,  then we can not append any digits
      static constexpr int posOverflowGuard = std::numeric_limits<int>::max() / 10;
      static constexpr int posLastDigitGuard = std::numeric_limits<int>::max() % 10;

      // Similar thersholds for negative numbers
      // Note: Staring with C++11, the remainder of a negative number is negative.
      // For example, -7/3 = -2 and -7 % 3 = -1. See e.g. for details:
      // https://en.cppreference.com/w/cpp/language/operator_arithmetic#Multiplicative_operators

      static constexpr int negOverflowGuard = std::numeric_limits<int>::min() / 10;
      static constexpr int negLastDigitGuard = std::numeric_limits<int>::min() % 10;

      result = 0;
      segment.Trim();
      if (segment.IsEmpty()) return false;

      int sign = (segment.FirstChar() == '-') ? (-1) : 1;

      if (sign == -1)
      {
         segment.RemovePrefix(1);
         if (segment.IsEmpty()) return false;
      }

      while (!segment.IsEmpty())
      {
         char nextChar = segment.ReadLeft();
         if ((nextChar < '0') || (nextChar > '9')) return false;

         // for negative numbers digit is negative or zero
         char digit = (nextChar - '0') * sign;

         if ((result > posOverflowGuard) || (result < negOverflowGuard) ||
             ((result == posOverflowGuard) && (digit > posLastDigitGuard)) ||
             ((result == negOverflowGuard) && (digit < negLastDigitGuard)))
         {
            return false;
         }

         result = result * 10 + digit;
      }

      return true;
   }

   bool Parse(StringSegment segment, std::string& result)
   {
      segment.Trim();
      char left = segment.ReadLeft();
      char right = segment.ReadRight();
      if ((left == '"') && (right == '"'))
      {
         segment.CopyTo(result);
         return true;
      }
      else
      {
         return false;
      }
   }

   bool Parse(StringSegment segment, bool& result)
   {
      segment.Trim();
      result = segment.Match("true") || segment.Match("yes");

      if (result) return true;

      if (segment.Match("false") || segment.Match("no")) return true;

      return false;
   }

   bool Parse(StringSegment segment, std::vector<int>& result)
   {
      result.clear();
      segment.Trim();

      char firstChar = segment.ReadLeft();
      char lastChar = segment.ReadRight();

      if ((firstChar != '[') || (lastChar != ']')) return false;

      size_t nCount = segment.CountChars(',') + 1;
      result.reserve(nCount);

      StringSegment prefix(segment);

      segment.Split(',', prefix);
      bool noErrors = true;

      while (!prefix.IsEmpty() && noErrors)
      {
         int nextValue = 0;
         noErrors = noErrors && Parse(prefix, nextValue);
         result.push_back(nextValue);
         segment.Split(',', prefix);
      }

      return noErrors;
   }

   ///////////////////////////////////////////////////////////////////////////////

   template<class T, class C>
   std::shared_ptr<BaseFieldAdapter<T>> CreateSharedInterface(FieldAdapter<T, C>* pAdapter)
   {
      std::shared_ptr<BaseFieldAdapter<T>> sharedInterface(dynamic_cast<BaseFieldAdapter<T>*>(pAdapter));
      return sharedInterface;
   }

   template<class T, class C>
   void AddColumn(TableAdapter<T>& tableAdapter, const char* name, C T::*field_pointer)
   {
      static_assert(std::is_class<C>::value, "This field must be a class.");
      auto newAdapter = CreateSharedInterface(new FieldAdapter<T, C>(field_pointer));
      tableAdapter.AddNamedColumn(name, newAdapter);
   }

   template<class T, class C>
   void AddColumn(TableAdapter<T>& tableAdapter,
                  const char* name,
                  C T::*field_pointer,
                  C defaultValue)
   {
      auto newAdapter = CreateSharedInterface(new FieldAdapter<T, C>(field_pointer, defaultValue));
      tableAdapter.AddNamedColumn(name, newAdapter);
   }

   template<class T>
   void AddColumn(TableAdapter<T>& tableAdapter,
                  const char* name,
                  int T::*field_pointer,
                  int defaultValue = -1)
   {
      auto newAdapter = CreateSharedInterface(new FieldAdapter<T, int>(field_pointer, defaultValue));
      tableAdapter.AddNamedColumn(name, newAdapter);
   }

   template<class T, class C>
   void AddColumn(RecordAdapter<T>& recordAdapter, const char* name, C T::*field_pointer)
   {
      static_assert(std::is_class<C>::value, "This field must be a class.");
      auto newAdapter = CreateSharedInterface(new FieldAdapter<T, C>(field_pointer));
      recordAdapter.AddNamedColumn(name, newAdapter);
   }

   template<class T, class C>
   void AddColumn(RecordAdapter<T>& recordAdapter, const char* name,
                  C T::*field_pointer, C defaultValue)
   {
      auto newAdapter = CreateSharedInterface(new FieldAdapter<T, C>(field_pointer, defaultValue));
      recordAdapter.AddNamedColumn(name, newAdapter);
   }

   template<class T>
   void AddColumn(RecordAdapter<T>& recordAdapter, const char* name,
                  int T::*field_pointer, int defaultValue = -1)
   {
      auto newAdapter = CreateSharedInterface(new FieldAdapter<T, int>(field_pointer, defaultValue));
      recordAdapter.AddNamedColumn(name, newAdapter);
   }

   void AddDefaultProblemSetColumns(RecordAdapter<ProblemSetHeader>& psAdapter)
   {
      AddColumn(psAdapter, "problem_set_number", &ProblemSetHeader::id, -1);
      AddColumn(psAdapter, "student_name", &ProblemSetHeader::student_name);
      AddColumn(psAdapter, "problems", &ProblemSetHeader::problem_count, -1);
      AddColumn(psAdapter, "time", &ProblemSetHeader::time, -1);
      AddColumn(psAdapter, "test_mistakes", &ProblemSetHeader::test_mistakes, -1);
   }

   template<class T>
   void AddDefaultProblemColumns(TableAdapter<T>& prAdapter)
   {
      AddColumn<T, int>(prAdapter, "problem", &T::id, -1);
      AddColumn<T, int>(prAdapter, "correct_answer", &T::correct_answer, -1);
   }

   template<class T>
   void AddDefaultProblemColumnsForOutput(TableAdapter<T>& prAdapter)
   {
      AddColumn<T, int>(prAdapter, "problem", &T::id, -1);
      AddColumn<T, int>(prAdapter, "student_answer", &T::student_answer, -1);
   }
   ///////////////////////////////////////////////////////////////////////////////

   StringSegment::StringSegment(const std::string& s) : str(s)
   {
      iBegin = 0;
      iEnd = s.length();
   }

   std::string StringSegment::ToString() const
   {
      std::string dest;
      dest.assign(str, iBegin, iEnd - iBegin);
      return dest;
   }

   void StringSegment::CopyTo(std::string& dest) const
   {
      dest.assign(str, iBegin, iEnd - iBegin);
   }

   bool StringSegment::CopyTo(char* buffer, size_t buffer_size) const
   {
      assert(iBegin <= iEnd);
      assert(buffer_size > 0);

      size_t count = std::min(iEnd - iBegin, buffer_size - 1);

      for (size_t i = 0; i < count; ++i)
      {
         buffer[i] = str[iBegin + i];
      }

      buffer[count] = 0;

      return (count == (iEnd - iBegin));
   }

   size_t StringSegment::CountChars(char c) const
   {
      return (std::count (str.begin() + iBegin, str.begin() + iEnd, c));
   }

   bool StringSegment::IsEmpty() const
   {
      return (iBegin == iEnd);
   }

   size_t StringSegment::Length() const
   {
      assert(iBegin <= iEnd);
      return (iEnd - iBegin);
   }

   char StringSegment::ReadLeft()
   {
      char result = 0;
      if (iBegin < iEnd)
      {
         result = str[iBegin];
         iBegin++;
      }

      return result;
   }

   char StringSegment::ReadRight()
   {
      if (iBegin < iEnd)
      {
         iEnd--;
         return str[iEnd];
      }

      return 0;
   }

   char StringSegment::FirstChar() const
   {
      assert(iEnd > iBegin);
      return str[iBegin];
   }

   char StringSegment::LastChar() const
   {
      assert(iEnd > iBegin);
      return str[iEnd - 1];
   }

   void StringSegment::RemovePrefix(size_t count)
   {
      iBegin = std::min(iEnd, iBegin + count);
   }

   void StringSegment::RemoveSuffix(size_t count)
   {
      iEnd = std::max(iBegin + count, iEnd) - count;
   }

   void StringSegment::Trim()
   {
      TrimLeft();
      TrimRight();
   }

   void StringSegment::TrimLeft()
   {
      while ((iBegin < iEnd) &&
             (isspace(str[iBegin]) || (str[iBegin] == '\r')))
      {
         iBegin++;
      }
   }

   void StringSegment::TrimRight()
   {
      while ((iBegin < iEnd) &&
             (isspace(str[iEnd - 1]) || (str[iEnd - 1] == '\r')))
      {
         iEnd--;
      }
   }

   bool StringSegment::Match(const char* pattern, bool bCaseSensitive /* = false */)
   {
      assert(iBegin <= iEnd);

      size_t iStr = iBegin;
      size_t iPattern = 0;

      bool bMatch = true;
      while ((pattern[iPattern] != 0) && (iStr < iEnd) && bMatch)
      {
         bool bEqualChars = (pattern[iPattern] == str[iStr]);

         if (!bEqualChars)
         {
            if (!bCaseSensitive)
            {
               bEqualChars = (tolower(pattern[iPattern]) == tolower(str[iPattern]));
            }
            bMatch = bMatch && bEqualChars;
         }

         iStr++;
         iPattern++;
      }

      bMatch = bMatch && (pattern[iPattern] == 0) && (iStr == iEnd);

      return bMatch;
   }

   bool StringSegment::Match(StringSegment pattern, bool bCaseSensitive /* = false */)
   {
      size_t len = Length();
      if (len != pattern.Length()) return false;

      for (size_t i = 0; i < len; i++)
      {
         bool bEqualChars = (pattern[i] == str[i]);

         if (!bEqualChars)
         {
            if (!bCaseSensitive)
            {
               bEqualChars = (tolower(pattern[i]) == tolower(str[i]));
            }

            if (!bEqualChars) return false;
         }
      }

      return true;
   }

   bool StringSegment::Split(char delimiter, StringSegment& prefix)
   {
      assert(this != &prefix);

      if (!IsEmpty())
      {
         size_t i = iBegin;

         while ((i < iEnd) && (str[i] != delimiter)) i++;

         prefix.iBegin = iBegin;
         prefix.iEnd = i;
         iBegin = std::min(i + 1, iEnd);

         return true;
      }
      else
      {
         prefix.iBegin = 0;
         prefix.iEnd = 0;

         return false;
      }
   }

   char StringSegment::operator [](size_t index) const
   {
      assert(iBegin + index < iEnd);
      return str[iBegin + index];
   }

   template<class T>
   bool BaseFieldAdapter<T>::FromString(T& var, const std::string& str) const
   {
      StringSegment s(str);
      return FromString(var, s);
   }

   template<class T, class C>
   bool FieldAdapter<T, C>::FromString(T& var, StringSegment s) const
   {
      return Parse(s, var.*field_pointer);
   }

   template<class T, class C>
   void FieldAdapter<T, C>::ToString(const T& var, std::string& s /* out */) const
   {
      Encode(var.*field_pointer, s);
   }

   template<class T, class C>
   bool FieldAdapter<T, C>::EqualsDefaultValue(const T& var) const
   {
      return (var.*field_pointer == defaultValue);
   }

   template<class T, class C>
   void FieldAdapter<T, C>::SetDefaultValue(T& var) const
   {
      var.*field_pointer = defaultValue;
   }

   AbstractLineParser::AbstractLineParser() : lineNumber(0), isOK (true)
   {
      //empty
   }

   bool AbstractLineParser::IsOK() const
   {
      return isOK;
   }

   void AbstractLineParser::ClearErrors()
   {
      isOK = true;
   }

   void AbstractLineParser::ParseFile(const char* filename, bool shouldExitOnError)
   {
      bExitOnError = shouldExitOnError;
      ClearErrors();

      lineNumber = 0;
      std::ifstream input;
      input.open(filename);

      CheckCondition(input.good(), "Cannot open input file.");
      if (!IsOK()) return;

      PreParse();
      if (!IsOK()) return;

      while (!input.eof())
      {
         lineNumber++;
         std::string str;
         std::getline(input, str);
         StringSegment segment(str);

         if (!segment.IsEmpty())
         {
            ParseLine(segment);
            if (!IsOK()) return;
         }
      }

      PostParse();
   }

   void AbstractLineParser::CheckCondition(bool bCondition, const char* error)
   {
      if (!bCondition)
      {
         isOK = false;

         if (bExitOnError)
         {
            std::cout << std::endl << std::endl << "Error: " << error;

            if (lineNumber != 0)
            {
               std::cout << std::endl
                         << "Line number: " << lineNumber << "."
                         << std::endl << std::endl;
            }
            else
            {
               std::cout << std::endl << std::endl;
            }

            exit(1);
         }
      }
   }

   void StringToLowerCase(std::string& str)
   {
      std::transform(str.begin(), str.end(), str.begin(), tolower);
   }

   void StringToLowerCase(const std::string& inStr, std::string& outStr)
   {
      outStr.resize(inStr.size());
      std::transform(inStr.begin(), inStr.end(), outStr.begin(), tolower);
   }

   bool ITable::GetValue(size_t row, StringSegment key, std::string& value) const
   {
      size_t col = 0;
      bool bResult = GetColumnByName(key, col);

      if (bResult)
      {
         bResult = GetValue(row, col, value);
      }

      return bResult;
   }

   bool ITable::SetValue(size_t row, StringSegment key, StringSegment value)
   {
      size_t col = 0;
      bool bResult = GetColumnByName(key, col);

      if (bResult)
      {
         bResult = SetValue(row, col, value);
      }

      return bResult;
   }

   template<class T>
   bool AbstractTableAdapter<T>::AddNamedColumn(const char* name, std::shared_ptr<BaseFieldAdapter<T>> pField)
   {
      std::string strFieldName(name);
      StringToLowerCase(strFieldName);

      auto it = name2id.find(strFieldName);

      //return false if the column already exists
      if (it != name2id.end()) return false;

      ColumnSpec cs;
      cs.name.assign(name);
      cs.fieldAdapter = pField;

      columnSpecs.push_back(cs);

      StringToLowerCase(strFieldName);
      name2id[strFieldName] = columnSpecs.size() - 1;

      return true;
   }

   template<class T>
   const std::string& AbstractTableAdapter<T>::GetColumnName(size_t col) const
   {
      assert(col < ColumnCount());
      return columnSpecs[col].name;
   }

   template<class T>
   void AbstractTableAdapter<T>::SetDefaultValues(size_t row)
   {
      assert(row < RowCount());

      T& theRecord = GetRecord(row);

      for (ColumnSpec& spec : columnSpecs)
      {
         spec.fieldAdapter->SetDefaultValue(theRecord);
      }
   }

   template<class T>
   bool AbstractTableAdapter<T>::GetValue(size_t row, size_t col, std::string& value) const
   {
      assert(col < ColumnCount());
      assert(row < RowCount());

      auto theField = columnSpecs[col].fieldAdapter;
      const T& theRecord = GetRecord(row);

      theField->ToString(theRecord, value);
      return true;
   }

   template<class T>
   bool AbstractTableAdapter<T>::SetValue(size_t row, size_t col, StringSegment value)
   {
      assert(col < ColumnCount());
      assert(row < RowCount());

      auto theField = columnSpecs[col].fieldAdapter;
      T& theRecord = GetRecord(row);

      bool bResult = theField->FromString(theRecord, value);
      return bResult;
   }

   template<class T>
   bool AbstractTableAdapter<T>::EqualsDefaultValue (size_t row, size_t col) const
   {
      assert(col < ColumnCount());
      assert(row < RowCount());

      auto theField = columnSpecs[col].fieldAdapter;
      const T& theRecord = GetRecord(row);

      return theField->EqualsDefaultValue (theRecord);
   }

   template<class T>
   bool AbstractTableAdapter<T>::GetColumnByName(StringSegment key, size_t& col) const
   {
      std::string strFieldName;
      key.CopyTo(strFieldName);
      StringToLowerCase(strFieldName);

      auto it = name2id.find(strFieldName);

      if (it == name2id.end()) return false; /* not found */
      col = it->second;

      assert(col < columnSpecs.size());

      return true;
   }

   template<class T>
   size_t AbstractTableAdapter<T>::ColumnCount() const
   {
      return columnSpecs.size();
   }

   template<class T>
   bool TableAdapter<T>::NewRow(size_t& row)
   {
      data.emplace_back();
      row = data.size() - 1;
      this->SetDefaultValues(row);
      return true;
   }

   template<class T>
   size_t TableAdapter<T>::RowCount() const
   {
      return data.size();
   }

   template<class T>
   bool TableAdapter<T>::IsFixedSize() const
   {
      return false;
   }

   template<class T>
   T& TableAdapter<T>::GetRecord(size_t i)
   {
      assert(i < data.size());
      return data[i];
   }

   template<class T>
   const T& TableAdapter<T>::GetRecord(size_t i) const
   {
      assert(i < data.size());
      return data[i];
   }

   template<class T>
   bool RecordAdapter<T>::NewRow(size_t& row)
   {
      return false;
   }

   template<class T>
   bool RecordAdapter<T>::IsFixedSize() const
   {
      return true;
   }

   template<class T>
   size_t RecordAdapter<T>::RowCount() const
   {
      return 1;
   }

   template<class T>
   T& RecordAdapter<T>::GetRecord(size_t i)
   {
      assert(i == 0);
      return data;
   }

   template<class T>
   const T& RecordAdapter<T>::GetRecord(size_t i) const
   {
      assert(i == 0);
      return data;
   }

   BasicYamlParser::BasicYamlParser(ITable* pHeader, ITable* pTable) :
                           AbstractLineParser(), pHeader(pHeader), pTable(pTable), isHeaderSection(true)
   {
      assert(pHeader != nullptr);
      assert(pHeader->RowCount() > 0);
      assert(pTable != nullptr);
      assert(!pTable->IsFixedSize());
   }

   void BasicYamlParser::SetHeaderAdapter(ITable* newAdapter)
   {
      assert(newAdapter != nullptr);
      assert(newAdapter->RowCount() > 0);
      pHeader = newAdapter;
   }

   void BasicYamlParser::SetTableAdapter(ITable* newAdapter)
   {
      assert(newAdapter != nullptr);
      assert(!newAdapter->IsFixedSize());
      pTable = newAdapter;
   }

   void BasicYamlParser::PreParse()
   {
      isHeaderSection = true;
   }

   void BasicYamlParser::ParseLine(StringSegment s)
   {
      assert(pTable != nullptr);

      s.Trim();
      if (s.IsEmpty()) return;

      char firstChar = s.FirstChar();

      if (firstChar == '#')
      {
         //comments
         return;
      }
      else if (s.Match("data:"))
      {
         //start data section
         isHeaderSection = false;
         return;
      }
      else if (firstChar == '-')
      {
         CheckCondition(!isHeaderSection, "Invalid entry in the header section.");

         size_t row;
         pTable->NewRow(row);
         s.RemovePrefix(1);

         s.Trim();
         if (s.IsEmpty()) return;
      }

      StringSegment prefix(s);
      s.Split(':', prefix);
      s.Trim();
      prefix.Trim();

      CheckCondition(!s.IsEmpty() && !prefix.IsEmpty(), "Key or value is empty.");
      if (!IsOK()) return;

      if (isHeaderSection)
      {
         CheckCondition(pHeader != nullptr, "Unexpected header.");
         if (!IsOK()) return;

         bool bResult = pHeader->SetValue(0, prefix, s);

         CheckCondition(bResult, "Cannot parse a line.");
         if (!IsOK()) return;
      }
      else
      {
         size_t tableSize = pTable->RowCount();
         bool bResult = pTable->SetValue(tableSize - 1, prefix, s);

         CheckCondition(bResult, "Cannot parse a line.");
         if (!IsOK()) return;
      }
   }

   void WriteRecordToStream(std::ostream& out, ITable* table,
                            size_t row, bool bWriteDefaultValues,
                            bool bIndent)
   {
      if (bIndent) out << std::endl;

      size_t nCols = table->ColumnCount();
      for (size_t j = 0; j < nCols; j++)
      {
          if (bIndent && (j == 0))
         {
            out << " - ";
         }

         if (bWriteDefaultValues || !table->EqualsDefaultValue(row, j))
         {
            if (bIndent && (j != 0))
            {
               out << "   ";
            }

            out << table->GetColumnName(j) << ": ";
            std::string value;
            table->GetValue(row, j, value);
            out << value << std::endl;
         }
      }
   }

   void WriteTableToStream(std::ostream& out, ITable* table, ITable* header /* can be nullptr */,
                           bool bWriteDefaultValues)
   {
      assert(table != nullptr);

      size_t nRows = table->RowCount();

      if (header != nullptr)
      {
         WriteRecordToStream(out, header, 0, bWriteDefaultValues, false);
      }

      out << std::endl << "data:" << std::endl;
      for (size_t i = 0; i < nRows; i++)
      {
         WriteRecordToStream(out, table, i, bWriteDefaultValues, true);
      }
   }

   void WriteTableToFile(const char* filename, ITable* header, ITable* table,
                         bool bWriteDefaultValues, const char* comments)
   {
      std::ofstream out;
      out.open(filename);
      ExitIfConditionFails(out.good(), "Cannot open output file!");

      if (comments != nullptr)
      {
         out << comments;
      }

      WriteTableToStream(out, header, table, bWriteDefaultValues);

      out.close();
   }

   template<class T>
   void PreprocessProblemSet(int problem_set_id, std::vector<T>& problems, ProblemSetHeader& header)
   {
      ExitIfConditionFails(header.id == problem_set_id, "Wrong problem set. Check problem set number.");
      ExitIfConditionFails(header.problem_count == (int) problems.size(), "Input file is corrupted.");
      ExitIfConditionFails(!header.student_name.empty (), "Please, enter your name in 'GetStudentName' function.");

      for (int i = 0; i < header.problem_count; i++)
      {
         ExitIfConditionFails(problems[i].id == (i + 1), "Input file is corrupted.");
      }

      header.tStart = std::chrono::high_resolution_clock::now();
   }

   template<class T>
   void ProcessResults(std::vector<T>& problems, ProblemSetHeader& header)
   {
      std::chrono::high_resolution_clock::time_point tNow = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(tNow - header.tStart);
      header.time = static_cast<int>(std::round(1000 * time_span.count()));

      int nMistakes = 0;

      for (int i = 0; i < header.problem_count; i++)
      {
         T& theProblem = problems[i];

         if ((theProblem.student_answer != theProblem.correct_answer))
         {
            nMistakes++;
            std::cout << std::endl;
            std::cout << "Mistake in problem #" << (i + 1) << "." << std::endl;
            std::cout << "Correct answer: " << theProblem.correct_answer << "." << std::endl;
            std::cout << "Your answer: " << theProblem.student_answer << "." << std::endl;
            std::cout << "=========================";
         }
      }

      header.test_mistakes = nMistakes;

      if (nMistakes > 0)
      {
         std::cout << std::endl << "Your algorithm made " << nMistakes << " mistake(s)." << std::endl;
      }
      else
      {
         std::cout << "Your algorithm solved all test problems correctly. Congratulations!" << std::endl;
      }
   }
}

#endif //_test_framework_h_