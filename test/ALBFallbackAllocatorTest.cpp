///////////////////////////////////////////////////////////////////
//
// Copyright 2014 Felix Petriconi
//
// License: http://boost.org/LICENSE_1_0.txt, Boost License 1.0
//
// Authors: http://petriconi.net, Felix Petriconi 
//
///////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>
#include "ALBFallbackAllocator.h"
#include "ALBTestHelpers.h"
#include "ALBStackAllocator.h"
#include "ALBMallocator.h"

using namespace ALB::TestHelpers;

class FallbackAllocatorTest : public 
  AllocatorBaseTest<
    ALB::FallbackAllocator<
      ALB::StackAllocator<32>, 
        ALB::Mallocator>>
{
protected:
  void SetUp() {
    mem = sut.allocate(1);
    StartPtrPrimary = mem.ptr;
    deallocateAndCheckBlockIsThenEmpty(mem);
  }

  void TearDown() {
    deallocateAndCheckBlockIsThenEmpty(mem);
  }

  void* StartPtrPrimary;
  ALB::Block mem;
};

TEST_F(FallbackAllocatorTest, ThatAllocatingZeroBytesReturnsAnEmptyBlock)
{
  mem = sut.allocate(0);
  EXPECT_FALSE(mem);
}

TEST_F(FallbackAllocatorTest, ThatAllocatingUpToTheLimitsOfThePrimaryIsDoneByThePrimary)
{
  mem = sut.allocate(32);
  EXPECT_EQ(32, mem.length);
  EXPECT_EQ(StartPtrPrimary, mem.ptr);
}

TEST_F(FallbackAllocatorTest, ThatAllocatingBeyondTheLimitsOfThePrimaryIsDoneByTheFallback)
{
  mem = sut.allocate(33);
  EXPECT_GE(33, mem.length);
  EXPECT_NE(StartPtrPrimary, mem.ptr);
}

TEST_F(FallbackAllocatorTest, ThatIncreasingReallocatingWithinTheLimitsOfThePrimaryIsHandledByThePrimary)
{
  mem = sut.allocate(8);
  ALB::TestHelpers::fillBlockWithReferenceData<int>(mem);

  EXPECT_TRUE(sut.reallocate(mem, 16));
  EXPECT_EQ(16, mem.length);
  EXPECT_EQ(StartPtrPrimary, mem.ptr);
  EXPECT_MEM_EQ(mem.ptr, (void*)ReferenceData.data(), 8);
}

TEST_F(FallbackAllocatorTest, ThatIncreasingReallocatingOfABlockOwnedByTheFallbackStaysAtTheFallback)
{
  mem = sut.allocate(64);
  ALB::TestHelpers::fillBlockWithReferenceData<int>(mem);

  EXPECT_TRUE(sut.reallocate(mem, 128));
  EXPECT_EQ(128, mem.length);
  EXPECT_NE(StartPtrPrimary, mem.ptr);
  EXPECT_MEM_EQ(mem.ptr, (void*)ReferenceData.data(), 64);
}


TEST_F(FallbackAllocatorTest, ThatDecreasingReallocatingOfAPrimaryOwnedBlockIsHandledByThePrimary)
{
  mem = sut.allocate(16);
  ALB::TestHelpers::fillBlockWithReferenceData<int>(mem);

  EXPECT_TRUE(sut.reallocate(mem, 8));
  EXPECT_EQ(8, mem.length);
  EXPECT_EQ(StartPtrPrimary, mem.ptr);
  EXPECT_MEM_EQ(mem.ptr, (void*)ReferenceData.data(), 8);
}

TEST_F(FallbackAllocatorTest, ThatIncreasingReallocatingBeyondTheLimitsOfThePrimaryIsHandledByTheFallback)
{
  mem = sut.allocate(8);
  ALB::TestHelpers::fillBlockWithReferenceData<int>(mem);

  EXPECT_TRUE(sut.reallocate(mem, 64));
  EXPECT_EQ(64, mem.length);
  EXPECT_NE(StartPtrPrimary, mem.ptr);
  EXPECT_MEM_EQ(mem.ptr, (void*)ReferenceData.data(), 8);
}


TEST_F(FallbackAllocatorTest, ThatDecreasingReallocatingOfAFallbackOwnedBlockIsHandledByTheFallback)
{
  mem = sut.allocate(64);
  ALB::TestHelpers::fillBlockWithReferenceData<int>(mem);

  EXPECT_TRUE(sut.reallocate(mem, 16));
  EXPECT_EQ(16, mem.length);
  EXPECT_NE(StartPtrPrimary, mem.ptr);
  EXPECT_MEM_EQ(mem.ptr, (void*)ReferenceData.data(), 16);
}

TEST_F(FallbackAllocatorTest, ThatExpandingOfABlockOwnedByThePrimaryWithinTheLimitsOfThePrimaryIsDone)
{
  mem = sut.allocate(16);
  ALB::TestHelpers::fillBlockWithReferenceData<int>(mem);

  EXPECT_TRUE(sut.expand(mem, 8));
  EXPECT_EQ(24, mem.length);
  EXPECT_EQ(StartPtrPrimary, mem.ptr);
  EXPECT_MEM_EQ(mem.ptr, (void*)ReferenceData.data(), 8);
}

TEST_F(FallbackAllocatorTest, ThatExpandingOfABlockOwnedByThePrimaryBeyondTheLimitsOfThePrimaryIsRejected)
{
  mem = sut.allocate(16);
  ALB::TestHelpers::fillBlockWithReferenceData<int>(mem);

  EXPECT_FALSE(sut.expand(mem, 64));
  EXPECT_EQ(16, mem.length);
  EXPECT_EQ(StartPtrPrimary, mem.ptr);
  EXPECT_MEM_EQ(mem.ptr, (void*)ReferenceData.data(), 16);
}

TEST_F(FallbackAllocatorTest, ThatExpandingOfABlockOwnedByTheFallbackIsRejectedBecauseItDoesNotSupportExpand)
{
  mem = sut.allocate(64);
  ALB::TestHelpers::fillBlockWithReferenceData<int>(mem);

  EXPECT_FALSE(sut.expand(mem, 64));
  EXPECT_EQ(64, mem.length);
  EXPECT_MEM_EQ(mem.ptr, (void*)ReferenceData.data(), 64);
}



