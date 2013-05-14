/*****************************************************************************
 *   GATB : Genome Assembly Tool Box                                         *
 *   Copyright (c) 2013                                                      *
 *                                                                           *
 *   GATB is free software; you can redistribute it and/or modify it under   *
 *   the CECILL version 2 License, that is compatible with the GNU General   *
 *   Public License                                                          *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
 *   CECILL version 2 License for more details.                              *
 *****************************************************************************/

#include <CppunitCommon.hpp>

#include <gatb/tools/designpattern/impl/IteratorWrappers.hpp>
#include <gatb/tools/designpattern/impl/IteratorHelpers.hpp>

#include <vector>
#include <string>

using namespace std;

using namespace gatb::core::tools::dp;
using namespace gatb::core::tools::dp::impl;

/********************************************************************************/
namespace gatb  {  namespace tests  {
/********************************************************************************/

/** \brief Test class for miscellaneous operations
 */
class TestIterators : public Test
{
    /********************************************************************************/
    CPPUNIT_TEST_SUITE_GATB (TestIterators);

        CPPUNIT_TEST_GATB (iterators_checkNullIterator);
        CPPUNIT_TEST_GATB (iterators_checkList);
        CPPUNIT_TEST_GATB (iterators_checkCartesianIterator);
        CPPUNIT_TEST_GATB (iterators_checkCompoundIterator);
        CPPUNIT_TEST_GATB (iterators_checkTruncateIterator);
        CPPUNIT_TEST_GATB (iterators_checkPairedIterator);

    CPPUNIT_TEST_SUITE_GATB_END();

public:
    /********************************************************************************/
    void setUp    ()  {}
    void tearDown ()  {}

    /********************************************************************************/
    /** \brief check the NULL iterator
     *
     * Test of \ref gatb::core::tools::dp::impl::NullIterator \n
     */
    void iterators_checkNullIterator ()
    {
        size_t nbItems = 0;

        /** We declare a null iterator. */
        NullIterator<int> it;
        for (it.first(); !it.isDone(); it.next())
        {
            CPPUNIT_ASSERT (false);
        }
        CPPUNIT_ASSERT (nbItems == 0);
    }

    /********************************************************************************/
    /** \brief check the std::list iterator wrapper.
     *
     * Test of \ref gatb::core::tools::dp::impl::ListIterator \n
     */
    void iterators_checkList ()
    {
        size_t nbItems = 0;

        /** We declare a STL list with some values. */
        int values[] = {1,2,3,5,8,13,21,34};
        list<int> l (values, values + sizeof(values)/sizeof(values[0]) );

        /** We iterate the list through a ListIterator instance. */
        ListIterator<int> it (l);
        for (it.first(); !it.isDone(); it.next())
        {
            CPPUNIT_ASSERT (*it == values[nbItems++]);
        }
        CPPUNIT_ASSERT (nbItems == sizeof(values)/sizeof(values[0]));
    }

    /********************************************************************************/
    /** \brief check the CartesianIterator
     *
     * Test of \ref gatb::core::tools::dp::impl::CartesianIterator\n
     */
    void iterators_checkCartesianIterator ()
    {
        size_t nbItems = 0;
        size_t i=0, j=0;

        /** We declare a STL list with some values. */
        int values1[] = {1,2,3,5,8,13,21,34};
        list<int> l1 (values1, values1 + sizeof(values1)/sizeof(values1[0]) );

        /** We declare a STL list with some values. */
        float values2[] = {0.5, 3.1415, 2.71};
        list<float> l2 (values2, values2 + sizeof(values2)/sizeof(values2[0]) );

        /** We build the pairs list that should be iterated by the Cartesian iterator. */
        vector <pair<int,float> > pairsCheck;
        for (size_t i=0; i<sizeof(values1)/sizeof(values1[0]); i++)
        {
            for (size_t j=0; j<sizeof(values2)/sizeof(values2[0]); j++)
            {
                pairsCheck.push_back (pair<int,float>(values1[i],values2[j]));
            }
        }

        /** We declare two iterators on the two lists. */
        ListIterator<int>   it1 (l1);
        ListIterator<float> it2 (l2);

        /** We declare a Cartesian iterator on the two iterators. */
        CartesianIterator<int,float> it (it1, it2);
        for (it.first(); !it.isDone(); it.next())
        {
            CPPUNIT_ASSERT (it->first  == pairsCheck[nbItems].first);
            CPPUNIT_ASSERT (it->second == pairsCheck[nbItems].second);
            nbItems++;
        }
        CPPUNIT_ASSERT (nbItems == sizeof(values1)/sizeof(values1[0]) * sizeof(values2)/sizeof(values2[0]));
    }

    /********************************************************************************/
    /** We define an iterator that loops every ith character of a given string */
    class MyIterator : public Iterator<char>
    {
    public:
        MyIterator (const string& s) : _str(s), _mod(1), _idx(0) {}
        void update (int modulo)  { _mod = modulo; }
        void first  () { _idx  = _mod; }
        void next   () { _idx += _mod; }
        bool isDone () { return _idx >= _str.size(); }
        char& item  () { return _str[_idx]; }
    private:
        string _str;
        int    _mod;
        int    _idx;
    };

