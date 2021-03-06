/*
 * Copyright (c) 2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX OR PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* ################################################################################
 * #
 * # PATENT NOTICE
 * #
 * # This software is distributed under the BSD 2-clause License (see LICENSE
 * # file).  This BSD License does not make any patent claims and as such, does
 * # not act as a patent grant.  The purpose of this section is for each contributor
 * # to define their intentions with respect to intellectual property.
 * #
 * # Each contributor to this source code is encouraged to state their patent
 * # claims and licensing mechanisms for any contributions made. At the end of
 * # this section contributors may each make their own statements.  Contributor's
 * # claims and grants only apply to the pieces (source code, programs, text,
 * # media, etc) that they have contributed directly to this software.
 * #
 * # There is no guarantee that this section is complete, up to date or accurate. It
 * # is up to the contributors to maintain their portion of this section and up to
 * # the user of the software to verify any claims herein.
 * #
 * # Do not remove this header notification.  The contents of this section must be
 * # present in all distributions of the software.  You may only modify your own
 * # intellectual property statements.  Please provide contact information.
 *
 * - Palo Alto Research Center, Inc
 * This software distribution does not grant any rights to patents owned by Palo
 * Alto Research Center, Inc (PARC). Rights to these patents are available via
 * various mechanisms. As of January 2016 PARC has committed to FRAND licensing any
 * intellectual property used by its contributions to this software. You may
 * contact PARC at cipo@parc.com for more information or visit http://www.ccnx.org
 */

#include "ns3/test.h"
#include "ns3/ccnx-codec-fixedheader.h"
#include "ns3/ccnx-time.h"
#include "ns3/ccnx-interestlifetime.h"
#include "../../TestMacros.h"

using namespace ns3;
using namespace ns3::ccnx;

namespace TestCCNxCodecFixedHeader {

BeginTest (TestGetSerializedSize)
{
  printf ("TestGetSerializedSize DoRun\n");
  CCNxCodecFixedHeader codec;
  size_t test1 = codec.GetSerializedSize ();
  NS_TEST_EXPECT_MSG_EQ (test1, 8, "wrong size");

  Ptr<CCNxInterestLifetime> interestLifetime1 = Create<CCNxInterestLifetime> (Create<CCNxTime>(3600));
  codec.GetPerHopHeader()->AddHeader(interestLifetime1);

  Ptr<CCNxInterestLifetime> interestLifetime2 = Create<CCNxInterestLifetime> (Create<CCNxTime>(1200));
  codec.GetPerHopHeader()->AddHeader(interestLifetime2);

  // expected size = 8 (Fixed header size) + 4 (TL) + 2 (bytes needed to accommodate time value)
  size_t expectedSize = 8 + (4 + 2) * 2;

  size_t test2 = codec.GetSerializedSize();
  NS_TEST_EXPECT_MSG_EQ (test2, expectedSize, "wrong size");

  codec.Print(std::cout);
  std::cout<<"\n"<<std::endl;
}
EndTest ()

BeginTest (TestSerialize)
{
  printf ("TestSerialize DoRun\n");

  CCNxCodecFixedHeader codec;

  Ptr<CCNxFixedHeader> fh = Create<CCNxFixedHeader> (1, CCNxFixedHeaderType_Interest, 300, 255, 3, 20);
  codec.SetFixedHeader (fh);

  Ptr<CCNxInterestLifetime> interestLifetime1 = Create<CCNxInterestLifetime> (Create<CCNxTime>(3600));
  codec.GetPerHopHeader()->AddHeader(interestLifetime1);

  Ptr<CCNxInterestLifetime> interestLifetime2 = Create<CCNxInterestLifetime> (Create<CCNxTime>(1200));
  codec.GetPerHopHeader()->AddHeader(interestLifetime2);

  Buffer buffer (0);
  buffer.AddAtStart (codec.GetSerializedSize ());
  codec.Serialize (buffer.Begin ());

  NS_TEST_EXPECT_MSG_EQ (buffer.GetSize (), codec.GetSerializedSize (), "Wrong size");

  const uint8_t truth[] = {
    0x01, 0x01, 0x01, 0x2c, 0xff, 0x03, 0x00, 0x14,
    // T_INT_LIFE
    0, 1, 0, 2,
    0xE, 0x10,
    0, 1, 0, 2,
    0x4, 0xB0
  };

  uint8_t test[sizeof(truth)];
  buffer.CopyData (test, sizeof(truth));

  hexdump ("truth", sizeof(truth), truth);
  hexdump ("test ", sizeof(test), test);

  NS_TEST_EXPECT_MSG_EQ (memcmp (truth, test, sizeof(truth)), 0, "Data in buffer wrong");
}
EndTest ()

BeginTest (TestDeserialize)
{
  printf ("TestDeserialize DoRun\n");

  CCNxCodecFixedHeader codec;

  // serialize it then make sure we got what we expected
  Ptr<CCNxFixedHeader> truth = Create<CCNxFixedHeader> (1, CCNxFixedHeaderType_Interest, 300, 255, 3, 20);
  codec.SetFixedHeader (truth);

  Ptr<CCNxInterestLifetime> interestLifetime1 = Create<CCNxInterestLifetime> (Create<CCNxTime>(3600));
  codec.GetPerHopHeader()->AddHeader(interestLifetime1);

  Ptr<CCNxInterestLifetime> interestLifetime2 = Create<CCNxInterestLifetime> (Create<CCNxTime>(1200));
  codec.GetPerHopHeader()->AddHeader(interestLifetime2);

  Buffer buffer (0);
  buffer.AddAtStart (codec.GetSerializedSize ());
  codec.Serialize (buffer.Begin ());

  CCNxCodecFixedHeader codectest;
  codectest.Deserialize (buffer.Begin ());

  Ptr<CCNxFixedHeader> test1 = codectest.GetFixedHeader ();
  NS_TEST_EXPECT_MSG_EQ (truth->Equals (test1), true, "Data in buffer wrong");

  bool equal = codec.GetPerHopHeader()->Equals(codectest.GetPerHopHeader());
  NS_TEST_EXPECT_MSG_EQ (equal, true, "Data in buffer wrong");
}
EndTest ()

/**
 * @ingroup ccnx-test
 *
 * Test Suite for CCNxCodecFixedHeader
 */
static class TestSuiteCCNxCodecFixedHeader : public TestSuite
{
public:
  TestSuiteCCNxCodecFixedHeader () : TestSuite ("ccnx-codec-fixedheader", UNIT)
  {
    AddTestCase (new TestGetSerializedSize (), TestCase::QUICK);
    AddTestCase (new TestSerialize (), TestCase::QUICK);
    AddTestCase (new TestDeserialize (), TestCase::QUICK);
  }
} g_TestSuiteCCNxCodecFixedHeader;

} // namespace

