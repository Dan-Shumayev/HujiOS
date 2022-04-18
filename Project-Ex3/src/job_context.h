#ifndef EX3_JOB_CONTEXT_H
#define EX3_JOB_CONTEXT_H

#include "MapReduceFramework.h" // MapReduce library API
#include "exceptions.h" // systemError, mapReduceLibraryError
#include <vector> // std::vector
#include "Barrier.h" // barrier
#include <memory> // std::unique_ptr
#include <atomic> // std::atomic
#include "thread_context.h"


class JobContext {
private:
    /** Client object's `map` and `reduce` routines.
        Note it's a reference to `client` as it's an abstract type, and there is no rationale in copying
        such a type, but only referring to its origin */
    const MapReduceClient& client_;

    /** Job-associated data */
    // TODO - Consider implementing an abstract class describing StageState which MapState/ReduceState
    //  inherits from so that ThreadContext doesn't have to deal with Output/Intermediate vectors
    //  explicitly but ThreadContext may may use instance of type like std::unique_ptr<StageState>
    //  for dealing with these matters.
    const InputVec& inputVec_; // isn't supposed to be modified
    OutputVec outputVec_; // emit3's outputs
    JobState currJobState_; // TODO - to be deleted upon implementing the JobStateBitwise_ correctly
    /** First 2 bits indicates stage_t (4 options), next 31 bits indicate the
        number of processed elements at the current stage, next 31 bits indicate
        number of total elements to process at this stage */
    std::atomic<std::uint64_t> jobStateBitwise_;

    /** Thread-management */
    size_t numOfThreads_; // amount of working threads
    std::vector<std::unique_ptr<ThreadContext>> threadContexts_; // working threads' context
    std::atomic<size_t> lastThreadWorker_; // currently last assigned working thread
    Barrier threadsBarrier_; // used to synchronize all threads in between each phase
    pthread_mutex_t outputVecMutex_; // update of the output vector is a critical section
    std::vector<IntermediateVec> shuffledQueue_; // thread-0's data structure for shuffling

public:
    /** API functions */
    JobContext(const MapReduceClient& client, const InputVec& inputVec, OutputVec& outputVec, int numOfThreads);

    ~JobContext();

    ThreadContext& getThreadContext(size_t ix) {return *threadContexts_[ix];};

    size_t getNumOfThreads() const {return numOfThreads_;};

    const InputVec &getInputVector() const {return inputVec_;};

    size_t getNumOfInputElems() const {return inputVec_.size();};

    void invokeClientMapRoutine(const K1* key, const V1* value, void* context) {client_.map(key, value, context);};

    void invokeClientReduceRoutine(const IntermediateVec* pairs, void* context) {client_.reduce(pairs, context);};

    size_t lastThreadAtomicGetIncrement() {return lastThreadWorker_.fetch_add(1);};

    void barrier() {threadsBarrier_.barrier();};

    void getJobDone();

    JobState getJobState() const {return currJobState_;};

    void setJobStateStage(stage_t currentStage) { currJobState_.stage = currentStage;};

    void setJobStatePercentage(float currentPercent) { currJobState_.percentage = currentPercent;};

    void updateOutputVector(OutputPair &&outputPair);

    std::vector<IntermediateVec>& getShuffledQueue() {return shuffledQueue_;};

    static size_t getNumOfShuffledPairs(std::vector<std::vector<IntermediatePair>> &shuffledQueue) ;

    size_t getNumOfIntermediatePairs();

    size_t atomicCounter() {return jobStateBitwise_++;}; // TODO - do the increment correctly

    /** Private methods - internal purposes */
    void _lockOutputVecMutex(); // TODO - consider creating a wrapper class to lock/unlock mutexes

    void _unlockOutputVecMutex();
};


#endif //EX3_JOB_CONTEXT_H
