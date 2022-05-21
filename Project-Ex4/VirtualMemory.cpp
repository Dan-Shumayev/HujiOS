#include "VirtualMemory.h"
#include "PhysicalMemory.h"

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

/**
 *
 * @return
 */
word_t createFrame(word_t currFrame, uint64_t depth = 0, word_t currMaxFrame = 0)
{ // TODO - to be validated and to implement evict()
    // TODO - currMaxFrame - we must introduce a new function to traverse the entire tree to find
    //    the max used frame, as in this function we're traversing only a sub-tree
    if (depth == TABLES_DEPTH)
    {
        return currMaxFrame < NUM_FRAMES - 1 ? currMaxFrame + 1 : evict();
    }

    word_t unusedFrame = 0;
    word_t rowValue;

    for (word_t rowIdx = 0; rowIdx < PAGE_SIZE; ++rowIdx)
    {
        PMread(currFrame + rowIdx, &rowValue);

        if (rowValue != 0)
        {
            currMaxFrame =  currMaxFrame < rowValue ? rowValue : currMaxFrame;

            unusedFrame = createFrame(rowValue, depth + 1, currMaxFrame);
        }
    }

    if (unusedFrame == 0) // TODO - check if necessary / refactor
    {
        return currMaxFrame < NUM_FRAMES - 1 ? currMaxFrame + 1 : evict();
    }

    return 0;
}

/** This function finds a frame associated with the given virtual address.
 *
 * @param pageAddress
 * @return
 */
uint64_t findFrame(uint64_t pageAddress)
{
    word_t currFrame = 0;
    auto currOffset = 0ULL;
    word_t nextFrame;

    for (auto currDepth = 0; currDepth < TABLES_DEPTH; ++currDepth)
    {
        currOffset = fetchDepthOffset(pageAddress, currDepth);

        PMread(currFrame * PAGE_SIZE + currOffset, &nextFrame);

        if (nextFrame == 0) // hit an empty frame
        {
            nextFrame = createFrame(currFrame, currDepth);
            PMwrite(currFrame * PAGE_SIZE + currOffset, nextFrame);
        }

        currFrame = nextFrame;
    }

    // It's a no-op in case the page is already in the RAM
    PMrestore(currFrame, pageAddress);

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