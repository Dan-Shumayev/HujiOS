#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#include <algorithm> // min()
#include <cmath> // abs()


/** ** Helper functions ** */

/**
 * This function clears a single table held inside the physical memory.
 *
 * @param frameIndex The index of the frame to initialize with zeroes.
 */
void clearTable(uint64_t frameIndex)
{
    // a constant should be defined more clearly, e.g
    // const uint64_t ROOT_FRAME_PHYS_ADDR = 0;
    auto rootFrame = 0;

    for (auto i = 0; i < PAGE_SIZE; ++i)
    {
        PMwrite(frameIndex * PAGE_SIZE + i, rootFrame);
    }
}

/**
 *
 * @param virtualAddress
 * @return
 */
uint64_t fetchOffset(uint64_t virtualAddress)
{
    // I think it's better to have here `uint64_t` instead of auto
    // Generally useful when doing casting and bitwise operations on numbers,
    // to make sure that 'auto' doesn't use a less precise numeric type.
    // (Though I don't believe it's a problem here)
    auto maskedBits = (1ULL << OFFSET_WIDTH) - 1;

    return virtualAddress & maskedBits;
}

/**
 *
 * @param virtualAddress
 * @return
 */
uint64_t fetchPageAddress(uint64_t virtualAddress) { return virtualAddress >> OFFSET_WIDTH; }

/** .... All offsets are of the same size ...
 *
 * @param address
 * @param depth
 * @return
 */
uint64_t fetchDepthOffset(uint64_t &address, uint64_t &depth)
{
    // See my previous comment about using uint64_t instead of auto
    // The name 'shiftRightByOffsetWidth' is bad, obvious I see in  a line below
    // that you use this variable for shifting right..
    // Think of a more succcessful name that explains what this offset does,
    auto shiftRightByOffsetWidth = (TABLES_DEPTH - depth - 1) * OFFSET_WIDTH;
    // Also a bad name
    auto fetchedPageByShift = address >> shiftRightByOffsetWidth;

    return fetchOffset(fetchedPageByShift);
}

/**
 * Function signature is too big
 * @param page
 * @param maxCycDist
 * @param addressInParent
 * @param currFrame
 * @param currLeafPage
 * @param evictedAddressInParent
 * @param evictedFrame
 * @param pageToEvict
 */
void computeAndUpdateMaxCycDist(uint64_t page, int &maxCycDist, uint64_t addressInParent, word_t currFrame,
                                uint64_t currLeafPage, uint64_t &evictedAddressInParent, word_t &evictedFrame,
                                uint64_t &pageToEvict)
{
    // use std::abs for consistency
    auto absDelta = abs(static_cast<int>(page - currLeafPage));
    auto cycDist = std::min(static_cast<int>NUM_PAGES - absDelta, absDelta);

    if (cycDist > maxCycDist)
    {
        maxCycDist = cycDist;

        evictedFrame = currFrame;
        pageToEvict = currLeafPage;
        evictedAddressInParent = addressInParent;
    }
}

/** Function signature is too big */
void findFrameToEvict(uint64_t page, int &maxCycDist, uint64_t &pageToEvict, word_t &evictedFrame,
                      uint64_t &evictedAddressInParent, uint64_t addressInParent = 0, uint64_t depth = 0,
                      word_t currFrame = 0, uint64_t currLeafPage = 0)
{
    if (depth == TABLES_DEPTH) // It's a leaf ==> actual page ==> cyclicDistance
    {
        computeAndUpdateMaxCycDist(page, maxCycDist, addressInParent, currFrame, currLeafPage,
                                   evictedAddressInParent, evictedFrame, pageToEvict);
        return;
    }

    word_t nextFrame;

    for (word_t rowIdx = 0; rowIdx < PAGE_SIZE; ++rowIdx)
    {
        auto physicalAddress = currFrame * PAGE_SIZE + rowIdx;
        PMread(physicalAddress, &nextFrame);

        if (nextFrame != 0)
        {
            findFrameToEvict(page, maxCycDist, pageToEvict, evictedFrame, evictedAddressInParent,
                             physicalAddress, depth + 1,nextFrame,
                             (currLeafPage << OFFSET_WIDTH) + rowIdx);
        }
    }
}



/**
 * - Function signature is too big
   - Function is called "getXXX" yet it doesn't return anything
     
 
 * @param unusedFrame
 * @param unusedAddressInParent
 * @param maxFrame
 * @param usedEmptyFrame
 * @param currDepth
 * @param currFrame
 * @param cameFromPhysicalAddress
 */
