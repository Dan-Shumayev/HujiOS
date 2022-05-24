#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#include <algorithm> // min()
#include <cmath> // abs()


/**
 * This function clears a single table held inside the physical memory.
 *
 * @param frameIndex The index of the frame to initialize with zeroes.
 */
void clearTable(uint64_t frameIndex)
{
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
    return virtualAddress & ((1ULL << OFFSET_WIDTH) - 1);
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
uint64_t fetchDepthOffset(uint64_t address, uint64_t depth)
{
    auto shiftRightByOffsetWidth = (TABLES_DEPTH - depth - 1) * OFFSET_WIDTH;
    auto fetchedPageByShift = address >> shiftRightByOffsetWidth;

    return fetchOffset(fetchedPageByShift);
}

void evict(uint64_t page, int &maxCycDist, uint64_t &pageToEvict, word_t &evictedFrame,
           uint64_t &unusedMappedFrameAdd, uint64_t addressInParent = 0, uint64_t depth = 0,
           word_t currFrame = 0, uint64_t byPath = 0)
{
    if (depth == TABLES_DEPTH) // It's a leaf ==> actual page ==> cyclicDistance
    {
        auto p = byPath;
        auto absDelta = abs(static_cast<int>(page - p));
        auto cycDist = std::min((int)NUM_PAGES - absDelta, absDelta);

        if (cycDist > maxCycDist)
        {
            evictedFrame = currFrame;
            unusedMappedFrameAdd = addressInParent;
            pageToEvict = p;

            maxCycDist = cycDist;
        }

        return;
    }

    word_t nextFrame;

    for (word_t rowIdx = 0; rowIdx < PAGE_SIZE; ++rowIdx)
    {
        PMread(currFrame * PAGE_SIZE + rowIdx, &nextFrame);

        if (nextFrame != 0)
        {
            evict(page, maxCycDist, pageToEvict, evictedFrame, unusedMappedFrameAdd,
                  currFrame * PAGE_SIZE + rowIdx, depth + 1,
                  nextFrame, (byPath << OFFSET_WIDTH) + rowIdx);
        }
    }
}

/**
 *
 * @param currMaxFrame
 * @param currFrame
 * @param currDepth
 * @return
 */
void getMaxUsedFrame(word_t &currMaxFrame, word_t currFrame = 0, uint64_t currDepth = 0)
{
    if (currDepth == TABLES_DEPTH)
    {
        return;
    }

    word_t nextFrame;

    for (word_t frameRow = 0; frameRow < PAGE_SIZE; ++frameRow)
    {
        PMread(currFrame * PAGE_SIZE + frameRow, &nextFrame);

        if (nextFrame != 0)
        {
            if (nextFrame > currMaxFrame)
            {
                currMaxFrame = nextFrame;
            }

            getMaxUsedFrame(currMaxFrame, nextFrame, currDepth + 1);
        }
    }
}

void getUnusedFrame(word_t &unusedFrame, word_t usedNullifiedFrame = 0, uint64_t currDepth = 0, word_t currFrame = 0,
                    uint64_t cameFromAddress = 0)
{
    if (currDepth == TABLES_DEPTH)
    {
        return;
    }

    word_t childFrame;
    bool reached = false;

    for (word_t rowIdx = 0; rowIdx < PAGE_SIZE; ++rowIdx)
    {
        PMread(currFrame * PAGE_SIZE + rowIdx, &childFrame);

        if (childFrame != 0)
        {
            reached = true;

            auto parent = currFrame;
            getUnusedFrame(unusedFrame, usedNullifiedFrame, currDepth + 1, childFrame,
                           parent * PAGE_SIZE + rowIdx);
        }
    }

    if (!reached && currFrame != usedNullifiedFrame)
    {
        PMwrite(cameFromAddress, 0);
        unusedFrame = currFrame;
    }
}

/**
 *
 * @param currFrame
 * @return
 */
word_t createFrame(uint64_t page, word_t isUsed, uint64_t depth)
{
    word_t frame = 0;
    getUnusedFrame(frame, isUsed); // O(n)

    if (frame)
    {
        return frame;
    }
    getMaxUsedFrame(frame); // O(n)

    if (frame < NUM_FRAMES - 1)
    {
        return frame + 1;
    }

    /** Evict stuff: */
    word_t maxCycDist = 0;
    uint64_t pageToEvict = 0;
    word_t evictedFrame = 0;
    uint64_t mappedFromAddress = 0;

    evict(page, maxCycDist, pageToEvict, evictedFrame, mappedFromAddress);

    PMevict(evictedFrame, pageToEvict);
    if (depth < static_cast<uint64_t>(TABLES_DEPTH) - 1) {
        clearTable(evictedFrame); // Mid-level page-fault
    }
    PMwrite(mappedFromAddress, 0);

    return evictedFrame;
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

    for (auto currDepth = 0; currDepth < TABLES_DEPTH; ++currDepth)
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
    if (virtualAddress >= VIRTUAL_MEMORY_SIZE) // TODO - what if virtualAddress = 0
    {
        return 0;
    }

    uint64_t frameIndex = findFrame(fetchPageAddress(virtualAddress));
    uint64_t offset = fetchOffset(virtualAddress);

    auto physicalAddress = frameIndex * PAGE_SIZE + offset;
    PMwrite(physicalAddress, value);

    return 1;
}