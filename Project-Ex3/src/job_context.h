#ifndef EX3_JOB_CONTEXT_H
#define EX3_JOB_CONTEXT_H

#include "MapReduceFramework.h" // MapReduce library API
#include "exceptions.h" // systemError, mapReduceLibraryError
#include <vector> // std::vector
#include "Barrier.h" // barrier
#include <memory> // std::unique_ptr
#include <atomic> // std::atomic
#include <map_reduce_utils.h> // ApplyMutex (RAII mutex)
#include <semaphore.h>
#include "thread_context.h"


class JobContext {
private:
    /** Client object's `map` and `reduce` routines.
        Note it's a reference to `client` as it's an abstract type, and there is no rationale in copying
        such a type, but only referring to its origin */
    const MapReduceClient& client_;

    /** Job-associated data */
    const InputVec& inputVec_; // isn't supposed to be modified
    OutputVec &outputVec_; // emit3's outputs
    /** First 2 bits indicates stage_t (4 options), next 31 bits indicate the
        number of processed elements at the current stage, next 31 bits indicate
        number of total elements to process at this stage */
    std::atomic<std::uint64_t> jobStateBitwise_;
    std::atomic<std::size_t> atomicNextItemToProcess_;

    /** Thread-management */
    size_t numOfThreads_; // amount of working threads
    std::vector<std::unique_ptr<ThreadContext>> threadContexts_; // working threads' context
    Barrier threadsBarrier_; // used to synchronize all threads in between each phase
    pthread_mutex_t jobStateMutex_; // mutex for job purposes
    pthread_mutex_t outputVecMutex_; // mutex for atomic update of the output vector

private:
    // mutex for exclusive update of the output vector
    std::vector<IntermediateVec> shuffledQueue_; // thread-0's data structure for shuffling
    bool areThreadsJoined_;
    pthread_mutex_t pthreadCreateMutex_;
public:
    pthread_mutex_t &getPthreadCreateMutex();

public:
    /** API functions */
    JobContext(const MapReduceClient& client, const InputVec& inputVec, OutputVec& outputVec, int numOfThreads);

    ~JobContext();

    JobContext(const JobContext&) = delete; // Rule of 5
    JobContext& operator=(const JobContext&) = delete; // Rule of 5
    JobContext(JobContext&&) = delete; // Rule of 5
    JobContext& operator=(JobContext&&) = delete; // Rule of 5

    ThreadContext& getThreadContext(size_t ix) {return *threadContexts_[ix];};

    size_t getNumOfThreads() const {return numOfThreads_;};

    const InputVec &getInputVector() const {return inputVec_;};

    size_t getNumOfInputPairs() const {return inputVec_.size();};

    void invokeClientMapRoutine(const K1* key, const V1* value, void* context) {client_.map(key, value, context);};

    void invokeClientReduceRoutine(const IntermediateVec* pairs, void* context) {client_.reduce(pairs, context);};

    void barrier() {threadsBarrier_.barrier();};

    void getJobDone();

    JobState getJobState();

    void setJobStage(stage_t currentStage);

    void updateOutputVector(OutputPair &&outputPair);

    std::vector<IntermediateVec>& getShuffledQueue() {return shuffledQueue_;};

    size_t getNumOfIntermediatePairs();

    void setNextPhase();

    size_t getNextItemToProcess();

    void resetAtomicNextItemToProcess() {atomicNextItemToProcess_.store(0);};

    void addNumberOfProcessedItems(stage_t currStage = UNDEFINED_STAGE, size_t amount = 1);

    /** Private methods - internal purposes */
    float _getTotalItemsOfCurrPhase(const JobState &currJobState);
};


#endif //EX3_JOB_CONTEXT_H