void getUnusedOrMaxFrame(word_t &unusedFrame, uint64_t &unusedAddressInParent, word_t &maxFrame,
                         word_t usedEmptyFrame = 0, uint64_t currDepth = 0,
                         word_t currFrame = 0, uint64_t cameFromPhysicalAddress = 0)
{
    if (currDepth == TABLES_DEPTH) // No more nodes in this branch
    {
        return;
    }

    word_t childFrame;
    bool hasChildren = false;

    for (word_t rowIdx = 0; rowIdx < PAGE_SIZE; ++rowIdx)
    {
        auto physicalAddress = currFrame * PAGE_SIZE + rowIdx;
        PMread(physicalAddress, &childFrame);

        if (childFrame != 0)
        {
            maxFrame = maxFrame < childFrame ? childFrame : maxFrame;
            hasChildren = true;

            getUnusedOrMaxFrame(unusedFrame, unusedAddressInParent, maxFrame, usedEmptyFrame,
                                currDepth + 1, childFrame,physicalAddress);
        }
    }

    if (!hasChildren && currFrame != usedEmptyFrame)
    {
        unusedAddressInParent = cameFromPhysicalAddress;
        unusedFrame = currFrame;
    }
}

/**
 *
 * @param formerChosenUnusedFrame
 * @return
 */
word_t createUnusedOrMaxFrame(word_t formerChosenUnusedFrame)
{
    word_t unusedFrame = 0;
    uint64_t physicalAddressInParent = 0;

    word_t maxFrame = 0;

    getUnusedOrMaxFrame(unusedFrame, physicalAddressInParent, maxFrame,
                        formerChosenUnusedFrame);

    if (unusedFrame)
    {
        PMwrite(physicalAddressInParent, 0); // Remove edge from old parent
        return unusedFrame;
    }

    if (maxFrame < NUM_FRAMES - 1)
    {
        return maxFrame + 1;
    }

    return 0; // No unused nor max available ==> Goto findFrameToEvict()
}

/**
 *
 * @param page
 * @param depthOfFrame
 * @return
 */
word_t evictFrame(uint64_t page, uint64_t depthOfFrame)
{
    word_t maxCycDist = 0;
    word_t evictedFrame = 0;
    uint64_t pageToEvict = 0;
    uint64_t mappedFromAddress = 0;

    findFrameToEvict(page, maxCycDist, pageToEvict, evictedFrame, mappedFromAddress);

    PMevict(evictedFrame, pageToEvict);
    PMwrite(mappedFromAddress, 0); // Remove edge from old parent

    if (depthOfFrame < static_cast<uint64_t>(TABLES_DEPTH) - 1) // If it's a mid-level page, clear it!
    {
        clearTable(evictedFrame); // Mid-level page-fault
    }

    return evictedFrame;
}

/**
 *
 */
word_t createFrame(uint64_t page, word_t formerChosenUnusedFrame, uint64_t depthOfFrame)
{
    auto maxOrUnusedFrame = createUnusedOrMaxFrame(formerChosenUnusedFrame);

    return maxOrUnusedFrame ? maxOrUnusedFrame : evictFrame(page, depthOfFrame);
}

/** This function finds a frame associated with the given virtual address.
 *
 * @param pageAddress
 * @return
 */
uint64_t findFrame(uint64_t pageAddress)
{
    word_t currFrame = 0;
    word_t nextFrame;
    auto currOffset = 0ULL;

    for (uint64_t currDepth = 0; currDepth < TABLES_DEPTH; ++currDepth)
    {
        currOffset = fetchDepthOffset(pageAddress, currDepth);
        auto physicalAddress = currFrame * PAGE_SIZE + currOffset;

        PMread(physicalAddress, &nextFrame);

        if (nextFrame == 0) // hit an empty frame
        {
            nextFrame = createFrame(pageAddress, currFrame, currDepth);
            PMwrite(physicalAddress, nextFrame);
        }

        currFrame = nextFrame;
    }

    // It's a no-op in case the page is already in the RAM
    PMrestore(currFrame, pageAddress); // Content page-fault
    return currFrame;
}

/** ** Paging interface ** */

/**
 * Initialize the virtual memory.
 */
void VMinitialize()
{
    for (auto i = 0; i < NUM_FRAMES; ++i)
    {
        clearTable(i);
    }
}

/** This function reads a word from the given virtual address.
 *
 * @virtualAddress The virtual address to read a value from its respective physical memory.
 * @value The value to write the read value to.
 *
 * @return 1 iff the write operation has been successfully done, 0 otherwise.
 */
int VMread(uint64_t virtualAddress, word_t* value)
{
    if (virtualAddress >= VIRTUAL_MEMORY_SIZE)
    {
        return 0;
    }

    uint64_t frameIndex = findFrame(fetchPageAddress(virtualAddress));
    uint64_t offset = fetchOffset(virtualAddress);

    auto physicalAddress = frameIndex * PAGE_SIZE + offset;

    PMread(physicalAddress, value);

    return 1;
}

/** This function writes a word to the given virtual address.
 *
 * @virtualAddress The virtual address to write a value to its respective physical memory.
 * @value The value to written at that address.
 *
 * @return 1 iff the write operation has been successfully done, 0 otherwise.
 */
int VMwrite(uint64_t virtualAddress, word_t value)
{
    if (virtualAddress >= VIRTUAL_MEMORY_SIZE)
    {
        return 0;
    }

    uint64_t frameIndex = findFrame(fetchPageAddress(virtualAddress));
    uint64_t offset = fetchOffset(virtualAddress);

    auto physicalAddress = frameIndex * PAGE_SIZE + offset;

    PMwrite(physicalAddress, value);

    return 1;
}
