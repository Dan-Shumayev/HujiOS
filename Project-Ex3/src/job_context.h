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
    /** Client's API */
    const MapReduceClient& client_; // client object's map and reduce routines
    // it's a reference to `client` as it's an abstract type, and there is no rationale in copying
    // such a type, but only referring to its origin

    /** Job-associated data */
    // TODO - Consider implementing an abstract class describing StageState which MapState/ReduceState inherits from
    //  so that ThreadContext doesn't have to deal with Output/Intermediate vectors explicitly but ThreadContext may
    //  may use instance of type like std::unique_ptr<StageState> for dealing with these matters.
    const InputVec& inputVec_; // isn't supposed to be modified
    OutputVec outputVec_; // emit3's outputs
    JobState currJobState_;
    std::atomic<size_t> lastProcessedInputElementGetAndIncrement_;
    std::atomic<size_t> lastProcessedShuffledElementGetAndIncrement_;
    pthread_mutex_t jobStateMutex_;
    // TODO - consider managing the JobState using this bitwise approach
/**std::atomic<std::uint64_t> jobStateBitwise_;*/ /** First 2 bits indicates stage_t (4 options), next 31 bits indicate
                                                number of processed elements at the current stage, next 31 bits indicate
                                                number of total elements to process at this stage */
    /** Thread-management */
    size_t numOfThreads_; // amount of working threads
    std::vector<std::unique_ptr<ThreadContext>> threadContexts_; // working threads' context
    std::atomic<size_t> lastThreadWorker_; // currently last assigned working thread
    Barrier threadsBarrier_; // used to synchronize all threads in between each phase
    pthread_mutex_t outputVecMutex_;
    std::vector<std::vector<IntermediatePair>> shuffledQueue_; // thread-0's data structure for shuffling
    std::atomic<size_t> shuffleQueAtomicCounter_; // count the number of shuffled vectors for Reduce use
public:
    JobContext(const MapReduceClient& client, const InputVec& inputVec, OutputVec& outputVec, int numOfThreads);

    ~JobContext();

    ThreadContext& getThreadContext(size_t ix) {return *threadContexts_[ix];};

    size_t getNumOfThreads() const {return numOfThreads_;};

    const InputVec &getInputVector() const {return inputVec_;};

    size_t getNumOfInputElems() const {return inputVec_.size();};

    void invokeClientMapRoutine(const K1* key, const V1* value, void* context) {client_.map(key, value, context);};

    void invokeClientReduceRoutine(const IntermediateVec* pairs, void* context) {client_.reduce(pairs, context);};

    size_t lastThreadAtomicGetIncrement() {return lastThreadWorker_.fetch_add(1);};

    size_t shuffleAtomicCounter() {return shuffleQueAtomicCounter_.fetch_add(1);};

    void barrier() {threadsBarrier_.barrier();};

    void getJobDone();

    JobState getJobState() const {return currJobState_;};

    void setJobStateStage(stage_t currentStage) { currJobState_.stage = currentStage;};

    void setJobStatePercentage(float currentPercent) { currJobState_.percentage = currentPercent;};

    void lockOutputVecMutex(); // TODO - consider creating a wrapper class to lock/unlock mutexes

    void unlockOutputVecMutex();

    void updateOutputVector(OutputPair &&outputPair);

    std::vector<IntermediateVec>& getShuffledQueue() {return shuffledQueue_;};

    void resetLastProcessedCounter() {lastProcessedInputElementGetAndIncrement_.store(0);};

    size_t lastProcessedInputElementGetAndIncrement() {return 1 + lastProcessedInputElementGetAndIncrement_.fetch_add(1);}

    size_t lastProcessedInputElementGetAndAdd(size_t currAdd) {return currAdd + lastProcessedInputElementGetAndIncrement_.fetch_add(currAdd);}

    size_t lastProcessedShuffledElementGetAndIncrement() {return lastProcessedShuffledElementGetAndIncrement_.fetch_add(1);}

    static size_t getNumOfShuffledPairs(std::vector<std::vector<IntermediatePair>> &shuffledQueue) ;

    size_t getNumOfIntermediatePairs();

    void lockThreadMutex();

    void unlockThreadMutex();
};


#endif //EX3_JOB_CONTEXT_H
