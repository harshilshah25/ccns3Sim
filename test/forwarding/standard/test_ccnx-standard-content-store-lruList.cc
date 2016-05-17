/*
 * Copyright (c) 2016, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Patent rights are not granted under this agreement. Patent rights are
 *       available under FRAND terms.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX or PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ns3/test.h"
#include "ns3/ccnx-standard-content-store-lruList.h"

#include "../../TestMacros.h"

using namespace ns3;
using namespace ns3::ccnx;

namespace TestSuiteCCNxStandardContentStoreLruList {


#define MAX_ENTRIES 10000

typedef std::vector<Ptr<CCNxStandardContentStoreEntry> > CCNxEntryVectorType;

static CCNxEntryVectorType CCNxEntryVector(MAX_ENTRIES);
static const char * contentString = "ccnx:/name=foo/name=sink/name=kitchen";


static Ptr<CCNxStandardContentStoreLruList>
CreateLruList ()
{
  Ptr<CCNxStandardContentStoreLruList> lruList = Create<CCNxStandardContentStoreLruList> ();
  return lruList;
}


static Ptr<CCNxPacket>
CreatePacket (uint32_t size, Ptr<CCNxName> name, CCNxMessage::MessageType msgType)
{
  Ptr<CCNxBuffer> payload = Create<CCNxBuffer> (size, true);
  Ptr<CCNxPacket> packet;

  switch (msgType)
    {
    case CCNxMessage::ContentObject:
      {
        Ptr<CCNxContentObject> content = Create<CCNxContentObject> (name, payload);
        packet = CCNxPacket::CreateFromMessage (content);
        break;
      }
    case  CCNxMessage::Interest:
      {
        Ptr<CCNxInterest> interest = Create<CCNxInterest> (name, payload);
        packet = CCNxPacket::CreateFromMessage (interest);
        break;
      }

    default:
      {
        NS_ASSERT_MSG (false, "Unsupported msg type: " << msgType);
        Ptr<CCNxInterest> wtf = Create<CCNxInterest> (name, payload);
        packet = CCNxPacket::CreateFromMessage (wtf);
        break;
      }
    }
  return packet;

}




static void CreateEntries(int entryCount)
{
uint32_t size=128;

for (int i=0; i< entryCount ; i++)
  {
    std::ostringstream itoa; itoa << i;
    std::string nameString = contentString + itoa.str();
    Ptr<CCNxName> name = Create<CCNxName> (nameString);
    Ptr<CCNxPacket> packet = CreatePacket (size, name, CCNxMessage::ContentObject);
    CCNxEntryVector[i] = Create<CCNxStandardContentStoreEntry>(packet);
  }

    return ;

}

BeginTest (Constructor)
{
  printf ("TestCCNxStandardContentStoreLruListConstructor DoRun\n");
  LogComponentEnable ("CCNxStandardContentStoreLruList", LOG_LEVEL_DEBUG);

}
EndTest ()




BeginTest (AddEntries)
{
  //AddEntries, see that they are present and in order
  printf ("TestCCNxStandardContentStoreLruList_AddEntries DoRun\n");

  int entryCount = 10;

  CreateEntries (entryCount);

  Ptr<CCNxStandardContentStoreLruList> dut = CreateLruList ();

  for (int i=0; i<entryCount; i++)
    {
      dut->AddEntry(CCNxEntryVector[i]);

    }
  NS_TEST_EXPECT_MSG_EQ (dut->GetSize(), entryCount, "wrong LRU list size !");
  NS_TEST_EXPECT_MSG_EQ (dut->GetBackEntry(), CCNxEntryVector[0], "back entry incorrect !");
  NS_TEST_EXPECT_MSG_EQ (dut->GetFrontEntry(), CCNxEntryVector[entryCount-1], "front entry incorrect !");


}
EndTest ()

BeginTest (RefreshEntry)
{
  // add existing entry, see that it is refreshed
  printf ("TestCCNxStandardContentStoreLruList_RefreshEntry DoRun\n");

  int entryCount = 3;

  CreateEntries (entryCount);

  Ptr<CCNxStandardContentStoreLruList> dut = CreateLruList ();

  for (int i=0; i<entryCount; i++)
    {
      dut->AddEntry(CCNxEntryVector[i]);

    }

  dut->AddEntry(CCNxEntryVector[0]);

  NS_TEST_EXPECT_MSG_EQ (dut->GetSize(), entryCount, "wrong LRU list size !");
  NS_TEST_EXPECT_MSG_EQ (dut->GetBackEntry(), CCNxEntryVector[1], "back entry incorrect !");
  NS_TEST_EXPECT_MSG_EQ (dut->GetFrontEntry(), CCNxEntryVector[0], "front entry incorrect !");


}
EndTest ()


BeginTest (DeleteEntry)
{
  // delete entry
  printf ("TestCCNxStandardContentStoreLruList_DeleteEntry DoRun\n");

  int entryCount = 3;

  CreateEntries (entryCount);

  Ptr<CCNxStandardContentStoreLruList> dut = CreateLruList ();

  for (int i=0; i<entryCount; i++)
    {
      dut->AddEntry(CCNxEntryVector[i]);

    }

  dut->DeleteEntry(CCNxEntryVector[1]);

  NS_TEST_EXPECT_MSG_EQ (dut->GetSize(), entryCount-1, "wrong LRU list size !");
  NS_TEST_EXPECT_MSG_EQ (dut->GetBackEntry(), CCNxEntryVector[0], "back entry incorrect !");
  NS_TEST_EXPECT_MSG_EQ (dut->GetFrontEntry(), CCNxEntryVector[2], "front entry incorrect !");

}
EndTest ()


BeginTest (DeleteNonExistentEntry)
{
  // delete entry that was not added to list
  printf ("TestCCNxStandardContentStoreLruList_DeleteNonExistentEntry DoRun\n");

  int entryCount = 10;

  CreateEntries (entryCount);

  Ptr<CCNxStandardContentStoreLruList> dut = CreateLruList ();

  int addedCount = entryCount -1;

  for (int i=0; i<addedCount; i++)
    {
      dut->AddEntry(CCNxEntryVector[i]);

    }

  dut->DeleteEntry(CCNxEntryVector[entryCount-1]);

  NS_TEST_EXPECT_MSG_EQ (dut->GetSize(), addedCount, "wrong LRU list size !");

}
EndTest ()

BeginTest (ScaleTest)
{
  // add 10K MAX_ENTRIES, refresh random MAX_ENTRIES,delete all MAX_ENTRIES, see no mem leak
  printf ("TestCCNxStandardContentStoreLruList_ScaleTest DoRun\n");

  int entryCount = MAX_ENTRIES;

  CreateEntries (entryCount);

  Ptr<CCNxStandardContentStoreLruList> dut = CreateLruList ();

  int addedCount = entryCount;

  for (int i=0; i<addedCount; i++)
    {
      dut->AddEntry(CCNxEntryVector[i]);

    }
  NS_TEST_EXPECT_MSG_EQ (dut->GetSize(), addedCount, "wrong LRU list size !");

  for (int i=0; i<addedCount; i++)
    {
      dut->DeleteEntry(CCNxEntryVector[i]);

    }

  NS_TEST_EXPECT_MSG_EQ (dut->GetSize(), 0, "wrong LRU list size !");



}
EndTest ()

/**
 * @ingroup ccnx-test
 *
 * Test Suite for CCNxStandardContentStoreLruList
 */
static class TestSuiteCCNxStandardContentStoreLruList : public TestSuite
{
public:
  TestSuiteCCNxStandardContentStoreLruList () : TestSuite ("ccnx-standard-content-store-lruList", UNIT)
  {
    AddTestCase (new Constructor (), TestCase::QUICK);
    AddTestCase (new AddEntries (), TestCase::QUICK);
    AddTestCase (new RefreshEntry (), TestCase::QUICK);
    AddTestCase (new DeleteEntry (), TestCase::QUICK);
    AddTestCase (new DeleteNonExistentEntry (), TestCase::QUICK);
    AddTestCase (new ScaleTest (), TestCase::QUICK);

  }
} g_TestSuiteCCNxStandardContentStoreLruList;


} // namespace TestSuiteCCNxStandardContentStoreLruList



