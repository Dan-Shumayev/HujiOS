#include "VirtualMemory.h"
#include "PhysicalMemory.h"

typedef struct result // TODO
{
    uint64_t IdxFrame;
    uint64_t frameFather;
    uint64_t pageNum;
    uint64_t offset;
    int frameDepth;
} result;

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

uint64_t fetchOffset(uint64_t virtualAddress)
{
    return virtualAddress & ((1ULL << OFFSET_WIDTH) - 1);
}

uint64_t fetchPageAddress(uint64_t virtualAddress) { return virtualAddress >> OFFSET_WIDTH; }

/**
 * This function calculates the current offset according to a given address and depth.
 *
 * @param virtualAddress ...
 * @param depth ...
 * @return ...
 */
uint64_t fetchDepthOffset(uint64_t virtualAddress, int depth) // TODO
{
    return fetchOffset(((virtualAddress >> ((TABLES_DEPTH - depth - 1) * OFFSET_WIDTH))));
}

/**
 * This function calculates the cyclic distance between two pages.
 *
 * @param pageSwappedIn - the original page ...
 * @param p - the currently checked page ...
 * @return the cyclic distance ...
 */
uint64_t cyclicDistance(uint64_t pageSwappedIn, uint64_t p) // TODO
{
    auto absVar = p > pageSwappedIn ? p - pageSwappedIn : pageSwappedIn - p;
    auto v1 = NUM_PAGES - absVar;

    return v1 <= absVar ? v1 : absVar;
}

/**
 * This function gets a result struct and performs all the needed implementations related to it.
 *
 * @maxCycResult ...
 */
void doEvict(result &maxCycResult) // TODO
{
    PMwrite(maxCycResult.frameFather * PAGE_SIZE + maxCycResult.offset,0);
    // write zero in the father of the evicted frame, to unlink the evicted son from his father.
    PMevict(maxCycResult.IdxFrame, maxCycResult.pageNum);
    clearTable(maxCycResult.IdxFrame);
}

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

/**
 * This function searches for frame using dfs algorithm
 * @param originFrameIdx- the original frame
 * @param pageToCheck - the original page number
 * @return
 */
uint64_t getUnusedFrame(uint64_t originFrameIdx, uint64_t pageToCheck) // TODO
{
    result maxCycResult  {0,0,0,0,0};
    result arrFrames[TABLES_DEPTH * PAGE_SIZE]; //arr that holds the current frame and it's suns
    result *startFrame = arrFrames;
    *startFrame = result{0, 0, 0, 0, 0};
    ++startFrame;
    uint64_t maxFrame = 0;
    uint64_t maxCyc = 0;
    uint64_t curDist = 0;

    while(arrFrames != startFrame) // as long as the arr holds frames
    {
        --startFrame;
        result frameStruct = *startFrame;
        bool emptyTable = true;

        if (frameStruct.IdxFrame >= maxFrame)
        {
            maxFrame = frameStruct.IdxFrame;
        }
        if (frameStruct.frameDepth == TABLES_DEPTH)
        {
            curDist = cyclicDistance(pageToCheck, frameStruct.pageNum); // calc this frames
            //cyc dist
            if (curDist > maxCyc)
            {
                maxCyc = curDist;
                maxCycResult = frameStruct;
            }
            continue;
        }

        for(uint64_t i = 0; i < PAGE_SIZE; ++ i)
        {
            word_t childFrame = 0;
            PMread(frameStruct.IdxFrame * PAGE_SIZE + i, &childFrame);

            if (childFrame != 0) // frame has child
            {
                emptyTable = false; // cur table is not empty

                int depthToAdd = frameStruct.frameDepth + 1;
                uint64_t pageToAdd = (frameStruct.pageNum << OFFSET_WIDTH) + i;
                *startFrame = result{(uint64_t)childFrame, frameStruct.IdxFrame, pageToAdd, i,
                                     depthToAdd};
                ++ startFrame ;
            }
        }
        if ((frameStruct.IdxFrame != originFrameIdx) && emptyTable)
        {
            PMwrite(frameStruct.frameFather * PAGE_SIZE + frameStruct.offset,
                    0); //write zero
            uint64_t foundFrameIdx = frameStruct.IdxFrame;
            return foundFrameIdx;
        }
    }

    if (NUM_FRAMES > maxFrame + 1) // not all frames are used
    {
        uint64_t foundFrameIdx = maxFrame + 1;
        return foundFrameIdx;
    }
    else // we can't find unused frame, we need to evict
    {
        doEvict(maxCycResult);
        uint64_t foundFrameIdx = maxCycResult.IdxFrame;
        return foundFrameIdx;
    }
}

/** This function finds a frame associated with the given virtual address.
 * ...
 *
 * @param virtualAddress - the virtual address to find a frame for.
 * @return ...
 */
uint64_t findFrame(uint64_t virtualAddress) // TODO
{
    auto page = fetchPageAddress(virtualAddress);

    auto currFrame = 0ULL;
    auto currOffset = 0ULL;
    word_t nextFrame;

    for (auto currDepth = 0; currDepth < TABLES_DEPTH; ++currDepth)
    {
        currOffset = fetchDepthOffset(page, currDepth);

        PMread(currFrame * PAGE_SIZE + currOffset, &nextFrame);

        if (nextFrame == 0) // hit an empty frame
        {
            nextFrame = getUnusedFrame(currFrame, page);
            PMwrite(currFrame * PAGE_SIZE + currOffset, nextFrame);
        }

        currFrame = nextFrame;
    }

    PMrestore(currFrame, page);
    return currFrame;
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

    uint64_t frameIndex = findFrame(virtualAddress);
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

    uint64_t frameIndex = findFrame(virtualAddress);
    uint64_t offset = fetchOffset(virtualAddress);

    auto physicalAddress = frameIndex * PAGE_SIZE + offset;
    PMwrite(physicalAddress, value);

    return 1;
}