    /** We need a functor for updating the inner loop iterator. */
    struct Update { void operator() (Iterator<char>* it2, int* val)  {  static_cast<MyIterator*> (it2)->update (*val);  }};

    /** \brief check the CompoundIterator
     *
     * Test of \ref gatb::core::tools::dp::impl::CompoundIterator\n
     */
    void iterators_checkCompoundIterator ()
    {
        size_t nbItems = 0;
        size_t i=0, j=0;
        const char* str = "abcdefghijklmnopqrstuvwxyz";

        /** We declare a STL list with some values. */
        int values1[] = {2, 3, 5};
        list<int> l1 (values1, values1 + sizeof(values1)/sizeof(values1[0]) );
        ListIterator<int> it1 (l1);

        /** We build the check table holding the items we are supposed to iterate. */
        char checkTable[] = {
            'c','e','g','i','k','m','o','q','s','u','w','y',    // loop modulo 2
            'd','g','j','m','p','s','v','y',                    // loop modulo 3
            'f','k','p','u','z'                                 // loop modulo 5
        };

        /** We declare an iterator on our custom iterator. */
        MyIterator it2 (str);

        /** We declare a Cartesian iterator on the two iterators. */
        CompoundIterator<int,char,Update> it (it1, it2, Update());
        for (it.first(); !it.isDone(); it.next(), nbItems++)
        {
            if (nbItems < sizeof(checkTable)/sizeof(checkTable[0]))
            {
                CPPUNIT_ASSERT (*it == checkTable[nbItems]);
            }
        }

        /** We  check that we got the correct number of iterated items. */
        CPPUNIT_ASSERT (nbItems == sizeof(checkTable)/sizeof(checkTable[0]));
    }

    /********************************************************************************/
    /** \brief check the truncation of the given iterator
     *
     * Test of \ref gatb::core::tools::dp::impl::TruncateIterator \n
     */
    void iterators_checkTruncateIterator ()
    {
        size_t nbItems = 0;

        /** We declare a STL list with some values. */
        int values[] = {1,2,3,5,8,13,21,34};
        int valuesLen = sizeof(values)/sizeof(values[0]);
        list<int> l (values, values + valuesLen);

        /** We declare an iterator on this list and loop the items. */
        ListIterator<int> itRef (l);
        nbItems = 0;
        for (itRef.first(); !itRef.isDone(); itRef.next())  {  CPPUNIT_ASSERT (*itRef == values[nbItems++]);  }
        CPPUNIT_ASSERT (nbItems == valuesLen);

        /** We declare a truncated iterator for the list iterator. */
        TruncateIterator<int> itTrunc (itRef, valuesLen/2);
        nbItems = 0;
        for (itTrunc.first(); !itTrunc.isDone(); itTrunc.next())  {  CPPUNIT_ASSERT (*itTrunc == values[nbItems++]);  }
        CPPUNIT_ASSERT (nbItems == valuesLen/2);

        /** We declare a truncated iterator for the list iterator. Note that we make it longer than the referred one. */
        TruncateIterator<int> itTrunc2 (itRef, 2*valuesLen);
        nbItems = 0;
        for (itTrunc2.first(); !itTrunc2.isDone(); itTrunc2.next())  {  CPPUNIT_ASSERT (*itTrunc2 == values[nbItems++]);  }
        CPPUNIT_ASSERT (nbItems == valuesLen);
    }

    /********************************************************************************/
    /** \brief check the PairedIterator
     *
     * Test of \ref gatb::core::tools::dp::impl::PairedIterator\n
     */
    void iterators_checkPairedIterator ()
    {
        size_t nbItems = 0;
        size_t i=0;

        /** We declare a STL list with some values. */
        int values1[] = {1,2,3,5,8,13,21,34};
        list<int> l1 (values1, values1 + sizeof(values1)/sizeof(values1[0]) );

        /** We declare a STL list with some values. */
        float values2[] = {0.5, 3.1415, 2.71, 5.87451};
        list<float> l2 (values2, values2 + sizeof(values2)/sizeof(values2[0]) );

        /** We declare two iterators on the two lists. */
        ListIterator<int>   it1 (l1);
        ListIterator<float> it2 (l2);

        /** We declare a paired iterator on the two iterators. */
        PairedIterator<int,float> it (it1, it2);
        for (it.first(); !it.isDone(); it.next(), i++)
        {
            CPPUNIT_ASSERT (it->first  == values1[i]);
            CPPUNIT_ASSERT (it->second == values2[i]);

            nbItems++;
        }

        CPPUNIT_ASSERT (nbItems > 0);
        CPPUNIT_ASSERT (nbItems == min (sizeof(values1)/sizeof(values1[0]), sizeof(values2)/sizeof(values2[0])));
    }
};

/********************************************************************************/

CPPUNIT_TEST_SUITE_REGISTRATION (TestIterators);

/********************************************************************************/
} } /* end of namespaces. */
/********************************************************************************/